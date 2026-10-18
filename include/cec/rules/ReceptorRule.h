#pragma once
#include <cstddef>
#include "cec/rules/IValidationRule.h"
#include "cec/rules/RuleValidationHelpers.h"

namespace cec::rules {

/**
 * @brief Regla 5: Datos del Receptor (presencia, identificación, nombre, ubicación, restricciones).
 */
class ReceptorRule : public IValidationRule {
public:
    void validate(const domain::ComprobanteCanonico& inv, domain::DiagnosticList& diags) const override {
        using domain::DiagnosticCategory;
        using domain::TipoIdentificacion;

        constexpr std::size_t kNombreMin = 3, kNombreMax = 100;
        constexpr std::size_t kNombreComercialMin = 3, kNombreComercialMax = 80;
        constexpr std::size_t kUbicacionOtrasSenasMax = 160;
        constexpr std::size_t kOtrasSenasExtranjeroMin = 5, kOtrasSenasExtranjeroMax = 300;

        // ---- Presencia del receptor ----
        if (inv.isREP || inv.isFEC) {
            if (!inv.receptor.has_value()) {
                diags.addError(inv.isREP ? "RULE_REP_RECEPTOR_REQUIRED" : "RULE_FEC_RECEPTOR_REQUIRED",
                    "receptor", "El receptor es obligatorio en el comprobante electrónico.",
                    DiagnosticCategory::BusinessRule);
            } else if (!inv.receptor->identificacion.has_value()) {
                diags.addError(inv.isREP ? "RULE_REP_RECEPTOR_ID_REQUIRED" : "RULE_RECEPTOR_ID_REQUIRED",
                    "receptor.identificacion", "La identificación del receptor es obligatoria.",
                    DiagnosticCategory::BusinessRule);
            }
        } else if (inv.isFE && !inv.receptor.has_value()) {
            diags.addError("RULE_FE_RECEPTOR_REQUIRED", "receptor",
                "El receptor es obligatorio en Factura Electrónica.",
                DiagnosticCategory::BusinessRule);
        }

        // Sin receptor no hay nada más que validar
        if (!inv.receptor.has_value()) {
            return;
        }

        const auto& rec = *inv.receptor;
        const bool esExtranjeroNoDomiciliado =
            rec.identificacion.has_value() &&
            rec.identificacion->tipo == TipoIdentificacion::ExtranjeroNoDomiciliado;

        // ---- Nombre ----
        if (!RuleValidationHelpers::textoEnRango(rec.nombre, kNombreMin, kNombreMax)) {
            diags.addError("RULE_RECEPTOR_NOMBRE_LENGTH", "receptor.nombre",
                "El nombre o razón social del receptor debe tener entre 3 y 100 caracteres.",
                DiagnosticCategory::BusinessRule);
        }

        // ---- Correo electrónico (condicional en todos los comprobantes; si viene, debe tener formato válido) ----
        if (!rec.correoElectronico.empty()) {
            RuleValidationHelpers::validarEmail("receptor.correoElectronico", rec.correoElectronico, diags);
        }

        // ---- Teléfono, nombre comercial, ubicación y otras señas extranjero ----
        if (inv.isREP) {
            if (rec.telefono.has_value()) {
                diags.addError("RULE_REP_RECEPTOR_TELEFONO_PROHIBIDO", "receptor.telefono",
                    "El teléfono del receptor está prohibido en el Recibo Electrónico de Pago.",
                    DiagnosticCategory::BusinessRule);
            }
            if (!rec.nombreComercial.empty()) {
                diags.addError("RULE_REP_RECEPTOR_NOMBRE_COMERCIAL_PROHIBIDO", "receptor.nombreComercial",
                    "El nombre comercial del receptor está prohibido en el Recibo Electrónico de Pago.",
                    DiagnosticCategory::BusinessRule);
            }
            if (rec.ubicacion.has_value()) {
                diags.addError("RULE_REP_RECEPTOR_UBICACION_PROHIBIDA", "receptor.ubicacion",
                    "La ubicación del receptor está prohibida en el Recibo Electrónico de Pago.",
                    DiagnosticCategory::BusinessRule);
            }
            if (!rec.otrasSenasExtranjero.value_or("").empty()) {
                diags.addError("RULE_REP_RECEPTOR_OTRAS_SENAS_EXTRANJERO_PROHIBIDO", "receptor.otrasSenasExtranjero",
                    "El Recibo Electrónico de Pago no admite el campo OtrasSenasExtranjero en el receptor.",
                    DiagnosticCategory::BusinessRule);
            }
        } else {
            // Identificación obligatoria (FEC ya se valida arriba)
            if (!rec.identificacion.has_value() && (inv.isFE || inv.isFEE)) {
                diags.addError("RULE_RECEPTOR_ID_REQUIRED", "receptor.identificacion",
                    "La identificación del receptor es obligatoria para este tipo de comprobante.",
                    DiagnosticCategory::BusinessRule);
            }

            if (!rec.nombreComercial.empty() &&
                !RuleValidationHelpers::textoEnRango(rec.nombreComercial, kNombreComercialMin, kNombreComercialMax)) {
                diags.addError("RULE_RECEPTOR_NOMBRE_COMERCIAL_LENGTH", "receptor.nombreComercial",
                    "El nombre comercial del receptor debe tener entre 3 y 80 caracteres.",
                    DiagnosticCategory::BusinessRule);
            }

            if (rec.telefono.has_value()) {
                RuleValidationHelpers::validarTelefono("receptor.telefono", *rec.telefono, diags);
            }

            if (rec.ubicacion.has_value()) {
                if (esExtranjeroNoDomiciliado) {
                    diags.addError("RULE_RECEPTOR_EXTRANJERO_UBICACION_PROHIBIDA", "receptor.ubicacion",
                        "La ubicación en Costa Rica no aplica para receptores extranjeros no domiciliados.",
                        DiagnosticCategory::BusinessRule);
                } else if (inv.isFEE) {
                    diags.addError("RULE_FEE_RECEPTOR_UBICACION_PROHIBIDA", "receptor.ubicacion",
                        "La Factura Electrónica de Exportación no admite el campo Ubicación en el receptor.",
                        DiagnosticCategory::BusinessRule);
                } else {
                    RuleValidationHelpers::validarUbicacion("receptor.ubicacion", *rec.ubicacion, diags,
                                                            kUbicacionOtrasSenasMax);
                }
            }

            if (rec.otrasSenasExtranjero.has_value() && !rec.otrasSenasExtranjero.value().empty()) {
                if (inv.isFEC) {
                    diags.addError("RULE_FEC_RECEPTOR_OTRAS_SENAS_EXTRANJERO_PROHIBIDO", "receptor.otrasSenasExtranjero",
                        "La Factura Electrónica de Compra no admite el campo OtrasSenasExtranjero en el receptor.",
                        DiagnosticCategory::BusinessRule);
                }
                if (inv.isFE && !esExtranjeroNoDomiciliado) {
                    diags.addError("RULE_FE_RECEPTOR_OTRAS_SENAS_EXTRANJERO_TIPO_ID", "receptor.otrasSenasExtranjero",
                        "En Factura Electrónica, el campo OtrasSenasExtranjero solo se permite cuando el tipo de identificación del receptor es Extranjero No Domiciliado (05).",
                        DiagnosticCategory::BusinessRule);
                }
                if (!RuleValidationHelpers::textoEnRango(*rec.otrasSenasExtranjero, kOtrasSenasExtranjeroMin, kOtrasSenasExtranjeroMax)) {
                    diags.addError("RULE_RECEPTOR_OTRAS_SENAS_EXTRANJERO_LENGTH", "receptor.otrasSenasExtranjero",
                        "El campo OtrasSenasExtranjero del receptor debe tener entre 5 y 300 caracteres.",
                        DiagnosticCategory::BusinessRule);
                }
            }
        }

        // ---- Identificación ----
        if (rec.identificacion.has_value()) {
            RuleValidationHelpers::validarIdentificacion("receptor.identificacion", *rec.identificacion, diags);

            // 06 No Contribuyente no es válido en el receptor
            if (rec.identificacion->tipo == TipoIdentificacion::ExtranjeroIdentificacion) {
                diags.addError("RULE_RECEPTOR_NO_CONTRIBUYENTE_PROHIBIDO", "receptor.identificacion.tipo",
                    "El tipo de identificación 'No Contribuyente (06)' no es válido para el receptor.",
                    DiagnosticCategory::BusinessRule);
            }

            if (esExtranjeroNoDomiciliado) {
                if (inv.isFEC || inv.isREP) {
                    diags.addError("RULE_RECEPTOR_EXTRANJERO_PROHIBIDO", "receptor.identificacion.tipo",
                        "La identificación de Extranjero No Domiciliado (05) en el receptor está prohibida en Factura de Compra y Recibo de Pago.",
                        DiagnosticCategory::BusinessRule);
                } else if (inv.isFE && inv.condicionVenta != domain::CondicionVenta::VentaMercanciaNoNacionalizada) {
                    diags.addError("RULE_FE_RECEPTOR_EXTRANJERO_CONDICION_12", "receptor.identificacion.tipo",
                        "En Factura Electrónica, el receptor solo puede ser Extranjero No Domiciliado si la condición de venta es 12 (Venta Mercancía No Nacionalizada).",
                        DiagnosticCategory::BusinessRule);
                }
            }
        }
    }
};

} // namespace cec::rules