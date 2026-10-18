#pragma once
#include <algorithm>
#include <vector>
#include <string>
#include <optional>
#include <cstdint>
#include <exception>
#include <stdexcept>
#include "cec/domain/Diagnostics.h"
#include "cec/domain/Comprobante.h"
#include "cec/common/CabysClasificacion.h"

namespace cec::calculation {

/**
 * @brief Motor de cálculo monetario determinista según la normativa v4.4 (Anexos y Estructuras).
 *
 * Por línea:
 *   MontoTotal   = Cantidad * PrecioUnitario
 *   SubTotal     = MontoTotal - SUM(Descuentos)
 *   Impuestos no IVA (02, 03, 04, 05, 06, 12, 99) se calculan primero.
 *   BaseImponible = SubTotal + impuestos 02, 04, 05 y 12 (editable con IVACobradoFabrica 01 o impuesto 07).
 *   IVA (01, 07, 08) sobre la base imponible (01 con regalías/bonificaciones: sobre MontoTotal; 08: factor * SubTotal).
 *   ImpuestoNeto = SUM(Impuestos) - MontoExoneracion - ImpuestoAsumidoEmisorFabrica
 *   MontoTotalLinea = SubTotal + ImpuestoNeto
 *
 * Resumen:
 *   Servicio / mercancía según el primer dígito del CABYS (5-9 servicios, 0-4 bienes).
 *   Totales gravado / exento / exonerado / no sujeto sobre MontoTotal.
 *   TotalComprobante = TotalVentaNeta + TotalImpuesto + TotalOtrosCargos - TotalIVADevuelto
 *
 * Recibo Electrónico de Pago (REP):
 *   MontoTotal = monto a pagar sobre lo originalmente facturado, sin impuestos (Cantidad * PrecioUnitario)
 *   SubTotal   = editable: monto del pago a registrar para el cálculo del IVA (por defecto MontoTotal)
 *   Monto del impuesto = Tarifa * SubTotal; sin descuentos, exoneraciones ni impuestos asumidos
 *   TotalVenta = SUM(MontoTotal); TotalVentaNeta = TotalVenta
 *   TotalComprobante = SUM("Total de la venta") (Anexo 1, pág. 56)
 */
class CalculationEngine {
public:
    static domain::DiagnosticList calculate(domain::ComprobanteCanonico& comprobante) {
        domain::DiagnosticList diags;

        domain::ResumenTotales totales{};

        for (size_t i = 0; i < comprobante.lineas.size(); ++i) {
            auto& linea = comprobante.lineas[i];
            const std::string linePath = "detalleServicio[" + std::to_string(i) + "]";

            try {
                std::vector<PartidaImpuesto> partidas;
                if (comprobante.isREP) {
                    calcularLineaREP(linea, partidas);
                    totales.totalVenta += linea.montoTotal;
                    totales.totalImpuesto += linea.impuestoNeto;
                    acumularDesglose(partidas, totales.desgloseImpuestos);
                    continue;
                }
                calcularLinea(linea, partidas);

                if (linea.montoDescuento > linea.montoTotal) {
                    diags.addError("DISCOUNT_EXCEEDS_TOTAL", linePath + ".descuentos", "El descuento total supera el monto bruto de la línea.", domain::DiagnosticCategory::BusinessRule);
                }

                acumularClasificacion(linea, comprobante.isFEE, totales);

                totales.totalVenta += linea.montoTotal;
                totales.totalDescuentos += linea.montoDescuento;
                totales.totalImpuesto += linea.impuestoNeto;
                totales.totalImpAsumEmisorFabrica += linea.impuestoAsumidoEmisorFabrica;

                acumularDesglose(partidas, totales.desgloseImpuestos);
            } catch (const std::exception& e) {
                diags.addError("CALC_OVERFLOW_LINEA", linePath, "Error aritmético al calcular la línea: " + std::string(e.what()), domain::DiagnosticCategory::Calculation);
                return diags;
            }
        }

        totales.totalGravado = totales.totalServiciosGravados + totales.totalMercanciasGravadas;
        totales.totalExento = totales.totalServiciosExentos + totales.totalMercanciasExentas;
        totales.totalExonerado = totales.totalServiciosExonerados + totales.totalMercanciasExoneradas;
        totales.totalNoSujeto = totales.totalServiciosNoSujetos + totales.totalMercanciasNoSujetas;
        totales.totalVentaNeta = totales.totalVenta - totales.totalDescuentos;

        // Otros Cargos
        domain::Money totalOtrosCargos;
        for (const auto& cargo : comprobante.otrosCargos) {
            try {
                totalOtrosCargos = totalOtrosCargos + cargo.montoCargo;
            } catch (const std::exception& e) {
                diags.addError("CALC_OVERFLOW_OTROS_CARGOS", "otrosCargos", "Overflow al acumular otros cargos: " + std::string(e.what()), domain::DiagnosticCategory::Calculation);
                return diags;
            }
        }
        totales.totalOtrosCargos = totalOtrosCargos;

        totales.totalIvaDevuelto = comprobante.isREP ? domain::Money{} : calcularIvaDevuelto(comprobante, totales.totalVentaNeta);

        totales.totalComprobante = comprobante.isREP
            ? totales.totalVenta
            : totales.totalVentaNeta + totales.totalImpuesto + totales.totalOtrosCargos - totales.totalIvaDevuelto;

        asignarMediosPago(comprobante, totales.totalComprobante);

        totales.codigoTipoMoneda = comprobante.resumen.codigoTipoMoneda;
        comprobante.resumen = totales;
        return diags;
    }

