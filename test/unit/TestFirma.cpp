#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <cstring>

#include "../support/CertificadoPrueba.h"
#include "../support/Fixtures.h"
#include "cec/crypto/Pkcs12Signer.h"
#include "cec/crypto/XmlCanonicalizer.h"
#include "cec/crypto/XadesSigner.h"
#include "cec/crypto/Base64.h"
#include "cec/c_api/cec_c_api.h"
#include "pugixml.hpp"
#include "json.hpp"

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pkcs12.h>
#include <openssl/evp.h>

using namespace cec;

using fixtures::generarP12Prueba;

TEST(FirmaTest, Pkcs12CargaCorrectaYExtraccionMetadatos) {
    std::string pin = "123456";
    auto p12Bytes = generarP12Prueba(pin, "CONTRIBUYENTE DEMO S.A.");

    auto [signer, diags] = crypto::Pkcs12Signer::loadFromBytes(p12Bytes, pin);
    ASSERT_FALSE(diags.hasErrors());
    ASSERT_TRUE(signer.isLoaded());

    const auto& cert = signer.getCertificadoInfo();
    EXPECT_TRUE(cert.esValido);
    EXPECT_NE(cert.sujeto.find("CONTRIBUYENTE DEMO S.A."), std::string::npos);
    EXPECT_NE(cert.emisor.find("BCCR-SINPE"), std::string::npos);
    EXPECT_EQ(cert.serialNumber, "987654321");
    EXPECT_FALSE(cert.validoDesde.empty());
    EXPECT_FALSE(cert.validoHasta.empty());
    EXPECT_FALSE(cert.digestSha256Hex.empty());
    EXPECT_FALSE(cert.digestSha256Base64.empty());
    EXPECT_FALSE(cert.certificadoBase64.empty());
}

TEST(FirmaTest, Pkcs12PinIncorrectoRechazado) {
    std::string pinValido = "correct-pin";
    auto p12Bytes = generarP12Prueba(pinValido);

    auto [signer, diags] = crypto::Pkcs12Signer::loadFromBytes(p12Bytes, "wrong-pin");
    EXPECT_TRUE(diags.hasErrors());
    EXPECT_FALSE(signer.isLoaded());

    bool foundAuthFail = false;
    for (const auto& d : diags.items()) {
        if (d.code == "CRYPTO_P12_AUTH_FAILED") foundAuthFail = true;
    }
    EXPECT_TRUE(foundAuthFail);
}

TEST(FirmaTest, CanonicalizacionXmlC14N) {
    std::string xmlRaw = "<root z=\"last\" a=\"first\" m=\"middle\"><child>Hola &amp; Mundo</child></root>";
    auto [c14n, diags] = crypto::XmlCanonicalizer::canonicalizeXmlString(xmlRaw);
    ASSERT_FALSE(diags.hasErrors());
    EXPECT_EQ(c14n, "<root a=\"first\" m=\"middle\" z=\"last\"><child>Hola &amp; Mundo</child></root>");
}

TEST(FirmaTest, CanonicalizacionSubarbolIncluyeNamespacesHeredados) {
    // C14N inclusivo de un nodo dentro de un documento: el elemento raíz del subárbol declara los
    // namespaces de sus ancestros. Resultado esperado obtenido de libxml2 (lxml, method="c14n").
    pugi::xml_document doc;
    ASSERT_TRUE(doc.load_string("<r xmlns=\"urn:a\" xmlns:ds=\"urn:ds\"><ds:S Id=\"x\"><ds:K>v</ds:K><b/></ds:S></r>"));
    auto [c14n, diags] = crypto::XmlCanonicalizer::canonicalize(doc.document_element().first_child(), false);
    ASSERT_FALSE(diags.hasErrors());
    EXPECT_EQ(c14n, "<ds:S xmlns=\"urn:a\" xmlns:ds=\"urn:ds\" Id=\"x\"><ds:K>v</ds:K><b></b></ds:S>");
}

