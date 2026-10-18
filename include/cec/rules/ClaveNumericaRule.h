#pragma once

#include <algorithm>
#include <cctype>
#include "cec/rules/IValidationRule.h"

namespace cec::rules {
/**
 * @brief Regla 1: Validación de Clave Numérica (notas 1, 3 y 4.1).
 *
 * Estructura (posiciones base 0):
 *   [0-2]   Código de país "506"
 *   [3-8]   Fecha DDMMYY
 *   [9-20]  Identificación del obligado tributario (12 dígitos, rellenada con ceros a la izquierda)
 *   [21-40] Número consecutivo (20 dígitos; tipo de documento en [29-30])
 *   [41]    Situación: 1 Normal, 2 Contingencia, 3 Sin internet
 *   [42-49] Código de seguridad
 */
class ClaveNumericaRule : public IValidationRule {
public:
    void validate(const domain::ComprobanteCanonico& inv, domain::DiagnosticList& diags) const override {
        std::string_view expectedTipoCode = domain::getTipoDocumentoCode(inv.tipoDocumento);

        if (inv.clave.length() != 50) {
            diags.addError("RULE_CLAVE_LENGTH", "clave",
                "La clave debe tener exactamente 50 caracteres.",
                domain::DiagnosticCategory::BusinessRule);
            return;
        }

        // Solo la identificación (posiciones 9-20) admite letras, para personas jurídicas
        for (std::size_t i = 0; i < inv.clave.length(); ++i) {
            const auto ch = static_cast<unsigned char>(inv.clave[i]);
            const bool enIdentificacion = i >= 9 && i <= 20;
            if (enIdentificacion ? !std::isalnum(ch) : !std::isdigit(ch)) {
                diags.addError("RULE_CLAVE_FORMAT", "clave",
                    "La clave solo admite dígitos, salvo la identificación (posiciones 9-20), que admite letras para personas jurídicas.",
                    domain::DiagnosticCategory::BusinessRule);
                return;
            }
        }

        if (inv.clave.compare(0, 3, "506") != 0) {
            diags.addError("RULE_CLAVE_CODIGO_PAIS", "clave",
                "La clave debe iniciar con el código de país 506.",
                domain::DiagnosticCategory::BusinessRule);
        }

        const char situacion = inv.clave[41];
        if (situacion < '1' || situacion > '3') {
            diags.addError("RULE_CLAVE_SITUACION", "clave",
                "La situación del comprobante en la clave (posición 41) debe ser 1 (Normal), 2 (Contingencia) o 3 (Sin internet).",
                domain::DiagnosticCategory::BusinessRule);
        }

        if (inv.numeroConsecutivo.length() == 20 && inv.clave.compare(21, 20, inv.numeroConsecutivo) != 0) {
            diags.addError("RULE_CLAVE_CONSECUTIVO_MISMATCH", "clave",
                "El número consecutivo contenido en la clave (posiciones 21-40) debe coincidir con NumeroConsecutivo.",
                domain::DiagnosticCategory::BusinessRule);
        }

        validarIdentificacion(inv, diags);

        if (inv.clave.substr(29, 2) != expectedTipoCode) {
            const std::string code = inv.isREP ? "RULE_REP_CLAVE_TIPO_DOC" : "RULE_CLAVE_TIPO_DOC";
            diags.addError(code, "clave",
                "El tipo de documento en la clave (posiciones 29-30) debe coincidir con el comprobante (" +
                std::string(expectedTipoCode) + ").",
                domain::DiagnosticCategory::BusinessRule);
        }

        if (inv.fechaEmision.length() >= 10 && inv.fechaEmision[4] == '-' && inv.fechaEmision[7] == '-') {
            const std::string expectedFechaClave =
                inv.fechaEmision.substr(8, 2) + inv.fechaEmision.substr(5, 2) + inv.fechaEmision.substr(2, 2);
            if (inv.clave.substr(3, 6) != expectedFechaClave) {
                diags.addError("RULE_CLAVE_FECHA_MISMATCH", "clave",
                    "La fecha en la clave numérica (DDMMYY) debe coincidir con la fecha de emisión del comprobante.",
                    domain::DiagnosticCategory::BusinessRule);
            }
        }
    }

private:
    // Nota 4.1: la identificación se rellena con ceros a la izquierda hasta 12 posiciones.
    // En la FEC quien genera y firma el comprobante es el receptor (Anexo 1, mecanismo de seguridad),
    // por lo que la clave lleva su identificación.
    static void validarIdentificacion(const domain::ComprobanteCanonico& inv, domain::DiagnosticList& diags) {
        const std::string* numero = nullptr;
        if (inv.isFEC) {
            if (inv.receptor.has_value() && inv.receptor->identificacion.has_value()) {
                numero = &inv.receptor->identificacion->numero;
            }
        } else {
            numero = &inv.emisor.identificacion.numero;
        }

        // Identificaciones de más de 12 caracteres (extranjeros) no caben en la clave y no se comparan.
        if (numero == nullptr || numero->empty() || numero->length() > 12 ||
            !std::all_of(numero->begin(), numero->end(), [](unsigned char ch) { return std::isalnum(ch) != 0; })) {
            return;
        }

        const std::string esperado = std::string(12 - numero->length(), '0') + *numero;
        if (inv.clave.compare(9, 12, esperado) != 0) {
            diags.addError("RULE_CLAVE_IDENTIFICACION_MISMATCH", "clave",
                inv.isFEC
                    ? "La identificación en la clave (posiciones 9-20) debe coincidir con la del receptor en Factura Electrónica de Compra."
                    : "La identificación en la clave (posiciones 9-20) debe coincidir con la del emisor.",
                domain::DiagnosticCategory::BusinessRule);
        }
    }
};
} // namespace cec::rules