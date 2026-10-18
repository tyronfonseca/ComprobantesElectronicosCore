#pragma once
#include <string>

namespace cec::crypto {

struct CertificadoInfo {
    std::string sujeto;             // Subject / Titular (CN, O, etc.)
    std::string emisor;             // Issuer (CN, O, C)
    std::string serialNumber;       // Número de serie decimal
    std::string validoDesde;        // ISO 8601 UTC
    std::string validoHasta;        // ISO 8601 UTC
    std::string digestSha256Hex;    // Hash SHA-256 en formato Hexadecimal
    std::string digestSha256Base64; // Hash SHA-256 en formato Base64
    std::string certificadoBase64;  // Certificado X.509 DER codificado en Base64
    std::string alias;              // friendlyName del PKCS#12 (alias de la llave en un keystore); vacío si no tiene
    bool esValido = false;          // Vigente: la fecha actual está entre validoDesde y validoHasta
};

} // namespace cec::crypto