TEST(FirmaTest, DigestsDeLasReferenciasCoincidenConElXmlFirmado) {
    // Hacienda recalcula cada DigestValue sobre el XML tal como lo recibe; si no coincide responde
    // "El XML fue modificado luego de haber sido firmado". Se verifica sobre el texto final, reparseado.
    for (const char* caso : {"fe_completa", "te", "fec", "rep"}) {
        SCOPED_TRACE(caso);
        auto res = fixtures::procesar(fixtures::request(caso));
        ASSERT_TRUE(res.isSuccess) << fixtures::diagnosticos(res);
        auto signRes = service::ComprobanteService::signXml(res.xml, generarP12Prueba("1234"), "1234");
        ASSERT_TRUE(signRes.isSuccess);

        pugi::xml_document doc;
        ASSERT_TRUE(doc.load_string(signRes.xml.c_str(), pugi::parse_default | pugi::parse_ws_pcdata));
        pugi::xml_node signedInfo = doc.document_element().child("ds:Signature").child("ds:SignedInfo");
        ASSERT_TRUE(signedInfo);

        int referencias = 0;
        for (pugi::xml_node ref = signedInfo.child("ds:Reference"); ref; ref = ref.next_sibling("ds:Reference"), ++referencias) {
            std::string uri = ref.attribute("URI").value();
            SCOPED_TRACE(uri);
            std::pair<std::string, domain::DiagnosticList> c14n;
            if (uri.empty()) {
                c14n = crypto::XmlCanonicalizer::canonicalize(doc, true);
            } else {
                pugi::xml_node nodo = doc.find_node([&](pugi::xml_node n) { return uri.substr(1) == n.attribute("Id").value(); });
                ASSERT_TRUE(nodo);
                c14n = crypto::XmlCanonicalizer::canonicalize(nodo, false);
            }
            ASSERT_FALSE(c14n.second.hasErrors());
            EXPECT_EQ(crypto::Pkcs12Signer::computeDigestSha256Base64(c14n.first), ref.child_value("ds:DigestValue"));
        }
        EXPECT_EQ(referencias, 3);
    }
}

