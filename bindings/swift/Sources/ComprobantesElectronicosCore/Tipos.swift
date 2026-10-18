import Foundation

/*
 * Request de un comprobante electrónico v4.4: el mismo JSON que define schemas/comprobante_request.schema.json.
 * Solo datos de entrada (los montos calculados los produce el core). Cada propiedad se llama como su
 * etiqueta del Anexo 1 en camelCase y las listas van en plural. La obligatoriedad de cada campo según el
 * tipo de comprobante la valida el motor de reglas.
 */

// MARK: - Valores

/// Monto o cantidad. Se serializa como texto ("1500.5") para no perder precisión.
public struct Monto: Codable, Hashable, Sendable, CustomStringConvertible,
                     ExpressibleByStringLiteral, ExpressibleByIntegerLiteral {
    public let valor: Decimal

    public init(_ valor: Decimal) { self.valor = valor }

    /// Texto decimal con punto ("1500.50"); nil si no es un número.
    public init?(_ texto: String) {
        guard let d = Decimal(string: texto, locale: Locale(identifier: "en_US_POSIX")) else { return nil }
        valor = d
    }

    public init(stringLiteral texto: String) {
        guard let m = Monto(texto) else { preconditionFailure("Monto inválido: \(texto)") }
        self = m
    }

    public init(integerLiteral entero: Int) { valor = Decimal(entero) }

    public var description: String { NSDecimalNumber(decimal: valor).description(withLocale: Locale(identifier: "en_US_POSIX")) }

    public init(from decoder: Decoder) throws {
        let c = try decoder.singleValueContainer()
        if let texto = try? c.decode(String.self), let m = Monto(texto) { self = m; return }
        valor = try c.decode(Decimal.self)
    }

    public func encode(to encoder: Encoder) throws {
        var c = encoder.singleValueContainer()
        try c.encode(description)
    }
}

/// Código de un catálogo del Anexo 1 ("01", "99"...). Los catálogos principales tienen su enum.
public typealias Codigo = String

// MARK: - Códigos de catálogo

/// Nota 3.
public enum TipoDocumento: String, Codable, Sendable, CaseIterable {
    case facturaElectronica = "01"
    case notaDebito = "02"
    case notaCredito = "03"
    case tiqueteElectronico = "04"
    case facturaCompra = "08"
    case facturaExportacion = "09"
    case reciboPago = "10"
}

/// Nota 4.
public enum TipoIdentificacion: String, Codable, Sendable, CaseIterable {
    case fisica = "01"
    case juridica = "02"
    case dimex = "03"
    case nite = "04"
    case extranjeroNoDomiciliado = "05"
    case noContribuyente = "06"
}

/// Nota 5.
public enum CondicionVenta: String, Codable, Sendable, CaseIterable {
    case contado = "01"
    case credito = "02"
    case consignacion = "03"
    case apartado = "04"
    case arrendamientoOpcionCompra = "05"
    case arrendamientoFuncionFinanciera = "06"
    case cobroAFavorDeTercero = "07"
    case serviciosEstadoACredito = "08"
    case pagoServiciosEstado = "09"
    case ventaCreditoIVA90Dias = "10"
    case pagoVentaCreditoIVA90Dias = "11"
    case ventaMercanciaNoNacionalizada = "12"
    case ventaBienesUsadosNoContribuyente = "13"
    case arrendamientoOperativo = "14"
    case arrendamientoFinanciero = "15"
    case otros = "99"
}

/// Nota 6.
public enum TipoMedioPago: String, Codable, Sendable, CaseIterable {
    case efectivo = "01"
    case tarjeta = "02"
    case cheque = "03"
    case transferencia = "04"
    case recaudadoPorTerceros = "05"
    case sinpeMovil = "06"
    case plataformaDigital = "07"
    case otros = "99"
}

/// Nota 8.
public enum CodigoImpuesto: String, Codable, Sendable, CaseIterable {
    case iva = "01"
    case selectivoConsumo = "02"
    case combustibles = "03"
    case bebidasAlcoholicas = "04"
    case bebidasSinAlcoholYJabones = "05"
    case tabaco = "06"
    case ivaCalculoEspecial = "07"
    case ivaBienesUsados = "08"
    case cemento = "12"
    case otros = "99"
}

/// Nota 8.1.
public enum CodigoTarifaIVA: String, Codable, Sendable, CaseIterable {
    case tarifa0 = "01"
    case reducida1 = "02"
    case reducida2 = "03"
    case reducida4 = "04"
    case transitoria0 = "05"
    case transitoria4 = "06"
    case transitoria8 = "07"
    case general13 = "08"
    case reducida05 = "09"
    case exenta = "10"
    case tarifa0SinCredito = "11"
}

