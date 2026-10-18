#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include "cec/visitor/IEscritorArbol.h"
#include "cec/domain/Comprobante.h"

namespace cec::visitor {

/**
 * @brief Recorre un comprobante calculado en el orden y con las condiciones del Anexo 1 v4.4
 * ("Especificaciones Técnicas y Formato de las Estructuras de los Comprobantes Electrónicos").
 *
 * Es el único lugar que sabe qué nodo va en qué comprobante, en qué orden y con cuántos decimales.
 * Los formatos en árbol (XML, JSON, proto) solo implementan IEscritorArbol.
 *
 * Cada campo lleva su "Condición del Campo" tal como aparece en el Anexo 1, como 7 dígitos en el
 * orden FE FEE FEC TE NC ND REP:
 *   1 = obligatorio: se emite siempre.
 *   2 = condicional, 3 = opcional: se emite si el comprobante trae el dato.
 *   4 = inexistente: nunca se emite.
 * Que un dato obligatorio falte o uno inexistente venga informado lo reportan las reglas, no el recorrido.
 */
class RecorridoComprobante {
public:
    static void recorrer(const domain::ComprobanteCanonico& inv, IEscritorArbol& w) {
        RecorridoComprobante(inv, w).documento();
    }

private:
    using Money = domain::Money;
    using Cond = std::string_view;

    const domain::ComprobanteCanonico& cec_;
    IEscritorArbol& w_;
    size_t col_; // Columna del tipo de comprobante en la condición

    RecorridoComprobante(const domain::ComprobanteCanonico& inv, IEscritorArbol& w)
        : cec_(inv), w_(w), col_(columna(inv.tipoDocumento)) {}

    static size_t columna(domain::TipoDocumento t) noexcept {
        switch (t) {
            case domain::TipoDocumento::FacturaElectronica:            return 0;
            case domain::TipoDocumento::FacturaElectronicaExportacion: return 1;
            case domain::TipoDocumento::FacturaElectronicaCompra:      return 2;
            case domain::TipoDocumento::TiqueteElectronico:            return 3;
            case domain::TipoDocumento::NotaCreditoElectronica:        return 4;
            case domain::TipoDocumento::NotaDebitoElectronica:         return 5;
            case domain::TipoDocumento::ReciboElectronicoPago:         return 6;
        }
        return 0;
    }

    // ── Condiciones ──────────────────────────────────────────────────────────────────────────────

    bool existe(Cond c) const noexcept { return c[col_] != '4'; }
    bool emitir(Cond c, bool hayDato) const noexcept { return existe(c) && (c[col_] == '1' || hayDato); }

    // ── Campos hoja ──────────────────────────────────────────────────────────────────────────────

    void texto(std::string_view nombre, Cond c, std::string_view v) {
        if (emitir(c, !v.empty())) w_.campo(nombre, v, TipoValor::Texto);
    }

    void entero(std::string_view nombre, Cond c, std::string_view v) {
        if (emitir(c, !v.empty())) w_.campo(nombre, v, TipoValor::Entero);
    }

    void entero(std::string_view nombre, Cond c, std::optional<int> v) {
        if (emitir(c, v.has_value())) w_.campo(nombre, std::to_string(v.value_or(0)), TipoValor::Entero);
    }

    /// @brief Monto Decimal 18,5. Los condicionales se omiten en cero.
    void monto(std::string_view nombre, Cond c, const Money& m) {
        if (emitir(c, !m.isZero())) w_.campo(nombre, m.toString(5), TipoValor::Decimal);
    }

    /// @brief Valor escalado x100000 con los decimales del campo (Cantidad 16,3; Tarifa 4,2; Factor 5,4...).
    void escalado(std::string_view nombre, Cond c, std::optional<int64_t> v, int decimales) {
        if (emitir(c, v.has_value())) {
            w_.campo(nombre, Money::fromScaled(v.value_or(0)).toString(decimales), TipoValor::Decimal);
        }
    }

    // ── Documento ────────────────────────────────────────────────────────────────────────────────

