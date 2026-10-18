#pragma once
#include <algorithm>
#include <exception>
#include "cec/rules/IValidationRule.h"
#include "cec/rules/RuleValidationHelpers.h"
#include "cec/rules/CatalogoFiscal.h"
#include "cec/common/Codigos.h"
#include "cec/common/CabysClasificacion.h"
namespace cec::rules {

/**
 * @brief Regla 9: Líneas de Detalle (CABYS, unidad, cantidades, bienes especiales, exoneraciones, etc.).
 */
class LineasDetalleRule : public IValidationRule {
private:
    static bool isOtroCargoPermitidoSinDetalle(const std::string& tipoDocumento) {
        using TipoOC = cec::common::TipoDocOtrosCargos;
        using cec::common::get_tipo_doc_otros_cargos_code;
        return tipoDocumento == get_tipo_doc_otros_cargos_code(TipoOC::CobroTercero) || tipoDocumento == "4"
            || tipoDocumento == get_tipo_doc_otros_cargos_code(TipoOC::DepositosGarantia) || tipoDocumento == "8"
            || tipoDocumento == get_tipo_doc_otros_cargos_code(TipoOC::MultasPenalizaciones) || tipoDocumento == "9"
            || tipoDocumento == get_tipo_doc_otros_cargos_code(TipoOC::InteresesMoratorios);
    }