/// Situación del comprobante en la clave (Nota 3).
public enum Situacion: String, Codable, Sendable, CaseIterable {
    case normal = "1"
    case contingencia = "2"
    case sinInternet = "3"
}

// MARK: - Request

public struct Identificacion: Codable, Hashable, Sendable {
    public var tipo: TipoIdentificacion
    public var numero: String
    public init(tipo: TipoIdentificacion, numero: String) { self.tipo = tipo; self.numero = numero }
}

/// Códigos de la Nota 14: provincia de 1 dígito, cantón y distrito de 2 ("08"). El barrio es texto.
public struct Ubicacion: Codable, Hashable, Sendable {
    public var provincia: String
    public var canton: String
    public var distrito: String
    public var barrio: String?
    public var otrasSenas: String
    public init(provincia: String, canton: String, distrito: String, barrio: String? = nil, otrasSenas: String) {
        self.provincia = provincia; self.canton = canton; self.distrito = distrito; self.barrio = barrio; self.otrasSenas = otrasSenas
    }
}

public struct Telefono: Codable, Hashable, Sendable {
    public var codigoPais: String
    public var numTelefono: String
    public init(codigoPais: String = "506", numTelefono: String) { self.codigoPais = codigoPais; self.numTelefono = numTelefono }
}

public struct Emisor: Codable, Hashable, Sendable {
    public var nombre: String
    public var identificacion: Identificacion
    public var registroFiscal8707: String?
    public var nombreComercial: String?
    public var ubicacion: Ubicacion?
    public var otrasSenasExtranjero: String?
    public var telefono: Telefono?
    /// De 1 a 4 correos.
    public var correoElectronico: [String]?
    public init(nombre: String, identificacion: Identificacion, registroFiscal8707: String? = nil, nombreComercial: String? = nil,
                ubicacion: Ubicacion? = nil, otrasSenasExtranjero: String? = nil, telefono: Telefono? = nil,
                correoElectronico: [String]? = nil) {
        self.nombre = nombre; self.identificacion = identificacion; self.registroFiscal8707 = registroFiscal8707
        self.nombreComercial = nombreComercial; self.ubicacion = ubicacion; self.otrasSenasExtranjero = otrasSenasExtranjero
        self.telefono = telefono; self.correoElectronico = correoElectronico
    }
}

public struct Receptor: Codable, Hashable, Sendable {
    public var nombre: String
    public var identificacion: Identificacion?
    public var nombreComercial: String?
    public var ubicacion: Ubicacion?
    public var otrasSenasExtranjero: String?
    public var telefono: Telefono?
    public var correoElectronico: String?
    public init(nombre: String, identificacion: Identificacion? = nil, nombreComercial: String? = nil, ubicacion: Ubicacion? = nil,
                otrasSenasExtranjero: String? = nil, telefono: Telefono? = nil, correoElectronico: String? = nil) {
        self.nombre = nombre; self.identificacion = identificacion; self.nombreComercial = nombreComercial; self.ubicacion = ubicacion
        self.otrasSenasExtranjero = otrasSenasExtranjero; self.telefono = telefono; self.correoElectronico = correoElectronico
    }
}

/// Nota 12. Con CABYS de surtido se exige el tipo 03 (fabricante).
public struct CodigoComercial: Codable, Hashable, Sendable {
    public var tipo: Codigo
    public var codigo: String
    public init(tipo: Codigo, codigo: String) { self.tipo = tipo; self.codigo = codigo }
}

/// Nota 20.
public struct Descuento: Codable, Hashable, Sendable {
    public var montoDescuento: Monto
    public var codigoDescuento: Codigo
    /// Con código 99.
    public var codigoDescuentoOTRO: String?
    /// Con código 99.
    public var naturalezaDescuento: String?
    public init(montoDescuento: Monto, codigoDescuento: Codigo, codigoDescuentoOTRO: String? = nil, naturalezaDescuento: String? = nil) {
        self.montoDescuento = montoDescuento; self.codigoDescuento = codigoDescuento
        self.codigoDescuentoOTRO = codigoDescuentoOTRO; self.naturalezaDescuento = naturalezaDescuento
    }
}

