import Foundation
import ComprobantesElectronicosCoreC

/*
 * Conversión del request de Swift a los structs del ABI de C (cec_request_c y sus partes).
 * El core solo lee la memoria durante la llamada, así que una arena la reserva y la libera al terminar.
 */

/// Memoria de C para una llamada: cadenas terminadas en cero y listas de structs.
final class Arena {
    private var cadenas: [UnsafeMutablePointer<CChar>] = []
    private var bloques: [UnsafeMutableRawPointer] = []

    deinit {
        cadenas.forEach { free($0) }
        bloques.forEach { $0.deallocate() }
    }

    /// `const char*`; nil = no informado.
    func c(_ texto: String?) -> UnsafePointer<CChar>? {
        guard let texto, let p = strdup(texto) else { return nil }
        cadenas.append(p)
        return UnsafePointer(p)
    }

    /// Lista de structs (puntero + cantidad); nil si está vacía.
    func lista<T>(_ valores: [T]) -> (UnsafePointer<T>?, Int) {
        guard !valores.isEmpty else { return (nil, 0) }
        let p = UnsafeMutablePointer<T>.allocate(capacity: valores.count)
        p.initialize(from: valores, count: valores.count)
        bloques.append(UnsafeMutableRawPointer(p))
        return (UnsafePointer(p), valores.count)
    }

    /// Nodo opcional (puntero a un solo struct).
    func uno<T>(_ valor: T?) -> UnsafePointer<T>? {
        guard let valor else { return nil }
        return lista([valor]).0
    }

    /// `const char* const*` (lista de textos).
    func textos(_ valores: [String]?) -> (UnsafePointer<UnsafePointer<CChar>?>?, Int) {
        lista((valores ?? []).map { c($0) })
    }
}

/// Valor de CEC_SIN_VALOR (enteros opcionales no informados).
private let sinValor: Int32 = -1

extension Arena {
    func identificacion(_ i: Identificacion) -> cec_identificacion_c {
        cec_identificacion_c(tipo: c(i.tipo.rawValue), numero: c(i.numero))
    }

    func ubicacion(_ u: Ubicacion?) -> UnsafePointer<cec_ubicacion_c>? {
        uno(u.map { cec_ubicacion_c(provincia: c($0.provincia), canton: c($0.canton), distrito: c($0.distrito),
                                    barrio: c($0.barrio), otrasSenas: c($0.otrasSenas)) })
    }

    func telefono(_ t: Telefono?) -> UnsafePointer<cec_telefono_c>? {
        uno(t.map { cec_telefono_c(codigoPais: c($0.codigoPais), numTelefono: c($0.numTelefono)) })
    }

    func codigosComerciales(_ l: [CodigoComercial]?) -> (UnsafePointer<cec_codigo_comercial_c>?, Int) {
        lista((l ?? []).map { cec_codigo_comercial_c(tipo: c($0.tipo), codigo: c($0.codigo)) })
    }

    func descuentos(_ l: [Descuento]?) -> (UnsafePointer<cec_descuento_c>?, Int) {
        lista((l ?? []).map {
            cec_descuento_c(montoDescuento: c($0.montoDescuento.description), codigoDescuento: c($0.codigoDescuento),
                            codigoDescuentoOTRO: c($0.codigoDescuentoOTRO), naturalezaDescuento: c($0.naturalezaDescuento))
        })
    }

    func impuestos(_ l: [Impuesto]?) -> (UnsafePointer<cec_impuesto_c>?, Int) {
        lista((l ?? []).map { i in
            let datos = i.datosImpuestoEspecifico.map {
                cec_datos_impuesto_especifico_c(
                    cantidadUnidadMedida: c($0.cantidadUnidadMedida?.description), porcentaje: c($0.porcentaje?.description),
                    proporcion: c($0.proporcion?.description), volumenUnidadConsumo: c($0.volumenUnidadConsumo?.description),
                    impuestoUnidad: c($0.impuestoUnidad?.description))
            }
            return cec_impuesto_c(codigo: c(i.codigo.rawValue), codigoImpuestoOTRO: c(i.codigoImpuestoOTRO),
                                  codigoTarifaIVA: c(i.codigoTarifaIVA?.rawValue), tarifa: c(i.tarifa?.description),
                                  factorCalculoIVA: c(i.factorCalculoIVA?.description), datosImpuestoEspecifico: uno(datos),
                                  montoExportacion: c(i.montoExportacion?.description))
        })
    }

    func exoneracion(_ e: Exoneracion?) -> UnsafePointer<cec_exoneracion_c>? {
        uno(e.map {
            cec_exoneracion_c(tipoDocumentoEX1: c($0.tipoDocumentoEX1), tipoDocumentoOTRO: c($0.tipoDocumentoOTRO),
                              numeroDocumento: c($0.numeroDocumento), articulo: $0.articulo.map(Int32.init) ?? sinValor,
                              inciso: $0.inciso.map(Int32.init) ?? sinValor, nombreInstitucion: c($0.nombreInstitucion),
                              nombreInstitucionOtros: c($0.nombreInstitucionOtros), fechaEmisionEX: c($0.fechaEmisionEX),
                              tarifaExonerada: c($0.tarifaExonerada.description))
        })
    }

    func surtido(_ l: [LineaDetalleSurtido]?) -> (UnsafePointer<cec_linea_surtido_c>?, Int) {
        lista((l ?? []).map { s in
            var r = cec_linea_surtido_c()
            r.codigoCABYS = c(s.codigoCABYS)
            (r.codigosComerciales, r.numCodigosComerciales) = codigosComerciales(s.codigosComerciales)
            r.cantidad = c(s.cantidad.description)
            r.unidadMedida = c(s.unidadMedida)
            r.unidadMedidaComercial = c(s.unidadMedidaComercial)
            r.detalle = c(s.detalle)
            r.precioUnitario = c(s.precioUnitario.description)
            (r.descuentos, r.numDescuentos) = descuentos(s.descuentos)
            (r.impuestos, r.numImpuestos) = impuestos(s.impuestos)
            return r
        })
    }