    /**
     * @brief Validaciones por impuesto (Nota 8) aplicables a líneas y a componentes de surtido.
     * @param lineaConSurtido true para la línea principal de un surtido: los datos de impuestos
     *        específicos 04, 05 y 06 van en los componentes, no en la línea.
     */
    static void validarImpuestos(const std::string& path,
                                 const std::vector<domain::ImpuestoLinea>& impuestos,
                                 const domain::ComprobanteCanonico& inv,
                                 bool lineaConSurtido,
                                 domain::DiagnosticList& diags) {
        using domain::CodigoImpuesto;
        const bool admiteEspecificos = !(inv.isFEE || inv.isFEC || inv.isREP);
        const bool admiteExportacion = inv.isFEE || inv.isNC || inv.isND;

        for (size_t k = 0; k < impuestos.size(); ++k) {
            const auto& imp = impuestos[k];
            const std::string ip = path + "[" + std::to_string(k) + "]";

            if (!CatalogoFiscal::esCodigoImpuestoValido(imp.codigo)) {
                diags.addError("RULE_IMPUESTO_CODIGO_INVALID", ip + ".codigo", "El código de impuesto no existe en el catálogo de Hacienda (Nota 8).", domain::DiagnosticCategory::Catalog);
                continue;
            }
            validarTarifa(ip, imp, inv, diags);

            if (imp.codigo == CodigoImpuesto::Otros &&
                !RuleValidationHelpers::textoEnRango(imp.codigoImpuestoOTRO, 5, 100)) {
                diags.addError("RULE_IMPUESTO_OTRO_LENGTH", ip + ".codigoImpuestoOTRO", "Con el código de impuesto 99 (Otros) se debe describir el impuesto (5 a 100 caracteres).", domain::DiagnosticCategory::BusinessRule);
            }

            if (imp.codigo == CodigoImpuesto::IVABienesUsados) {
                if (!imp.factorCalculoIVAScaled.has_value() || *imp.factorCalculoIVAScaled <= 0) {
                    diags.addError("RULE_IMPUESTO_FACTOR_IVA_REQUIRED", ip + ".factorCalculoIVA", "El factor para cálculo del IVA es obligatorio con el código de impuesto 08 (IVA Régimen de Bienes Usados).", domain::DiagnosticCategory::BusinessRule);
                }
            } else if (imp.factorCalculoIVAScaled.has_value()) {
                diags.addError("RULE_IMPUESTO_FACTOR_IVA_NO_APLICA", ip + ".factorCalculoIVA", "El factor para cálculo del IVA solo aplica con el código de impuesto 08.", domain::DiagnosticCategory::BusinessRule);
            }

            const bool esEspecifico = imp.codigo == CodigoImpuesto::Combustibles || imp.codigo == CodigoImpuesto::Alcoholicas ||
                                      imp.codigo == CodigoImpuesto::BebidasEnvasadasJabones || imp.codigo == CodigoImpuesto::Tabaco;
            // En la línea de un surtido, 04/05/06 se calculan en los componentes.
            const bool datosEnComponentes = lineaConSurtido && esEspecifico && imp.codigo != CodigoImpuesto::Combustibles;

            if (imp.datosEspecificos.has_value() && (!admiteEspecificos || !esEspecifico || datosEnComponentes)) {
                diags.addError("RULE_IMPUESTO_DATOS_ESPECIFICOS_NO_APLICA", ip + ".datosImpuestoEspecifico", "Los datos de impuesto específico solo aplican a los códigos 03, 04, 05 y 06, no en FEE/FEC/REP, ni en la línea principal de un surtido para 04, 05 y 06.", domain::DiagnosticCategory::BusinessRule);
            } else if (esEspecifico && admiteEspecificos && !datosEnComponentes) {
                if (!imp.datosEspecificos.has_value()) {
                    diags.addError("RULE_IMPUESTO_DATOS_ESPECIFICOS_REQUIRED", ip + ".datosImpuestoEspecifico", "Los datos de impuesto específico son obligatorios con los códigos de impuesto 03, 04, 05 y 06.", domain::DiagnosticCategory::BusinessRule);
                } else {
                    const auto& d = *imp.datosEspecificos;
                    if (d.cantidadUnidadMedidaScaled <= 0) {
                        diags.addError("RULE_IMPUESTO_CANTIDAD_UNIDAD_REQUIRED", ip + ".datosImpuestoEspecifico.cantidadUnidadMedida", "La cantidad de la unidad de medida es obligatoria y debe ser mayor a cero.", domain::DiagnosticCategory::BusinessRule);
                    }
                    if (!d.impuestoUnidad.isPositive()) {
                        diags.addError("RULE_IMPUESTO_UNIDAD_REQUIRED", ip + ".datosImpuestoEspecifico.impuestoUnidad", "El impuesto por unidad es obligatorio y debe ser mayor a cero.", domain::DiagnosticCategory::BusinessRule);
                    }
                    if (imp.codigo == CodigoImpuesto::Alcoholicas && (!d.porcentajeScaled.has_value() || *d.porcentajeScaled <= 0)) {
                        diags.addError("RULE_IMPUESTO_PORCENTAJE_REQUIRED", ip + ".datosImpuestoEspecifico.porcentaje", "El porcentaje de alcohol es obligatorio con el código de impuesto 04.", domain::DiagnosticCategory::BusinessRule);
                    }
                    if (imp.codigo == CodigoImpuesto::BebidasEnvasadasJabones && (!d.volumenUnidadConsumoScaled.has_value() || *d.volumenUnidadConsumoScaled <= 0)) {
                        diags.addError("RULE_IMPUESTO_VOLUMEN_REQUIRED", ip + ".datosImpuestoEspecifico.volumenUnidadConsumo", "El volumen por unidad de consumo es obligatorio con el código de impuesto 05.", domain::DiagnosticCategory::BusinessRule);
                    }
                }
            }

            if (imp.montoExportacion.isNegative()) {
                diags.addError("RULE_IMPUESTO_MONTO_EXPORTACION_NEGATIVO", ip + ".montoExportacion", "El monto de impuesto de exportación no puede ser negativo.", domain::DiagnosticCategory::BusinessRule);
            } else if (imp.montoExportacion.isPositive() && !admiteExportacion) {
                diags.addError("RULE_IMPUESTO_MONTO_EXPORTACION_NO_APLICA", ip + ".montoExportacion", "El monto de impuesto de exportación solo se permite en FEE, NC y ND.", domain::DiagnosticCategory::BusinessRule);
            }
        }
    }
    /**
     * @brief Tarifa de cada impuesto (Nota 8 y 8.1).
     * - IVA (01) e IVA de cálculo especial (07): CodigoTarifaIVA obligatorio y del catálogo; si se informa
     *   la tarifa, debe coincidir con la del código.
     * - FEE: las tarifas 01 (0 %) y 11 (0 % sin derecho a crédito) son de bienes y servicios no sujetos, y la
     *   FEE no tiene totales de no sujeto; Hacienda la rechaza (error -101) y pide la 10 (Tarifa Exenta).
     *   El 07 exige BaseImponible, que la FEE no usa.
     * - Selectivo de consumo (02) y otros (99): la tarifa es obligatoria. Cemento (12): 5 %.
     */
    static void validarTarifa(const std::string& ip, const domain::ImpuestoLinea& imp,
                              const domain::ComprobanteCanonico& inv, domain::DiagnosticList& diags) {
        using domain::CodigoImpuesto;
        using domain::CodigoTarifaIVA;

        if (imp.codigo == CodigoImpuesto::IVA || imp.codigo == CodigoImpuesto::IVACalculoEspecial) {
            if (static_cast<int>(imp.codigoTarifa) == 0) {
                diags.addError("RULE_TARIFA_IVA_REQUIRED", ip + ".codigoTarifaIVA", "El código de tarifa del IVA es obligatorio con los códigos de impuesto 01 y 07 (Nota 8.1).", domain::DiagnosticCategory::BusinessRule);
            } else if (!CatalogoFiscal::esTarifaIVAValida(imp.codigoTarifa)) {
                diags.addError("RULE_TARIFA_IVA_INVALID", ip + ".codigoTarifaIVA", "El código de tarifa del IVA no existe en el catálogo de Hacienda (Nota 8.1).", domain::DiagnosticCategory::Catalog);
            } else {
                if (imp.tarifaPorcentajeScaled != domain::getTarifaPorcentajeScaled(imp.codigoTarifa)) {
                    diags.addError("RULE_TARIFA_IVA_NO_COINCIDE", ip + ".tarifa", "La tarifa del IVA debe coincidir con la de su código de tarifa (Nota 8.1).", domain::DiagnosticCategory::BusinessRule);
                }
                if (inv.isFEE && (imp.codigoTarifa == CodigoTarifaIVA::Tarifa0 || imp.codigoTarifa == CodigoTarifaIVA::Tarifa0SinCredito)) {
                    diags.addError("RULE_FEE_TARIFA_NO_SUJETO", ip + ".codigoTarifaIVA", "La Factura Electrónica de Exportación no admite las tarifas 01 ni 11 (no sujeto); si la línea es exenta de impuestos se debe usar la tarifa 10 (Tarifa Exenta).", domain::DiagnosticCategory::BusinessRule);
                }
            }
            if (inv.isFEE && imp.codigo == CodigoImpuesto::IVACalculoEspecial) {
                diags.addError("RULE_FEE_IMPUESTO_07_NO_APLICA", ip + ".codigo", "El impuesto 07 (IVA de cálculo especial) requiere la base imponible, que no se usa en la Factura Electrónica de Exportación.", domain::DiagnosticCategory::BusinessRule);
            }
        }
        if ((imp.codigo == CodigoImpuesto::SelectivoConsumo || imp.codigo == CodigoImpuesto::Otros) && imp.tarifaPorcentajeScaled <= 0) {
            diags.addError("RULE_IMPUESTO_TARIFA_REQUIRED", ip + ".tarifa", "Con los códigos de impuesto 02 y 99 se debe indicar la tarifa (porcentaje mayor a cero).", domain::DiagnosticCategory::BusinessRule);
        }
        if (imp.codigo == CodigoImpuesto::Cemento && imp.tarifaPorcentajeScaled != 5 * domain::Money::SCALE) {
            diags.addError("RULE_IMPUESTO_CEMENTO_TARIFA", ip + ".tarifa", "La tarifa del impuesto al cemento (12) es del 5 %.", domain::DiagnosticCategory::BusinessRule);
        }
    }