TEST(FirmaTest, FirmaFacturaElectronicaXadesEpesYVerificacionCriptografica) {
    // 1. Factura Electrónica válida
    auto invRes = fixtures::procesar(fixtures::request("fe_servicios"));
    ASSERT_TRUE(invRes.isSuccess) << fixtures::diagnosticos(invRes);
    ASSERT_FALSE(invRes.xml.empty());

    // 2. Firmar con certificado P12
    std::string pin = "p12pass2026";
    auto p12Bytes = generarP12Prueba(pin, "EMPRESA PRUEBA S.A.");

    auto signRes = service::ComprobanteService::signXml(invRes.xml, p12Bytes, pin);
    ASSERT_TRUE(signRes.isSuccess);
    ASSERT_FALSE(signRes.diagnostics.hasErrors());
    ASSERT_FALSE(signRes.xml.empty());

    const std::string& xmlFirmado = signRes.xml;

    // 3. Validar estructura de firma conforme a la resolución de Hacienda
    EXPECT_NE(xmlFirmado.find("<FacturaElectronica"), std::string::npos);
    EXPECT_NE(xmlFirmado.find("<ds:Signature xmlns:ds=\"http://www.w3.org/2000/09/xmldsig#\""), std::string::npos);
    EXPECT_NE(xmlFirmado.find("<ds:SignedInfo"), std::string::npos);
    EXPECT_NE(xmlFirmado.find("<ds:CanonicalizationMethod Algorithm=\"http://www.w3.org/TR/2001/REC-xml-c14n-20010315\""), std::string::npos);
    EXPECT_NE(xmlFirmado.find("<ds:SignatureMethod Algorithm=\"http://www.w3.org/2001/04/xmldsig-more#rsa-sha256\""), std::string::npos);
    EXPECT_NE(xmlFirmado.find("<ds:Transform Algorithm=\"http://www.w3.org/2000/09/xmldsig#enveloped-signature\""), std::string::npos);
    EXPECT_NE(xmlFirmado.find("<ds:SignatureValue>"), std::string::npos);
    EXPECT_NE(xmlFirmado.find("<ds:KeyInfo"), std::string::npos);
    EXPECT_NE(xmlFirmado.find("<ds:X509Certificate>"), std::string::npos);
    EXPECT_NE(xmlFirmado.find("<xades:QualifyingProperties"), std::string::npos);
    EXPECT_NE(xmlFirmado.find("<xades:SignedProperties"), std::string::npos);
    EXPECT_NE(xmlFirmado.find("<xades:Identifier>https://tribunet.hacienda.go.cr/docs/esquemas/2016/v4.2/ResolucionComprobantesElectronicosDGT-R-48-2016_4.2.pdf</xades:Identifier>"), std::string::npos);
    EXPECT_NE(xmlFirmado.find("<xades:Description>Politica de Firma de Comprobantes Electronicos</xades:Description>"), std::string::npos);
    EXPECT_NE(xmlFirmado.find("<ds:DigestValue>"), std::string::npos);

    // 4. Verificación matemática estricta con OpenSSL
    // Parsear el XML firmado para extraer SignedInfo canónico, SignatureValue y el Certificado
    pugi::xml_document doc;
    ASSERT_TRUE(doc.load_string(xmlFirmado.c_str()));

    pugi::xml_node sigNode = doc.select_node("//ds:Signature").node();
    ASSERT_TRUE(sigNode);

    pugi::xml_node signedInfoNode = sigNode.child("ds:SignedInfo");
    ASSERT_TRUE(signedInfoNode);

    pugi::xml_node sigValNode = sigNode.child("ds:SignatureValue");
    ASSERT_TRUE(sigValNode);
    std::string sigValBase64 = sigValNode.child_value();
    auto sigValBytes = crypto::Base64::decode(sigValBase64);
    ASSERT_FALSE(sigValBytes.empty());

    pugi::xml_node certNode = doc.select_node("//ds:X509Certificate").node();
    ASSERT_TRUE(certNode);
    std::string certBase64 = certNode.child_value();
    auto certDerBytes = crypto::Base64::decode(certBase64);
    ASSERT_FALSE(certDerBytes.empty());

    // Canonicalizar el SignedInfo
    auto [canonicalSignedInfo, siDiags] = crypto::XmlCanonicalizer::canonicalize(signedInfoNode, false);
    ASSERT_FALSE(siDiags.hasErrors());

    // Cargar la clave pública desde el certificado X.509
    const unsigned char* pDer = certDerBytes.data();
    X509* x509Cert = d2i_X509(nullptr, &pDer, static_cast<long>(certDerBytes.size()));
    ASSERT_NE(x509Cert, nullptr);

    EVP_PKEY* pubKey = X509_get_pubkey(x509Cert);
    ASSERT_NE(pubKey, nullptr);

    // Verificar firma con OpenSSL
    EVP_MD_CTX* vCtx = EVP_MD_CTX_new();
    ASSERT_NE(vCtx, nullptr);

    int vInit = EVP_DigestVerifyInit(vCtx, nullptr, EVP_sha256(), nullptr, pubKey);
    ASSERT_EQ(vInit, 1);

    int vUpdate = EVP_DigestVerifyUpdate(vCtx, canonicalSignedInfo.data(), canonicalSignedInfo.size());
    ASSERT_EQ(vUpdate, 1);

    int vFinal = EVP_DigestVerifyFinal(vCtx, sigValBytes.data(), sigValBytes.size());
    EXPECT_EQ(vFinal, 1); // 1 = Firma criptográficamente válida y verificada

    EVP_MD_CTX_free(vCtx);
    EVP_PKEY_free(pubKey);
    X509_free(x509Cert);
}

