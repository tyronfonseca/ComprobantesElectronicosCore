#include "cec/crypto/Pkcs12Signer.h"
#include "cec/crypto/Base64.h"

#include <openssl/pkcs12.h>
#include <openssl/evp.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/err.h>
#include <openssl/sha.h>
#include <openssl/asn1.h>
#include <openssl/bio.h>
#include <openssl/bn.h>

#include <sstream>
#include <iomanip>
#include <cstring>
#include <cstdint>

namespace cec::crypto {

namespace {

static std::string getOpenSSLError() {
    char buf[256];
    unsigned long err = ERR_get_error();
    if (err == 0) return "Error criptográfico desconocido.";
    ERR_error_string_n(err, buf, sizeof(buf));
    return std::string(buf);
}

static std::string asn1TimeToIsoString(const ASN1_TIME* time) {
    if (!time) return "";
    BIO* bio = BIO_new(BIO_s_mem());
    if (!bio) return "";
    ASN1_TIME_print(bio, time);
    char buf[128] = {0};
    int len = BIO_read(bio, buf, sizeof(buf) - 1);
    BIO_free(bio);
    if (len <= 0) return "";
    return std::string(buf, len);
}

static std::string asn1IntegerToString(const ASN1_INTEGER* serial) {
    if (!serial) return "";
    BIGNUM* bn = ASN1_INTEGER_to_BN(serial, nullptr);
    if (!bn) return "";
    char* decStr = BN_bn2dec(bn);
    std::string result = decStr ? decStr : "";
    OPENSSL_free(decStr);
    BN_free(bn);
    return result;
}

/// Metadatos del certificado: titular, emisor, serie, vigencia, DER en Base64 y su huella SHA-256.
static CertificadoInfo infoDesdeX509(X509* cert) {
    CertificadoInfo info;
    char* subjectOneline = X509_NAME_oneline(X509_get_subject_name(cert), nullptr, 0);
    info.sujeto = subjectOneline ? subjectOneline : "";
    OPENSSL_free(subjectOneline);

    char* issuerOneline = X509_NAME_oneline(X509_get_issuer_name(cert), nullptr, 0);
    info.emisor = issuerOneline ? issuerOneline : "";
    OPENSSL_free(issuerOneline);

    info.serialNumber = asn1IntegerToString(X509_get_serialNumber(cert));

    // PKCS12_parse deja el friendlyName de la bolsa como alias del certificado.
    int aliasLen = 0;
    const unsigned char* alias = X509_alias_get0(cert, &aliasLen);
    if (alias && aliasLen > 0) info.alias.assign(reinterpret_cast<const char*>(alias), static_cast<size_t>(aliasLen));
    info.validoDesde = asn1TimeToIsoString(X509_get0_notBefore(cert));
    info.validoHasta = asn1TimeToIsoString(X509_get0_notAfter(cert));

    unsigned char* derBuf = nullptr;
    int derLen = i2d_X509(cert, &derBuf);
    if (derLen > 0 && derBuf) {
        info.certificadoBase64 = Base64::encode(derBuf, static_cast<size_t>(derLen));
        auto certDigest = Pkcs12Signer::computeDigestSha256(derBuf, static_cast<size_t>(derLen));
        info.digestSha256Base64 = Base64::encode(certDigest);
        info.digestSha256Hex = Pkcs12Signer::computeDigestSha256Hex(derBuf, static_cast<size_t>(derLen));
        OPENSSL_free(derBuf);
    }

    // Vigente si la fecha actual está entre notBefore y notAfter.
    info.esValido = X509_cmp_current_time(X509_get0_notBefore(cert)) <= 0 &&
                    X509_cmp_current_time(X509_get0_notAfter(cert)) >= 0;
    return info;
}

static X509* x509DesdeDer(const std::vector<uint8_t>& der) {
    const unsigned char* p = der.data();
    return d2i_X509(nullptr, &p, static_cast<long>(der.size()));
}

} // namespace

Pkcs12Signer::Pkcs12Signer() = default;

Pkcs12Signer::~Pkcs12Signer() {
    cleanup();
}

void Pkcs12Signer::cleanup() {
    if (_privateKey) {
        EVP_PKEY_free(_privateKey);
        _privateKey = nullptr;
    }
    if (_certificate) {
        X509_free(_certificate);
        _certificate = nullptr;
    }
}

Pkcs12Signer::Pkcs12Signer(Pkcs12Signer&& other) noexcept
    : _privateKey(other._privateKey), _certificate(other._certificate), _info(std::move(other._info)) {
    other._privateKey = nullptr;
    other._certificate = nullptr;
}

Pkcs12Signer& Pkcs12Signer::operator=(Pkcs12Signer&& other) noexcept {
    if (this != &other) {
        cleanup();
        _privateKey = other._privateKey;
        _certificate = other._certificate;
        _info = std::move(other._info);
        other._privateKey = nullptr;
        other._certificate = nullptr;
    }
    return *this;
}

std::vector<uint8_t> Pkcs12Signer::computeDigestSha256(const uint8_t* data, size_t len) {
    std::vector<uint8_t> digest(SHA256_DIGEST_LENGTH);
    SHA256(data, len, digest.data());
    return digest;
}

std::string Pkcs12Signer::computeDigestSha256Base64(const std::string& data) {
    auto d = computeDigestSha256(reinterpret_cast<const uint8_t*>(data.data()), data.size());
    return Base64::encode(d);
}

std::string Pkcs12Signer::computeDigestSha256Hex(const uint8_t* data, size_t len) {
    auto d = computeDigestSha256(data, len);
    std::ostringstream ss;
    for (uint8_t b : d) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
    }
    return ss.str();
}