    /// @brief Servicio si el CABYS inicia con 5-9; sin CABYS (NC/ND financieras) se usa la unidad de medida.
    static bool esServicio(const std::string& cabys, const std::string& unidadMedida) noexcept {
        if (!cabys.empty() && cabys[0] >= '0' && cabys[0] <= '9') {
            return common::CabysClasificacion::EsServicio(cabys);
        }
        return unidadMedida == "Sp" || unidadMedida == "Spe" || unidadMedida == "h" || unidadMedida == "St";
    }

    /// @brief IVA calculado en líneas de servicios de salud (CABYS 931) con tarifa reducida 4% (nota al pie 16).
    static domain::Money ivaServiciosSalud(const domain::ComprobanteCanonico& comprobante) {
        domain::Money total;
        for (const auto& l : comprobante.lineas) {
            if (!common::CabysClasificacion::EsServicioSalud(l.cabys)) continue;
            for (const auto& imp : l.impuestos) {
                if (imp.codigo == domain::CodigoImpuesto::IVA && imp.codigoTarifa == domain::CodigoTarifaIVA::TarifaReducida4) {
                    total += imp.monto;
                }
            }
        }
        return total;
    }

private:
    static constexpr int64_t kTarifaCementoScaled = 5 * domain::Money::SCALE; // 5%

    /// @brief Monto de un impuesto a nivel de desglose (código + tarifa) antes de exoneraciones/asumidos.
    struct PartidaImpuesto {
        domain::CodigoImpuesto codigo;
        domain::CodigoTarifaIVA codigoTarifa;
        domain::Money monto;
    };

    static bool sumaABaseImponible(domain::CodigoImpuesto c) noexcept {
        using domain::CodigoImpuesto;
        return c == CodigoImpuesto::SelectivoConsumo || c == CodigoImpuesto::Alcoholicas ||
               c == CodigoImpuesto::BebidasEnvasadasJabones || c == CodigoImpuesto::Cemento;
    }

    static domain::Money sumaDescuentos(const std::vector<domain::DescuentoLinea>& descuentos, bool soloRegalias) {
        domain::Money total;
        for (const auto& d : descuentos) {
            if (!soloRegalias || d.codigo == "01" || d.codigo == "03") {
                total += d.monto;
            }
        }
        return total;
    }

