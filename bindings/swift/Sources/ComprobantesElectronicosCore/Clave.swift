import Foundation

/*
 * Numeración del comprobante (Notas 3 y 4.1 del Anexo 1). Funciones puras: no llaman al core.
 */

public enum ErrorNumeracion: Error, Equatable, LocalizedError {
    case fueraDeRango(String)
    case formato(String)
    public var errorDescription: String? {
        switch self {
        case .fueraDeRango(let m), .formato(let m): return m
        }
    }
}

private func entero(_ nombre: String, _ valor: UInt64, maximo: UInt64, digitos: Int) throws -> String {
    guard (1...maximo).contains(valor) else { throw ErrorNumeracion.fueraDeRango("\(nombre) debe estar entre 1 y \(maximo).") }
    let s = String(valor)
    return String(repeating: "0", count: digitos - s.count) + s
}

/// NumeroConsecutivo de 20 dígitos: sucursal (3) + terminal (5) + tipo de comprobante (2) + número (10).
public func generarConsecutivo(sucursal: UInt64, terminal: UInt64, tipoDocumento: TipoDocumento, numero: UInt64) throws -> String {
    try entero("La sucursal", sucursal, maximo: 999, digitos: 3) +
        entero("La terminal", terminal, maximo: 99_999, digitos: 5) +
        tipoDocumento.rawValue +
        entero("El número", numero, maximo: 9_999_999_999, digitos: 10)
}

/// Clave de 50 posiciones: 506 + fecha DDMMYY + identificación (12) + consecutivo (20) + situación (1)
/// + código de seguridad (8).
///
/// - Parameters:
///   - fechaEmision: RFC 3339; la fecha de la clave es la de este texto (ver `fechaEmision(_:)`).
///   - identificacion: del emisor, o del receptor en la Factura Electrónica de Compra. Se rellena con ceros
///     hasta 12 posiciones; admite letras para personas jurídicas.
public func generarClave(fechaEmision: String, identificacion: String, numeroConsecutivo: String,
                         situacion: Situacion = .normal, codigoSeguridad: String) throws -> String {
    let f = Array(fechaEmision)
    guard f.count >= 11, f[4] == "-", f[7] == "-", f[10] == "T",
          f[0..<4].allSatisfy(\.isNumber), f[5..<7].allSatisfy(\.isNumber), f[8..<10].allSatisfy(\.isNumber) else {
        throw ErrorNumeracion.formato("fechaEmision debe tener formato RFC 3339 (YYYY-MM-DDThh:mm:ss±hh:mm).")
    }
    guard (1...12).contains(identificacion.count), identificacion.allSatisfy({ $0.isASCII && ($0.isLetter || $0.isNumber) }) else {
        throw ErrorNumeracion.formato("La identificación debe tener de 1 a 12 caracteres alfanuméricos, sin guiones.")
    }
    guard numeroConsecutivo.count == 20, numeroConsecutivo.allSatisfy(\.isNumber) else {
        throw ErrorNumeracion.formato("El número consecutivo debe tener 20 dígitos.")
    }
    guard (1...8).contains(codigoSeguridad.count), codigoSeguridad.allSatisfy(\.isNumber) else {
        throw ErrorNumeracion.formato("El código de seguridad debe tener hasta 8 dígitos.")
    }
    let dia = String(f[8..<10]), mes = String(f[5..<7]), anio = String(f[2..<4])
    return "506" + dia + mes + anio +
        String(repeating: "0", count: 12 - identificacion.count) + identificacion +
        numeroConsecutivo + situacion.rawValue +
        String(repeating: "0", count: 8 - codigoSeguridad.count) + codigoSeguridad
}

/// Fecha de emisión RFC 3339 en hora de Costa Rica (UTC-06:00, sin horario de verano), sin milisegundos.
public func fechaEmision(_ fecha: Date = Date()) -> String {
    let formato = DateFormatter()
    formato.locale = Locale(identifier: "en_US_POSIX")
    formato.timeZone = TimeZone(secondsFromGMT: -6 * 3600)
    formato.dateFormat = "yyyy-MM-dd'T'HH:mm:ss"
    return formato.string(from: fecha) + "-06:00"
}
