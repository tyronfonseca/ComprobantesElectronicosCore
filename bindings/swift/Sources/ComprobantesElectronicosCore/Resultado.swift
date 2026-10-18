import Foundation
import ComprobantesElectronicosCoreC

/// Diagnóstico del core.
public struct Diagnostico: Hashable, Sendable {
    public enum Severidad: String, Sendable { case error, advertencia }
    public let severidad: Severidad
    /// Código estable, ej. "RULE_CABYS_FORMAT".
    public let codigo: String
    /// Ruta del campo en el request, ej. "detalleServicio[0].impuestos[1].codigo".
    public let ruta: String
    public let mensaje: String
}

/// Totales del ResumenFactura (Anexo 1 v4.4) calculados por el core.
public struct Totales: Hashable, Sendable {
    public let totalServGravados: Monto
    public let totalServExentos: Monto
    public let totalServExonerado: Monto
    public let totalServNoSujeto: Monto
    public let totalMercanciasGravadas: Monto
    public let totalMercanciasExentas: Monto
    public let totalMercExonerada: Monto
    public let totalMercNoSujeta: Monto
    public let totalGravado: Monto
    public let totalExento: Monto
    public let totalExonerado: Monto
    public let totalNoSujeto: Monto
    public let totalVenta: Monto
    public let totalDescuentos: Monto
    public let totalVentaNeta: Monto
    public let totalImpuesto: Monto
    public let totalImpAsumEmisorFabrica: Monto
    public let totalIVADevuelto: Monto
    public let totalOtrosCargos: Monto
    public let totalComprobante: Monto

    init(_ t: cec_resumen_totales_c) {
        func m(_ escalado: Int64) -> Monto { Monto(Decimal(escalado) / 100_000) }
        totalServGravados = m(t.totalServGravados); totalServExentos = m(t.totalServExentos)
        totalServExonerado = m(t.totalServExonerado); totalServNoSujeto = m(t.totalServNoSujeto)
        totalMercanciasGravadas = m(t.totalMercanciasGravadas); totalMercanciasExentas = m(t.totalMercanciasExentas)
        totalMercExonerada = m(t.totalMercExonerada); totalMercNoSujeta = m(t.totalMercNoSujeta)
        totalGravado = m(t.totalGravado); totalExento = m(t.totalExento); totalExonerado = m(t.totalExonerado)
        totalNoSujeto = m(t.totalNoSujeto); totalVenta = m(t.totalVenta); totalDescuentos = m(t.totalDescuentos)
        totalVentaNeta = m(t.totalVentaNeta); totalImpuesto = m(t.totalImpuesto)
        totalImpAsumEmisorFabrica = m(t.totalImpAsumEmisorFabrica); totalIVADevuelto = m(t.totalIVADevuelto)
        totalOtrosCargos = m(t.totalOtrosCargos); totalComprobante = m(t.totalComprobante)
    }
}

public struct ResultadoComprobante: Hashable, Sendable {
    /// true si no hubo errores (puede haber advertencias).
    public let exito: Bool
    public let diagnosticos: [Diagnostico]
    /// XML sin firmar; solo si exito.
    public let xml: String?
    /// Totales calculados; solo si exito.
    public let totales: Totales?
}

/// Vista previa del cálculo: los totales vienen aunque haya errores de reglas (no si hay errores de formato).
/// Nunca trae XML: para emitir se usa `procesar`.
public struct ResultadoCalculo: Hashable, Sendable {
    public let exito: Bool
    public let diagnosticos: [Diagnostico]
    public let totales: Totales?
}

public struct Certificado: Hashable, Sendable {
    public let sujeto: String
    public let emisor: String
    public let numeroSerie: String
    /// ISO 8601 UTC.
    public let validoDesde: String
    /// ISO 8601 UTC.
    public let validoHasta: String
    public let sha256Hex: String
    public let sha256Base64: String
    /// X.509 DER en Base64.
    public let certificadoBase64: String
    /// La fecha actual está dentro de la vigencia.
    public let vigente: Bool
}

public struct ResultadoFirma: Hashable, Sendable {
    public let exito: Bool
    public let diagnosticos: [Diagnostico]
    /// XML firmado (XAdES-EPES); solo si exito.
    public let xml: String?
    public let certificado: Certificado?
}

