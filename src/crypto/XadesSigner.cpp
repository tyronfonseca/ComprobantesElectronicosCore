#include "cec/crypto/XadesSigner.h"
#include "cec/crypto/XmlCanonicalizer.h"
#include "pugixml.hpp"

#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <cctype>
#include <cstdint>

namespace cec::crypto {

namespace {

static std::string generateUniqueSuffix() {
    auto now = std::chrono::system_clock::now().time_since_epoch().count();
    // Un generador por hilo: uno compartido sería una condición de carrera al firmar desde varios hilos.
    thread_local std::mt19937_64 rng(static_cast<uint64_t>(now) ^ (static_cast<uint64_t>(std::random_device{}()) << 32));
    uint32_t r = static_cast<uint32_t>(rng() % 1000000);
    std::ostringstream ss;
    ss << now << "-" << std::setw(6) << std::setfill('0') << r;
    return ss.str();
}

static std::string currentIsoDateTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#if defined(_WIN32)
    gmtime_s(&tm, &tt);   // MSVC no tiene gmtime_r
#else
    gmtime_r(&tt, &tm);
#endif
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &tm);
    return std::string(buf);
}

/// Busca el <ds:Reference> de SignedInfo con el URI indicado.
static pugi::xml_node referenceByUri(const pugi::xml_node& signedInfo, const std::string& uri) {
    for (pugi::xml_node ref = signedInfo.child("ds:Reference"); ref; ref = ref.next_sibling("ds:Reference")) {
        if (uri == ref.attribute("URI").value()) return ref;
    }
    return {};
}

/// XML sin reformatear: cualquier espacio agregado después de firmar invalida la firma.
static std::string imprimir(const pugi::xml_node& root) {
    std::ostringstream out;
    out << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
    root.print(out, "", pugi::format_raw | pugi::format_no_declaration);
    return out.str();
}

} // namespace

std::pair<std::string, domain::DiagnosticList> XadesSigner::signXml(
    const std::string& xmlSinFirmar,
    const std::vector<uint8_t>& p12Bytes,
    const std::string& pin
) {
    auto [signer, loadDiags] = Pkcs12Signer::loadFromBytes(p12Bytes, pin);
    if (loadDiags.hasErrors()) {
        return {"", loadDiags};
    }
    return signXml(xmlSinFirmar, signer);
}

std::pair<std::string, domain::DiagnosticList> XadesSigner::signXml(
    const std::string& xmlSinFirmar,
    const Pkcs12Signer& signer
) {
    auto [preparada, diags] = prepararFirma(xmlSinFirmar, signer.getCertificadoInfo());
    if (diags.hasErrors()) {
        return {{}, diags};
    }
    auto [signatureValueBase64, signDiags] = signer.signSha256Base64(preparada.signedInfo);
    if (signDiags.hasErrors()) {
        diags.merge(signDiags);
        return {{}, diags};
    }
    // La llave es la del certificado de KeyInfo: no hace falta verificar.
    auto [xml, completarDiags] = completarFirma(preparada.xml, signatureValueBase64, false);
    diags.merge(completarDiags);
    return {completarDiags.hasErrors() ? "" : xml, diags};
}