TEST(FirmaTest, FirmaReciboElectronicoPago) {
    auto res = fixtures::procesar(fixtures::request("rep"));
    ASSERT_TRUE(res.isSuccess) << fixtures::diagnosticos(res);

    std::string pin = "rep-pin-456";
    auto p12 = generarP12Prueba(pin);

    auto signRes = service::ComprobanteService::signXml(res.xml, p12, pin);
    ASSERT_TRUE(signRes.isSuccess);
    EXPECT_NE(signRes.xml.find("<ReciboElectronicoPago"), std::string::npos);
    EXPECT_NE(signRes.xml.find("<ds:Signature xmlns:ds=\"http://www.w3.org/2000/09/xmldsig#\""), std::string::npos);
}

TEST(FirmaTest, CApiFirmarXmlHelpers) {
    // 1. Generar XML simple
    std::string xmlSimple = "<DocumentoPrueba xmlns=\"https://cdn.comprobantes.cr\"><Dato>Prueba</Dato></DocumentoPrueba>";
    std::string pin = "c-api-pin";
    auto p12 = generarP12Prueba(pin, "C API TEST USER");

    cec_result* res = nullptr;
    cec_status st = cec_sign_xml(
        reinterpret_cast<const uint8_t*>(xmlSimple.data()),
        xmlSimple.size(),
        p12.data(),
        p12.size(),
        pin.c_str(),
        &res
    );

    ASSERT_EQ(st, CEC_STATUS_OK);
    ASSERT_NE(res, nullptr);

    // Verificar XML firmado obtenido desde cec_result_xml
    const uint8_t* outXml = nullptr;
    size_t outXmlLen = 0;
    st = cec_result_xml(res, &outXml, &outXmlLen);
    ASSERT_EQ(st, CEC_STATUS_OK);
    ASSERT_NE(outXml, nullptr);
    ASSERT_GT(outXmlLen, 0);

    std::string signedXmlStr(reinterpret_cast<const char*>(outXml), outXmlLen);
    EXPECT_NE(signedXmlStr.find("<ds:Signature"), std::string::npos);

    // Verificar metadata JSON desde cec_result_data
    const uint8_t* outJson = nullptr;
    size_t outJsonLen = 0;
    st = cec_result_data(res, &outJson, &outJsonLen);
    ASSERT_EQ(st, CEC_STATUS_OK);
    ASSERT_NE(outJson, nullptr);

    std::string jsonStr(reinterpret_cast<const char*>(outJson), outJsonLen);
    EXPECT_NE(jsonStr.find("\"status\":\"ok\""), std::string::npos);
    EXPECT_NE(jsonStr.find("C API TEST USER"), std::string::npos);

    cec_result_free(res);
}


TEST(FirmaTest, CApiLeerCertificado) {
    const std::string pin = "lectura-pin";
    auto p12 = generarP12Prueba(pin, "TITULAR DE PRUEBA");

    cec_result* res = nullptr;
    ASSERT_EQ(cec_leer_certificado(p12.data(), p12.size(), pin.c_str(), &res), CEC_STATUS_OK);
    int32_t ok = 0;
    cec_result_is_success(res, &ok);
    EXPECT_EQ(ok, 1);

    cec_certificado_c cert{};
    ASSERT_EQ(cec_result_certificado(res, &cert), CEC_STATUS_OK);
    EXPECT_NE(std::string(cert.sujeto).find("TITULAR DE PRUEBA"), std::string::npos);
    EXPECT_EQ(cert.vigente, 1);
    EXPECT_EQ(std::string(cert.sha256Hex).size(), 64u);
    cec_result_free(res);

    ASSERT_EQ(cec_leer_certificado(p12.data(), p12.size(), "pin-incorrecto", &res), CEC_STATUS_OK);
    cec_result_is_success(res, &ok);
    EXPECT_EQ(ok, 0);
    EXPECT_EQ(cec_result_certificado(res, &cert), CEC_STATUS_ERR_OUT_OF_RANGE);
    cec_result_free(res);
}