/// Impuestos específicos no tarifarios (códigos 03, 04, 05 y 06).
public struct DatosImpuestoEspecifico: Codable, Hashable, Sendable {
    public var cantidadUnidadMedida: Monto?
    public var porcentaje: Monto?
    public var proporcion: Monto?
    public var volumenUnidadConsumo: Monto?
    public var impuestoUnidad: Monto?
    public init(cantidadUnidadMedida: Monto? = nil, porcentaje: Monto? = nil, proporcion: Monto? = nil,
                volumenUnidadConsumo: Monto? = nil, impuestoUnidad: Monto? = nil) {
        self.cantidadUnidadMedida = cantidadUnidadMedida; self.porcentaje = porcentaje; self.proporcion = proporcion
        self.volumenUnidadConsumo = volumenUnidadConsumo; self.impuestoUnidad = impuestoUnidad
    }
}

public struct Impuesto: Codable, Hashable, Sendable {
    public var codigo: CodigoImpuesto
    /// Con código 99.
    public var codigoImpuestoOTRO: String?
    /// Con los códigos 01 y 07: la tarifa del IVA sale de este código.
    public var codigoTarifaIVA: CodigoTarifaIVA?
    /// Tarifa propia de los impuestos 02, 12 y 99 (13 = 13%).
    public var tarifa: Monto?
    /// Con el código 08 (IVA bienes usados).
    public var factorCalculoIVA: Monto?
    public var datosImpuestoEspecifico: DatosImpuestoEspecifico?
    /// FEE, NC y ND: exportación de mercancías con impuestos especiales.
    public var montoExportacion: Monto?
    public init(codigo: CodigoImpuesto, codigoImpuestoOTRO: String? = nil, codigoTarifaIVA: CodigoTarifaIVA? = nil, tarifa: Monto? = nil,
                factorCalculoIVA: Monto? = nil, datosImpuestoEspecifico: DatosImpuestoEspecifico? = nil, montoExportacion: Monto? = nil) {
        self.codigo = codigo; self.codigoImpuestoOTRO = codigoImpuestoOTRO; self.codigoTarifaIVA = codigoTarifaIVA; self.tarifa = tarifa
        self.factorCalculoIVA = factorCalculoIVA; self.datosImpuestoEspecifico = datosImpuestoEspecifico; self.montoExportacion = montoExportacion
    }

    /// IVA con la tarifa indicada (Nota 8.1).
    public static func iva(_ tarifa: CodigoTarifaIVA = .general13) -> Impuesto { Impuesto(codigo: .iva, codigoTarifaIVA: tarifa) }
}

/// Exonera el IVA de la línea (Nota 10.1); en el XML va dentro del Impuesto de IVA.
public struct Exoneracion: Codable, Hashable, Sendable {
    public var tipoDocumentoEX1: Codigo
    public var tipoDocumentoOTRO: String?
    public var numeroDocumento: String
    /// Obligatorio con los tipos 02, 03, 06, 07 y 08.
    public var articulo: Int?
    public var inciso: Int?
    /// Nota 23.
    public var nombreInstitucion: Codigo
    public var nombreInstitucionOtros: String?
    public var fechaEmisionEX: String
    public var tarifaExonerada: Monto
    public init(tipoDocumentoEX1: Codigo, tipoDocumentoOTRO: String? = nil, numeroDocumento: String, articulo: Int? = nil, inciso: Int? = nil,
                nombreInstitucion: Codigo, nombreInstitucionOtros: String? = nil, fechaEmisionEX: String, tarifaExonerada: Monto) {
        self.tipoDocumentoEX1 = tipoDocumentoEX1; self.tipoDocumentoOTRO = tipoDocumentoOTRO; self.numeroDocumento = numeroDocumento
        self.articulo = articulo; self.inciso = inciso; self.nombreInstitucion = nombreInstitucion
        self.nombreInstitucionOtros = nombreInstitucionOtros; self.fechaEmisionEX = fechaEmisionEX; self.tarifaExonerada = tarifaExonerada
    }
}

public struct LineaDetalleSurtido: Codable, Hashable, Sendable {
    public var codigoCABYS: String
    public var codigosComerciales: [CodigoComercial]?
    public var cantidad: Monto
    public var unidadMedida: String
    public var unidadMedidaComercial: String?
    public var detalle: String
    public var precioUnitario: Monto
    public var descuentos: [Descuento]?
    public var impuestos: [Impuesto]
    public init(codigoCABYS: String, codigosComerciales: [CodigoComercial]? = nil, cantidad: Monto, unidadMedida: String,
                unidadMedidaComercial: String? = nil, detalle: String, precioUnitario: Monto, descuentos: [Descuento]? = nil,
                impuestos: [Impuesto]) {
        self.codigoCABYS = codigoCABYS; self.codigosComerciales = codigosComerciales; self.cantidad = cantidad
        self.unidadMedida = unidadMedida; self.unidadMedidaComercial = unidadMedidaComercial; self.detalle = detalle
        self.precioUnitario = precioUnitario; self.descuentos = descuentos; self.impuestos = impuestos
    }
}

