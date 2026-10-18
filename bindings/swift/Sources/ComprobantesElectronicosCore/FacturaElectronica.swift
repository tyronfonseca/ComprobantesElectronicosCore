import Foundation
import ComprobantesElectronicosCoreC

/// Punto de entrada: procesa, calcula y firma comprobantes electrónicos v4.4 con el core en C++, a través de
/// los structs del ABI de C (cec_request_c).
///
/// ```swift
/// let resultado = try FacturaElectronica.procesar(request)
/// if resultado.exito { print(resultado.xml!) }
/// else { resultado.diagnosticos.forEach { print($0.ruta, $0.mensaje) } }
/// ```
///
/// Un comprobante inválido no lanza: viene con `exito == false` y sus diagnósticos. Solo se lanza
/// `ErrorCEC` ante un error de uso de la API de C.
public enum FacturaElectronica {
    /// Versión del ABI de C que espera este binding.
    public static let versionAbiRequerida: UInt32 = 3

    /// Versión del ABI del core enlazado.
    public static var versionAbi: UInt32 { cec_abi_version() }

    /// Catálogos del Anexo 1 (los mismos que validan las reglas).
    public static let catalogos = Catalogos()

    /// Provincias, cantones, distritos y barrios (Nota 14).
    public static let territorio = DivisionTerritorial()

    /// Valida las reglas del Anexo 1, calcula los montos y genera el XML sin firmar.
    public static func procesar(_ request: ComprobanteRequest) throws -> ResultadoComprobante {
        let r = try conRequest(request, funcion: "cec_build", cec_build)
        return ResultadoComprobante(exito: r.exito, diagnosticos: r.diagnosticos, xml: r.xml, totales: r.totales)
    }

    /// Vista previa para edición en vivo: calcula los totales aunque las reglas reporten errores.
    /// No genera XML; para emitir use `procesar`.
    public static func calcular(_ request: ComprobanteRequest) throws -> ResultadoCalculo {
        let r = try conRequest(request, funcion: "cec_calcular", cec_calcular)
        return ResultadoCalculo(exito: r.exito, diagnosticos: r.diagnosticos, totales: r.totales)
    }

    /// Firma un XML con XAdES-EPES usando un certificado PKCS#12 (.p12/.pfx).
    public static func firmar(xml: String, p12: Data, pin: String) throws -> ResultadoFirma {
        let xmlBytes = Array(xml.utf8)
        let p12Bytes = [UInt8](p12)
        let r = try xmlBytes.withUnsafeBufferPointer { x in
            try p12Bytes.withUnsafeBufferPointer { p in
                try ejecutar("cec_sign_xml") { cec_sign_xml(x.baseAddress, x.count, p.baseAddress, p.count, pin, $0) }
            }
        }
        return ResultadoFirma(exito: r.exito, diagnosticos: r.diagnosticos, xml: r.xml, certificado: r.certificado)
    }

    /// Firma en dos pasos, paso 1: para cuando la llave privada está fuera del proceso (keystore, HSM). Recibe el
    /// certificado público (X.509 DER) y devuelve el XML con `SignatureValue` vacío y el `signedInfo` que se firma
    /// con RSA-SHA256 donde esté la llave.
    public static func prepararFirma(xml: String, certificadoDer: Data) throws -> ResultadoFirmaPreparada {
        try verificarAbi()
        let xmlBytes = Array(xml.utf8)
        let derBytes = [UInt8](certificadoDer)
        let r = try xmlBytes.withUnsafeBufferPointer { x in
            try derBytes.withUnsafeBufferPointer { d in
                try ejecutar("cec_preparar_firma") { cec_preparar_firma(x.baseAddress, x.count, d.baseAddress, d.count, $0) }
            }
        }
        return ResultadoFirmaPreparada(exito: r.exito, diagnosticos: r.diagnosticos, xml: r.xml,
                                       signedInfo: r.signedInfo, certificado: r.certificado)
    }

