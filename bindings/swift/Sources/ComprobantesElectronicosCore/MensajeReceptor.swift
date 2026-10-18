import Foundation
import ComprobantesElectronicosCoreC

/*
 * Mensaje Receptor v4.4 (MensajeReceptor_V4.4.xsd): confirmación (total o parcial) o rechazo de una factura o nota
 * recibida de un proveedor. Se lee el XML recibido con `FacturaElectronica.leerComprobante`, se arma el mensaje y
 * `FacturaElectronica.mensajeReceptor` lo valida y genera su XML sin firmar, que se firma igual que un comprobante.
 * Las propiedades se llaman como los nodos del XSD en camelCase.
 */

/// Qué se responde al proveedor.
public enum CodigoMensajeReceptor: String, Codable, Sendable, CaseIterable {
    case aceptado = "1"
    case aceptadoParcialmente = "2"
    case rechazado = "3"
}

/// Datos del Mensaje Receptor.
public struct MensajeReceptorRequest: Codable, Hashable, Sendable {
    /// Clave (50 dígitos) del comprobante que se confirma o rechaza.
    public var clave: String
    /// Identificación de quien emitió el comprobante (el proveedor): hasta 20 caracteres.
    public var numeroCedulaEmisor: String
    /// Fecha del mensaje, RFC 3339 (ver `fechaEmision()`).
    public var fechaEmisionDoc: String
    public var mensaje: CodigoMensajeReceptor
    public var detalleMensaje: String?
    public var montoTotalImpuesto: Monto?
    /// Actividad económica del receptor.
    public var codigoActividad: String?
    /// Nota 18: "01" a "05".
    public var condicionImpuesto: String?
    public var montoTotalImpuestoAcreditar: Monto?
    public var montoTotalDeGastoAplicable: Monto?
    public var totalFactura: Monto
    /// Identificación de quien confirma (el receptor del comprobante): 9 a 12 dígitos.
    public var numeroCedulaReceptor: String
    /// 20 dígitos; el tipo (posiciones 9 y 10) es 05, 06 o 07 según el mensaje.
    public var numeroConsecutivoReceptor: String
    /// Opcional: FechaEmision del comprobante recibido. Con ella se revisa el plazo del art. 10.
    public var fechaEmisionComprobante: String?

    public init(clave: String, numeroCedulaEmisor: String, fechaEmisionDoc: String, mensaje: CodigoMensajeReceptor,
                detalleMensaje: String? = nil, montoTotalImpuesto: Monto? = nil, codigoActividad: String? = nil,
                condicionImpuesto: String? = nil, montoTotalImpuestoAcreditar: Monto? = nil,
                montoTotalDeGastoAplicable: Monto? = nil, totalFactura: Monto, numeroCedulaReceptor: String,
                numeroConsecutivoReceptor: String, fechaEmisionComprobante: String? = nil) {
        self.clave = clave
        self.numeroCedulaEmisor = numeroCedulaEmisor
        self.fechaEmisionDoc = fechaEmisionDoc
        self.mensaje = mensaje
        self.detalleMensaje = detalleMensaje
        self.montoTotalImpuesto = montoTotalImpuesto
        self.codigoActividad = codigoActividad
        self.condicionImpuesto = condicionImpuesto
        self.montoTotalImpuestoAcreditar = montoTotalImpuestoAcreditar
        self.montoTotalDeGastoAplicable = montoTotalDeGastoAplicable
        self.totalFactura = totalFactura
        self.numeroCedulaReceptor = numeroCedulaReceptor
        self.numeroConsecutivoReceptor = numeroConsecutivoReceptor
        self.fechaEmisionComprobante = fechaEmisionComprobante
    }
}

/// Emisor o receptor de un comprobante recibido. Los textos que el XML no trae vienen vacíos.
public struct ParteRecibida: Hashable, Sendable {
    public let nombre: String
    public let tipoIdentificacion: String
    public let numeroIdentificacion: String
    /// Solo el emisor trae correo.
    public let correo: String
}