    /**
     * @brief Impuestos específicos no tarifarios (03, 04, 05, 06). Sin datos, el monto es 0 (la regla ya lo rechaza).
     */
    static domain::Money calcularEspecifico(const domain::ImpuestoLinea& imp, int64_t cantidadScaled, const std::string& cabys) {
        using domain::CodigoImpuesto;
        if (!imp.datosEspecificos.has_value()) return {};
        const auto& d = *imp.datosEspecificos;

        switch (imp.codigo) {
            case CodigoImpuesto::Combustibles:
                // CantidadUnidadMedida (litros) * ImpuestoUnidad
                return d.impuestoUnidad.multiplyScaled(d.cantidadUnidadMedidaScaled);
            case CodigoImpuesto::Alcoholicas: {
                // Cantidad * Proporción * ImpuestoUnidad; Proporción = ml del envase * % de alcohol
                int64_t proporcion = d.proporcionScaled.value_or(0);
                if (!d.proporcionScaled.has_value() && d.porcentajeScaled.has_value()) {
                    proporcion = domain::Money::fromScaled(d.cantidadUnidadMedidaScaled)
                                     .multiplyPercent(*d.porcentajeScaled).scaledValue();
                }
                return d.impuestoUnidad.multiplyScaled(cantidadScaled).multiplyScaled(proporcion);
            }
            case CodigoImpuesto::BebidasEnvasadasJabones: {
                const int64_t volumen = d.volumenUnidadConsumoScaled.value_or(0);
                if (common::CabysClasificacion::EsJabonTocador(cabys)) {
                    // Jabón de tocador: Cantidad * Volumen (gramos) * ImpuestoUnidad
                    return d.impuestoUnidad.multiplyScaled(cantidadScaled).multiplyScaled(volumen);
                }
                // Bebidas sin alcohol (CABYS 244) y cualquier otro CABYS:
                // Cantidad * CantidadUnidadMedida * (ImpuestoUnidad / VolumenUnidadConsumo)
                if (volumen == 0) return {};
                return d.impuestoUnidad.divideScaled(volumen)
                           .multiplyScaled(cantidadScaled)
                           .multiplyScaled(d.cantidadUnidadMedidaScaled);
            }
            case CodigoImpuesto::Tabaco:
                // Cantidad * CantidadUnidadMedida * ImpuestoUnidad
                return d.impuestoUnidad.multiplyScaled(cantidadScaled).multiplyScaled(d.cantidadUnidadMedidaScaled);
            default:
                return {};
        }
    }

    /**
     * @brief Calcula los impuestos de una línea o componente de surtido y devuelve la base imponible del IVA.
     */
    static domain::Money calcularImpuestos(std::vector<domain::ImpuestoLinea>& impuestos,
                                           const domain::Money& montoTotal,
                                           const domain::Money& subTotal,
                                           int64_t cantidadScaled,
                                           const std::string& cabys,
                                           const std::optional<domain::Money>& baseEditada,
                                           bool tieneRegalias) {
        using domain::CodigoImpuesto;

        // 1. Impuestos no IVA
        domain::Money sumaParaBase;
        for (auto& imp : impuestos) {
            switch (imp.codigo) {
                case CodigoImpuesto::SelectivoConsumo:
                case CodigoImpuesto::Otros:
                    imp.monto = subTotal.multiplyPercent(imp.tarifaPorcentajeScaled);
                    break;
                case CodigoImpuesto::Cemento:
                    imp.tarifaPorcentajeScaled = kTarifaCementoScaled;
                    imp.monto = subTotal.multiplyPercent(kTarifaCementoScaled);
                    break;
                case CodigoImpuesto::Combustibles:
                case CodigoImpuesto::Alcoholicas:
                case CodigoImpuesto::BebidasEnvasadasJabones:
                case CodigoImpuesto::Tabaco:
                    imp.monto = calcularEspecifico(imp, cantidadScaled, cabys);
                    break;
                default:
                    continue; // IVA en el paso 2
            }
            if (sumaABaseImponible(imp.codigo)) {
                sumaParaBase += imp.monto;
            }
        }

        // 2. Base imponible e IVA
        const domain::Money baseCalculada = (tieneRegalias ? montoTotal : subTotal) + sumaParaBase;
        const domain::Money baseImponible = baseEditada.value_or(baseCalculada);

        for (auto& imp : impuestos) {
            switch (imp.codigo) {
                case CodigoImpuesto::IVA:
                case CodigoImpuesto::IVACalculoEspecial:
                    imp.monto = baseImponible.multiplyPercent(imp.tarifaPorcentajeScaled);
                    break;
                case CodigoImpuesto::IVABienesUsados:
                    // Factor establecido por Hacienda * SubTotal
                    imp.monto = subTotal.multiplyScaled(imp.factorCalculoIVAScaled.value_or(0));
                    break;
                default:
                    break;
            }
        }
        return baseImponible;
    }

    static void calcularLineaREP(domain::LineaDetalle& linea, std::vector<PartidaImpuesto>& partidas) {
        linea.montoTotal = linea.precioUnitario.multiplyScaled(linea.cantidadScaled);
        linea.montoDescuento = {};
        linea.subTotal = linea.subTotalEditado.value_or(linea.montoTotal);
        linea.baseImponible = linea.subTotal;

        domain::Money totalImpuesto;
        for (auto& imp : linea.impuestos) {
            imp.monto = imp.codigo == domain::CodigoImpuesto::IVABienesUsados
                ? linea.subTotal.multiplyScaled(imp.factorCalculoIVAScaled.value_or(0))
                : linea.subTotal.multiplyPercent(imp.tarifaPorcentajeScaled);
            totalImpuesto += imp.monto;
            partidas.push_back({ imp.codigo, imp.codigoTarifa, imp.monto });
        }
        linea.impuestoAsumidoEmisorFabrica = {};
        linea.impuestoNeto = totalImpuesto;
        linea.montoTotalLinea = linea.subTotal + linea.impuestoNeto;
    }