std::pair<FirmaPreparada, domain::DiagnosticList> XadesSigner::prepararFirma(
    const std::string& xmlSinFirmar,
    const CertificadoInfo& certInfo
) {
    domain::DiagnosticList diags;

    pugi::xml_document doc;
    pugi::xml_parse_result parseRes = doc.load_string(xmlSinFirmar.c_str(), pugi::parse_default | pugi::parse_ws_pcdata);
    if (!parseRes) {
        diags.addError("SIGN_PARSE_ERROR", "xml", std::string("Error al parsear XML original: ") + parseRes.description(), domain::DiagnosticCategory::Structural);
        return {{}, diags};
    }

    pugi::xml_node rootNode = doc.document_element();
    if (!rootNode) {
        diags.addError("SIGN_NO_ROOT", "xml", "El XML no contiene un elemento raíz.", domain::DiagnosticCategory::Structural);
        return {{}, diags};
    }

    if (certInfo.certificadoBase64.empty()) {
        diags.addError("SIGN_CERT_MISSING", "cert", "Falta el certificado X.509 del firmante.", domain::DiagnosticCategory::Structural);
        return {{}, diags};
    }
    if (!certInfo.esValido) {
        diags.addError("SIGN_CERT_INVALID", "cert", "El certificado digital no es válido.", domain::DiagnosticCategory::BusinessRule);
        return {{}, diags};
    }

    std::string uniqueId = generateUniqueSuffix();
    std::string sigId = "Signature-" + uniqueId;
    std::string signedPropsId = "SignedProperties-" + sigId;
    std::string keyInfoId = "KeyInfoId-" + sigId;
    std::string referenceDocId = "Reference-" + sigId;
    std::string signingTime = currentIsoDateTime();

    // 1. Digest del documento (C14N excluyendo cualquier firma previa: transform enveloped-signature)
    auto [canonicalDoc, docC14nDiags] = XmlCanonicalizer::canonicalize(doc, true);
    if (docC14nDiags.hasErrors()) {
        diags.merge(docC14nDiags);
        return {{}, diags};
    }
    std::string docDigestBase64 = Pkcs12Signer::computeDigestSha256Base64(canonicalDoc);

    // 2. Esqueleto de <ds:Signature>. Los digests de KeyInfo y SignedProperties y el
    //    SignatureValue se calculan después, con los nodos ya dentro del documento, porque
    //    el C14N inclusivo de cada nodo incluye los namespaces heredados de sus ancestros.
    const std::string sha256 = "http://www.w3.org/2001/04/xmlenc#sha256";
    std::ostringstream sig;
    sig << "<ds:Signature xmlns:ds=\"http://www.w3.org/2000/09/xmldsig#\" Id=\"" << sigId << "\">"
        << "<ds:SignedInfo>"
        << "<ds:CanonicalizationMethod Algorithm=\"http://www.w3.org/TR/2001/REC-xml-c14n-20010315\"></ds:CanonicalizationMethod>"
        << "<ds:SignatureMethod Algorithm=\"http://www.w3.org/2001/04/xmldsig-more#rsa-sha256\"></ds:SignatureMethod>"
        << "<ds:Reference Id=\"" << referenceDocId << "\" URI=\"\">"
        << "<ds:Transforms>"
        << "<ds:Transform Algorithm=\"http://www.w3.org/2000/09/xmldsig#enveloped-signature\"></ds:Transform>"
        << "<ds:Transform Algorithm=\"http://www.w3.org/TR/2001/REC-xml-c14n-20010315\"></ds:Transform>"
        << "</ds:Transforms>"
        << "<ds:DigestMethod Algorithm=\"" << sha256 << "\"></ds:DigestMethod>"
        << "<ds:DigestValue>" << docDigestBase64 << "</ds:DigestValue>"
        << "</ds:Reference>"
        << "<ds:Reference Type=\"http://uri.etsi.org/01903#SignedProperties\" URI=\"#" << signedPropsId << "\">"
        << "<ds:DigestMethod Algorithm=\"" << sha256 << "\"></ds:DigestMethod>"
        << "<ds:DigestValue></ds:DigestValue>"
        << "</ds:Reference>"
        << "<ds:Reference URI=\"#" << keyInfoId << "\">"
        << "<ds:DigestMethod Algorithm=\"" << sha256 << "\"></ds:DigestMethod>"
        << "<ds:DigestValue></ds:DigestValue>"
        << "</ds:Reference>"
        << "</ds:SignedInfo>"
        << "<ds:SignatureValue></ds:SignatureValue>"
        << "<ds:KeyInfo Id=\"" << keyInfoId << "\">"
        << "<ds:X509Data>"
        << "<ds:X509Certificate>" << certInfo.certificadoBase64 << "</ds:X509Certificate>"
        << "</ds:X509Data>"
        << "</ds:KeyInfo>"
        << "<ds:Object>"
        << "<xades:QualifyingProperties xmlns:xades=\"http://uri.etsi.org/01903/v1.3.2#\" Target=\"#" << sigId << "\">"
        << "<xades:SignedProperties Id=\"" << signedPropsId << "\">"
        << "<xades:SignedSignatureProperties>"
        << "<xades:SigningTime>" << signingTime << "</xades:SigningTime>"
        << "<xades:SigningCertificate>"
        << "<xades:Cert>"
        << "<xades:CertDigest>"
        << "<ds:DigestMethod Algorithm=\"" << sha256 << "\"></ds:DigestMethod>"
        << "<ds:DigestValue>" << certInfo.digestSha256Base64 << "</ds:DigestValue>"
        << "</xades:CertDigest>"
        << "<xades:IssuerSerial>"
        << "<ds:X509IssuerName>" << certInfo.emisor << "</ds:X509IssuerName>"
        << "<ds:X509SerialNumber>" << certInfo.serialNumber << "</ds:X509SerialNumber>"
        << "</xades:IssuerSerial>"
        << "</xades:Cert>"
        << "</xades:SigningCertificate>"
        << "<xades:SignaturePolicyIdentifier>"
        << "<xades:SignaturePolicyId>"
        << "<xades:SigPolicyId>"
        << "<xades:Identifier>" << HACIENDA_POLICY_URL << "</xades:Identifier>"
        << "<xades:Description>" << HACIENDA_POLICY_DESC << "</xades:Description>"
        << "</xades:SigPolicyId>"
        << "<xades:SigPolicyHash>"
        << "<ds:DigestMethod Algorithm=\"" << sha256 << "\"></ds:DigestMethod>"
        << "<ds:DigestValue>" << HACIENDA_POLICY_HASH << "</ds:DigestValue>"
        << "</xades:SigPolicyHash>"
        << "</xades:SignaturePolicyId>"
        << "</xades:SignaturePolicyIdentifier>"
        << "</xades:SignedSignatureProperties>"
        << "</xades:SignedProperties>"
        << "</xades:QualifyingProperties>"
        << "</ds:Object>"
        << "</ds:Signature>";

    pugi::xml_document sigDoc;
    if (!sigDoc.load_string(sig.str().c_str())) {
        diags.addError("SIGN_BLOCK_PARSE_FAILED", "signature", "Error al parsear el nodo final de firma XAdES-EPES.", domain::DiagnosticCategory::Structural);
        return {{}, diags};
    }

    // Insertar <ds:Signature> como último nodo hijo del root
    pugi::xml_node sigNode = rootNode.append_copy(sigDoc.document_element());
    pugi::xml_node signedInfo = sigNode.child("ds:SignedInfo");
    pugi::xml_node keyInfo = sigNode.child("ds:KeyInfo");
    pugi::xml_node signedProps = sigNode.child("ds:Object").child("xades:QualifyingProperties").child("xades:SignedProperties");
    pugi::xml_node refSignedProps = referenceByUri(signedInfo, "#" + signedPropsId);
    pugi::xml_node refKeyInfo = referenceByUri(signedInfo, "#" + keyInfoId);
    if (!signedInfo || !keyInfo || !signedProps || !refSignedProps || !refKeyInfo) {
        diags.addError("SIGN_BLOCK_PARSE_FAILED", "signature", "La estructura de la firma XAdES-EPES está incompleta.", domain::DiagnosticCategory::Structural);
        return {{}, diags};
    }

    // 3. Digests de SignedProperties y KeyInfo, canonicalizados en contexto
    auto [canonicalSignedProps, spDiags] = XmlCanonicalizer::canonicalize(signedProps, false);
    auto [canonicalKeyInfo, kiDiags] = XmlCanonicalizer::canonicalize(keyInfo, false);
    diags.merge(spDiags);
    diags.merge(kiDiags);
    if (diags.hasErrors()) {
        return {{}, diags};
    }
    refSignedProps.child("ds:DigestValue").text().set(Pkcs12Signer::computeDigestSha256Base64(canonicalSignedProps).c_str());
    refKeyInfo.child("ds:DigestValue").text().set(Pkcs12Signer::computeDigestSha256Base64(canonicalKeyInfo).c_str());

    // 4. SignedInfo (ya con todos los digests) canonicalizado: es lo que se firma con RSA-SHA256.
    auto [canonicalSignedInfo, siDiags] = XmlCanonicalizer::canonicalize(signedInfo, false);
    if (siDiags.hasErrors()) {
        diags.merge(siDiags);
        return {{}, diags};
    }
    return {FirmaPreparada{imprimir(rootNode), canonicalSignedInfo}, diags};
}

