#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include "cec/crypto/CertificadoInfo.h"
#include "cec/domain/Diagnostics.h"

// Forward declarations de OpenSSL
typedef struct evp_pkey_st EVP_PKEY;
typedef struct x509_st X509;

namespace cec::crypto {

class Pkcs12Signer {
private:
    EVP_PKEY* _privateKey = nullptr;
    X509* _certificate = nullptr;
    CertificadoInfo _info;

    void cleanup();

public:
    Pkcs12Signer();
    ~Pkcs12Signer();

    Pkcs12Signer(const Pkcs12Signer&) = delete;
    Pkcs12Signer& operator=(const Pkcs12Signer&) = delete;

    Pkcs12Signer(Pkcs12Signer&& other) noexcept;
    Pkcs12Signer& operator=(Pkcs12Signer&& other) noexcept;

    static std::pair<Pkcs12Signer, domain::DiagnosticList> loadFromBytes(const std::vector<uint8_t>& p12Bytes, const std::string& pin);
    static std::pair<Pkcs12Signer, domain::DiagnosticList> loadFromBase64(const std::string& p12Base64, const std::string& pin);

    /// Datos de un certificado X.509 (DER) sin llave privada: lo necesario para preparar una firma
    /// cuando la llave vive en otro lugar (keystore del servidor, HSM).
    static std::pair<CertificadoInfo, domain::DiagnosticList> leerCertificadoDer(const std::vector<uint8_t>& der);

    /// Verifica una firma RSA-SHA256 (Base64) de `datos` con la llave pública del certificado (DER en Base64).
    static domain::DiagnosticList verificarSha256(const std::string& certificadoBase64,
                                                  const std::string& datos,
                                                  const std::string& firmaBase64);

    static std::vector<uint8_t> computeDigestSha256(const uint8_t* data, size_t len);
    static std::string computeDigestSha256Base64(const std::string& data);
    static std::string computeDigestSha256Hex(const uint8_t* data, size_t len);

    std::pair<std::vector<uint8_t>, domain::DiagnosticList> signSha256(const uint8_t* data, size_t len) const;
    std::pair<std::string, domain::DiagnosticList> signSha256Base64(const std::string& canonicalXml) const;

    const CertificadoInfo& getCertificadoInfo() const { return _info; }
    bool isLoaded() const { return _privateKey != nullptr && _certificate != nullptr; }
};

} // namespace cec::crypto

