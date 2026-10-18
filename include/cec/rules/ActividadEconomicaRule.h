#pragma once
#include "cec/rules/IValidationRule.h"

#include <string_view>

namespace cec::rules {

/**
 * @brief Regla 3: Código de Actividad Económica del Emisor y Receptor.
 */
class ActividadEconomicaRule final : public IValidationRule {
public:
    void validate(const domain::ComprobanteCanonico& inv,
                  domain::DiagnosticList& diags) const override {
        using domain::TipoDocumento;

        if (inv.tipoDocumento == TipoDocumento::ReciboElectronicoPago) {
            validarReciboPago(inv, diags);
            return;
        }

        const bool emisorRequerido = inv.isFE || inv.isFEE || inv.isTE;
        const bool receptorRequerido = inv.isFEC;
        const bool receptorProhibido = inv.isFEE || inv.isTE;

        validarEmisor(inv, diags, emisorRequerido);
        validarReceptor(inv, diags, receptorRequerido, receptorProhibido);
    }

private:
    static constexpr std::size_t kLongitudCodigo = 6;

    static void addBusinessError(domain::DiagnosticList& diags,
                                 std::string_view code,
                                 std::string_view path,
                                 std::string_view message) {
        diags.addError(std::string(code), std::string(path), std::string(message),
                       domain::DiagnosticCategory::BusinessRule);
    }

    static bool longitudValida(std::string_view codigo) noexcept {
        return codigo.size() == kLongitudCodigo;
    }

    static void validarReciboPago(const domain::ComprobanteCanonico& inv,
                                  domain::DiagnosticList& diags) {
        if (!inv.codigoActividadEmisor.empty()) {
            addBusinessError(diags, "RULE_REP_ACTIVIDAD_PROHIBIDA", "codigoActividadEmisor",
                "El código de actividad económica del emisor está prohibido en el Recibo Electrónico de Pago.");
        }
        if (!inv.codigoActividadReceptor.empty()) {
            addBusinessError(diags, "RULE_ACTIVIDAD_RECEPTOR_PROHIBIDA", "codigoActividadReceptor",
                "El código de actividad económica del receptor está prohibido en el Recibo Electrónico de Pago.");
        }
        if (!inv.emisor.nombreComercial.empty()) {
            addBusinessError(diags, "RULE_REP_NOMBRE_COMERCIAL_PROHIBIDO", "emisor.nombreComercial",
                "El nombre comercial del emisor está prohibido en el Recibo Electrónico de Pago.");
        }
    }

    static void validarEmisor(const domain::ComprobanteCanonico& inv,
                            domain::DiagnosticList& diags,
                            bool requerido) {
        const auto& codigo = inv.codigoActividadEmisor;

        if (codigo.empty()) {
            if (requerido) {
                addBusinessError(diags, "RULE_ACTIVIDAD_EMISOR_REQUIRED", "codigoActividadEmisor",
                    "El código de actividad económica del emisor es obligatorio para este tipo de comprobante.");
            }
            return;
        }
        if (!longitudValida(codigo)) {
            addBusinessError(diags, "RULE_ACTIVIDAD_FORMAT", "codigoActividadEmisor",
                "El código de actividad económica del emisor debe tener exactamente 6 caracteres.");
        }
    }

    static void validarReceptor(const domain::ComprobanteCanonico& inv,
                                domain::DiagnosticList& diags,
                                bool requerido, bool prohibido) {
        const auto& codigo = inv.codigoActividadReceptor;

        if (codigo.empty()) {
            if (requerido) {
                addBusinessError(diags, "RULE_ACTIVIDAD_RECEPTOR_REQUIRED", "codigoActividadReceptor",
                    "El código de actividad económica del receptor es obligatorio en Factura Electrónica de Compra.");
            }
            return;
        }
        if (prohibido) {
            addBusinessError(diags, "RULE_ACTIVIDAD_RECEPTOR_PROHIBIDA", "codigoActividadReceptor",
                "El código de actividad económica del receptor está prohibido en Factura Electrónica de Exportación y Tiquete Electrónico.");
        } else if (!longitudValida(codigo)) {
            addBusinessError(diags, "RULE_ACTIVIDAD_RECEPTOR_FORMAT", "codigoActividadReceptor",
                "El código de actividad económica del receptor debe tener exactamente 6 caracteres.");
        }
    }
    };

} // namespace cec::rules