    void documento() {
        w_.abrirDocumento(domain::getTipoDocumentoXmlTagName(cec_.tipoDocumento),
                          domain::getTipoDocumentoNamespace(cec_.tipoDocumento));

        texto("Clave",                   "1111111", cec_.clave);
        texto("ProveedorSistemas",       "1111111", cec_.proveedorSistemas);
        texto("CodigoActividadEmisor",   "1131224", cec_.codigoActividadEmisor);
        texto("CodigoActividadReceptor", "2414224", cec_.codigoActividadReceptor);
        texto("NumeroConsecutivo",       "1111111", cec_.numeroConsecutivo);
        texto("FechaEmision",            "1111111", cec_.fechaEmision);

        emisor();
        if (cec_.receptor.has_value() && existe("1212221")) receptor(*cec_.receptor);

        texto("CondicionVenta", "1111111", domain::getCondicionVentaCode(cec_.condicionVenta));
        if (cec_.condicionVenta == domain::CondicionVenta::Otros) {
            texto("CondicionVentaOtros", "2222224", cec_.condicionVentaOtros);
        }
        entero("PlazoCredito", "2222224", cec_.plazoCredito);

        if (!cec_.lineas.empty() && existe("2112221")) detalleServicio();
        otrosCargos();
        resumen();
        informacionReferencia();

        w_.cerrarDocumento();
    }

    // ── Encabezado ───────────────────────────────────────────────────────────────────────────────

    void identificacion(std::string_view nombre, const domain::Identificacion& id) {
        w_.abrir(nombre);
        texto("Tipo",   "1111111", domain::getTipoIdentificacionCode(id.tipo));
        texto("Numero", "1111111", id.numero);
        w_.cerrar();
    }

    /// @brief Ubicación. `condBarrio` y `condResto` cambian entre emisor y receptor.
    void ubicacion(const domain::Ubicacion& u, Cond condResto, Cond condBarrio) {
        w_.abrir("Ubicacion");
        texto("Provincia",  condResto, u.provincia);
        texto("Canton",     condResto, u.canton);
        texto("Distrito",   condResto, u.distrito);
        texto("Barrio",     condBarrio, u.barrio.value_or(""));
        texto("OtrasSenas", condResto, u.otrasSenas);
        w_.cerrar();
    }

    void telefono(const domain::Telefono& t) {
        if (t.numero.empty() || !existe("3333334")) return;
        w_.abrir("Telefono");
        entero("CodigoPais",  "1111114", t.codigoPais);
        entero("NumTelefono", "1111114", t.numero);
        w_.cerrar();
    }

    void emisor() {
        const auto& e = cec_.emisor;
        w_.abrir("Emisor");
        texto("Nombre", "1111111", e.nombre);
        identificacion("Identificacion", e.identificacion);
        texto("Registrofiscal8707", "2222224", e.registroFiscal8707);
        texto("NombreComercial",    "3333334", e.nombreComercial);
        if (e.ubicacion.has_value() && existe("1121224")) ubicacion(*e.ubicacion, "1111114", "3333224");
        texto("OtrasSenasExtranjero", "4424444", e.otrasSenasExtranjero.value_or(""));
        if (e.telefono.has_value()) telefono(*e.telefono);
        if (!e.correoElectronico.empty() && existe("1121221")) {
            w_.abrirLista("CorreoElectronico");
            for (const auto& correo : e.correoElectronico) texto("CorreoElectronico", "1111111", correo);
            w_.cerrarLista();
        }
        w_.cerrar();
    }

    void receptor(const domain::Receptor& r) {
        w_.abrir("Receptor");
        texto("Nombre", "1111111", r.nombre);
        if (r.identificacion.has_value() && existe("1112221")) identificacion("Identificacion", *r.identificacion);
        texto("NombreComercial", "3333334", r.nombreComercial);
        if (r.ubicacion.has_value() && existe("2423224")) ubicacion(*r.ubicacion, "1411114", "3432224");
        texto("OtrasSenasExtranjero", "2243224", r.otrasSenasExtranjero.value_or(""));
        if (r.telefono.has_value()) telefono(*r.telefono);
        texto("CorreoElectronico", "2222222", r.correoElectronico);
        w_.cerrar();
    }

    // ── Detalle ──────────────────────────────────────────────────────────────────────────────────

    void detalleServicio() {
        w_.abrir("DetalleServicio");
        w_.abrirLista("LineaDetalle");
        for (const auto& l : cec_.lineas) lineaDetalle(l);
        w_.cerrarLista();
        w_.cerrar();
    }

