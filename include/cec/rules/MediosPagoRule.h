#pragma once
#include <algorithm>
#include "cec/rules/IValidationRule.h"
#include "cec/rules/RuleValidationHelpers.h"
#include "cec/rules/CatalogoFiscal.h"

namespace cec::rules {

/**
 * @brief Regla 7: Medios de Pago (mínimo 1, máximo 4).
 */
class MediosPagoRule : public IValidationRule {
public:
    void validate(const domain::ComprobanteCanonico& inv, domain::DiagnosticList& diags) const override {
        // Exento en ventas a crédito: códigos 02, 08 y 10 de la Nota 5.
        const bool esCredito = inv.condicionVenta == domain::CondicionVenta::Credito ||
                               inv.condicionVenta == domain::CondicionVenta::ServiciosPrestadosAlEstado ||
                               inv.condicionVenta == domain::CondicionVenta::VentaCreditoEnPlazo;

        if (inv.mediosPago.empty() && !esCredito) {
            diags.addError("RULE_MEDIO_PAGO_REQUIRED", "resumenFactura.mediosPago", "Debe indicarse al menos un medio de pago.", domain::DiagnosticCategory::BusinessRule);
        } else if (inv.mediosPago.size() > 4) {
            diags.addError("RULE_MEDIO_PAGO_MAX_EXCEEDED", "resumenFactura.mediosPago", "No se permiten más de 4 medios de pago.", domain::DiagnosticCategory::BusinessRule);
        }

        // Si todos vienen sin monto, el motor de cálculo reparte el total; no se permite mezclar medios con y sin monto.
        const bool algunoConMonto = std::any_of(inv.mediosPago.begin(), inv.mediosPago.end(),
            [](const domain::MedioPagoInfo& m) { return !m.totalMedioPago.isZero(); });
        const bool variosMedios = inv.mediosPago.size() > 1 && algunoConMonto;
        for (size_t i = 0; i < inv.mediosPago.size(); ++i) {
            const auto& mp = inv.mediosPago[i];
            const std::string p = "resumenFactura.mediosPago[" + std::to_string(i) + "]";

            if (!CatalogoFiscal::esMedioPagoValido(mp.tipo)) {
                diags.addError("RULE_MEDIO_PAGO_TIPO_INVALID", p + ".tipoMedioPago", "El tipo de medio de pago no existe en el catálogo de Hacienda (Nota 6).", domain::DiagnosticCategory::Catalog);
            }
            if (mp.tipo == domain::MedioPago::Otros && !RuleValidationHelpers::textoEnRango(mp.otros, 3, 100)) {
                diags.addError("RULE_MEDIO_PAGO_OTROS_LENGTH", p + ".medioPagoOtros", "El detalle del medio de pago 'Otros' (99) es obligatorio y debe tener entre 3 y 100 caracteres.", domain::DiagnosticCategory::BusinessRule);
            }
            // TotalMedioPago es obligatorio con más de un medio de pago y, si se indica, debe ser mayor a cero.
            if (mp.totalMedioPago.isNegative() || (variosMedios && mp.totalMedioPago.isZero())) {
                diags.addError("RULE_MEDIO_PAGO_MONTO_NON_POSITIVE", p + ".totalMedioPago", "El monto por medio de pago debe ser mayor a cero (con más de un medio de pago, o todos los montos se indican o ninguno).", domain::DiagnosticCategory::BusinessRule);
            }
        }
        // La suma de los medios de pago vs. TotalComprobante se valida post-cálculo (ComprobanteService::validateInvariants).
    }
};

} // namespace cec::rules