    static void calcularLinea(domain::LineaDetalle& linea, std::vector<PartidaImpuesto>& partidas) {
        // 0. Componentes de surtido: cada uno se calcula como una línea independiente (por unidad de surtido).
        domain::Money surtidoSubTotal;
        domain::Money surtidoBase;
        for (auto& comp : linea.surtido) {
            comp.montoTotal = comp.precioUnitario.multiplyScaled(comp.cantidadScaled);
            comp.montoDescuento = sumaDescuentos(comp.descuentos, false);
            comp.subTotal = comp.montoTotal - comp.montoDescuento;
            const bool compRegalias = !sumaDescuentos(comp.descuentos, true).isZero();
            comp.baseImponible = calcularImpuestos(comp.impuestos, comp.montoTotal, comp.subTotal,
                                                   comp.cantidadScaled, comp.codigoCABYS, std::nullopt, compRegalias);
            surtidoSubTotal += comp.subTotal;
            surtidoBase += comp.baseImponible;
        }
        const bool esSurtido = !linea.surtido.empty();
        if (esSurtido && linea.precioUnitario.isZero()) {
            linea.precioUnitario = surtidoSubTotal;
        }

        // 1-3. MontoTotal, descuentos y SubTotal
        linea.montoTotal = linea.precioUnitario.multiplyScaled(linea.cantidadScaled);
        linea.montoDescuento = sumaDescuentos(linea.descuentos, false);
        linea.subTotal = linea.montoTotal - linea.montoDescuento;

        const domain::Money descuentoRegalias = sumaDescuentos(linea.descuentos, true);
        const bool tieneRegalias = !descuentoRegalias.isZero();

        // 4. Impuestos
        if (esSurtido) {
            // Con surtido la línea no lleva CodigoTarifaIVA ni Tarifa (Anexo 1): hay un Impuesto por código cuyo
            // monto es la suma de los impuestos de los componentes por la cantidad de surtidos.
            linea.baseImponible = surtidoBase.multiplyScaled(linea.cantidadScaled);
            std::vector<domain::ImpuestoLinea> porCodigo;
            for (const auto& comp : linea.surtido) {
                for (const auto& cimp : comp.impuestos) {
                    auto it = std::find_if(porCodigo.begin(), porCodigo.end(), [&](const domain::ImpuestoLinea& i) {
                        return i.codigo == cimp.codigo && i.codigoImpuestoOTRO == cimp.codigoImpuestoOTRO;
                    });
                    if (it == porCodigo.end()) {
                        domain::ImpuestoLinea imp{};
                        imp.codigo = cimp.codigo;
                        imp.codigoTarifa = cimp.codigoTarifa;
                        imp.codigoImpuestoOTRO = cimp.codigoImpuestoOTRO;
                        it = porCodigo.insert(porCodigo.end(), imp);
                    }
                    it->monto += cimp.monto;
                }
            }
            for (auto& imp : porCodigo) imp.monto = imp.monto.multiplyScaled(linea.cantidadScaled);
            linea.impuestos = std::move(porCodigo);
            // El desglose se hace con la tarifa de cada componente.
            for (const auto& comp : linea.surtido) {
                for (const auto& cimp : comp.impuestos) {
                    partidas.push_back({ cimp.codigo, cimp.codigoTarifa, cimp.monto.multiplyScaled(linea.cantidadScaled) });
                }
            }
        } else {
            linea.baseImponible = calcularImpuestos(linea.impuestos, linea.montoTotal, linea.subTotal,
                                                    linea.cantidadScaled, linea.cabys,
                                                    linea.baseImponibleEditada, tieneRegalias);
            for (const auto& imp : linea.impuestos) {
                partidas.push_back({ imp.codigo, imp.codigoTarifa, imp.monto });
            }
        }

        domain::Money totalImpuestoLinea;
        domain::Money ivaLinea;
        for (const auto& imp : linea.impuestos) {
            totalImpuestoLinea += imp.monto;
            if (domain::esIVA(imp.codigo)) ivaLinea += imp.monto;
        }

        // 5. Exoneración del IVA: TarifaExonerada * base imponible, sin superar el IVA calculado.
        domain::Money montoExonerado;
        if (linea.exoneracion.has_value() && ivaLinea.isPositive()) {
            auto& exon = *linea.exoneracion;
            exon.montoExoneracion = linea.baseImponible.multiplyPercent(exon.porcentajeExoneracionScaled);
            if (exon.montoExoneracion > ivaLinea) exon.montoExoneracion = ivaLinea;
            montoExonerado = exon.montoExoneracion;
        }

        // 6. Impuesto asumido: IVA de regalías/bonificaciones (01/03) + el indicado por el emisor (fábrica, específicos).
        domain::Money asumidoRegalias;
        if (tieneRegalias) {
            for (const auto& imp : linea.impuestos) {
                if (imp.codigo == domain::CodigoImpuesto::IVA) {
                    asumidoRegalias += descuentoRegalias.multiplyPercent(imp.tarifaPorcentajeScaled);
                }
            }
        }
        linea.impuestoAsumidoEmisorFabrica += asumidoRegalias;

        domain::Money deducciones = montoExonerado + linea.impuestoAsumidoEmisorFabrica;
        if (deducciones > totalImpuestoLinea) deducciones = totalImpuestoLinea;

        linea.impuestoNeto = totalImpuestoLinea - deducciones;
        linea.montoTotalLinea = linea.subTotal + linea.impuestoNeto;

        // Aplicar deducciones a las partidas de desglose: primero IVA, luego el resto.
        for (int pasada = 0; pasada < 2 && deducciones.isPositive(); ++pasada) {
            for (auto& part : partidas) {
                if ((pasada == 0) != domain::esIVA(part.codigo) || !deducciones.isPositive()) continue;
                const domain::Money aplicar = deducciones < part.monto ? deducciones : part.monto;
                part.monto -= aplicar;
                deducciones -= aplicar;
            }
        }
    }