    void lineaDetalle(const domain::LineaDetalle& l) {
        const bool esSurtido = !l.surtido.empty();
        w_.abrir("LineaDetalle");
        entero("NumeroLinea",        "1111111", std::to_string(l.numeroLinea));
        texto("PartidaArancelaria",  "4244224", l.partidaArancelaria);
        texto("CodigoCABYS",         "1111224", l.cabys);
        if (!l.codigosComerciales.empty() && existe("2222224")) {
            w_.abrirLista("CodigoComercial");
            for (const auto& cc : l.codigosComerciales) {
                w_.abrir("CodigoComercial");
                texto("Tipo",   "1111114", cc.tipo);
                texto("Codigo", "1111114", cc.codigo);
                w_.cerrar();
            }
            w_.cerrarLista();
        }
        escalado("Cantidad",        "1111114", l.cantidadScaled, 3);
        texto("UnidadMedida",       "1111114", l.unidadMedida);
        texto("TipoTransaccion",    "2224224", l.tipoTransaccion);
        texto("UnidadMedidaComercial", "3333334", l.unidadMedidaComercial);
        texto("Detalle",            "1111111", l.descripcion);
        if (!l.numerosVIN.empty() && existe("2222224")) {
            w_.abrirLista("NumeroVINoSerie");
            for (const auto& vin : l.numerosVIN) texto("NumeroVINoSerie", "1111111", vin);
            w_.cerrarLista();
        }
        texto("RegistroMedicamento", "2222224", l.registroMedicamento);
        texto("FormaFarmaceutica",   "2222224", l.formaFarmaceutica);
        if (esSurtido && existe("2242224")) detalleSurtido(l);
        monto("PrecioUnitario", "1111114", l.precioUnitario);
        monto("MontoTotal",     "1111111", l.montoTotal);
        if (!l.descuentos.empty() && existe("3333224")) {
            w_.abrirLista("Descuento");
            for (const auto& d : l.descuentos) {
                w_.abrir("Descuento");
                monto("MontoDescuento",      "1111114", d.monto);
                texto("CodigoDescuento",     "1111114", d.codigo);
                texto("CodigoDescuentoOTRO", "2222224", d.codigoDescuentoOTRO);
                texto("NaturalezaDescuento", "2222224", d.naturaleza);
                w_.cerrar();
            }
            w_.cerrarLista();
        }
        monto("SubTotal",          "1111111", l.subTotal);
        texto("IVACobradoFabrica", "2442224", l.ivaCobradoFabrica);
        monto("BaseImponible",     "1411114", l.baseImponible);
        if (!l.impuestos.empty() && existe("1111112")) {
            // La exoneración (Nota 10.1) va dentro del Impuesto de IVA que exonera.
            const domain::ImpuestoLinea* impuestoExonerado = nullptr;
            if (l.exoneracion.has_value()) {
                for (const auto& imp : l.impuestos) {
                    if (domain::esIVA(imp.codigo)) { impuestoExonerado = &imp; break; }
                }
            }
            w_.abrirLista("Impuesto");
            for (const auto& imp : l.impuestos) {
                impuesto(imp, esSurtido, &imp == impuestoExonerado ? &*l.exoneracion : nullptr);
            }
            w_.cerrarLista();
        }
        monto("ImpuestoAsumidoEmisorFabrica", "1441224", l.impuestoAsumidoEmisorFabrica);
        monto("ImpuestoNeto",                 "1411111", l.impuestoNeto);
        monto("MontoTotalLinea",              "1111111", l.montoTotalLinea);
        w_.cerrar();
    }

    /// @brief Tarifa (Decimal 4,2): impuestos tarifarios. Los específicos (03-06) y el IVA de bienes usados (08) no la llevan.
    static bool llevaTarifa(domain::CodigoImpuesto c) noexcept {
        using enum domain::CodigoImpuesto;
        return c == IVA || c == SelectivoConsumo || c == IVACalculoEspecial || c == Cemento || c == Otros;
    }

    /// @brief CodigoTarifaIVA: obligatorio para los códigos 01 y 07 (Nota 8.1).
    static bool llevaCodigoTarifaIVA(domain::CodigoImpuesto c) noexcept {
        return c == domain::CodigoImpuesto::IVA || c == domain::CodigoImpuesto::IVACalculoEspecial;
    }