public struct LineaDetalle: Codable, Hashable, Sendable {
    /// Correlativo 1..N; si se omite se numera según la posición.
    public var numeroLinea: Int?
    /// FEE: obligatoria para bienes (12 dígitos).
    public var partidaArancelaria: String?
    public var codigoCABYS: String?
    public var codigosComerciales: [CodigoComercial]?
    public var cantidad: Monto?
    public var unidadMedida: String?
    /// Nota 22.
    public var tipoTransaccion: Codigo?
    public var unidadMedidaComercial: String?
    public var detalle: String
    public var numerosVINoSerie: [String]?
    public var registroMedicamento: String?
    public var formaFarmaceutica: String?
    /// Componentes de un surtido, combo o paquete (hasta 20).
    public var detalleSurtido: [LineaDetalleSurtido]?
    public var precioUnitario: Monto?
    /// Solo REP: monto a pagar de lo facturado, sin impuestos.
    public var montoTotal: Monto?
    public var descuentos: [Descuento]?
    /// Solo REP: monto del pago a registrar para el cálculo del IVA.
    public var subTotal: Monto?
    /// Nota 21.
    public var ivaCobradoFabrica: Codigo?
    /// Solo cuando es editable (IVACobradoFabrica 01 o impuesto 07).
    public var baseImponible: Monto?
    public var impuestos: [Impuesto]?
    public var impuestoAsumidoEmisorFabrica: Monto?
    public var exoneracion: Exoneracion?

    public init(numeroLinea: Int? = nil, partidaArancelaria: String? = nil, codigoCABYS: String? = nil,
                codigosComerciales: [CodigoComercial]? = nil, cantidad: Monto? = nil, unidadMedida: String? = nil,
                tipoTransaccion: Codigo? = nil, unidadMedidaComercial: String? = nil, detalle: String,
                numerosVINoSerie: [String]? = nil, registroMedicamento: String? = nil, formaFarmaceutica: String? = nil,
                detalleSurtido: [LineaDetalleSurtido]? = nil, precioUnitario: Monto? = nil, montoTotal: Monto? = nil,
                descuentos: [Descuento]? = nil, subTotal: Monto? = nil, ivaCobradoFabrica: Codigo? = nil,
                baseImponible: Monto? = nil, impuestos: [Impuesto]? = nil, impuestoAsumidoEmisorFabrica: Monto? = nil,
                exoneracion: Exoneracion? = nil) {
        self.numeroLinea = numeroLinea; self.partidaArancelaria = partidaArancelaria; self.codigoCABYS = codigoCABYS
        self.codigosComerciales = codigosComerciales; self.cantidad = cantidad; self.unidadMedida = unidadMedida
        self.tipoTransaccion = tipoTransaccion; self.unidadMedidaComercial = unidadMedidaComercial; self.detalle = detalle
        self.numerosVINoSerie = numerosVINoSerie; self.registroMedicamento = registroMedicamento; self.formaFarmaceutica = formaFarmaceutica
        self.detalleSurtido = detalleSurtido; self.precioUnitario = precioUnitario; self.montoTotal = montoTotal
        self.descuentos = descuentos; self.subTotal = subTotal; self.ivaCobradoFabrica = ivaCobradoFabrica
        self.baseImponible = baseImponible; self.impuestos = impuestos
        self.impuestoAsumidoEmisorFabrica = impuestoAsumidoEmisorFabrica; self.exoneracion = exoneracion
    }
}

/// Nota 16.
public struct OtrosCargos: Codable, Hashable, Sendable {
    public var tipoDocumentoOC: Codigo
    public var tipoDocumentoOTROS: String?
    public var identificacionTercero: Identificacion?
    public var nombreTercero: String?
    public var detalle: String
    public var porcentajeOC: Monto?
    public var montoCargo: Monto
    public init(tipoDocumentoOC: Codigo, tipoDocumentoOTROS: String? = nil, identificacionTercero: Identificacion? = nil,
                nombreTercero: String? = nil, detalle: String, porcentajeOC: Monto? = nil, montoCargo: Monto) {
        self.tipoDocumentoOC = tipoDocumentoOC; self.tipoDocumentoOTROS = tipoDocumentoOTROS
        self.identificacionTercero = identificacionTercero; self.nombreTercero = nombreTercero
        self.detalle = detalle; self.porcentajeOC = porcentajeOC; self.montoCargo = montoCargo
    }
}