    /// @brief Nota 8.1, notas al pie 30-32: las tarifas transitorias 05, 06 y 07 solo se usan en NC y ND.
    static void validarTarifasTransitorias(const std::string& path,
                                           const std::vector<domain::ImpuestoLinea>& impuestos,
                                           const domain::ComprobanteCanonico& inv,
                                           domain::DiagnosticList& diags) {
        if (inv.isNC || inv.isND) return;
        for (const auto& imp : impuestos) {
            if (domain::esIVA(imp.codigo) &&
                (imp.codigoTarifa == domain::CodigoTarifaIVA::TarifaTransitoria0 ||
                 imp.codigoTarifa == domain::CodigoTarifaIVA::TarifaTransitoria4 ||
                 imp.codigoTarifa == domain::CodigoTarifaIVA::TarifaTransitoria8)) {
                diags.addError("RULE_TARIFA_TRANSITORIA_SOLO_NOTAS", path, "Las tarifas transitorias de IVA (05, 06 y 07) solo se permiten en Notas de Crédito y Débito.", domain::DiagnosticCategory::BusinessRule);
                return;
            }
        }
    }
    /// @brief CodigoComercial {0,5} (Nota 12) de una línea o de un componente de surtido.
    static void validarCodigosComerciales(const std::string& p, const std::vector<domain::CodigoComercial>& codigos, domain::DiagnosticList& diags) {
        if (codigos.size() > 5) {
            diags.addError("RULE_CODIGO_COMERCIAL_MAX_EXCEEDED", p + ".codigosComerciales", "No se permiten más de 5 códigos comerciales por línea o componente.", domain::DiagnosticCategory::BusinessRule);
        }
        for (size_t k = 0; k < codigos.size(); ++k) {
            const auto& cc = codigos[k];
            const std::string cp = p + ".codigosComerciales[" + std::to_string(k) + "]";
            if (!CatalogoFiscal::esTipoCodigoComercialValido(cc.tipo)) {
                diags.addError("RULE_CODIGO_COMERCIAL_TIPO_INVALID", cp + ".tipo", "Tipo de código comercial no válido (Nota 12).", domain::DiagnosticCategory::Catalog);
            }
            if (!RuleValidationHelpers::textoEnRango(cc.codigo, 1, 20)) {
                diags.addError("RULE_CODIGO_COMERCIAL_LENGTH", cp + ".codigo", "El código comercial debe tener entre 1 y 20 caracteres.", domain::DiagnosticCategory::BusinessRule);
            }
        }
    }

    /// @brief Códigos comerciales de la línea; con CABYS de surtido es obligatorio el tipo 03 con al menos 3 caracteres.
    static void validarCodigosComerciales(const std::string& p, const domain::LineaDetalle& l, domain::DiagnosticList& diags) {
        validarCodigosComerciales(p, l.codigosComerciales, diags);
        if (common::CabysClasificacion::EsSurtidoHabilitado(l.cabys)) {
            const bool tieneFabricante = std::any_of(l.codigosComerciales.begin(), l.codigosComerciales.end(),
                [](const domain::CodigoComercial& cc) { return cc.tipo == "03" && RuleValidationHelpers::utf8Length(cc.codigo) >= 3; });
            if (!tieneFabricante) {
                diags.addError("RULE_SURTIDO_CODIGO_FABRICANTE_REQUIRED", p + ".codigosComerciales", "Las líneas con CABYS de surtido deben incluir un código comercial tipo 03 (SKU, GTIN o equivalente del fabricante o importador) de al menos 3 caracteres.", domain::DiagnosticCategory::BusinessRule);
            }
        }
    }