std::pair<std::string, domain::DiagnosticList> XadesSigner::completarFirma(
    const std::string& xmlPreparado,
    const std::string& signatureValueBase64,
    bool verificar
) {
    domain::DiagnosticList diags;
    pugi::xml_document doc;
    if (!doc.load_string(xmlPreparado.c_str(), pugi::parse_default | pugi::parse_ws_pcdata)) {
        diags.addError("SIGN_PARSE_ERROR", "xml", "El XML preparado para firmar no es válido.", domain::DiagnosticCategory::Structural);
        return {"", diags};
    }
    pugi::xml_node rootNode = doc.document_element();
    pugi::xml_node sigNode;
    for (pugi::xml_node n = rootNode.child("ds:Signature"); n; n = n.next_sibling("ds:Signature")) sigNode = n;
    pugi::xml_node signedInfo = sigNode.child("ds:SignedInfo");
    pugi::xml_node signatureValue = sigNode.child("ds:SignatureValue");
    if (!signedInfo || !signatureValue) {
        diags.addError("SIGN_NOT_PREPARED", "xml", "El XML no tiene una firma preparada (ds:SignedInfo y ds:SignatureValue).", domain::DiagnosticCategory::Structural);
        return {"", diags};
    }
    if (signatureValue.text().get()[0] != '\0') {
        diags.addError("SIGN_ALREADY_SIGNED", "xml", "El XML ya tiene SignatureValue.", domain::DiagnosticCategory::Structural);
        return {"", diags};
    }
    std::string firma;
    for (char c : signatureValueBase64) {
        if (!std::isspace(static_cast<unsigned char>(c))) firma += c;
    }
    if (firma.empty()) {
        diags.addError("SIGN_VALUE_EMPTY", "signatureValue", "Falta el SignatureValue.", domain::DiagnosticCategory::Structural);
        return {"", diags};
    }

    if (verificar) {
        auto [canonicalSignedInfo, siDiags] = XmlCanonicalizer::canonicalize(signedInfo, false);
        diags.merge(siDiags);
        const char* cert = sigNode.child("ds:KeyInfo").child("ds:X509Data").child("ds:X509Certificate").text().get();
        if (!diags.hasErrors()) diags.merge(Pkcs12Signer::verificarSha256(cert, canonicalSignedInfo, firma));
        if (diags.hasErrors()) return {"", diags};
    }

    signatureValue.text().set(firma.c_str());
    return {imprimir(rootNode), diags};
}

} // namespace cec::crypto