/// Paso 1 de la firma en dos pasos (`prepararFirma`).
public struct ResultadoFirmaPreparada: Hashable, Sendable {
    public let exito: Bool
    public let diagnosticos: [Diagnostico]
    /// XML con la firma XAdES-EPES armada y `SignatureValue` vacío; solo si exito.
    public let xml: String?
    /// SignedInfo canonicalizado (C14N): se firma con RSA-SHA256 y el resultado (Base64) va a `completarFirma`.
    public let signedInfo: String?
    public let certificado: Certificado?
}

public struct ResultadoCertificado: Hashable, Sendable {
    public let exito: Bool
    public let diagnosticos: [Diagnostico]
    public let certificado: Certificado?
}

/// Error de uso de la API de C (puntero nulo, tamaño, versión del ABI...). Un comprobante inválido no es un
/// error: viene en el resultado con sus diagnósticos.
public enum ErrorCEC: Error, Equatable, LocalizedError {
    case versionAbi(encontrada: UInt32, requerida: UInt32)
    case estado(funcion: String, codigo: UInt32)

    public var errorDescription: String? {
        switch self {
        case let .versionAbi(encontrada, requerida):
            return "El core tiene el ABI \(encontrada); este binding requiere el \(requerida)."
        case let .estado(funcion, codigo):
            return "\(funcion) devolvió el estado \(codigo)."
        }
    }
}

// MARK: - Lectura de cec_result con los accesores de C

/// Datos de un cec_result. Los textos del core viven hasta cec_result_free, por eso se copian aquí.
struct LecturaResultado {
    let exito: Bool
    let diagnosticos: [Diagnostico]
    let xml: String?
    let totales: Totales?
    let certificado: Certificado?
    let plazoConfirmacion: String?
    let signedInfo: String?
    let comprobanteRecibido: ComprobanteRecibido?

    init(_ res: OpaquePointer) {
        var ok: Int32 = 0
        cec_result_is_success(res, &ok)
        exito = ok == 1

        var total = 0
        cec_result_total_diagnosticos(res, &total)
        diagnosticos = (0..<total).map { i in
            var severidad = CEC_SEVERIDAD_ERROR
            var codigo: UnsafePointer<CChar>?, ruta: UnsafePointer<CChar>?, mensaje: UnsafePointer<CChar>?
            cec_result_diagnostico(res, i, &severidad, &codigo, &ruta, &mensaje)
            return Diagnostico(severidad: severidad == CEC_SEVERIDAD_ERROR ? .error : .advertencia,
                               codigo: String(cString: codigo!), ruta: String(cString: ruta!), mensaje: String(cString: mensaje!))
        }

        var datos: UnsafePointer<UInt8>?
        var largo = 0
        cec_result_xml(res, &datos, &largo)
        xml = datos.map { String(decoding: UnsafeBufferPointer(start: $0, count: largo), as: UTF8.self) }

        var t = cec_resumen_totales_c()
        totales = cec_result_totales(res, &t) == CEC_STATUS_OK ? Totales(t) : nil

        var c = cec_certificado_c()
        if cec_result_certificado(res, &c) == CEC_STATUS_OK {
            certificado = Certificado(
                sujeto: String(cString: c.sujeto), emisor: String(cString: c.emisor), numeroSerie: String(cString: c.numeroSerie),
                validoDesde: String(cString: c.validoDesde), validoHasta: String(cString: c.validoHasta),
                sha256Hex: String(cString: c.sha256Hex), sha256Base64: String(cString: c.sha256Base64),
                certificadoBase64: String(cString: c.certificadoBase64), vigente: c.vigente == 1)
        } else {
            certificado = nil
        }

        var plazo: UnsafePointer<CChar>?
        plazoConfirmacion = cec_result_plazo_confirmacion(res, &plazo) == CEC_STATUS_OK ? plazo.map { String(cString: $0) } : nil

        var si: UnsafePointer<UInt8>?
        var siLargo = 0
        signedInfo = cec_result_signed_info(res, &si, &siLargo) == CEC_STATUS_OK
            ? si.map { String(decoding: UnsafeBufferPointer(start: $0, count: siLargo), as: UTF8.self) } : nil

        var recibido = cec_comprobante_recibido_c()
        comprobanteRecibido = cec_result_comprobante_recibido(res, &recibido) == CEC_STATUS_OK ? ComprobanteRecibido(recibido) : nil
    }
}