// ── Firma en dos pasos (llave privada fuera del proceso) ─────────────────────────────────────────

namespace {

/// Firma RSA-SHA256 en Base64 con la llave de un .p12, como lo haría un keystore externo.
std::string firmarExterno(const std::vector<uint8_t>& p12, const std::string& pin, const std::string& datos) {
    auto [signer, diags] = crypto::Pkcs12Signer::loadFromBytes(p12, pin);
    EXPECT_FALSE(diags.hasErrors());
    return signer.signSha256Base64(datos).first;
}

std::vector<uint8_t> certificadoDer(const std::vector<uint8_t>& p12, const std::string& pin) {
    auto [signer, diags] = crypto::Pkcs12Signer::loadFromBytes(p12, pin);
    EXPECT_FALSE(diags.hasErrors());
    return crypto::Base64::decode(signer.getCertificadoInfo().certificadoBase64);
}

} // namespace

TEST(FirmaTest, FirmaEnDosPasosConLlaveExterna) {
    auto res = fixtures::procesar(fixtures::request("fe_servicios"));
    ASSERT_TRUE(res.isSuccess) << fixtures::diagnosticos(res);
    std::string pin = "dos-pasos-2026";
    auto p12 = generarP12Prueba(pin, "FIRMA EXTERNA S.A.");

    // Paso 1: solo con el certificado público
    auto prep = service::ComprobanteService::prepararFirma(res.xml, certificadoDer(p12, pin));
    ASSERT_TRUE(prep.isSuccess);
    ASSERT_FALSE(prep.signedInfo.empty());
    // C14N inclusivo: SignedInfo lleva los namespaces heredados del comprobante
    EXPECT_EQ(prep.signedInfo.rfind("<ds:SignedInfo xmlns=", 0), 0u);
    EXPECT_NE(prep.signedInfo.find("xmlns:ds=\"http://www.w3.org/2000/09/xmldsig#\""), std::string::npos);
    EXPECT_NE(prep.xml.find("<ds:SignatureValue/>"), std::string::npos);
    EXPECT_NE(prep.certInfo.sujeto.find("FIRMA EXTERNA S.A."), std::string::npos);
    auto [signer, d] = crypto::Pkcs12Signer::loadFromBytes(p12, pin);
    EXPECT_EQ(signer.getCertificadoInfo().alias, "Certificado Prueba Core");  // friendlyName de PKCS12_create

    // La llave firma SignedInfo fuera del core; paso 2: completar y verificar
    auto firma = firmarExterno(p12, pin, prep.signedInfo);
    auto comp = service::ComprobanteService::completarFirma(prep.xml, firma);
    ASSERT_TRUE(comp.isSuccess) << (comp.diagnostics.items().empty() ? "" : comp.diagnostics.items()[0].message);

    // El resultado verifica igual que una firma en un solo paso: SignedInfo canónico + llave pública de KeyInfo
    pugi::xml_document doc;
    ASSERT_TRUE(doc.load_string(comp.xml.c_str(), pugi::parse_default | pugi::parse_ws_pcdata));
    pugi::xml_node sig = doc.select_node("//ds:Signature").node();
    auto [canonicalSignedInfo, siDiags] = crypto::XmlCanonicalizer::canonicalize(sig.child("ds:SignedInfo"), false);
    ASSERT_FALSE(siDiags.hasErrors());
    EXPECT_EQ(canonicalSignedInfo, prep.signedInfo);
    EXPECT_EQ(std::string(sig.child_value("ds:SignatureValue")), firma);
    auto verif = crypto::Pkcs12Signer::verificarSha256(
        sig.child("ds:KeyInfo").child("ds:X509Data").child_value("ds:X509Certificate"), canonicalSignedInfo, firma);
    EXPECT_FALSE(verif.hasErrors());
}

