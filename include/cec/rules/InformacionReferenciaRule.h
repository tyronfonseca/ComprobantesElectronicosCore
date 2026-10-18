#pragma once
#include <algorithm>
#include "cec/rules/IValidationRule.h"
#include "cec/rules/RuleValidationHelpers.h"
#include "cec/rules/CatalogoFiscal.h"

namespace cec::rules {

/**
 * @brief Regla 10: Información de Referencia (obligatoria en notas y REP, catálogo v4.4).
 */
class InformacionReferenciaRule : public IValidationRule {
public:
    void validate(const domain::ComprobanteCanonico& inv, domain::DiagnosticList& diags) const override {
        bool isNota = inv.isNC || inv.isND;

        if (isNota && inv.referencias.empty()) {
            diags.addError("RULE_REFERENCIA_REQUIRED", "informacionReferencia", "La Nota de Crédito o Débito debe tener al menos una información de referencia.", domain::DiagnosticCategory::BusinessRule);
        }
        if (inv.isFEC && inv.referencias.empty()) {
            diags.addError("RULE_FEC_REFERENCIA_REQUIRED", "informacionReferencia", "Información de Referencia es obligatoria en Factura Electrónica de Compra.", domain::DiagnosticCategory::BusinessRule);
        }
        if (inv.isREP && inv.referencias.empty()) {
            diags.addError("RULE_REP_REFERENCIA_REQUIRED", "informacionReferencia", "El Recibo Electrónico de Pago requiere al menos una referencia al comprobante original.", domain::DiagnosticCategory::BusinessRule);
        }
        if (inv.isFE && inv.referencias.empty()) {
            bool hasExo11 = false;
            for (const auto& l : inv.lineas) {
                if (l.exoneracion.has_value() && l.exoneracion->tipoDocumento == "11") {
                    hasExo11 = true;
                    break;
                }
            }
            if (hasExo11) {
                diags.addError("RULE_FE_EXONERACION_11_REFERENCIA_REQUIRED", "informacionReferencia", "Información de Referencia es obligatoria en Factura Electrónica cuando se utiliza el tipo de exoneración 11 (Exenciones DGH Autorización Local Concreta).", domain::DiagnosticCategory::BusinessRule);
            }
        }
        // FEC con emisor Extranjero No Domiciliado (05): debe complementarse con TipoDocIR 16.
        if (inv.isFEC && inv.emisor.identificacion.tipo == domain::TipoIdentificacion::ExtranjeroNoDomiciliado) {
            const bool tieneRef16 = std::any_of(inv.referencias.begin(), inv.referencias.end(),
                [](const domain::InformacionReferencia& r) { return r.tipoDoc == "16"; });
            if (!tieneRef16) {
                diags.addError("RULE_FEC_EXTRANJERO_REF_16_REQUIRED", "informacionReferencia", "En Factura Electrónica de Compra con emisor Extranjero No Domiciliado (05) se debe incluir una referencia con tipo de documento 16 (Comprobante de Proveedor No Domiciliado).", domain::DiagnosticCategory::BusinessRule);
            }
        }
        // FE con receptor Extranjero No Domiciliado (05): se exige el documento de respaldo en Información de Referencia.
        if (inv.isFE && inv.referencias.empty() && inv.receptor.has_value() &&
            inv.receptor->identificacion.has_value() &&
            inv.receptor->identificacion->tipo == domain::TipoIdentificacion::ExtranjeroNoDomiciliado) {
            diags.addError("RULE_FE_RECEPTOR_EXTRANJERO_REFERENCIA_REQUIRED", "informacionReferencia", "En Factura Electrónica con receptor Extranjero No Domiciliado (05) se debe indicar en Información de Referencia el documento de respaldo.", domain::DiagnosticCategory::BusinessRule);
        }
        if (inv.referencias.size() > 10) {
            diags.addError("RULE_REFERENCIA_MAX_EXCEEDED", "informacionReferencia", "No se permiten más de 10 referencias en el comprobante.", domain::DiagnosticCategory::BusinessRule);
        }
        for (size_t i = 0; i < inv.referencias.size(); ++i) {
            const auto& ref = inv.referencias[i];
            std::string rp = "informacionReferencia[" + std::to_string(i) + "]";
            if (inv.isREP) {
                if (!CatalogoFiscal::esTipoDocumentoReferenciaRepValido(ref.tipoDoc)) {
                    diags.addError("RULE_REP_REF_TIPO_INVALID", rp + ".tipoDocIR", "Tipo de documento de referencia no permitido en REP (solo 01, 04, 07, 08, 10, 20).", domain::DiagnosticCategory::Catalog);
                }
            } else {
                if (!CatalogoFiscal::esTipoDocumentoReferenciaValido(ref.tipoDoc)) {
                    diags.addError("RULE_REF_TIPO_DOC_INVALID", rp + ".tipoDocIR", "Tipo de documento de referencia no válido en el catálogo.", domain::DiagnosticCategory::Catalog);
                }
            }
            // El XSD v4.4 publicado solo enumera los tipos 01 a 18 y 99; el Anexo (nota 10) incluye también 19 y 20.
            if (ref.tipoDoc == "19" || ref.tipoDoc == "20") {
                diags.addWarning("RULE_REF_TIPO_DOC_FUERA_DEL_XSD", rp + ".tipoDocIR",
                    "El tipo de documento de referencia " + ref.tipoDoc + " está en el Anexo pero no en el XSD v4.4 publicado; Hacienda podría rechazar el XML por esquema.");
            }
            if (ref.tipoDoc == "16" && !inv.isFEC) {
                diags.addError("RULE_REF_TIPO_16_SOLO_FEC", rp + ".tipoDocIR", "El tipo de documento de referencia 16 (Comprobante de Proveedor No Domiciliado) es exclusivo de Factura de Compra.", domain::DiagnosticCategory::BusinessRule);
            }
            // Con TipoDocIR 13 (Facturación mes vencido) Numero, Codigo y Razon son opcionales.
            const bool esMesVencido = ref.tipoDoc == "13";

            if ((!esMesVencido || !ref.codigo.empty()) && !CatalogoFiscal::esCodigoReferenciaValido(ref.codigo)) {
                diags.addError("RULE_REF_CODIGO_INVALID", rp + ".codigo", "Código de referencia no válido en el catálogo.", domain::DiagnosticCategory::Catalog);
            }
            // El XSD v4.4 publicado solo enumera los códigos 01 a 12 y 99; el Anexo (nota 9) incluye también 13 a 17.
            if (ref.codigo >= "13" && ref.codigo <= "17" && ref.codigo.size() == 2) {
                diags.addWarning("RULE_REF_CODIGO_FUERA_DEL_XSD", rp + ".codigo",
                    "El código de referencia " + ref.codigo + " está en el Anexo pero no en el XSD v4.4 publicado; Hacienda podría rechazar el XML por esquema.");
            }
            if (ref.tipoDoc == "99" && !RuleValidationHelpers::textoEnRango(ref.tipoDocRefOTRO, 5, 100)) {
                diags.addError("RULE_REF_TIPO_DOC_OTRO_LENGTH", rp + ".tipoDocRefOTRO", "Con el tipo de documento de referencia 99 se debe describir el documento (5 a 100 caracteres).", domain::DiagnosticCategory::BusinessRule);
            }
            if (ref.codigo == "99" && !RuleValidationHelpers::textoEnRango(ref.codigoReferenciaOTRO, 5, 100)) {
                diags.addError("RULE_REF_CODIGO_OTRO_LENGTH", rp + ".codigoReferenciaOTRO", "Con el código de referencia 99 se debe describir el código (5 a 100 caracteres).", domain::DiagnosticCategory::BusinessRule);
            }
            if (ref.codigo == "12" && inv.tipoDocumento != domain::TipoDocumento::NotaCreditoElectronica) {
                diags.addError("RULE_REF_CODIGO_12_ONLY_NC", rp + ".codigo", "El código de referencia 12 es exclusivo de Notas de Crédito.", domain::DiagnosticCategory::BusinessRule);
            }
            // Nota 9, nota al pie 35: el código 17 es de uso exclusivo del Recibo Electrónico de Pago.
            if (ref.codigo == "17" && !inv.isREP) {
                diags.addError("RULE_REF_CODIGO_17_ONLY_REP", rp + ".codigo", "El código de referencia 17 (Pago a comprobante electrónico) es de uso exclusivo del Recibo Electrónico de Pago.", domain::DiagnosticCategory::BusinessRule);
            }
            // Nota 10, nota al pie 36: el tipo 09 (Devolución mercadería) solo se usa en NC y ND.
            if (ref.tipoDoc == "09" && !isNota) {
                diags.addError("RULE_REF_TIPO_09_SOLO_NOTAS", rp + ".tipoDocIR", "El tipo de documento de referencia 09 (Devolución mercadería) solo se permite en Notas de Crédito y Débito.", domain::DiagnosticCategory::BusinessRule);
            }
            if ((!esMesVencido && ref.numero.empty()) || ref.numero.length() > 50) {
                diags.addError("RULE_REF_NUMERO_LENGTH", rp + ".numero", "El número de documento de referencia es obligatorio y no puede superar 50 caracteres.", domain::DiagnosticCategory::BusinessRule);
            } else if (CatalogoFiscal::tipoDocReferenciaRequiereClave(ref.tipoDoc) &&
                       !RuleValidationHelpers::esClaveEstructuraValida(ref.numero)) {
                diags.addError("RULE_REF_NUMERO_CLAVE_INVALIDA", rp + ".numero", "Para este tipo de documento de referencia el número debe ser una clave numérica válida de 50 posiciones (Nota 10).", domain::DiagnosticCategory::BusinessRule);
            }
            if (ref.fechaEmision.empty()) {
                diags.addError("RULE_REF_FECHA_EMPTY", rp + ".fechaEmisionIR", "La fecha de emisión de referencia no puede estar vacía.", domain::DiagnosticCategory::BusinessRule);
            } else if (!RuleValidationHelpers::esFechaRFC3339(ref.fechaEmision)) {
                diags.addError("RULE_REF_FECHA_FORMAT", rp + ".fechaEmisionIR", "La fecha de emisión de referencia debe tener formato RFC3339 (YYYY-MM-DDThh:mm:ss±hh:mm).", domain::DiagnosticCategory::BusinessRule);
            } else if (RuleValidationHelpers::esFechaRFC3339(inv.fechaEmision)) {
                // No puede superar 10 años de antigüedad respecto a la fecha de emisión del comprobante.
                const std::string limite = std::to_string(std::stoi(inv.fechaEmision.substr(0, 4)) - 10) + inv.fechaEmision.substr(4, 6);
                if (ref.fechaEmision.compare(0, 10, limite) < 0) {
                    diags.addError("RULE_REF_FECHA_ANTIGUEDAD", rp + ".fechaEmisionIR", "La fecha del documento de referencia no puede tener más de 10 años de antigüedad.", domain::DiagnosticCategory::BusinessRule);
                }
            }
            if ((!esMesVencido || !ref.razon.empty()) && !RuleValidationHelpers::textoEnRango(ref.razon, 3, 180)) {
                diags.addError("RULE_REF_RAZON_LENGTH", rp + ".razon", "La razón de la referencia es obligatoria y debe tener entre 3 y 180 caracteres.", domain::DiagnosticCategory::BusinessRule);
            }
        }
    }
};

} // namespace cec::rules

