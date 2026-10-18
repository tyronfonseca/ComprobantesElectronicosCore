#pragma once
// Certificado PKCS#12 temporal en memoria para los tests de firma (no es un certificado real).
#include <cstdint>
#include <string>
#include <vector>

#include <openssl/evp.h>
#include <openssl/pkcs12.h>
#include <openssl/x509.h>

namespace fixtures {

// Generador de contenedor PKCS#12 (.p12) temporal en memoria con OpenSSL para pruebas
inline std::vector<uint8_t> generarP12Prueba(const std::string& pin, const std::string& subjectCN = "EMPRESA PRUEBA S.A.") {
    EVP_PKEY* pkey = nullptr;
    EVP_PKEY_CTX* pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
    EVP_PKEY_keygen_init(pctx);
    EVP_PKEY_CTX_set_rsa_keygen_bits(pctx, 2048);
    EVP_PKEY_keygen(pctx, &pkey);
    EVP_PKEY_CTX_free(pctx);

    X509* x509 = X509_new();
    ASN1_INTEGER_set(X509_get_serialNumber(x509), 987654321);
    X509_gmtime_adj(X509_getm_notBefore(x509), 0);
    X509_gmtime_adj(X509_getm_notAfter(x509), 31536000L); // 1 año

    X509_set_pubkey(x509, pkey);

    // Un nombre propio en vez de modificar el que devuelve X509_get_subject_name: en las versiones recientes de
    // OpenSSL ese getter devuelve const X509_NAME*.
    X509_NAME* name = X509_NAME_new();
    X509_NAME_add_entry_by_txt(name, "C", MBSTRING_ASC, (const unsigned char*)"CR", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "O", MBSTRING_ASC, (const unsigned char*)"BCCR-SINPE", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (const unsigned char*)subjectCN.c_str(), -1, -1, 0);
    X509_set_subject_name(x509, name);
    X509_set_issuer_name(x509, name);
    X509_NAME_free(name);

    X509_sign(x509, pkey, EVP_sha256());

    PKCS12* p12 = PKCS12_create(
        pin.c_str(),
        "Certificado Prueba Core",
        pkey,
        x509,
        nullptr,
        0, 0, 0, 0, 0
    );

    unsigned char* p12Buf = nullptr;
    int p12Len = i2d_PKCS12(p12, &p12Buf);

    std::vector<uint8_t> result(p12Buf, p12Buf + p12Len);

    OPENSSL_free(p12Buf);
    PKCS12_free(p12);
    X509_free(x509);
    EVP_PKEY_free(pkey);

    return result;
}

} // namespace fixtures