    /**
     * @brief Clasifica el MontoTotal de la línea en gravado / exento / exonerado / no sujeto (servicio o mercancía).
     */
    static void acumularClasificacion(const domain::LineaDetalle& linea, bool esFEE, domain::ResumenTotales& t) {
        using domain::CodigoTarifaIVA;

        const domain::ImpuestoLinea* iva = nullptr;
        domain::Money totalImpuesto;
        for (const auto& imp : linea.impuestos) {
            totalImpuesto += imp.monto;
            if (iva == nullptr && domain::esIVA(imp.codigo)) iva = &imp;
        }

        domain::Money gravado, exento, exonerado, noSujeto;
        const domain::Money& base = linea.montoTotal;

        if (iva == nullptr) {
            (totalImpuesto.isPositive() ? gravado : exento) = base;
        } else if (iva->codigoTarifa == CodigoTarifaIVA::TarifaExento ||
                   linea.ivaCobradoFabrica == "02") {
            exento = base;
        } else if (iva->codigoTarifa == CodigoTarifaIVA::Tarifa0 ||
                   iva->codigoTarifa == CodigoTarifaIVA::Tarifa0SinCredito) {
            // La FEE no tiene totales de no sujeto: las reglas rechazan estas tarifas en la FEE (RULE_FEE_TARIFA_NO_SUJETO);
            // aquí solo se evita perder el monto en la vista previa.
            (esFEE ? exento : noSujeto) = base;
        } else {
            domain::Money ivaTotal, ivaExonerado;
            for (const auto& imp : linea.impuestos) {
                if (domain::esIVA(imp.codigo)) ivaTotal += imp.monto;
            }
            if (linea.exoneracion.has_value()) ivaExonerado = linea.exoneracion->montoExoneracion;

            if (ivaExonerado.isZero() || ivaTotal.isZero()) {
                gravado = base;
            } else {
                // Porcentaje de exoneración = monto exonerado / monto del impuesto
                int64_t ratio = 0;
                if (!domain::aritmetica128::multiplicarDividirTruncado(ivaExonerado.scaledValue(), domain::Money::SCALE, ivaTotal.scaledValue(), ratio)) {
                    throw std::overflow_error("Overflow al calcular el porcentaje de exoneración");
                }
                exonerado = base.multiplyScaled(ratio);
                gravado = base - exonerado;
            }
        }

        if (esServicio(linea.cabys, linea.unidadMedida)) {
            t.totalServiciosGravados += gravado;
            t.totalServiciosExentos += exento;
            t.totalServiciosExonerados += exonerado;
            t.totalServiciosNoSujetos += noSujeto;
        } else {
            t.totalMercanciasGravadas += gravado;
            t.totalMercanciasExentas += exento;
            t.totalMercanciasExoneradas += exonerado;
            t.totalMercanciasNoSujetas += noSujeto;
        }
    }