public struct MedioPago: Codable, Hashable, Sendable {
    public var tipoMedioPago: TipoMedioPago
    public var medioPagoOtros: String?
    /// Obligatorio con dos o más medios de pago (deben sumar el total del comprobante).
    public var totalMedioPago: Monto?
    public init(tipoMedioPago: TipoMedioPago, medioPagoOtros: String? = nil, totalMedioPago: Monto? = nil) {
        self.tipoMedioPago = tipoMedioPago; self.medioPagoOtros = medioPagoOtros; self.totalMedioPago = totalMedioPago
    }
}

/// Notas 9 y 10.
public struct InformacionReferencia: Codable, Hashable, Sendable {
    public var tipoDocIR: Codigo
    public var tipoDocRefOTRO: String?
    public var numero: String?
    public var fechaEmisionIR: String
    public var codigo: Codigo?
    public var codigoReferenciaOTRO: String?
    public var razon: String?
    public init(tipoDocIR: Codigo, tipoDocRefOTRO: String? = nil, numero: String? = nil, fechaEmisionIR: String,
                codigo: Codigo? = nil, codigoReferenciaOTRO: String? = nil, razon: String? = nil) {
        self.tipoDocIR = tipoDocIR; self.tipoDocRefOTRO = tipoDocRefOTRO; self.numero = numero; self.fechaEmisionIR = fechaEmisionIR
        self.codigo = codigo; self.codigoReferenciaOTRO = codigoReferenciaOTRO; self.razon = razon
    }
}

public struct CodigoTipoMoneda: Codable, Hashable, Sendable {
    public var codigoMoneda: String
    /// En colones se puede omitir (vale 1).
    public var tipoCambio: Monto?
    public init(codigoMoneda: String = "CRC", tipoCambio: Monto? = nil) { self.codigoMoneda = codigoMoneda; self.tipoCambio = tipoCambio }
}

public struct ResumenFacturaRequest: Codable, Hashable, Sendable {
    public var codigoTipoMoneda: CodigoTipoMoneda
    public var mediosPago: [MedioPago]?
    public init(codigoTipoMoneda: CodigoTipoMoneda = CodigoTipoMoneda(), mediosPago: [MedioPago]? = nil) {
        self.codigoTipoMoneda = codigoTipoMoneda; self.mediosPago = mediosPago
    }
}

public struct ComprobanteRequest: Codable, Hashable, Sendable {
    public var tipoDocumento: TipoDocumento
    public var clave: String
    public var proveedorSistemas: String
    public var codigoActividadEmisor: String?
    public var codigoActividadReceptor: String?
    public var numeroConsecutivo: String
    /// RFC 3339, ej. "2026-10-01T10:00:00-06:00" (ver `fechaEmision(_:)`).
    public var fechaEmision: String
    public var emisor: Emisor
    public var receptor: Receptor?
    public var condicionVenta: CondicionVenta
    public var condicionVentaOtros: String?
    /// Días; obligatorio con condición de venta 02 (crédito).
    public var plazoCredito: Int?
    public var detalleServicio: [LineaDetalle]?
    public var otrosCargos: [OtrosCargos]?
    public var resumenFactura: ResumenFacturaRequest
    public var informacionReferencia: [InformacionReferencia]?

    public init(tipoDocumento: TipoDocumento, clave: String, proveedorSistemas: String, codigoActividadEmisor: String? = nil,
                codigoActividadReceptor: String? = nil, numeroConsecutivo: String, fechaEmision: String, emisor: Emisor,
                receptor: Receptor? = nil, condicionVenta: CondicionVenta, condicionVentaOtros: String? = nil,
                plazoCredito: Int? = nil, detalleServicio: [LineaDetalle]? = nil, otrosCargos: [OtrosCargos]? = nil,
                resumenFactura: ResumenFacturaRequest = ResumenFacturaRequest(), informacionReferencia: [InformacionReferencia]? = nil) {
        self.tipoDocumento = tipoDocumento; self.clave = clave; self.proveedorSistemas = proveedorSistemas
        self.codigoActividadEmisor = codigoActividadEmisor; self.codigoActividadReceptor = codigoActividadReceptor
        self.numeroConsecutivo = numeroConsecutivo; self.fechaEmision = fechaEmision; self.emisor = emisor
        self.receptor = receptor; self.condicionVenta = condicionVenta; self.condicionVentaOtros = condicionVentaOtros
        self.plazoCredito = plazoCredito; self.detalleServicio = detalleServicio; self.otrosCargos = otrosCargos
        self.resumenFactura = resumenFactura; self.informacionReferencia = informacionReferencia
    }
}