    /// @brief DescuentoSurtido {0,5}: Nota 20, monto <= MontoTotalSurtido, DescuentoSurtidoOtros con código 99.
    static void validarDescuentosSurtido(const std::string& cp, const domain::ComponenteSurtido& comp, domain::DiagnosticList& diags) {
        if (comp.descuentos.size() > 5) {
            diags.addError("RULE_SURTIDO_DESCUENTO_MAX_EXCEEDED", cp + ".descuentos", "No se permiten más de 5 descuentos por componente de surtido.", domain::DiagnosticCategory::BusinessRule);
        }
        domain::Money suma;
        for (size_t d = 0; d < comp.descuentos.size(); ++d) {
            const auto& desc = comp.descuentos[d];
            const std::string dp = cp + ".descuentos[" + std::to_string(d) + "]";
            if (!CatalogoFiscal::esCodigoDescuentoValido(desc.codigo)) {
                diags.addError("RULE_SURTIDO_DESCUENTO_CODIGO_INVALID", dp + ".codigoDescuento", "Código de descuento del componente no válido (Nota 20).", domain::DiagnosticCategory::Catalog);
            }
            if (desc.monto.isNegative()) {
                diags.addError("RULE_SURTIDO_DESCUENTO_MONTO_NEGATIVO", dp + ".montoDescuento", "El monto del descuento del componente no puede ser negativo.", domain::DiagnosticCategory::BusinessRule);
            }
            if (desc.codigo == "99" && !RuleValidationHelpers::textoEnRango(desc.naturaleza, 3, 80)) {
                diags.addError("RULE_SURTIDO_DESCUENTO_OTROS_LENGTH", dp + ".naturalezaDescuento", "Con el código 99 se debe indicar la naturaleza del descuento del componente (3 a 80 caracteres).", domain::DiagnosticCategory::BusinessRule);
            }
            suma += desc.monto;
        }
        // MontoTotalSurtido = CantidadSurtido * PrecioUnitarioSurtido.
        try {
            if (suma > comp.precioUnitario.multiplyScaled(comp.cantidadScaled)) {
                diags.addError("RULE_SURTIDO_DESCUENTO_EXCEDE_TOTAL", cp + ".descuentos", "Los descuentos del componente no pueden superar su monto total.", domain::DiagnosticCategory::BusinessRule);
            }
        } catch (const std::exception&) {
            // El desbordamiento lo reporta el motor de cálculo.
        }
    }
public:
    void validate(const domain::ComprobanteCanonico& inv, domain::DiagnosticList& diags) const override {

        if (inv.lineas.empty()) {
            const bool allowsEmpty = inv.isFE || inv.isTE || inv.isNC || inv.isND;

            if (!allowsEmpty) {
                // 1. Tipo de comprobante que no admite detalle vacío
                diags.addError("RULE_NO_LINES", "detalleServicio",
                    "El Detalle del Servicio es de carácter obligatorio para este tipo de comprobante.",
                    domain::DiagnosticCategory::BusinessRule);
            } else if (inv.otrosCargos.empty()) {
                // 2. Sin líneas y sin Otros Cargos
                diags.addError("RULE_NO_LINES_OTROS_CARGOS_REQUIRED", "detalleServicio",
                    "El Detalle del Servicio es obligatorio si no se incluye el nodo de Otros Cargos.",
                    domain::DiagnosticCategory::BusinessRule);
            } else {
                // 3. TODOS los Otros Cargos deben ser 04, 08, 09 o 10
                for (size_t i = 0; i < inv.otrosCargos.size(); ++i) {
                    if (!isOtroCargoPermitidoSinDetalle(inv.otrosCargos[i].tipoDocumento)) {
                        diags.addError("RULE_NO_LINES_OTROS_CARGOS_INVALIDO",
                            "otrosCargos[" + std::to_string(i) + "].tipoDocumentoOC",
                            "Sin líneas de detalle, Otros Cargos solo admite los tipos 04 (Cobro de un tercero), 08 (Depósitos de garantía), 09 (Multas o penalizaciones) y 10 (Intereses moratorios) de la Nota 16. Elija uno de ellos o agregue una línea de detalle.",
                            domain::DiagnosticCategory::BusinessRule);
                    }
                }
            }
        } else if (inv.lineas.size() > 1000) {
            diags.addError("RULE_MAX_LINEAS_EXCEEDED", "detalleServicio", "No se permiten más de 1000 líneas de detalle.", domain::DiagnosticCategory::BusinessRule);
        }

        // Una NC/ND modifica una FEE cuando la referencia es TipoDocIR 19 (Factura Electrónica de Exportación)
        // o la clave referenciada es de tipo 09 (posiciones 29-30). Ojo: TipoDocIR 09 es "Devolución mercadería".
        bool modifiesFEE = false;
        // En NC/ND el CABYS no es obligatorio en notas financieras (códigos de referencia 09/10)
        // ni cuando el documento referenciado es anterior al 01/12/2020.
        bool cabysOpcional = false;
        if (inv.isNC || inv.isND) {
            bool todasAntesDeCabys = !inv.referencias.empty();
            for (const auto& ref : inv.referencias) {
                if (ref.tipoDoc == "19" || (ref.numero.length() == 50 && ref.numero.substr(29, 2) == "09")) {
                    modifiesFEE = true;
                }
                if (ref.codigo == "09" || ref.codigo == "10") {
                    cabysOpcional = true;
                }
                if (ref.fechaEmision.empty() || ref.fechaEmision.compare(0, 10, "2020-12-01") >= 0) {
                    todasAntesDeCabys = false;
                }
            }
            cabysOpcional = cabysOpcional || todasAntesDeCabys;
        }

        for (size_t i = 0; i < inv.lineas.size(); ++i) {
            const auto& l = inv.lineas[i];
            std::string p = "detalleServicio[" + std::to_string(i) + "]";

            // NumeroLinea: número correlativo de la línea de detalle (de 1 a 1000), en el orden del detalle.
            if (l.numeroLinea != static_cast<int>(i + 1)) {
                diags.addError("RULE_NUMERO_LINEA_CORRELATIVO", p + ".numeroLinea", "El número de línea debe ser correlativo: se esperaba " + std::to_string(i + 1) + " y se recibió " + std::to_string(l.numeroLinea) + ".", domain::DiagnosticCategory::BusinessRule);
            }

            if (!RuleValidationHelpers::textoEnRango(l.descripcion, 3, 200)) {
                diags.addError("RULE_LINEA_DESC_EMPTY", p + ".detalle", "La descripción de la línea debe tener entre 3 y 200 caracteres.", domain::DiagnosticCategory::BusinessRule);
            }

            if (inv.isREP) {
                if (!l.descuentos.empty()) {
                    diags.addError("RULE_REP_DESCUENTO_PROHIBIDO", p + ".descuentos", "Los descuentos están prohibidos en las líneas de Recibo Electrónico de Pago.", domain::DiagnosticCategory::BusinessRule);
                }
                if (!l.numerosVIN.empty() || !l.registroMedicamento.empty() || !l.formaFarmaceutica.empty() || !l.surtido.empty()) {
                    diags.addError("RULE_REP_BIENES_ESPECIALES_PROHIBIDOS", p, "Los bienes especiales están prohibidos en el Recibo Electrónico de Pago.", domain::DiagnosticCategory::BusinessRule);
                }
                if (!l.partidaArancelaria.empty()) {
                    diags.addError("RULE_PARTIDA_ARANCELARIA_NOT_ALLOWED", p + ".partidaArancelaria", "La partida arancelaria solo está permitida en Factura Electrónica de Exportación.", domain::DiagnosticCategory::BusinessRule);
                }
                // MontoTotal = monto a pagar sobre lo originalmente facturado, sin impuestos (se registra como 1 x monto).
                if (l.precioUnitario.isNegative() || l.precioUnitario.isZero() || l.cantidadScaled <= 0) {
                    diags.addError("RULE_REP_MONTO_PAGO_NON_POSITIVE", p + ".montoTotal", "El monto a pagar de la línea del Recibo Electrónico de Pago debe ser mayor a cero.", domain::DiagnosticCategory::BusinessRule);
                }
                // SubTotal editable: monto del pago a registrar para el cálculo del IVA.
                if (l.subTotalEditado.has_value() && !l.subTotalEditado->isPositive()) {
                    diags.addError("RULE_REP_SUBTOTAL_NON_POSITIVE", p + ".subTotal", "El subtotal (monto del pago para el cálculo del IVA) debe ser mayor a cero.", domain::DiagnosticCategory::BusinessRule);
                }
                // Impuesto: obligatorio en REP, que paga comprobantes con condición de venta 08 o 10.
                if (l.impuestos.empty()) {
                    diags.addError("RULE_REP_IMPUESTO_REQUIRED", p + ".impuestos", "Las líneas del Recibo Electrónico de Pago deben incluir el impuesto del pago.", domain::DiagnosticCategory::BusinessRule);
                }
                validarImpuestos(p + ".impuestos", l.impuestos, inv, false, diags);
                validarTarifasTransitorias(p + ".impuestos", l.impuestos, inv, diags);
                // Campos inexistentes (condición 4) en el REP.
                if (l.exoneracion.has_value()) {
                    diags.addError("RULE_REP_EXONERACION_PROHIBIDA", p + ".exoneracion", "La exoneración no aplica en el Recibo Electrónico de Pago.", domain::DiagnosticCategory::BusinessRule);
                }
                if (l.baseImponibleEditada.has_value() || !l.ivaCobradoFabrica.empty() || !l.tipoTransaccion.empty() ||
                    l.impuestoAsumidoEmisorFabrica.isPositive()) {
                    diags.addError("RULE_REP_CAMPOS_NO_APLICAN", p, "BaseImponible, IVACobradoFabrica, TipoTransaccion e ImpuestoAsumidoEmisorFabrica no aplican en el Recibo Electrónico de Pago.", domain::DiagnosticCategory::BusinessRule);
                }
            } else {
                if (l.subTotalEditado.has_value()) {
                    diags.addError("RULE_SUBTOTAL_NO_EDITABLE", p + ".subTotal", "El subtotal solo es editable en el Recibo Electrónico de Pago; en los demás comprobantes se calcula.", domain::DiagnosticCategory::BusinessRule);
                }
                if (!(cabysOpcional && l.cabys.empty()) &&
                    (l.cabys.length() != 13 || !std::all_of(l.cabys.begin(), l.cabys.end(), ::isdigit))) {
                    diags.addError("RULE_CABYS_FORMAT", p + ".codigoCABYS", "El código CABYS debe tener exactamente 13 dígitos numéricos.", domain::DiagnosticCategory::Catalog);
                }
                validarCodigosComerciales(p, l, diags);
                if (!CatalogoFiscal::esUnidadMedidaValida(l.unidadMedida)) {
                    diags.addError("RULE_UNIDAD_MEDIDA_INVALID", p + ".unidadMedida", "La unidad de medida no es válida en el catálogo oficial.", domain::DiagnosticCategory::Catalog);
                }
                if (l.cantidadScaled <= 0) {
                    diags.addError("RULE_CANTIDAD_NON_POSITIVE", p + ".cantidad", "La cantidad debe ser estrictamente mayor a cero.", domain::DiagnosticCategory::BusinessRule);
                }
                // Debe ser mayor a cero, salvo surtidos cuyo desglose incluya regalías (01) o bonificaciones (03).
                const bool surtidoConRegalias = std::any_of(l.surtido.begin(), l.surtido.end(), [](const domain::ComponenteSurtido& c) {
                    return std::any_of(c.descuentos.begin(), c.descuentos.end(), [](const domain::DescuentoLinea& d) {
                        return d.codigo == "01" || d.codigo == "03";
                    });
                });
                if (l.precioUnitario.isNegative() || (l.precioUnitario.isZero() && !surtidoConRegalias)) {
                    diags.addError("RULE_PRECIO_NON_POSITIVE", p + ".precioUnitario", "El precio unitario debe ser mayor a cero.", domain::DiagnosticCategory::BusinessRule);
                }
                if (l.impuestos.empty()) {
                    diags.addError("RULE_IMPUESTO_REQUIRED", p + ".impuestos", "Cada línea de detalle debe incluir al menos un impuesto.", domain::DiagnosticCategory::BusinessRule);
                }
                validarImpuestos(p + ".impuestos", l.impuestos, inv, !l.surtido.empty(), diags);
                validarTarifasTransitorias(p + ".impuestos", l.impuestos, inv, diags);

                // IVA cobrado a nivel de fábrica (Nota 21): solo FE, TE, NC y ND.
                const auto tieneImpuesto = [&l](domain::CodigoImpuesto c) {
                    return std::any_of(l.impuestos.begin(), l.impuestos.end(), [c](const domain::ImpuestoLinea& i) { return i.codigo == c; });
                };
                if (!l.ivaCobradoFabrica.empty()) {
                    if (l.ivaCobradoFabrica != "01" && l.ivaCobradoFabrica != "02") {
                        diags.addError("RULE_IVA_FABRICA_INVALID", p + ".ivaCobradoFabrica", "IVA cobrado a nivel de fábrica debe ser 01 o 02 (Nota 21).", domain::DiagnosticCategory::Catalog);
                    } else if (inv.isFEE || inv.isFEC) {
                        diags.addError("RULE_IVA_FABRICA_NO_APLICA", p + ".ivaCobradoFabrica", "IVA cobrado a nivel de fábrica no aplica en FEE ni FEC.", domain::DiagnosticCategory::BusinessRule);
                    } else if (l.ivaCobradoFabrica == "02") {
                        // Con 02 el producto se entiende exento: el IVA debe usar la tarifa 10.
                        for (const auto& imp : l.impuestos) {
                            if (domain::esIVA(imp.codigo) && imp.codigoTarifa != domain::CodigoTarifaIVA::TarifaExento) {
                                diags.addError("RULE_IVA_FABRICA_02_TARIFA_EXENTA", p + ".impuestos", "Con IVA cobrado a nivel de fábrica 02 se debe usar la tarifa exenta (código 10).", domain::DiagnosticCategory::BusinessRule);
                                break;
                            }
                        }
                    }
                }

                // Coherencia CABYS / impuesto específico (advertencias: Hacienda lo verifica en la recepción).
                if (!l.cabys.empty()) {
                    const bool esAlcoholica = common::CabysClasificacion::EsBebidaAlcoholica(l.cabys);
                    if (tieneImpuesto(domain::CodigoImpuesto::Alcoholicas) && !esAlcoholica) {
                        diags.addWarning("RULE_IMPUESTO_04_CABYS_NO_ALCOHOLICO", p + ".impuestos", "El impuesto 04 (bebidas alcohólicas) se aplica a un CABYS que no corresponde a bebidas alcohólicas (241, 242, 2431).");
                    }
                    if (tieneImpuesto(domain::CodigoImpuesto::BebidasEnvasadasJabones) &&
                        !common::CabysClasificacion::EsBebidaSinAlcohol(l.cabys) &&
                        !common::CabysClasificacion::EsJabonTocador(l.cabys)) {
                        diags.addWarning("RULE_IMPUESTO_05_CABYS_INVALIDO", p + ".impuestos", "El impuesto 05 aplica a bebidas envasadas sin contenido alcohólico (CABYS 244) y jabones de tocador (CABYS 3532101); el CABYS de la línea no corresponde.");
                    }
                }

                // Base imponible editable solo con IVA cobrado en fábrica 01 o impuesto 07 (en ambos casos es obligatoria).
                const bool requiereBaseEditada = l.ivaCobradoFabrica == "01" || tieneImpuesto(domain::CodigoImpuesto::IVACalculoEspecial);
                if (requiereBaseEditada && (!l.baseImponibleEditada.has_value() || !l.baseImponibleEditada->isPositive())) {
                    diags.addError("RULE_BASE_IMPONIBLE_REQUIRED", p + ".baseImponible", "La base imponible es obligatoria y mayor a cero con IVA cobrado a nivel de fábrica 01 o con el impuesto 07.", domain::DiagnosticCategory::BusinessRule);
                } else if (!requiereBaseEditada && l.baseImponibleEditada.has_value()) {
                    diags.addError("RULE_BASE_IMPONIBLE_NO_EDITABLE", p + ".baseImponible", "La base imponible solo puede editarse con IVA cobrado a nivel de fábrica 01 o con el impuesto 07.", domain::DiagnosticCategory::BusinessRule);
                }

                // Tipo de transacción (Nota 22): no aplica en TE.
                if (!l.tipoTransaccion.empty()) {
                    if (inv.isTE) {
                        diags.addError("RULE_TIPO_TRANSACCION_NO_APLICA", p + ".tipoTransaccion", "El tipo de transacción no aplica en Tiquete Electrónico.", domain::DiagnosticCategory::BusinessRule);
                    } else if (!CatalogoFiscal::esTipoTransaccionValido(l.tipoTransaccion)) {
                        diags.addError("RULE_TIPO_TRANSACCION_INVALID", p + ".tipoTransaccion", "Tipo de transacción no válido (Nota 22).", domain::DiagnosticCategory::Catalog);
                    }
                }

                // Descuentos en línea
                if (l.descuentos.size() > 5) {
                    diags.addError("RULE_DESCUENTO_MAX_EXCEEDED", p + ".descuentos", "No se permiten más de 5 descuentos por línea.", domain::DiagnosticCategory::BusinessRule);
                }
                for (size_t dIdx = 0; dIdx < l.descuentos.size(); ++dIdx) {
                    const auto& desc = l.descuentos[dIdx];
                    std::string dp = p + ".descuentos[" + std::to_string(dIdx) + "]";
                    if (!CatalogoFiscal::esCodigoDescuentoValido(desc.codigo)) {
                        diags.addError("RULE_DESCUENTO_CODIGO_INVALID", dp + ".codigoDescuento", "Código de descuento no válido (Nota 20).", domain::DiagnosticCategory::Catalog);
                    }
                    if (desc.monto.isNegative()) {
                        diags.addError("RULE_DESCUENTO_MONTO_NEGATIVO", dp + ".montoDescuento", "El monto del descuento no puede ser negativo.", domain::DiagnosticCategory::BusinessRule);
                    }
                    if (desc.codigo == "99" && !RuleValidationHelpers::textoEnRango(desc.naturaleza, 3, 80)) {
                        diags.addError("RULE_DESCUENTO_NATURALEZA_LENGTH", dp + ".naturalezaDescuento", "La naturaleza del descuento 'Otros' debe tener entre 3 y 80 caracteres.", domain::DiagnosticCategory::BusinessRule);
                    }
                    if (desc.codigo == "99" && !RuleValidationHelpers::textoEnRango(desc.codigoDescuentoOTRO, 5, 100)) {
                        diags.addError("RULE_DESCUENTO_OTRO_LENGTH", dp + ".codigoDescuentoOTRO", "Con el código de descuento 99 se debe describir el descuento (5 a 100 caracteres).", domain::DiagnosticCategory::BusinessRule);
                    }
                }

                // Partida Arancelaria según Tipo de Comprobante
                const bool allowsPartida = inv.isFEE || modifiesFEE;
                const bool esBien = !l.cabys.empty() && l.cabys[0] >= '0' && l.cabys[0] <= '4';
                if (allowsPartida) {
                    if (esBien) {
                        if (l.partidaArancelaria.empty()) {
                            diags.addError("RULE_PARTIDA_ARANCELARIA_REQUIRED", p + ".partidaArancelaria",
                                "La partida arancelaria es obligatoria para bienes en FEE (o Notas que la modifican).",
                                domain::DiagnosticCategory::BusinessRule);
                        } else if (l.partidaArancelaria.length() < 12 ||
                                !std::all_of(l.partidaArancelaria.begin(), l.partidaArancelaria.end(), ::isdigit)) {
                            diags.addError("RULE_PARTIDA_ARANCELARIA_FORMAT", p + ".partidaArancelaria",
                                "La partida arancelaria debe contener al menos 12 dígitos numéricos.",
                                domain::DiagnosticCategory::BusinessRule);
                        }
                    } else if (!l.partidaArancelaria.empty()) {
                        diags.addError("RULE_PARTIDA_ARANCELARIA_SERVICIO_PROHIBIDA", p + ".partidaArancelaria",
                            "La partida arancelaria está prohibida en servicios de exportación.",
                            domain::DiagnosticCategory::BusinessRule);
                    }
                } else {
                    if (!l.partidaArancelaria.empty()) {
                        diags.addError("RULE_PARTIDA_ARANCELARIA_NOT_ALLOWED", p + ".partidaArancelaria", "La partida arancelaria solo está permitida en Factura Electrónica de Exportación.", domain::DiagnosticCategory::BusinessRule);
                    }
                }

                if ((inv.isFEE || inv.isFEC || inv.isREP) && l.impuestoAsumidoEmisorFabrica.isPositive()) {
                    diags.addError("RULE_IMPUESTO_ASUMIDO_FABRICA_PROHIBIDO", p + ".impuestoAsumidoEmisorFabrica", "El campo ImpuestoAsumidoEmisorFabrica está prohibido en este tipo de comprobante.", domain::DiagnosticCategory::BusinessRule);
                }

                // Bienes Especiales v4.4
                // 1. Números VIN / Serie
                if (l.numerosVIN.size() > 1000) {
                    diags.addError("RULE_VIN_MAX_EXCEEDED", p + ".numerosVINoSerie", "No se permiten más de 1000 números VIN por línea.", domain::DiagnosticCategory::BusinessRule);
                }
                // Advertencia (no rechazo) si el CABYS es de un medio de transporte y falta el VIN o serie.
                if (l.numerosVIN.empty() && common::CabysClasificacion::EsMedioTransporte(l.cabys)) {
                    diags.addWarning("RULE_VIN_REQUERIDO", p + ".numerosVINoSerie", "El CABYS corresponde a un medio de transporte: se debe indicar el número de VIN, serie o consecutivo de fabricación.");
                }
                for (size_t vIdx = 0; vIdx < l.numerosVIN.size(); ++vIdx) {
                    if (!CatalogoFiscal::esVinValido(l.numerosVIN[vIdx])) {
                        diags.addError("RULE_VIN_LENGTH", p + ".numerosVINoSerie[" + std::to_string(vIdx) + "]", "El VIN o número de serie debe tener entre 1 y 17 caracteres alfanuméricos.", domain::DiagnosticCategory::BusinessRule);
                    }
                }

                // 2. Sector Farmacéutico / Medicamentos
                // Advertencia (no rechazo) si el CABYS es de un medicamento y faltan registro o forma farmacéutica.
                if (common::CabysClasificacion::EsMedicamento(l.cabys) &&
                    (l.registroMedicamento.empty() || l.formaFarmaceutica.empty())) {
                    diags.addWarning("RULE_MEDICAMENTO_DATOS_REQUERIDOS", p + ".registroMedicamento", "El CABYS corresponde a un medicamento: si posee registro sanitario se deben indicar el registro de medicamento y la forma farmacéutica (Nota 19).");
                }
                if (!l.registroMedicamento.empty() && l.formaFarmaceutica.empty()) {
                    diags.addError("RULE_FORMA_FARMACEUTICA_REQUIRED", p + ".formaFarmaceutica", "La forma farmacéutica es obligatoria si se especifica registro de medicamento.", domain::DiagnosticCategory::BusinessRule);
                }
                if (l.registroMedicamento.empty() && !l.formaFarmaceutica.empty()) {
                    diags.addError("RULE_REGISTRO_MEDICAMENTO_REQUIRED", p + ".registroMedicamento", "El registro de medicamento es obligatorio si se especifica forma farmacéutica.", domain::DiagnosticCategory::BusinessRule);
                }
                if (!l.formaFarmaceutica.empty()) {
                    if (!CatalogoFiscal::esFormaFarmaceuticaValida(l.formaFarmaceutica)) {
                        diags.addError("RULE_FORMA_FARMACEUTICA_INVALID", p + ".formaFarmaceutica", "Código de forma farmacéutica inexistente en catálogo Nota 9.", domain::DiagnosticCategory::Catalog);
                    }
                }

                // 3. Surtidos / Paquetes / Combos
                if (inv.isFEC && !l.surtido.empty()) {
                    diags.addError("RULE_FEC_SURTIDO_PROHIBIDO", p + ".detalleSurtido", "Detalle de surtidos no permitido en Factura de Compra.", domain::DiagnosticCategory::BusinessRule);
                }
                if (common::CabysClasificacion::EsSurtidoHabilitado(l.cabys) && l.surtido.empty() && !inv.isFEC) {
                    // Nota al pie 8: el desglose es obligatorio si el surtido combina bienes con distintas tarifas o impuestos.
                    diags.addWarning("RULE_SURTIDO_DETALLE_RECOMENDADO", p + ".detalleSurtido", "La línea usa un CABYS de surtido: el detalle de productos del surtido es obligatorio si combina bienes con distintas tarifas de IVA o impuestos específicos.");
                }
                if (!l.surtido.empty() && !l.cabys.empty() && !common::CabysClasificacion::EsSurtidoHabilitado(l.cabys)) {
                    // Nota al pie 8: el nodo solo procede para surtidos de fabricantes o importadores con CABYS de surtido.
                    diags.addWarning("RULE_SURTIDO_CABYS_NO_HABILITADO", p + ".detalleSurtido", "El detalle de surtido se usa con un CABYS que no es de surtido; los componentes deberían facturarse como líneas de detalle.");
                }
                if (!l.surtido.empty()) {
                    if (l.surtido.size() > 20) {
                        diags.addError("RULE_SURTIDO_MAX_COMPONENTES", p + ".detalleSurtido", "Un surtido no puede contener más de 20 componentes.", domain::DiagnosticCategory::BusinessRule);
                    }
                    for (size_t sIdx = 0; sIdx < l.surtido.size(); ++sIdx) {
                        const auto& comp = l.surtido[sIdx];
                        std::string cp = p + ".detalleSurtido[" + std::to_string(sIdx) + "]";
                        if (comp.codigoCABYS.length() != 13 || !std::all_of(comp.codigoCABYS.begin(), comp.codigoCABYS.end(), ::isdigit)) {
                            diags.addError("RULE_SURTIDO_CABYS_FORMAT", cp + ".codigoCABYS", "El código CABYS del surtido debe tener 13 dígitos numéricos.", domain::DiagnosticCategory::Catalog);
                        }
                        validarCodigosComerciales(cp, comp.codigosComerciales, diags);
                        validarDescuentosSurtido(cp, comp, diags);
                        if (!RuleValidationHelpers::textoEnRango(comp.descripcion, 3, 200)) {
                            diags.addError("RULE_SURTIDO_DESC_LENGTH", cp + ".detalle", "La descripción del componente de surtido debe tener entre 3 y 200 caracteres.", domain::DiagnosticCategory::BusinessRule);
                        }
                        if (comp.cantidadScaled <= 0) {
                            diags.addError("RULE_SURTIDO_CANTIDAD_NON_POSITIVE", cp + ".cantidad", "La cantidad del componente de surtido debe ser mayor a cero.", domain::DiagnosticCategory::BusinessRule);
                        }
                        if (comp.precioUnitario.isNegative() || comp.precioUnitario.isZero()) {
                            diags.addError("RULE_SURTIDO_PRECIO_NON_POSITIVE", cp + ".precioUnitario", "El precio unitario del componente de surtido debe ser mayor a cero.", domain::DiagnosticCategory::BusinessRule);
                        }
                        if (!CatalogoFiscal::esUnidadMedidaValida(comp.unidadMedida)) {
                            diags.addError("RULE_SURTIDO_UNIDAD_MEDIDA_INVALID", cp + ".unidadMedida", "La unidad de medida del componente de surtido no es válida.", domain::DiagnosticCategory::Catalog);
                        }
                        if (comp.impuestos.empty()) {
                            diags.addError("RULE_SURTIDO_IMPUESTO_REQUIRED", cp + ".impuestos", "Debe indicarse al menos un impuesto en el componente de surtido.", domain::DiagnosticCategory::BusinessRule);
                        }
                        validarImpuestos(cp + ".impuestos", comp.impuestos, inv, false, diags);
                        validarTarifasTransitorias(cp + ".impuestos", comp.impuestos, inv, diags);
                    }
                }

                // Exoneración vs Tarifa
                if (l.exoneracion.has_value()) {
                    if (inv.isFEE) {
                        diags.addError("RULE_FEE_EXONERACION_PROHIBIDA", p + ".exoneracion", "El nodo de Exoneración está prohibido en Factura Electrónica de Exportación.", domain::DiagnosticCategory::BusinessRule);
                    }
                    const auto& ex = l.exoneracion.value();
                    if (!CatalogoFiscal::esTipoDocumentoExoneracionValido(ex.tipoDocumento)) {
                        diags.addError("RULE_EXONERACION_TIPO_DOC_INVALID", p + ".exoneracion.tipoDocumentoEX1", "Tipo de documento de exoneración no válido.", domain::DiagnosticCategory::Catalog);
                    } else if (!(inv.isNC || inv.isND) &&
                               (ex.tipoDocumento == "01" || ex.tipoDocumento == "05" ||
                                ex.tipoDocumento == "06" || ex.tipoDocumento == "07")) {
                        // Nota 10.1, notas al pie 39-42: códigos inhabilitados, solo para NC y ND que lo requieran.
                        diags.addError("RULE_EXONERACION_TIPO_DOC_INHABILITADO", p + ".exoneracion.tipoDocumentoEX1", "Los tipos de exoneración 01, 05, 06 y 07 están inhabilitados; solo se permiten en Notas de Crédito y Débito.", domain::DiagnosticCategory::BusinessRule);
                    }
                    if (!RuleValidationHelpers::textoEnRango(ex.numeroDocumento, 3, 40)) {
                        diags.addError("RULE_EXONERACION_NUM_DOC_LENGTH", p + ".exoneracion.numeroDocumento", "El número de documento de exoneración debe tener entre 3 y 40 caracteres.", domain::DiagnosticCategory::BusinessRule);
                    }
                    if (ex.nombreInstitucion.empty()) {
                        diags.addError("RULE_EXONERACION_INSTITUCION_EMPTY", p + ".exoneracion.nombreInstitucion", "El nombre de la institución que exonera es obligatorio.", domain::DiagnosticCategory::BusinessRule);
                    } else if (!CatalogoFiscal::esNombreInstitucionValido(ex.nombreInstitucion)) {
                        diags.addError("RULE_EXONERACION_INSTITUCION_INVALID", p + ".exoneracion.nombreInstitucion", "El código de institución que emitió la exoneración no es válido (Nota 23).", domain::DiagnosticCategory::Catalog);
                    }
                    if (!RuleValidationHelpers::esFechaRFC3339(ex.fechaEmision)) {
                        diags.addError("RULE_EXONERACION_FECHA_FORMAT", p + ".exoneracion.fechaEmisionEX", "La fecha de emisión del documento de exoneración es obligatoria y debe tener formato RFC3339.", domain::DiagnosticCategory::BusinessRule);
                    }
                    if (ex.tipoDocumento == "99" && !RuleValidationHelpers::textoEnRango(ex.tipoDocumentoOTRO, 5, 100)) {
                        diags.addError("RULE_EXONERACION_TIPO_DOC_OTRO_LENGTH", p + ".exoneracion.tipoDocumentoOTRO", "Con el tipo de documento de exoneración 99 se debe describir el documento (5 a 100 caracteres).", domain::DiagnosticCategory::BusinessRule);
                    }
                    if (ex.nombreInstitucion == "99" && !RuleValidationHelpers::textoEnRango(ex.nombreInstitucionOtros, 5, 160)) {
                        diags.addError("RULE_EXONERACION_INSTITUCION_OTROS_LENGTH", p + ".exoneracion.nombreInstitucionOtros", "Con la institución 99 se debe indicar su nombre (5 a 160 caracteres).", domain::DiagnosticCategory::BusinessRule);
                    }
                    const bool requiereArticulo = ex.tipoDocumento == "02" || ex.tipoDocumento == "03" ||
                                                  ex.tipoDocumento == "06" || ex.tipoDocumento == "07" || ex.tipoDocumento == "08";
                    if (requiereArticulo && !ex.articulo.has_value()) {
                        diags.addError("RULE_EXONERACION_ARTICULO_REQUIRED", p + ".exoneracion.articulo", "El artículo de la ley es obligatorio con los tipos de exoneración 02, 03, 06, 07 y 08.", domain::DiagnosticCategory::BusinessRule);
                    }
                    for (const auto& [valor, campo] : { std::pair{ex.articulo, "articulo"}, std::pair{ex.inciso, "inciso"} }) {
                        if (valor.has_value() && (*valor <= 0 || *valor > 999999)) {
                            diags.addError("RULE_EXONERACION_ARTICULO_INCISO_FORMAT", p + ".exoneracion." + campo, "El artículo e inciso deben ser enteros positivos de máximo 6 dígitos.", domain::DiagnosticCategory::BusinessRule);
                        }
                    }
                    if (!std::any_of(l.impuestos.begin(), l.impuestos.end(), [](const domain::ImpuestoLinea& i) { return domain::esIVA(i.codigo); })) {
                        diags.addError("RULE_EXONERACION_SIN_IVA", p + ".exoneracion", "La exoneración requiere un impuesto de IVA en la línea.", domain::DiagnosticCategory::BusinessRule);
                    }
                    for (const auto& imp : l.impuestos) {
                        if (domain::esIVA(imp.codigo) && ex.porcentajeExoneracionScaled > imp.tarifaPorcentajeScaled) {
                            diags.addError("RULE_EXONERACION_EXCEDE_TARIFA", p + ".exoneracion.tarifaExonerada", "El porcentaje de exoneración supera la tarifa del impuesto gravado.", domain::DiagnosticCategory::BusinessRule);
                        }
                    }
                }
            }
        }

        // Resumen
        if ((inv.isFEE || inv.isFEC || inv.isREP) && inv.resumen.totalIvaDevuelto.isPositive()) {
            diags.addError("RULE_TOTAL_IVA_DEVUELTO_PROHIBIDO", "resumen.totalIvaDevuelto", "El campo TotalIVADevuelto está prohibido en este tipo de comprobante.", domain::DiagnosticCategory::BusinessRule);
        }
        if (inv.isREP) {
            if (inv.resumen.totalImpAsumEmisorFabrica.isPositive()) {
                diags.addError("RULE_REP_TOTAL_IMP_ASUM_FABRICA_PROHIBIDO", "resumen.totalImpAsumEmisorFabrica", "El campo TotalImpAsumEmisorFabrica está prohibido en Recibo Electrónico de Pago.", domain::DiagnosticCategory::BusinessRule);
            }
            if (inv.resumen.totalOtrosCargos.isPositive()) {
                diags.addError("RULE_REP_TOTAL_OTROS_CARGOS_PROHIBIDO", "resumen.totalOtrosCargos", "El campo TotalOtrosCargos está prohibido en Recibo Electrónico de Pago.", domain::DiagnosticCategory::BusinessRule);
            }
            if (inv.resumen.totalDescuentos.isPositive()) {
                diags.addError("RULE_REP_TOTAL_DESCUENTOS_PROHIBIDO", "resumen.totalDescuentos", "El campo TotalDescuentos está prohibido en Recibo Electrónico de Pago.", domain::DiagnosticCategory::BusinessRule);
            }
        }
    }
};

} // namespace cec::rules

