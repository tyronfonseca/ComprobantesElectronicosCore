#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include "cec/crypto/Pkcs12Signer.h"
#include "cec/domain/Diagnostics.h"

namespace cec::crypto {

/// Resultado de XadesSigner::prepararFirma.
struct FirmaPreparada {
    /// XML con <ds:Signature> completo salvo SignatureValue (vacío).
    std::string xml;
    /// SignedInfo canonicalizado (C14N): los bytes exactos que se firman con RSA-SHA256.
    std::string signedInfo;
};

class XadesSigner {
public:
    // Constantes oficiales de la Política de Firma de Costa Rica (Resolución DGT-R-48-2016 v4.2/v4.3/v4.4)
    static constexpr const char* HACIENDA_POLICY_URL = "https://tribunet.hacienda.go.cr/docs/esquemas/2016/v4.2/ResolucionComprobantesElectronicosDGT-R-48-2016_4.2.pdf";
    static constexpr const char* HACIENDA_POLICY_DESC = "Politica de Firma de Comprobantes Electronicos";
    static constexpr const char* HACIENDA_POLICY_HASH = "dMoQENAjgMcSNMglTl/3n2bGammD7UeAKax+phExc38=";

    /**
     * Firma un documento XML oficial de Hacienda utilizando el estándar XAdES-EPES Enveloped (RSA-SHA256).
     */
    static std::pair<std::string, domain::DiagnosticList> signXml(
        const std::string& xmlSinFirmar,
        const std::vector<uint8_t>& p12Bytes,
        const std::string& pin
    );

    static std::pair<std::string, domain::DiagnosticList> signXml(
        const std::string& xmlSinFirmar,
        const Pkcs12Signer& signer
    );

    /**
     * Firma en dos pasos, para cuando la llave privada no está en este proceso (keystore de un servidor, HSM).
     * Paso 1: arma la firma XAdES-EPES con el certificado público (digests del documento, KeyInfo y
     * SignedProperties) y devuelve el SignedInfo canonicalizado que se debe firmar con RSA-SHA256.
     */
    static std::pair<FirmaPreparada, domain::DiagnosticList> prepararFirma(
        const std::string& xmlSinFirmar,
        const CertificadoInfo& certificado
    );

    /**
     * Paso 2: inserta el SignatureValue (Base64) en el XML preparado. Con `verificar`, comprueba la firma
     * contra SignedInfo con la llave pública del certificado de KeyInfo antes de devolver el XML.
     */
    static std::pair<std::string, domain::DiagnosticList> completarFirma(
        const std::string& xmlPreparado,
        const std::string& signatureValueBase64,
        bool verificar = true
    );
};

} // namespace cec::crypto