/// Datos del XML de un comprobante recibido (el que envía el proveedor).
public struct ComprobanteRecibido: Hashable, Sendable {
    /// "01", "02", "03"... según el elemento raíz.
    public let tipoDocumento: String
    /// FacturaElectronica, NotaCreditoElectronica...
    public let nombreDocumento: String
    /// "4.4", "4.3"... según el namespace.
    public let version: String
    public let clave: String
    public let numeroConsecutivo: String
    public let fechaEmision: String
    public let emisor: ParteRecibida
    public let receptor: ParteRecibida
    public let codigoActividadReceptor: String
    public let codigoMoneda: String
    public let tipoCambio: String
    public let totalImpuesto: String
    public let totalComprobante: String
    /// El XML trae un nodo de firma (no se verifica).
    public let firmado: Bool
    /// Último día para confirmarlo (AAAA-MM-DD); vacío si la fecha de emisión no es válida.
    public let plazoConfirmacion: String

    init(_ c: cec_comprobante_recibido_c) {
        func s(_ p: UnsafePointer<CChar>?) -> String { p.map { String(cString: $0) } ?? "" }
        tipoDocumento = s(c.tipoDocumento); nombreDocumento = s(c.nombreDocumento); version = s(c.version)
        clave = s(c.clave); numeroConsecutivo = s(c.numeroConsecutivo); fechaEmision = s(c.fechaEmision)
        emisor = ParteRecibida(nombre: s(c.emisorNombre), tipoIdentificacion: s(c.emisorTipoIdentificacion),
                               numeroIdentificacion: s(c.emisorNumeroIdentificacion), correo: s(c.emisorCorreo))
        receptor = ParteRecibida(nombre: s(c.receptorNombre), tipoIdentificacion: s(c.receptorTipoIdentificacion),
                                 numeroIdentificacion: s(c.receptorNumeroIdentificacion), correo: "")
        codigoActividadReceptor = s(c.codigoActividadReceptor); codigoMoneda = s(c.codigoMoneda); tipoCambio = s(c.tipoCambio)
        totalImpuesto = s(c.totalImpuesto); totalComprobante = s(c.totalComprobante)
        firmado = c.firmado == 1
        plazoConfirmacion = s(c.plazoConfirmacion)
    }
}

public struct ResultadoMensajeReceptor: Hashable, Sendable {
    public let exito: Bool
    public let diagnosticos: [Diagnostico]
    /// XML sin firmar; solo si exito.
    public let xml: String?
    /// Último día del plazo del art. 10 (AAAA-MM-DD), si se indicó `fechaEmisionComprobante`.
    public let plazoConfirmacion: String?
}

public struct ResultadoLecturaComprobante: Hashable, Sendable {
    /// false si no se puede confirmar con un Mensaje Receptor (los diagnósticos dicen por qué).
    public let exito: Bool
    public let diagnosticos: [Diagnostico]
    /// Lo que se alcanzó a leer; nil si el texto ni siquiera es XML.
    public let comprobante: ComprobanteRecibido?
}

extension Arena {
    func mensajeReceptor(_ m: MensajeReceptorRequest) -> cec_mensaje_receptor_c {
        var r = cec_mensaje_receptor_c()
        r.clave = c(m.clave)
        r.numeroCedulaEmisor = c(m.numeroCedulaEmisor)
        r.fechaEmisionDoc = c(m.fechaEmisionDoc)
        r.mensaje = c(m.mensaje.rawValue)
        r.detalleMensaje = c(m.detalleMensaje)
        r.montoTotalImpuesto = c(m.montoTotalImpuesto?.description)
        r.codigoActividad = c(m.codigoActividad)
        r.condicionImpuesto = c(m.condicionImpuesto)
        r.montoTotalImpuestoAcreditar = c(m.montoTotalImpuestoAcreditar?.description)
        r.montoTotalDeGastoAplicable = c(m.montoTotalDeGastoAplicable?.description)
        r.totalFactura = c(m.totalFactura.description)
        r.numeroCedulaReceptor = c(m.numeroCedulaReceptor)
        r.numeroConsecutivoReceptor = c(m.numeroConsecutivoReceptor)
        r.fechaEmisionComprobante = c(m.fechaEmisionComprobante)
        return r
    }
}
