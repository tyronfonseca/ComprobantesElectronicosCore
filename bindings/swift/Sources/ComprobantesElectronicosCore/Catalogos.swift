import Foundation
import ComprobantesElectronicosCoreC

/*
 * Catálogos del Anexo 1 y división territorial (Nota 14), leídos del core: son los mismos que validan las reglas.
 */

public struct ElementoCatalogo: Hashable, Sendable {
    public let codigo: String
    public let descripcion: String
}

public struct ElementoConSimbolo: Hashable, Sendable {
    public let codigo: String
    public let simbolo: String
    public let descripcion: String
}

/// Elemento territorial con el código tal como va en `Ubicacion` (provincia "1", cantón "08").
public struct Territorio: Hashable, Sendable {
    public let codigo: String
    public let nombre: String
}

/// Catálogos disponibles por nombre.
public enum NombreCatalogo: String, Sendable, CaseIterable {
    case condicionesVenta = "CondicionesVenta"
    case mediosPago = "MediosPago"
    case tiposIdentificacion = "TiposIdentificacion"
    case tarifasIVA = "TarifasIVA"
    case tiposImpuesto = "TiposImpuesto"
    case tiposDescuento = "TiposDescuento"
    case tiposTransaccion = "TiposTransaccion"
    case tiposExoneracion = "TiposExoneracion"
    case institucionesExoneracion = "InstitucionesExoneracion"
    case tiposOtrosCargos = "TiposOtrosCargos"
    case tiposDocumentoReferencia = "TiposDocumentoReferencia"
    case codigosReferencia = "CodigosReferencia"
    case tiposCodigoComercial = "TiposCodigoComercial"
    case ivaCobradoFabrica = "IVACobradoFabrica"
    case tiposDocumento = "TiposDocumento"
    case formasFarmaceuticas = "FormasFarmaceuticas"
}

/// Texto de un `const char*` del core (vive mientras viva el proceso).
private func texto(_ p: UnsafePointer<CChar>?) -> String { p.map { String(cString: $0) } ?? "" }

public struct Catalogos: Sendable {
    public func porNombre(_ nombre: NombreCatalogo) -> [ElementoCatalogo] {
        let n = nombre.rawValue
        let total = cec_catalogo_total_elementos(n)
        return (0..<total).compactMap { i in
            var codigo: UnsafePointer<CChar>?
            var descripcion: UnsafePointer<CChar>?
            guard cec_catalogo_elemento_por_indice(n, i, &codigo, &descripcion) != 0 else { return nil }
            return ElementoCatalogo(codigo: texto(codigo), descripcion: texto(descripcion))
        }
    }

    /// Monedas ISO 4217.
    public func monedas() -> [ElementoConSimbolo] {
        (0..<cec_catalogo_total_monedas()).compactMap { i in
            var c: UnsafePointer<CChar>?, s: UnsafePointer<CChar>?, d: UnsafePointer<CChar>?
            guard cec_catalogo_moneda_por_indice(i, &c, &s, &d) != 0 else { return nil }
            return ElementoConSimbolo(codigo: texto(c), simbolo: texto(s), descripcion: texto(d))
        }
    }

    /// Unidades de medida de la línea.
    public func unidadesMedida() -> [ElementoConSimbolo] {
        (0..<cec_catalogo_total_unidades_medida()).compactMap { i in
            var c: UnsafePointer<CChar>?, s: UnsafePointer<CChar>?, d: UnsafePointer<CChar>?
            guard cec_catalogo_unidad_medida_por_indice(i, &c, &s, &d) != 0 else { return nil }
            return ElementoConSimbolo(codigo: texto(c), simbolo: texto(s), descripcion: texto(d))
        }
    }

    public func formasFarmaceuticas() -> [ElementoCatalogo] { porNombre(.formasFarmaceuticas) }
}

public struct DivisionTerritorial: Sendable {
    private func numero(_ codigo: String) -> UInt8? {
        guard codigo.count <= 2, let n = UInt8(codigo) else { return nil }
        return n
    }

    private func dosDigitos(_ n: UInt8) -> String { n < 10 ? "0\(n)" : "\(n)" }

    public func provincias() -> [Territorio] {
        (0..<cec_dta_total_provincias()).compactMap { i in
            var codigo: UInt8 = 0, nombre: UnsafePointer<CChar>?, total: UInt32 = 0
            guard cec_dta_provincia_por_indice(i, &codigo, &nombre, &total) != 0 else { return nil }
            return Territorio(codigo: "\(codigo)", nombre: texto(nombre))
        }
    }

    public func cantones(provincia: String) -> [Territorio] {
        guard let p = numero(provincia) else { return [] }
        return (0..<cec_dta_total_cantones(p)).compactMap { i in
            var codigo: UInt8 = 0, nombre: UnsafePointer<CChar>?, total: UInt32 = 0
            guard cec_dta_canton_por_indice(p, i, &codigo, &nombre, &total) != 0 else { return nil }
            return Territorio(codigo: dosDigitos(codigo), nombre: texto(nombre))
        }
    }

    public func distritos(provincia: String, canton: String) -> [Territorio] {
        guard let p = numero(provincia), let c = numero(canton) else { return [] }
        return (0..<cec_dta_total_distritos(p, c)).compactMap { i in
            var codigo: UInt8 = 0, nombre: UnsafePointer<CChar>?, total: UInt32 = 0
            guard cec_dta_distrito_por_indice(p, c, i, &codigo, &nombre, &total) != 0 else { return nil }
            return Territorio(codigo: dosDigitos(codigo), nombre: texto(nombre))
        }
    }

    /// Barrios del distrito. En `Ubicacion`, el barrio es el nombre (texto), no el código.
    public func barrios(provincia: String, canton: String, distrito: String) -> [Territorio] {
        guard let p = numero(provincia), let c = numero(canton), let d = numero(distrito) else { return [] }
        return (0..<cec_dta_total_barrios(p, c, d)).compactMap { i in
            var codigo: UInt8 = 0, nombre: UnsafePointer<CChar>?
            guard cec_dta_barrio_por_indice(p, c, d, i, &codigo, &nombre) != 0 else { return nil }
            return Territorio(codigo: dosDigitos(codigo), nombre: texto(nombre))
        }
    }

    /// true si provincia, cantón y distrito existen y son coherentes entre sí.
    public func esUbicacionValida(provincia: String, canton: String, distrito: String) -> Bool {
        guard let p = numero(provincia), let c = numero(canton), let d = numero(distrito) else { return false }
        return cec_dta_validar_ubicacion(p, c, d) == 1
    }
}