std::pair<Pkcs12Signer, domain::DiagnosticList> Pkcs12Signer::loadFromBytes(const std::vector<uint8_t>& p12Bytes, const std::string& pin) {
    domain::DiagnosticList diags;
    Pkcs12Signer signer;

    if (p12Bytes.empty()) {
        diags.addError("CRYPTO_P12_EMPTY", "p12", "El buffer del certificado PKCS#12 (.p12) está vacío.", domain::DiagnosticCategory::Structural);
        return {std::move(signer), diags};
    }

    const unsigned char* p = p12Bytes.data();
    PKCS12* p12 = d2i_PKCS12(nullptr, &p, static_cast<long>(p12Bytes.size()));
    if (!p12) {
        diags.addError("CRYPTO_P12_FORMAT", "p12", "Formato de archivo inválido: no es un contenedor PKCS#12 válido (.p12/.pfx). " + getOpenSSLError(), domain::DiagnosticCategory::Structural);
        return {std::move(signer), diags};
    }

    EVP_PKEY* pkey = nullptr;
    X509* cert = nullptr;
    STACK_OF(X509)* ca = nullptr;

    int parseRes = PKCS12_parse(p12, pin.c_str(), &pkey, &cert, &ca);
    PKCS12_free(p12);

    if (parseRes != 1 || !pkey || !cert) {
        if (pkey) EVP_PKEY_free(pkey);
        if (cert) X509_free(cert);
        if (ca) sk_X509_pop_free(ca, X509_free);
        diags.addError("CRYPTO_P12_AUTH_FAILED", "pin", "Contraseña/PIN incorrecto o error al extraer llave privada/certificado: " + getOpenSSLError(), domain::DiagnosticCategory::BusinessRule);
        return {std::move(signer), diags};
    }

    if (ca) {
        sk_X509_pop_free(ca, X509_free);
    }

    signer._privateKey = pkey;
    signer._certificate = cert;
    signer._info = infoDesdeX509(cert);
    return {std::move(signer), diags};
}

std::pair<CertificadoInfo, domain::DiagnosticList> Pkcs12Signer::leerCertificadoDer(const std::vector<uint8_t>& der) {
    domain::DiagnosticList diags;
    if (der.empty()) {
        diags.addError("CRYPTO_CERT_EMPTY", "certificado", "El certificado X.509 está vacío.", domain::DiagnosticCategory::Structural);
        return {{}, diags};
    }
    X509* cert = x509DesdeDer(der);
    if (!cert) {
        diags.addError("CRYPTO_CERT_FORMAT", "certificado", "El certificado no es un X.509 DER válido: " + getOpenSSLError(), domain::DiagnosticCategory::Structural);
        return {{}, diags};
    }
    CertificadoInfo info = infoDesdeX509(cert);
    X509_free(cert);
    return {info, diags};
}