    /// @brief Impuestos específicos no tarifarios (03, 04, 05, 06) con DatosImpuestoEspecifico.
    static bool esEspecifico(domain::CodigoImpuesto c) noexcept {
        using enum domain::CodigoImpuesto;
        return c == Combustibles || c == Alcoholicas || c == BebidasEnvasadasJabones || c == Tabaco;
    }

    /// @brief Impuesto de la línea. Con surtido, CodigoTarifaIVA, Tarifa y DatosImpuestoEspecifico no se usan:
    /// el monto es la suma de los impuestos de los componentes.
    void impuesto(const domain::ImpuestoLinea& imp, bool esSurtido, const domain::ExoneracionLinea* exoneracion) {
        w_.abrir("Impuesto");
        texto("Codigo", "1111111", domain::getCodigoImpuestoCode(imp.codigo));
        if (imp.codigo == domain::CodigoImpuesto::Otros) {
            texto("CodigoImpuestoOTRO", "2222222", imp.codigoImpuestoOTRO);
        }
        if (!esSurtido && llevaCodigoTarifaIVA(imp.codigo)) {
            texto("CodigoTarifaIVA", "2222222", domain::getCodigoTarifaIVACode(imp.codigoTarifa));
        }
        if (!esSurtido && llevaTarifa(imp.codigo)) {
            escalado("Tarifa", "2222222", imp.tarifaPorcentajeScaled, 2);
        }
        if (imp.codigo == domain::CodigoImpuesto::IVABienesUsados) {
            escalado("FactorCalculoIVA", "2222222", imp.factorCalculoIVAScaled, 4);
        }
        if (!esSurtido && esEspecifico(imp.codigo) && imp.datosEspecificos.has_value() && existe("2442224")) {
            datosImpuestoEspecifico(*imp.datosEspecificos, "");
        }
        monto("Monto",            "1111111", imp.monto);
        monto("MontoExportacion", "4244224", imp.montoExportacion);
        if (exoneracion != nullptr && existe("2422224")) this->exoneracion(*exoneracion);
        w_.cerrar();
    }

    /// @brief DatosImpuestoEspecifico de la línea (sufijo "") o del componente de surtido (sufijo "Surtido").
    void datosImpuestoEspecifico(const domain::DatosImpuestoEspecifico& d, std::string_view sufijo) {
        const std::string s(sufijo);
        w_.abrir("DatosImpuestoEspecifico" + s);
        escalado("CantidadUnidadMedida" + s, "1441114", d.cantidadUnidadMedidaScaled, 2);
        escalado("Porcentaje" + s,           "2442224", d.porcentajeScaled, 2);
        escalado("Proporcion" + s,           "2442224", d.proporcionScaled, 5);
        escalado("VolumenUnidadConsumo" + s, "2442224", d.volumenUnidadConsumoScaled, 2);
        monto("ImpuestoUnidad" + s,          "1441114", d.impuestoUnidad);
        w_.cerrar();
    }

    void exoneracion(const domain::ExoneracionLinea& ex) {
        w_.abrir("Exoneracion");
        texto("TipoDocumentoEX1",       "1411114", ex.tipoDocumento);
        texto("TipoDocumentoOTRO",      "2422224", ex.tipoDocumentoOTRO);
        texto("NumeroDocumento",        "1411114", ex.numeroDocumento);
        entero("Articulo",              "2422224", ex.articulo);
        entero("Inciso",                "2422224", ex.inciso);
        texto("NombreInstitucion",      "1411114", ex.nombreInstitucion);
        texto("NombreInstitucionOtros", "2422224", ex.nombreInstitucionOtros);
        texto("FechaEmisionEX",         "1411114", ex.fechaEmision);
        escalado("TarifaExonerada",     "1411114", ex.porcentajeExoneracionScaled, 2);
        monto("MontoExoneracion",       "1411114", ex.montoExoneracion);
        w_.cerrar();
    }

