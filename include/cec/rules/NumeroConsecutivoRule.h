#pragma once
#include <string>
#include <cctype>
#include <string_view>
#include <algorithm>
#include "cec/rules/IValidationRule.h"

namespace cec::rules {

/**
 * @brief Regla 2: Validación de Número Consecutivo (20 dígitos y coherencia tipo documento).
 */
class NumeroConsecutivoRule : public IValidationRule {
public:
    void validate(const domain::ComprobanteCanonico& inv, domain::DiagnosticList& diags) const override {
        std::string_view expectedTipoCode = domain::getTipoDocumentoCode(inv.tipoDocumento);
        const auto& c = inv.numeroConsecutivo;

        const bool soloDigitos = std::all_of(c.begin(), c.end(),
            [](unsigned char ch) { return std::isdigit(ch) != 0; });

        if (c.length() != 20 || !soloDigitos) {
            diags.addError("RULE_CONSECUTIVO_LENGTH", "numeroConsecutivo",
                "El número consecutivo debe tener exactamente 20 dígitos numéricos.",
                domain::DiagnosticCategory::BusinessRule);
            return;
        }

        if (c.substr(8, 2) != expectedTipoCode) {
            diags.addError(inv.isREP ? "RULE_REP_CONSECUTIVO_TIPO_DOC" : "RULE_CONSECUTIVO_TIPO_DOC",
                "numeroConsecutivo",
                "El tipo de documento en el consecutivo (posiciones 8-9) debe coincidir con el comprobante (" +
                std::string(expectedTipoCode) + ").",
                domain::DiagnosticCategory::BusinessRule);
        }

        // La verificación de la secuencia la realiza Hacienda en la recepción; no es validable localmente.
    }
};

} // namespace cec::rules

