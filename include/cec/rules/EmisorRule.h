#pragma once
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <string>
#include "cec/rules/IValidationRule.h"
#include "cec/rules/RuleValidationHelpers.h"
#include "cec/common/CabysClasificacion.h"

namespace cec::rules {

/**
 * @brief Regla 4: Datos del Emisor (nombre, identificación, ubicación, teléfono, nombre comercial).
 */
class EmisorRule : public IValidationRule {
public:
    void validate(const domain::ComprobanteCanonico& inv, domain::DiagnosticList& diags) const override {
        using domain::DiagnosticCategory;
        using domain::TipoDocumento;
        using domain::TipoIdentificacion;

        constexpr std::size_t kNombreMin = 5, kNombreMax = 100;
        constexpr std::size_t kNombreComercialMin = 3, kNombreComercialMax = 80;
        constexpr std::size_t kRegistroFiscalMax = 12;
        constexpr std::size_t kUbicacionOtrasSenasMax = 250;
        constexpr std::size_t kEmailMin = 1, kEmailMax = 4;

        const auto& emisor = inv.emisor;

        // ---- Proveedor de Sistemas ----
        // Obligatorio en todos los comprobantes; es una identificación (cédula, DIMEX, NITE o extranjero).
        if (inv.proveedorSistemas.empty()) {
            diags.addError("RULE_PROVEEDOR_SISTEMAS_REQUIRED", "proveedorSistemas",
                "El campo ProveedorSistemas es obligatorio (identificación del proveedor de sistemas o del propio emisor si es desarrollo propio).",
                DiagnosticCategory::BusinessRule);
        } else if (inv.proveedorSistemas.length() > 20 ||
                   !std::all_of(inv.proveedorSistemas.begin(), inv.proveedorSistemas.end(),
                                [](unsigned char ch) { return std::isalnum(ch) != 0; })) {
            diags.addError("RULE_PROVEEDOR_SISTEMAS_FORMAT", "proveedorSistemas",
                "El campo ProveedorSistemas debe ser una identificación de máximo 20 caracteres alfanuméricos, sin guiones.",
                DiagnosticCategory::BusinessRule);
        }

        // ---- Nombre ----
        if (!RuleValidationHelpers::textoEnRango(emisor.nombre, kNombreMin, kNombreMax)) {
            diags.addError("RULE_EMISOR_NOMBRE_LENGTH", "emisor.nombre",
                "El nombre o razón social del emisor debe tener entre 5 y 100 caracteres.",
                DiagnosticCategory::BusinessRule);
        }

        // ---- Identificación ----
        const auto tipoId = emisor.identificacion.tipo;

        if (tipoId == TipoIdentificacion::ExtranjeroNoDomiciliado && !inv.isFEC) {
            diags.addError("RULE_EMISOR_EXTRANJERO_SOLO_FEC", "emisor.identificacion.tipo",
                "El tipo de identificación Extranjero No Domiciliado (05) en el emisor es de uso exclusivo para Factura Electrónica de Compra.",
                DiagnosticCategory::BusinessRule);
        }

        // 06 No Contribuyente
        if (tipoId == TipoIdentificacion::ExtranjeroIdentificacion &&
            (!inv.isFEC || inv.condicionVenta != domain::CondicionVenta::VentaBienesUsadosNoContribuyentes)) {
            diags.addError("RULE_EMISOR_NO_CONTRIBUYENTE_CONDICION_13", "emisor.identificacion.tipo",
                "El código de No Contribuyente (06) en el emisor solo se permite en Factura de Compra con condición de venta 13.",
                DiagnosticCategory::BusinessRule);
        }

        RuleValidationHelpers::validarIdentificacion("emisor.identificacion", emisor.identificacion, diags);

        // ---- Nombre comercial ----
        if (!inv.isREP && !emisor.nombreComercial.empty() &&
            !RuleValidationHelpers::textoEnRango(emisor.nombreComercial, kNombreComercialMin, kNombreComercialMax)) {
            diags.addError("RULE_EMISOR_NOMBRE_COMERCIAL_LENGTH", "emisor.nombreComercial",
                "El nombre comercial del emisor debe tener entre 3 y 80 caracteres.",
                DiagnosticCategory::BusinessRule);
        }

        // ---- Registro fiscal, ubicación y teléfono ----
        if (inv.isREP) {
            if (!emisor.registroFiscal8707.empty()) {
                diags.addError("RULE_REP_REGISTRO_FISCAL_PROHIBIDO", "emisor.registroFiscal8707",
                    "El Recibo Electrónico de Pago no admite el campo Registrofiscal8707 en el emisor.",
                    DiagnosticCategory::BusinessRule);
            }
            if (emisor.ubicacion.has_value()) {
                diags.addError("RULE_REP_EMISOR_UBICACION_PROHIBIDA", "emisor.ubicacion",
                    "La ubicación del emisor está prohibida en el Recibo Electrónico de Pago.",
                    DiagnosticCategory::BusinessRule);
            }
            if (emisor.telefono.has_value()) {
                diags.addError("RULE_REP_EMISOR_TELEFONO_PROHIBIDO", "emisor.telefono",
                    "El teléfono del emisor está prohibido en el Recibo Electrónico de Pago.",
                    DiagnosticCategory::BusinessRule);
            }
        } else {
            if (!emisor.registroFiscal8707.empty()) {
                if (emisor.registroFiscal8707.length() > kRegistroFiscalMax) {
                    diags.addError("RULE_EMISOR_REGISTRO_FISCAL_LENGTH", "emisor.registroFiscal8707",
                        "El número de Registro Fiscal de bebidas alcohólicas (Ley 8707) no puede tener más de 12 caracteres.",
                        DiagnosticCategory::BusinessRule);
                }
            } else {
                bool tieneBebidasAlcoholicas = false;
                for (const auto& l : inv.lineas) {
                    if (common::CabysClasificacion::EsBebidaAlcoholica(l.cabys)) {
                        tieneBebidasAlcoholicas = true;
                        break;
                    }
                    for (const auto& imp : l.impuestos) {
                        if (imp.codigo == domain::CodigoImpuesto::Alcoholicas) {
                            tieneBebidasAlcoholicas = true;
                            break;
                        }
                    }
                    if (tieneBebidasAlcoholicas) break;
                }
                if (tieneBebidasAlcoholicas) {
                    // La normativa indica advertencia, no rechazo.
                    diags.addWarning("RULE_EMISOR_REGISTRO_FISCAL_8707_REQUERIDO", "emisor.registroFiscal8707",
                        "Se requiere el número de Registro Fiscal de bebidas alcohólicas (Ley 8707) en el emisor al facturar bienes con CAByS que empiezan en: 241, 242, 2431.");
                }
            }

            if (emisor.ubicacion.has_value()) {
                RuleValidationHelpers::validarUbicacion("emisor.ubicacion", *emisor.ubicacion, diags, kUbicacionOtrasSenasMax);
            } else if (inv.isFE || inv.isFEE || inv.isTE ||
                       // En FEC es obligatoria salvo emisor Extranjero No Domiciliado (05) o No Contribuyente (06).
                       (inv.isFEC && tipoId != TipoIdentificacion::ExtranjeroNoDomiciliado &&
                                     tipoId != TipoIdentificacion::ExtranjeroIdentificacion)) {
                diags.addError("RULE_UBICACION_REQUERIDO", "emisor.ubicacion",
                    "La ubicación es requerida para este tipo de comprobante.",
                    DiagnosticCategory::BusinessRule);
            }

            if (emisor.telefono.has_value()) {
                RuleValidationHelpers::validarTelefono("emisor.telefono", *emisor.telefono, diags);
            }
        }

        // ---- Correos electrónicos ----
        const std::size_t totalCorreos = emisor.correoElectronico.size();

        if (totalCorreos < kEmailMin && (inv.isFE || inv.isFEE || inv.isTE || inv.isREP)) {
            diags.addError("RULE_EMISOR_EMAIL_MIN", "emisor.correoElectronico",
                "El correo electrónico es requerido para este tipo de comprobante.",
                DiagnosticCategory::BusinessRule);
        }
        if (totalCorreos > kEmailMax) {
            diags.addError("RULE_EMISOR_EMAIL_MAX", "emisor.correoElectronico",
                "El emisor no puede tener más de 4 correos electrónicos.",
                DiagnosticCategory::BusinessRule);
        }
        for (std::size_t i = 0; i < totalCorreos; ++i) {
            RuleValidationHelpers::validarEmail("emisor.correoElectronico[" + std::to_string(i) + "]",
                                                emisor.correoElectronico[i], diags);
        }

        // ---- Otras Senas Extranjero ----
        if(!inv.emisor.otrasSenasExtranjero.value_or("").empty()) {
            if(!inv.isFEC || emisor.identificacion.tipo != domain::TipoIdentificacion::ExtranjeroNoDomiciliado){
                diags.addError("RULE_EMISOR_OTRAS_SENAS_EXTRANJERO_PROHIBIDO", "emisor.otrasSenasExtranjero",
                    "El campo OtrasSenasExtranjero del emisor es de uso exclusivo para Factura Electrónica de Compra cuando el tipo de identificación del emisor es Extranjero No Domiciliado (05).");
            } else if (!RuleValidationHelpers::textoEnRango(*emisor.otrasSenasExtranjero, 5, 300)) {
                diags.addError("RULE_EMISOR_OTRAS_SENAS_EXTRANJERO_LENGHT", "emisor.otrasSenasExtranjero",
                    "El campo OtrasSenasExtranjero del emisor debe tener entre 5 y 300 caracteres.");
            }
        }
        
    }
};

} // namespace cec::rules