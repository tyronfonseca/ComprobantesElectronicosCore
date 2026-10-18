#pragma once
#include "cec/rules/IValidationRule.h"
#include "cec/rules/RuleValidationHelpers.h"
#include "cec/rules/CatalogoFiscal.h"

namespace cec::rules {

/**
 * @brief Regla 11: Otros Cargos (prohibidos en REP, máximo 15, catálogo).
 */
class OtrosCargosRule : public IValidationRule {
public:
    void validate(const domain::ComprobanteCanonico& inv, domain::DiagnosticList& diags) const override {
        if (inv.isREP && !inv.otrosCargos.empty()) {
            diags.addError("RULE_REP_OTROS_CARGOS_PROHIBIDO", "otrosCargos", "El bloque de Otros Cargos está prohibido en Recibo Electrónico de Pago.", domain::DiagnosticCategory::BusinessRule);
        }
        if (inv.otrosCargos.size() > 15) {
            diags.addError("RULE_OTROS_CARGOS_MAX_EXCEEDED", "otrosCargos", "No se permiten más de 15 otros cargos.", domain::DiagnosticCategory::BusinessRule);
        }
        for (size_t i = 0; i < inv.otrosCargos.size(); ++i) {
            const auto& cargo = inv.otrosCargos[i];
            std::string cp = "otrosCargos[" + std::to_string(i) + "]";
            if (!CatalogoFiscal::esTipoOtrosCargosValido(cargo.tipoDocumento)) {
                diags.addError("RULE_CARGO_TIPO_INVALID", cp + ".tipoDocumentoOC", "Tipo de cargo no válido en el catálogo.", domain::DiagnosticCategory::Catalog);
            }
            if (!RuleValidationHelpers::textoEnRango(cargo.detalle, 1, 160)) {
                diags.addError("RULE_CARGO_DETALLE_LENGTH", cp + ".detalle", "El detalle del cargo es obligatorio y debe tener como máximo 160 caracteres.", domain::DiagnosticCategory::BusinessRule);
            }
            if (cargo.montoCargo.isNegative() || cargo.montoCargo.isZero()) {
                diags.addError("RULE_CARGO_MONTO_NON_POSITIVE", cp + ".montoCargo", "El monto del cargo debe ser mayor a cero.", domain::DiagnosticCategory::BusinessRule);
            }
            if (inv.isFEE && !cargo.nombreTercero.empty()) {
                diags.addError("RULE_FEE_CARGO_NOMBRE_TERCERO_PROHIBIDO", cp + ".nombreTercero", "El campo NombreTercero en OtrosCargos está prohibido en Factura Electrónica de Exportación.", domain::DiagnosticCategory::BusinessRule);
            }
            if (cargo.tipoDocumento == "99" && !RuleValidationHelpers::textoEnRango(cargo.tipoDocumentoOTROS, 5, 100)) {
                diags.addError("RULE_CARGO_TIPO_OTROS_LENGTH", cp + ".tipoDocumentoOTROS", "Con el tipo de cargo 99 se debe describir el tipo de documento (5 a 100 caracteres).", domain::DiagnosticCategory::BusinessRule);
            }
            if (cargo.porcentajeScaled.has_value() && *cargo.porcentajeScaled < 0) {
                diags.addError("RULE_CARGO_PORCENTAJE_NEGATIVO", cp + ".porcentajeOC", "El porcentaje del cargo no puede ser negativo.", domain::DiagnosticCategory::BusinessRule);
            }
            // Identificación del tercero: el tipo es obligatorio junto al número y el número debe cumplir el formato (Nota 4).
            if (!cargo.numeroIdentidadTercero.empty()) {
                if (!cargo.tipoIdentidadTercero.has_value()) {
                    diags.addError("RULE_CARGO_TERCERO_TIPO_REQUIRED", cp + ".identificacionTercero.tipo", "El tipo de identificación del tercero es obligatorio cuando se indica su número.", domain::DiagnosticCategory::BusinessRule);
                } else if (*cargo.tipoIdentidadTercero == domain::TipoIdentificacion::ExtranjeroIdentificacion) {
                    diags.addError("RULE_CARGO_TERCERO_NO_CONTRIBUYENTE", cp + ".identificacionTercero.tipo", "El tercero debe estar inscrito como contribuyente; no se permite el tipo 06 (No Contribuyente).", domain::DiagnosticCategory::BusinessRule);
                } else {
                    RuleValidationHelpers::validarIdentificacion(cp + ".identificacionTercero",
                        domain::Identificacion{ *cargo.tipoIdentidadTercero, cargo.numeroIdentidadTercero }, diags);
                }
            }
            if (cargo.tipoDocumento == "04") { // Cobro a favor de un tercero
                // En FEE el NombreTercero no aplica; solo se exige la identificación.
                const bool requiereNombre = !inv.isFEE;
                if (cargo.numeroIdentidadTercero.empty() || (requiereNombre && cargo.nombreTercero.empty())) {
                    diags.addError("RULE_CARGO_TERCERO_REQUIRED", cp,
                        requiereNombre ? "Para cobro de un tercero, el número de identidad y nombre son obligatorios."
                                       : "Para cobro de un tercero, el número de identidad es obligatorio.",
                        domain::DiagnosticCategory::BusinessRule);
                } else if (requiereNombre && !RuleValidationHelpers::textoEnRango(cargo.nombreTercero, 5, 100)) {
                    diags.addError("RULE_CARGO_NOMBRE_TERCERO_LENGTH", cp + ".nombreTercero", "El nombre del tercero debe tener entre 5 y 100 caracteres.", domain::DiagnosticCategory::BusinessRule);
                }
            }
        }
    }
};

} // namespace cec::rules