    void detalleSurtido(const domain::LineaDetalle& l) {
        w_.abrir("DetalleSurtido");
        w_.abrirLista("LineaDetalleSurtido");
        for (const auto& comp : l.surtido) {
            w_.abrir("LineaDetalleSurtido");
            texto("CodigoCABYSSurtido", "1141114", comp.codigoCABYS);
            if (!comp.codigosComerciales.empty() && existe("3342224")) {
                w_.abrirLista("CodigoComercialSurtido");
                for (const auto& cc : comp.codigosComerciales) {
                    w_.abrir("CodigoComercialSurtido");
                    texto("TipoSurtido",   "1141114", cc.tipo);
                    texto("CodigoSurtido", "1141114", cc.codigo);
                    w_.cerrar();
                }
                w_.cerrarLista();
            }
            escalado("CantidadSurtido",             "1141114", comp.cantidadScaled, 3);
            texto("UnidadMedidaSurtido",            "1141114", comp.unidadMedida);
            texto("UnidadMedidaComercialSurtido",   "3342224", comp.unidadMedidaComercial);
            texto("DetalleSurtido",                 "1141114", comp.descripcion);
            monto("PrecioUnitarioSurtido",          "1141114", comp.precioUnitario);
            monto("MontoTotalSurtido",              "1141114", comp.montoTotal);
            if (!comp.descuentos.empty() && existe("3342224")) {
                w_.abrirLista("DescuentoSurtido");
                for (const auto& d : comp.descuentos) {
                    w_.abrir("DescuentoSurtido");
                    monto("MontoDescuentoSurtido",  "1141114", d.monto);
                    texto("CodigoDescuentoSurtido", "1141114", d.codigo);
                    texto("DescuentoSurtidoOtros",  "2242224", d.naturaleza);
                    w_.cerrar();
                }
                w_.cerrarLista();
            }
            monto("SubTotalSurtido",      "1141114", comp.subTotal);
            monto("BaseImponibleSurtido", "1441114", comp.baseImponible);
            if (!comp.impuestos.empty()) {
                w_.abrirLista("ImpuestoSurtido");
                for (const auto& imp : comp.impuestos) {
                    w_.abrir("ImpuestoSurtido");
                    texto("CodigoImpuestoSurtido", "1141114", domain::getCodigoImpuestoCode(imp.codigo));
                    if (imp.codigo == domain::CodigoImpuesto::Otros) {
                        texto("CodigoImpuestoOTROSurtido", "2242224", imp.codigoImpuestoOTRO);
                    }
                    if (llevaCodigoTarifaIVA(imp.codigo)) {
                        texto("CodigoTarifaIVASurtido", "2242224", domain::getCodigoTarifaIVACode(imp.codigoTarifa));
                    }
                    if (llevaTarifa(imp.codigo)) {
                        escalado("TarifaSurtido", "2242224", imp.tarifaPorcentajeScaled, 2);
                    }
                    if (esEspecifico(imp.codigo) && imp.datosEspecificos.has_value() && existe("2442224")) {
                        datosImpuestoEspecifico(*imp.datosEspecificos, "Surtido");
                    }
                    monto("MontoImpuestoSurtido", "1141114", imp.monto);
                    w_.cerrar();
                }
                w_.cerrarLista();
            }
            w_.cerrar();
        }
        w_.cerrarLista();
        w_.cerrar();
    }

    // ── Otros cargos ─────────────────────────────────────────────────────────────────────────────

    void otrosCargos() {
        if (cec_.otrosCargos.empty() || !existe("2222224")) return;
        w_.abrirLista("OtrosCargos");
        for (const auto& oc : cec_.otrosCargos) {
            w_.abrir("OtrosCargos");
            texto("TipoDocumentoOC",    "1111114", oc.tipoDocumento);
            texto("TipoDocumentoOTROS", "2222224", oc.tipoDocumentoOTROS);
            if (!oc.numeroIdentidadTercero.empty() && existe("2222224")) {
                w_.abrir("IdentificacionTercero");
                if (oc.tipoIdentidadTercero.has_value()) {
                    texto("Tipo", "1111114", domain::getTipoIdentificacionCode(*oc.tipoIdentidadTercero));
                }
                texto("Numero", "1111114", oc.numeroIdentidadTercero);
                w_.cerrar();
            }
            texto("NombreTercero", "2422224", oc.nombreTercero);
            texto("Detalle",       "1111114", oc.detalle);
            escalado("PorcentajeOC", "2222224", oc.porcentajeScaled, 5);
            monto("MontoCargo",    "1111114", oc.montoCargo);
            w_.cerrar();
        }
        w_.cerrarLista();
    }