    /// Firma en dos pasos, paso 2: inserta el `SignatureValue` (RSA-SHA256 de `signedInfo`, en Base64) y verifica la
    /// firma con el certificado de `KeyInfo`. Diagnóstico `SIGN_VERIFY_FAILED` si la llave no corresponde.
    public static func completarFirma(xmlPreparado: String, signatureValueBase64: String) throws -> ResultadoFirma {
        try verificarAbi()
        let xmlBytes = Array(xmlPreparado.utf8)
        let r = try xmlBytes.withUnsafeBufferPointer { x in
            try ejecutar("cec_completar_firma") { cec_completar_firma(x.baseAddress, x.count, signatureValueBase64, $0) }
        }
        return ResultadoFirma(exito: r.exito, diagnosticos: r.diagnosticos, xml: r.xml, certificado: r.certificado)
    }

    /// Mensaje Receptor: valida la confirmación o el rechazo de un comprobante recibido y genera el XML sin firmar
    /// (se firma con `firmar`, o `prepararFirma` y `completarFirma`).
    public static func mensajeReceptor(_ request: MensajeReceptorRequest) throws -> ResultadoMensajeReceptor {
        try verificarAbi()
        let arena = Arena()
        var c = arena.mensajeReceptor(request)
        let r = try withExtendedLifetime(arena) {
            try ejecutar("cec_mensaje_receptor") { cec_mensaje_receptor(&c, $0) }
        }
        return ResultadoMensajeReceptor(exito: r.exito, diagnosticos: r.diagnosticos, xml: r.xml, plazoConfirmacion: r.plazoConfirmacion)
    }

    /// Lee el XML de un comprobante recibido (el del proveedor) para confirmarlo o rechazarlo. No verifica la firma.
    public static func leerComprobante(xml: String) throws -> ResultadoLecturaComprobante {
        try verificarAbi()
        let xmlBytes = Array(xml.utf8)
        let r = try xmlBytes.withUnsafeBufferPointer { x in
            try ejecutar("cec_leer_comprobante_xml") { cec_leer_comprobante_xml(x.baseAddress, x.count, $0) }
        }
        return ResultadoLecturaComprobante(exito: r.exito, diagnosticos: r.diagnosticos, comprobante: r.comprobanteRecibido)
    }

    /// Lee titular, emisor, vigencia y huella de un certificado PKCS#12 sin firmar nada.
    public static func leerCertificado(p12: Data, pin: String) throws -> ResultadoCertificado {
        let p12Bytes = [UInt8](p12)
        let r = try p12Bytes.withUnsafeBufferPointer { p in
            try ejecutar("cec_leer_certificado") { cec_leer_certificado(p.baseAddress, p.count, pin, $0) }
        }
        return ResultadoCertificado(exito: r.exito, diagnosticos: r.diagnosticos, certificado: r.certificado)
    }

    // MARK: - Interno

    typealias FuncionRequest = (UnsafePointer<cec_request_c>?, UnsafeMutablePointer<OpaquePointer?>?) -> cec_status

    private static func verificarAbi() throws {
        let version = cec_abi_version()
        guard version == versionAbiRequerida else {
            throw ErrorCEC.versionAbi(encontrada: version, requerida: versionAbiRequerida)
        }
    }

    private static func conRequest(_ request: ComprobanteRequest, funcion: String, _ f: FuncionRequest) throws -> LecturaResultado {
        try verificarAbi()
        let arena = Arena()
        var c = arena.request(request)
        return try withExtendedLifetime(arena) {
            try ejecutar(funcion) { f(&c, $0) }
        }
    }

    /// Ejecuta una función del ABI que produce un cec_result, lo lee y lo libera.
    private static func ejecutar(_ funcion: String, _ llamar: (UnsafeMutablePointer<OpaquePointer?>) -> cec_status) throws -> LecturaResultado {
        var resultado: OpaquePointer?
        let estado = llamar(&resultado)
        guard estado == CEC_STATUS_OK, let res = resultado else {
            throw ErrorCEC.estado(funcion: funcion, codigo: estado.rawValue)
        }
        defer { cec_result_free(res) }
        return LecturaResultado(res)
    }
}
