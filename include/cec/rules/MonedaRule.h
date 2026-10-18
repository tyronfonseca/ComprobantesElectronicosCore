#pragma once
#include <cstdint>
#include "cec/rules/IValidationRule.h"
#include "cec/rules/CatalogoFiscal.h"

namespace cec::rules {

/**
 * @brief Regla 8: Moneda y Tipo de Cambio.
 */
class MonedaRule : public IValidationRule {
public:
    void validate(const domain::ComprobanteCanonico& inv, domain::DiagnosticList& diags) const override {
        if (!CatalogoFiscal::esMonedaValida(inv.resumen.codigoTipoMoneda.codigoMoneda)) {
            diags.addError("RULE_MONEDA_INVALID", "resumenFactura.codigoTipoMoneda.codigoMoneda", "El código de moneda no es válido en el catálogo de Hacienda.", domain::DiagnosticCategory::Catalog);
        }
        // En CRC el tipo de cambio debe ser 1 (si se omite, el parseo lo normaliza a 1).
        const int64_t tipoCambio = inv.resumen.codigoTipoMoneda.tipoCambioScaled;
        if (inv.resumen.codigoTipoMoneda.codigoMoneda == "CRC" && tipoCambio != 0 && tipoCambio != domain::Money::SCALE) {
            diags.addError("RULE_TIPO_CAMBIO_CRC", "resumenFactura.codigoTipoMoneda.tipoCambio", "Cuando la moneda es colones (CRC) el tipo de cambio debe ser 1.", domain::DiagnosticCategory::BusinessRule);
        }
        if (inv.resumen.codigoTipoMoneda.codigoMoneda != "CRC" && tipoCambio <= 0) {
            diags.addError("RULE_TIPO_CAMBIO_REQUIRED", "resumenFactura.codigoTipoMoneda.tipoCambio", "El tipo de cambio es obligatorio y debe ser positivo para moneda extranjera.", domain::DiagnosticCategory::BusinessRule);
        }
    }
};

} // namespace cec::rules