    // ── Resumen ──────────────────────────────────────────────────────────────────────────────────

    void resumen() {
        const auto& r = cec_.resumen;
        w_.abrir("ResumenFactura");

        w_.abrir("CodigoTipoMoneda");
        texto("CodigoMoneda", "1111111", r.codigoTipoMoneda.codigoMoneda);
        escalado("TipoCambio", "1111111", r.codigoTipoMoneda.tipoCambioScaled, 5);
        w_.cerrar();

        monto("TotalServGravados",       "2222224", r.totalServiciosGravados);
        monto("TotalServExentos",        "2222224", r.totalServiciosExentos);
        monto("TotalServExonerado",      "2422224", r.totalServiciosExonerados);
        monto("TotalServNoSujeto",       "2422224", r.totalServiciosNoSujetos);
        monto("TotalMercanciasGravadas", "2222224", r.totalMercanciasGravadas);
        monto("TotalMercanciasExentas",  "2222224", r.totalMercanciasExentas);
        monto("TotalMercExonerada",      "2422224", r.totalMercanciasExoneradas);
        monto("TotalMercNoSujeta",       "2422224", r.totalMercanciasNoSujetas);
        monto("TotalGravado",            "2222224", r.totalGravado);
        monto("TotalExento",             "2222224", r.totalExento);
        monto("TotalExonerado",          "2422224", r.totalExonerado);
        monto("TotalNoSujeto",           "2422224", r.totalNoSujeto);
        monto("TotalVenta",              "1111111", r.totalVenta);
        monto("TotalDescuentos",         "2222224", r.totalDescuentos);
        monto("TotalVentaNeta",          "1111111", r.totalVentaNeta);

        if (!r.desgloseImpuestos.empty() && existe("2222222")) {
            w_.abrirLista("TotalDesgloseImpuesto");
            for (const auto& d : r.desgloseImpuestos) {
                w_.abrir("TotalDesgloseImpuesto");
                texto("Codigo", "1111111", domain::getCodigoImpuestoCode(d.codigo));
                if (llevaCodigoTarifaIVA(d.codigo)) {
                    texto("CodigoTarifaIVA", "2222222", domain::getCodigoTarifaIVACode(d.codigoTarifa));
                }
                monto("TotalMontoImpuesto", "1111111", d.totalMonto);
                w_.cerrar();
            }
            w_.cerrarLista();
        }

        monto("TotalImpuesto",             "2222222", r.totalImpuesto);
        monto("TotalImpAsumEmisorFabrica", "2222224", r.totalImpAsumEmisorFabrica);
        monto("TotalIVADevuelto",          "2442224", r.totalIvaDevuelto);
        monto("TotalOtrosCargos",          "2222224", r.totalOtrosCargos);

        if (!cec_.mediosPago.empty() && existe("2222221")) {
            w_.abrirLista("MedioPago");
            for (const auto& m : cec_.mediosPago) {
                w_.abrir("MedioPago");
                texto("TipoMedioPago", "2222221", domain::getMedioPagoCode(m.tipo));
                if (m.tipo == domain::MedioPago::Otros) texto("MedioPagoOtros", "2222222", m.otros);
                monto("TotalMedioPago", "2222222", m.totalMedioPago);
                w_.cerrar();
            }
            w_.cerrarLista();
        }

        monto("TotalComprobante", "1111111", r.totalComprobante);
        w_.cerrar();
    }

    // ── Información de referencia ───────────────────────────────────────────────────────────────

    void informacionReferencia() {
        if (cec_.referencias.empty() || !existe("2313111")) return;
        w_.abrirLista("InformacionReferencia");
        for (const auto& ref : cec_.referencias) {
            w_.abrir("InformacionReferencia");
            texto("TipoDocIR",            "1111111", ref.tipoDoc);
            texto("TipoDocRefOTRO",       "2222222", ref.tipoDocRefOTRO);
            texto("Numero",               "2222221", ref.numero);
            texto("FechaEmisionIR",       "1111111", ref.fechaEmision);
            texto("Codigo",               "2222222", ref.codigo);
            texto("CodigoReferenciaOTRO", "2222222", ref.codigoReferenciaOTRO);
            texto("Razon",                "2222222", ref.razon);
            w_.cerrar();
        }
        w_.cerrarLista();
    }
};

} // namespace cec::visitor
