#pragma once
#include <algorithm>
#include "cec/rules/IValidationRule.h"
#include "cec/rules/RuleValidationHelpers.h"
#include "cec/rules/CatalogoFiscal.h"

namespace cec::rules {

/**
 * @brief Regla 6: Condición de Venta y Plazo de Crédito.
 */
class CondicionVentaRule : public IValidationRule {
public:
    void validate(const domain::ComprobanteCanonico& inv, domain::DiagnosticList& diags) const override {
        if (!CatalogoFiscal::esCondicionVentaValida(inv.condicionVenta)) {
            diags.addError("RULE_CONDICION_VENTA_INVALID", "condicionVenta", "La condición de venta no existe en el catálogo de Hacienda (Nota 5).", domain::DiagnosticCategory::Catalog);
            return;
        }
        if (inv.isREP) {
            if (inv.condicionVenta != domain::CondicionVenta::PagoServiciosPrestadosAlEstado &&
                inv.condicionVenta != domain::CondicionVenta::PagoACreditoEnPlazo) {
                diags.addError("RULE_REP_CONDICION_VENTA_INVALID", "condicionVenta", "El Recibo Electrónico de Pago únicamente permite condición de venta 09 u 11.", domain::DiagnosticCategory::BusinessRule);
            }
            if (!inv.plazoCredito.empty()) {
                diags.addError("RULE_REP_PLAZO_CREDITO_PROHIBIDO", "plazoCredito", "El plazo de crédito está prohibido en el Recibo Electrónico de Pago.", domain::DiagnosticCategory::BusinessRule);
            }
        } else {
            if (inv.condicionVenta == domain::CondicionVenta::PagoServiciosPrestadosAlEstado ||
                inv.condicionVenta == domain::CondicionVenta::PagoACreditoEnPlazo) {
                diags.addError("RULE_CONDICION_VENTA_EXCLUSIVA_REP", "condicionVenta", "Las condiciones de venta 09 y 11 son exclusivas del Recibo Electrónico de Pago.", domain::DiagnosticCategory::BusinessRule);
            }
            if (inv.condicionVenta == domain::CondicionVenta::VentaMercanciaNoNacionalizada && !inv.isFE) {
                diags.addError("RULE_CONDICION_VENTA_12_SOLO_FE", "condicionVenta", "La condición de venta 12 (Venta Mercancía No Nacionalizada) es exclusiva de Factura Electrónica.", domain::DiagnosticCategory::BusinessRule);
            }
            if (inv.condicionVenta == domain::CondicionVenta::VentaBienesUsadosNoContribuyentes && !inv.isFEC) {
                diags.addError("RULE_CONDICION_VENTA_13_SOLO_FEC", "condicionVenta", "La condición de venta '13 - Venta Bienes Usados No Contribuyente' aplica únicamente para Factura Electrónica de Compra.", domain::DiagnosticCategory::BusinessRule);
            }
            if (inv.condicionVenta == domain::CondicionVenta::Otros) {
                if (!RuleValidationHelpers::textoEnRango(inv.condicionVentaOtros, 5, 100)) {
                    diags.addError("RULE_CONDICION_VENTA_OTROS_REQUIRED", "condicionVentaOtros", "El detalle de condición de venta 'Otros' es obligatorio y debe tener entre 5 y 100 caracteres.", domain::DiagnosticCategory::BusinessRule);
                }
            }
            if (inv.condicionVenta == domain::CondicionVenta::Credito || inv.condicionVenta == domain::CondicionVenta::VentaCreditoEnPlazo) {
                if (inv.plazoCredito.empty()) {
                    diags.addError("RULE_PLAZO_CREDITO_REQUIRED", "plazoCredito", "El plazo de crédito es obligatorio cuando la condición de venta es Crédito o Venta a crédito en IVA hasta 90 días.", domain::DiagnosticCategory::BusinessRule);
                } else if (inv.plazoCredito.length() > 5 || !std::all_of(inv.plazoCredito.begin(), inv.plazoCredito.end(), ::isdigit) || std::stoi(inv.plazoCredito) <= 0) {
                    diags.addError("RULE_PLAZO_CREDITO_FORMAT", "plazoCredito", "El plazo de crédito debe ser un número entero mayor a cero de máximo 5 dígitos.", domain::DiagnosticCategory::BusinessRule);
                }
            }
        }
    }
};

} // namespace cec::rules