    static void acumularDesglose(const std::vector<PartidaImpuesto>& partidas,
                                 std::vector<domain::DesgloseImpuesto>& desglose) {
        for (const auto& part : partidas) {
            bool found = false;
            for (auto& d : desglose) {
                if (d.codigo == part.codigo && d.codigoTarifa == part.codigoTarifa) {
                    d.totalMonto += part.monto;
                    found = true;
                    break;
                }
            }
            if (!found) {
                desglose.push_back({ part.codigo, part.codigoTarifa, part.monto });
            }
        }
    }

    /**
     * @brief TotalIVADevuelto (Anexo 1 pág. 55, nota al pie 16). Si viene indicado se conserva (se valida después).
     *  - Solo servicios de salud pagados únicamente con tarjeta: el IVA de esos servicios.
     *  - Venta mixta: (monto pagado con tarjeta / venta neta) * IVA de salud al 4%, con el monto de la
     *    tarjeta definido de previo. Si no se indicó el monto de la tarjeta queda en 0 y la validación posterior lo reporta.
     *  - Nunca mayor al IVA de salud al 4%.
     */
    static domain::Money calcularIvaDevuelto(const domain::ComprobanteCanonico& comprobante, const domain::Money& ventaNeta) {
        if (comprobante.resumen.totalIvaDevuelto.isPositive()) return comprobante.resumen.totalIvaDevuelto;

        const domain::Money ivaSalud = ivaServiciosSalud(comprobante);
        if (!ivaSalud.isPositive()) return {};

        domain::Money montoTarjeta;
        bool hayTarjeta = false, tarjetaSinMonto = false, soloTarjeta = true;
        for (const auto& m : comprobante.mediosPago) {
            if (m.tipo == domain::MedioPago::Tarjeta) {
                hayTarjeta = true;
                montoTarjeta += m.totalMedioPago;
                tarjetaSinMonto = tarjetaSinMonto || m.totalMedioPago.isZero();
            } else {
                soloTarjeta = false;
            }
        }
        if (!hayTarjeta) return {};

        const bool soloSalud = std::all_of(comprobante.lineas.begin(), comprobante.lineas.end(),
            [](const domain::LineaDetalle& l) { return common::CabysClasificacion::EsServicioSalud(l.cabys); });
        if (soloSalud && soloTarjeta && comprobante.otrosCargos.empty()) return ivaSalud;
        if (tarjetaSinMonto || !ventaNeta.isPositive()) return {};

        int64_t proporcion = 0;
        if (!domain::aritmetica128::multiplicarDividirTruncado(montoTarjeta.scaledValue(), domain::Money::SCALE, ventaNeta.scaledValue(), proporcion)) {
            throw std::overflow_error("Overflow al calcular la proporción pagada con tarjeta");
        }
        const domain::Money devuelto = ivaSalud.multiplyScaled(proporcion);
        return devuelto > ivaSalud ? ivaSalud : devuelto;
    }

    /**
     * @brief Con un medio de pago sin monto se asigna el total; con varios sin monto se reparte en partes iguales.
     */
    static void asignarMediosPago(domain::ComprobanteCanonico& comprobante, const domain::Money& totalComprobante) {
        auto& medios = comprobante.mediosPago;
        if (medios.empty()) return;

        if (medios.size() == 1) {
            if (medios[0].totalMedioPago.isZero()) medios[0].totalMedioPago = totalComprobante;
            return;
        }
        for (const auto& m : medios) {
            if (!m.totalMedioPago.isZero()) return;
        }
        const int64_t totalScaled = totalComprobante.scaledValue();
        const int64_t parte = totalScaled / static_cast<int64_t>(medios.size());
        int64_t acumulado = 0;
        for (size_t i = 0; i < medios.size(); ++i) {
            const int64_t monto = (i + 1 == medios.size()) ? totalScaled - acumulado : parte;
            medios[i].totalMedioPago = domain::Money::fromScaled(monto);
            acumulado += monto;
        }
    }
};

} // namespace cec::calculation