domain::DiagnosticList Pkcs12Signer::verificarSha256(const std::string& certificadoBase64,
                                                     const std::string& datos,
                                                     const std::string& firmaBase64) {
    domain::DiagnosticList diags;
    auto der = Base64::decode(certificadoBase64);
    auto firma = Base64::decode(firmaBase64);
    if (der.empty() || firma.empty()) {
        diags.addError("CRYPTO_VERIFY_INPUT", "signatureValue", "El certificado o la firma no son Base64 válido.", domain::DiagnosticCategory::Structural);
        return diags;
    }
    X509* cert = x509DesdeDer(der);
    EVP_PKEY* pub = cert ? X509_get_pubkey(cert) : nullptr;
    EVP_MD_CTX* ctx = pub ? EVP_MD_CTX_new() : nullptr;
    int ok = ctx &&
             EVP_DigestVerifyInit(ctx, nullptr, EVP_sha256(), nullptr, pub) == 1 &&
             EVP_DigestVerifyUpdate(ctx, datos.data(), datos.size()) == 1 &&
             EVP_DigestVerifyFinal(ctx, firma.data(), firma.size()) == 1;
    if (ctx) EVP_MD_CTX_free(ctx);
    if (pub) EVP_PKEY_free(pub);
    if (cert) X509_free(cert);
    if (!ok) {
        ERR_clear_error();
        diags.addError("SIGN_VERIFY_FAILED", "signatureValue",
                       "La firma (SignatureValue) no corresponde a SignedInfo con la llave del certificado. "
                       "Revise que la llave usada sea la del certificado y que se firmara SignedInfo sin cambios.",
                       domain::DiagnosticCategory::BusinessRule);
    }
    return diags;
}

std::pair<Pkcs12Signer, domain::DiagnosticList> Pkcs12Signer::loadFromBase64(const std::string& p12Base64, const std::string& pin) {
    domain::DiagnosticList diags;
    auto bytes = Base64::decode(p12Base64);
    if (bytes.empty()) {
        diags.addError("CRYPTO_P12_BASE64_INVALID", "p12Base64", "Cadena Base64 de PKCS#12 inválida o vacía.", domain::DiagnosticCategory::Structural);
        Pkcs12Signer emptySigner;
        return {std::move(emptySigner), diags};
    }
    return loadFromBytes(bytes, pin);
}

std::pair<std::vector<uint8_t>, domain::DiagnosticList> Pkcs12Signer::signSha256(const uint8_t* data, size_t len) const {
    domain::DiagnosticList diags;
    if (!_privateKey) {
        diags.addError("CRYPTO_NO_KEY", "privateKey", "La clave privada no está cargada.", domain::DiagnosticCategory::BusinessRule);
        return {{}, diags};
    }

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) {
        diags.addError("CRYPTO_CTX_ALLOC", "ctx", "Error al inicializar contexto EVP_MD_CTX.", domain::DiagnosticCategory::BusinessRule);
        return {{}, diags};
    }

    if (EVP_DigestSignInit(ctx, nullptr, EVP_sha256(), nullptr, _privateKey) <= 0) {
        EVP_MD_CTX_free(ctx);
        diags.addError("CRYPTO_SIGN_INIT_FAILED", "signature", "Error en EVP_DigestSignInit: " + getOpenSSLError(), domain::DiagnosticCategory::BusinessRule);
        return {{}, diags};
    }

    if (EVP_DigestSignUpdate(ctx, data, len) <= 0) {
        EVP_MD_CTX_free(ctx);
        diags.addError("CRYPTO_SIGN_UPDATE_FAILED", "signature", "Error en EVP_DigestSignUpdate: " + getOpenSSLError(), domain::DiagnosticCategory::BusinessRule);
        return {{}, diags};
    }

    size_t sigLen = 0;
    if (EVP_DigestSignFinal(ctx, nullptr, &sigLen) <= 0 || sigLen == 0) {
        EVP_MD_CTX_free(ctx);
        diags.addError("CRYPTO_SIGN_LENGTH_FAILED", "signature", "Error al calcular longitud de firma: " + getOpenSSLError(), domain::DiagnosticCategory::BusinessRule);
        return {{}, diags};
    }

    std::vector<uint8_t> signature(sigLen);
    if (EVP_DigestSignFinal(ctx, signature.data(), &sigLen) <= 0) {
        EVP_MD_CTX_free(ctx);
        diags.addError("CRYPTO_SIGN_FINAL_FAILED", "signature", "Error al generar firma RSA-SHA256: " + getOpenSSLError(), domain::DiagnosticCategory::BusinessRule);
        return {{}, diags};
    }

    signature.resize(sigLen);
    EVP_MD_CTX_free(ctx);
    return {std::move(signature), diags};
}

std::pair<std::string, domain::DiagnosticList> Pkcs12Signer::signSha256Base64(const std::string& canonicalXml) const {
    auto [sigBytes, diags] = signSha256(reinterpret_cast<const uint8_t*>(canonicalXml.data()), canonicalXml.size());
    if (diags.hasErrors()) {
        return {"", diags};
    }
    return {Base64::encode(sigBytes), diags};
}

} // namespace cec::crypto