TEST(FirmaTest, CompletarFirmaRechazaFirmaDeOtraLlave) {
    auto res = fixtures::procesar(fixtures::request("fe_servicios"));
    ASSERT_TRUE(res.isSuccess);
    std::string pin = "1234";
    auto p12 = generarP12Prueba(pin, "TITULAR S.A.");
    auto otro = generarP12Prueba(pin, "OTRO S.A.");

    auto prep = service::ComprobanteService::prepararFirma(res.xml, certificadoDer(p12, pin));
    ASSERT_TRUE(prep.isSuccess);

    auto comp = service::ComprobanteService::completarFirma(prep.xml, firmarExterno(otro, pin, prep.signedInfo));
    EXPECT_FALSE(comp.isSuccess);
    EXPECT_TRUE(comp.xml.empty());
    ASSERT_FALSE(comp.diagnostics.items().empty());
    EXPECT_EQ(comp.diagnostics.items()[0].code, "SIGN_VERIFY_FAILED");

    // Tampoco acepta un XML ya firmado ni un certificado inválido
    auto firmado = service::ComprobanteService::completarFirma(prep.xml, firmarExterno(p12, pin, prep.signedInfo));
    ASSERT_TRUE(firmado.isSuccess);
    EXPECT_FALSE(service::ComprobanteService::completarFirma(firmado.xml, "AAAA").isSuccess);
    EXPECT_FALSE(service::ComprobanteService::prepararFirma(res.xml, {1, 2, 3}).isSuccess);
}

TEST(FirmaTest, CApiFirmaEnDosPasos) {
    auto res = fixtures::procesar(fixtures::request("fe_servicios"));
    ASSERT_TRUE(res.isSuccess);
    std::string pin = "c-api";
    auto p12 = generarP12Prueba(pin);
    auto der = certificadoDer(p12, pin);

    cec_result* prep = nullptr;
    ASSERT_EQ(cec_preparar_firma(reinterpret_cast<const uint8_t*>(res.xml.data()), res.xml.size(), der.data(), der.size(), &prep), CEC_STATUS_OK);
    int32_t ok = 0;
    cec_result_is_success(prep, &ok);
    ASSERT_EQ(ok, 1);
    const uint8_t* xml = nullptr;
    size_t xmlLen = 0;
    cec_result_xml(prep, &xml, &xmlLen);
    std::string xmlPreparado(reinterpret_cast<const char*>(xml), xmlLen);
    const uint8_t* data = nullptr;
    size_t dataLen = 0;
    cec_result_data(prep, &data, &dataLen);
    auto json = nlohmann::json::parse(std::string(reinterpret_cast<const char*>(data), dataLen));
    std::string signedInfo = json.at("signedInfo").get<std::string>();
    // El acceso sin JSON entrega el mismo SignedInfo
    const uint8_t* si = nullptr;
    size_t siLen = 0;
    ASSERT_EQ(cec_result_signed_info(prep, &si, &siLen), CEC_STATUS_OK);
    EXPECT_EQ(std::string(reinterpret_cast<const char*>(si), siLen), signedInfo);
    cec_result_free(prep);

    std::string firma = firmarExterno(p12, pin, signedInfo);
    cec_result* comp = nullptr;
    ASSERT_EQ(cec_completar_firma(reinterpret_cast<const uint8_t*>(xmlPreparado.data()), xmlPreparado.size(), firma.c_str(), &comp), CEC_STATUS_OK);
    cec_result_is_success(comp, &ok);
    EXPECT_EQ(ok, 1);
    cec_result_free(comp);

    EXPECT_EQ(cec_completar_firma(nullptr, 0, firma.c_str(), &comp), CEC_STATUS_ERR_NULL_POINTER);
}