    func linea(_ l: LineaDetalle) -> cec_linea_c {
        var r = cec_linea_c()
        r.numeroLinea = l.numeroLinea.map(Int32.init) ?? sinValor
        r.partidaArancelaria = c(l.partidaArancelaria)
        r.codigoCABYS = c(l.codigoCABYS)
        (r.codigosComerciales, r.numCodigosComerciales) = codigosComerciales(l.codigosComerciales)
        r.cantidad = c(l.cantidad?.description)
        r.unidadMedida = c(l.unidadMedida)
        r.tipoTransaccion = c(l.tipoTransaccion)
        r.unidadMedidaComercial = c(l.unidadMedidaComercial)
        r.detalle = c(l.detalle)
        (r.numerosVINoSerie, r.numNumerosVINoSerie) = textos(l.numerosVINoSerie)
        r.registroMedicamento = c(l.registroMedicamento)
        r.formaFarmaceutica = c(l.formaFarmaceutica)
        (r.detalleSurtido, r.numDetalleSurtido) = surtido(l.detalleSurtido)
        r.precioUnitario = c(l.precioUnitario?.description)
        r.montoTotal = c(l.montoTotal?.description)
        (r.descuentos, r.numDescuentos) = descuentos(l.descuentos)
        r.subTotal = c(l.subTotal?.description)
        r.ivaCobradoFabrica = c(l.ivaCobradoFabrica)
        r.baseImponible = c(l.baseImponible?.description)
        (r.impuestos, r.numImpuestos) = impuestos(l.impuestos)
        r.impuestoAsumidoEmisorFabrica = c(l.impuestoAsumidoEmisorFabrica?.description)
        r.exoneracion = exoneracion(l.exoneracion)
        return r
    }

    func request(_ q: ComprobanteRequest) -> cec_request_c {
        var r = cec_request_c()
        r.tipoDocumento = c(q.tipoDocumento.rawValue)
        r.clave = c(q.clave)
        r.proveedorSistemas = c(q.proveedorSistemas)
        r.codigoActividadEmisor = c(q.codigoActividadEmisor)
        r.codigoActividadReceptor = c(q.codigoActividadReceptor)
        r.numeroConsecutivo = c(q.numeroConsecutivo)
        r.fechaEmision = c(q.fechaEmision)

        r.emisor.nombre = c(q.emisor.nombre)
        r.emisor.identificacion = identificacion(q.emisor.identificacion)
        r.emisor.registroFiscal8707 = c(q.emisor.registroFiscal8707)
        r.emisor.nombreComercial = c(q.emisor.nombreComercial)
        r.emisor.ubicacion = ubicacion(q.emisor.ubicacion)
        r.emisor.otrasSenasExtranjero = c(q.emisor.otrasSenasExtranjero)
        r.emisor.telefono = telefono(q.emisor.telefono)
        (r.emisor.correoElectronico, r.emisor.numCorreoElectronico) = textos(q.emisor.correoElectronico)

        r.receptor = uno(q.receptor.map { rec in
            var x = cec_receptor_c()
            x.nombre = c(rec.nombre)
            x.identificacion = uno(rec.identificacion.map(identificacion))
            x.nombreComercial = c(rec.nombreComercial)
            x.ubicacion = ubicacion(rec.ubicacion)
            x.otrasSenasExtranjero = c(rec.otrasSenasExtranjero)
            x.telefono = telefono(rec.telefono)
            x.correoElectronico = c(rec.correoElectronico)
            return x
        })

        r.condicionVenta = c(q.condicionVenta.rawValue)
        r.condicionVentaOtros = c(q.condicionVentaOtros)
        r.plazoCredito = q.plazoCredito.map(Int32.init) ?? sinValor
        (r.detalleServicio, r.numDetalleServicio) = lista((q.detalleServicio ?? []).map(linea))
        (r.otrosCargos, r.numOtrosCargos) = lista((q.otrosCargos ?? []).map {
            cec_otro_cargo_c(tipoDocumentoOC: c($0.tipoDocumentoOC), tipoDocumentoOTROS: c($0.tipoDocumentoOTROS),
                             identificacionTercero: uno($0.identificacionTercero.map(identificacion)),
                             nombreTercero: c($0.nombreTercero), detalle: c($0.detalle),
                             porcentajeOC: c($0.porcentajeOC?.description), montoCargo: c($0.montoCargo.description))
        })
        r.codigoMoneda = c(q.resumenFactura.codigoTipoMoneda.codigoMoneda)
        r.tipoCambio = c(q.resumenFactura.codigoTipoMoneda.tipoCambio?.description)
        (r.mediosPago, r.numMediosPago) = lista((q.resumenFactura.mediosPago ?? []).map {
            cec_medio_pago_c(tipoMedioPago: c($0.tipoMedioPago.rawValue), medioPagoOtros: c($0.medioPagoOtros),
                             totalMedioPago: c($0.totalMedioPago?.description))
        })
        (r.informacionReferencia, r.numInformacionReferencia) = lista((q.informacionReferencia ?? []).map {
            cec_referencia_c(tipoDocIR: c($0.tipoDocIR), tipoDocRefOTRO: c($0.tipoDocRefOTRO), numero: c($0.numero),
                             fechaEmisionIR: c($0.fechaEmisionIR), codigo: c($0.codigo),
                             codigoReferenciaOTRO: c($0.codigoReferenciaOTRO), razon: c($0.razon))
        })
        return r
    }
}
