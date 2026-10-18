import Foundation
import Testing
@testable import ComprobantesElectronicosCore

/*
 * Tests del binding contra el core real. Los requests son los casos golden del core
 * (test/golden/<caso>/request.json): el XML producido a través de los structs debe ser idéntico al esperado.
 */

private let raiz = URL(fileURLWithPath: #filePath)
    .deletingLastPathComponent().deletingLastPathComponent().deletingLastPathComponent()
    .deletingLastPathComponent().deletingLastPathComponent()
private let golden = raiz.appendingPathComponent("test/golden")

private func caso(_ nombre: String) throws -> ComprobanteRequest {
    let datos = try Data(contentsOf: golden.appendingPathComponent("\(nombre)/request.json"))
    return try JSONDecoder().decode(ComprobanteRequest.self, from: datos)
}

private func esperado(_ nombre: String) throws -> String {
    try String(contentsOf: golden.appendingPathComponent("\(nombre)/expected.xml"), encoding: .utf8)
}

@Suite("procesar")
struct Procesar {
    @Test("mismo XML que el golden del core", arguments: ["fe_servicios", "fe_completa", "fe_surtido", "fe_usd", "fe_vehiculo_vin",
                                                          "te", "nc", "nd", "fec", "fee", "rep"])
    func golden(_ nombre: String) throws {
        let r = try FacturaElectronica.procesar(try caso(nombre))
        #expect(r.exito, "\(r.diagnosticos)")
        #expect(r.xml == (try esperado(nombre)))
    }

    @Test func totalesDelComprobante() throws {
        let r = try FacturaElectronica.procesar(try caso("fe_completa"))
        let t = try #require(r.totales)
        #expect(t.totalComprobante == "1645798.695")
        #expect(t.totalOtrosCargos == "33827")
        #expect(t.totalServExonerado == "89270")
    }

    @Test func diagnosticosConLaRutaDelCampo() throws {
        var req = try caso("fe_servicios")
        req.detalleServicio![0].codigoCABYS = "123"
        let r = try FacturaElectronica.procesar(req)
        #expect(!r.exito)
        #expect(r.xml == nil)
        #expect(r.totales == nil)
        let d = try #require(r.diagnosticos.first { $0.codigo == "RULE_CABYS_FORMAT" })
        #expect(d.severidad == .error)
        #expect(d.ruta == "detalleServicio[0].codigoCABYS")
    }

    @Test func requestArmadoEnSwift() throws {
        let fecha = "2026-10-01T10:00:00-06:00"
        let consecutivo = try generarConsecutivo(sucursal: 1, terminal: 1, tipoDocumento: .facturaElectronica, numero: 7)
        let req = ComprobanteRequest(
            tipoDocumento: .facturaElectronica,
            clave: try generarClave(fechaEmision: fecha, identificacion: "3101000101", numeroConsecutivo: consecutivo, codigoSeguridad: "1"),
            proveedorSistemas: "3101000999", codigoActividadEmisor: "6201.0",
            numeroConsecutivo: consecutivo, fechaEmision: fecha,
            emisor: Emisor(nombre: "SOFTWARE EJEMPLO S.A.", identificacion: Identificacion(tipo: .juridica, numero: "3101000101"),
                           ubicacion: Ubicacion(provincia: "1", canton: "01", distrito: "08", otrasSenas: "Avenida 10, edificio azul"),
                           correoElectronico: ["facturacion@software.example.com"]),
            receptor: Receptor(nombre: "CLIENTE EJEMPLO S.A.", identificacion: Identificacion(tipo: .juridica, numero: "3101000202"),
                               correoElectronico: "pagos@cliente.example.com"),
            condicionVenta: .contado,
            detalleServicio: [LineaDetalle(codigoCABYS: "8314100000200", cantidad: "2", unidadMedida: "Sp",
                                           detalle: "Diseño & análisis — señalización", precioUnitario: "1500.25",
                                           impuestos: [.iva()])],
            resumenFactura: ResumenFacturaRequest(mediosPago: [MedioPago(tipoMedioPago: .sinpeMovil)]))
        let r = try FacturaElectronica.procesar(req)
        #expect(r.exito, "\(r.diagnosticos)")
        #expect(r.totales?.totalComprobante == "3390.565")   // 3000.50 + 13%
        #expect(r.xml?.contains("<Detalle>Diseño &amp; análisis — señalización</Detalle>") == true)
    }
}

@Suite("calcular")
struct Calcular {
    @Test func totalesAunqueElComprobanteEsteIncompleto() throws {
        var req = try caso("fe_servicios")
        req.emisor.nombre = ""
        let r = try FacturaElectronica.calcular(req)
        #expect(!r.exito)
        #expect(r.diagnosticos.contains { $0.codigo == "RULE_EMISOR_NOMBRE_LENGTH" })
        #expect(r.totales?.totalComprobante == "100875.1")
    }

    @Test func sinTotalesConErrorDeFormato() throws {
        var req = try caso("fe_servicios")
        req.detalleServicio![0].descuentos = [Descuento(montoDescuento: Monto(Decimal.nan), codigoDescuento: "07")]
        let r = try FacturaElectronica.calcular(req)
        #expect(r.diagnosticos.contains { $0.codigo == "PARSE_DECIMAL_INVALID" })
        #expect(r.totales == nil)
    }
}

@Suite("catálogos")
struct CatalogosTests {
    @Test func porNombre() {
        let codigos = FacturaElectronica.catalogos.porNombre(.codigosReferencia).map(\.codigo)
        #expect(codigos.contains("17"))
        #expect(!codigos.contains("03"))
        #expect(FacturaElectronica.catalogos.monedas().contains { $0.codigo == "CRC" })
        #expect(FacturaElectronica.catalogos.unidadesMedida().contains { $0.codigo == "Sp" })
        #expect(!FacturaElectronica.catalogos.formasFarmaceuticas().isEmpty)
    }

    @Test func divisionTerritorial() {
        let t = FacturaElectronica.territorio
        #expect(t.provincias().count == 7)
        #expect(t.cantones(provincia: "1").first?.codigo == "01")
        #expect(!t.distritos(provincia: "1", canton: "08").isEmpty)
        #expect(t.esUbicacionValida(provincia: "1", canton: "08", distrito: "01"))
        #expect(!t.esUbicacionValida(provincia: "1", canton: "99", distrito: "01"))
        #expect(t.cantones(provincia: "x").isEmpty)
    }
}

@Suite("clave y consecutivo")
struct ClaveTests {
    @Test func reproduceLaClaveDeUnCasoValido() throws {
        let req = try caso("nc")
        let consecutivo = try generarConsecutivo(sucursal: 1, terminal: 1, tipoDocumento: .notaCredito, numero: 1)
        #expect(consecutivo == req.numeroConsecutivo)
        let clave = try generarClave(fechaEmision: req.fechaEmision, identificacion: req.emisor.identificacion.numero,
                                     numeroConsecutivo: consecutivo, codigoSeguridad: "12345678")
        #expect(clave == req.clave)
    }

    @Test func validaLosRangos() {
        #expect(throws: ErrorNumeracion.self) { try generarConsecutivo(sucursal: 0, terminal: 1, tipoDocumento: .facturaElectronica, numero: 1) }
        #expect(throws: ErrorNumeracion.self) {
            try generarClave(fechaEmision: "01/10/2026", identificacion: "1", numeroConsecutivo: String(repeating: "0", count: 20), codigoSeguridad: "1")
        }
    }

    @Test func fechaEnHoraDeCostaRica() {
        #expect(fechaEmision(Date(timeIntervalSince1970: 1_792_035_000)) == "2026-10-14T21:30:00-06:00") // 2026-10-15T03:30Z
    }
}

@Suite("firma y certificado")
struct FirmaTests {
    static let pin = "pin-de-prueba"

    /// Certificado autofirmado generado con el CLI de openssl, con su llave privada en PEM; nil si no está instalado.
    static let material: (p12: Data, clavePem: Data)? = {
        let dir = FileManager.default.temporaryDirectory.appendingPathComponent(UUID().uuidString)
        try? FileManager.default.createDirectory(at: dir, withIntermediateDirectories: true)
        defer { try? FileManager.default.removeItem(at: dir) }
        func openssl(_ args: [String]) -> Bool {
            let p = Process()
            p.executableURL = URL(fileURLWithPath: "/usr/bin/env")
            p.arguments = ["openssl"] + args
            p.standardOutput = FileHandle.nullDevice
            p.standardError = FileHandle.nullDevice
            guard (try? p.run()) != nil else { return false }
            p.waitUntilExit()
            return p.terminationStatus == 0
        }
        let k = dir.appendingPathComponent("k.pem").path, c = dir.appendingPathComponent("c.pem").path
        let salida = dir.appendingPathComponent("c.p12")
        guard openssl(["req", "-x509", "-newkey", "rsa:2048", "-nodes", "-keyout", k, "-out", c, "-days", "30",
                       "-subj", "/CN=TITULAR DE PRUEBA/C=CR"]),
              openssl(["pkcs12", "-export", "-inkey", k, "-in", c, "-out", salida.path, "-passout", "pass:\(pin)",
                       "-keypbe", "AES-256-CBC", "-certpbe", "AES-256-CBC", "-macalg", "sha256"]) else { return nil }
        guard let p12 = try? Data(contentsOf: salida), let clave = try? Data(contentsOf: dir.appendingPathComponent("k.pem")) else { return nil }
        return (p12, clave)
    }()

    static var p12: Data? { material?.p12 }

    /// Firma `signedInfo` con RSA-SHA256 usando solo la llave privada en PEM, como lo haría un keystore o un HSM.
    static func firmarExterno(_ signedInfo: String) -> String? {
        guard let clave = material?.clavePem else { return nil }
        let dir = FileManager.default.temporaryDirectory.appendingPathComponent(UUID().uuidString)
        try? FileManager.default.createDirectory(at: dir, withIntermediateDirectories: true)
        defer { try? FileManager.default.removeItem(at: dir) }
        let k = dir.appendingPathComponent("k.pem"), datos = dir.appendingPathComponent("si.txt"), firma = dir.appendingPathComponent("firma.bin")
        guard (try? clave.write(to: k)) != nil, (try? Data(signedInfo.utf8).write(to: datos)) != nil else { return nil }
        let p = Process()
        p.executableURL = URL(fileURLWithPath: "/usr/bin/env")
        p.arguments = ["openssl", "dgst", "-sha256", "-sign", k.path, "-out", firma.path, datos.path]
        p.standardOutput = FileHandle.nullDevice
        p.standardError = FileHandle.nullDevice
        guard (try? p.run()) != nil else { return nil }
        p.waitUntilExit()
        return p.terminationStatus == 0 ? (try? Data(contentsOf: firma))?.base64EncodedString() : nil
    }

    @Test(.enabled(if: p12 != nil, "openssl no disponible"))
    func firmaElXmlDeUnComprobante() throws {
        let xml = try #require(try FacturaElectronica.procesar(try caso("fe_servicios")).xml)
        let r = try FacturaElectronica.firmar(xml: xml, p12: Self.p12!, pin: Self.pin)
        #expect(r.exito, "\(r.diagnosticos)")
        #expect(r.xml?.contains("<ds:Signature xmlns:ds=\"http://www.w3.org/2000/09/xmldsig#\"") == true)
        #expect(r.certificado?.sujeto.contains("TITULAR DE PRUEBA") == true)
    }

    @Test(.enabled(if: p12 != nil, "openssl no disponible"))
    func firmaEnDosPasosConLaLlaveFueraDelProceso() throws {
        let xml = try #require(try FacturaElectronica.procesar(try caso("fe_servicios")).xml)
        let cert = try #require(try FacturaElectronica.leerCertificado(p12: Self.p12!, pin: Self.pin).certificado)
        let der = try #require(Data(base64Encoded: cert.certificadoBase64))

        let prep = try FacturaElectronica.prepararFirma(xml: xml, certificadoDer: der)
        #expect(prep.exito, "\(prep.diagnosticos)")
        let preparado = try #require(prep.xml)
        let signedInfo = try #require(prep.signedInfo)
        #expect(signedInfo.hasPrefix("<ds:SignedInfo"))
        #expect(preparado.contains("<ds:SignatureValue/>"))   // todavía sin valor

        let firma = try #require(Self.firmarExterno(signedInfo), "no se pudo firmar con openssl")
        let final = try FacturaElectronica.completarFirma(xmlPreparado: preparado, signatureValueBase64: firma)
        #expect(final.exito, "\(final.diagnosticos)")
        #expect(final.xml?.contains("<ds:SignatureValue>\(firma)</ds:SignatureValue>") == true)

        // Una firma que no corresponde al certificado se rechaza
        let mala = try FacturaElectronica.completarFirma(xmlPreparado: preparado, signatureValueBase64: Data("no es una firma".utf8).base64EncodedString())
        #expect(!mala.exito)
        #expect(mala.diagnosticos.contains { $0.codigo == "SIGN_VERIFY_FAILED" })
    }

    @Test func prepararFirmaConUnCertificadoInvalidoDiagnostica() throws {
        let r = try FacturaElectronica.prepararFirma(xml: "<a/>", certificadoDer: Data([1, 2, 3]))
        #expect(!r.exito)
        #expect(r.xml == nil)
        #expect(r.signedInfo == nil)
        #expect(!r.diagnosticos.isEmpty)
    }

    @Test(.enabled(if: p12 != nil, "openssl no disponible"))
    func leeElCertificadoYDetectaElPinIncorrecto() throws {
        let ok = try FacturaElectronica.leerCertificado(p12: Self.p12!, pin: Self.pin)
        #expect(ok.exito)
        #expect(ok.certificado?.vigente == true)
        #expect(ok.certificado?.sha256Hex.count == 64)

        let malo = try FacturaElectronica.leerCertificado(p12: Self.p12!, pin: "otro")
        #expect(!malo.exito)
        #expect(malo.certificado == nil)
        #expect(malo.diagnosticos.first?.codigo == "CRYPTO_P12_AUTH_FAILED")
    }
}

// MARK: - Mensaje Receptor

/// Factura del proveedor 3101123456 al receptor 3101987654 (100000 + IVA 13000), con un nodo de firma.
private let facturaRecibida = #"""
<?xml version="1.0" encoding="utf-8"?>
<FacturaElectronica xmlns="https://cdn.comprobanteselectronicos.go.cr/xml-schemas/v4.4/facturaElectronica">
  <Clave>50601102600310112345600100001010000000001112345678</Clave>
  <ProveedorSistemas>3101123456</ProveedorSistemas>
  <CodigoActividadEmisor>620100</CodigoActividadEmisor>
  <CodigoActividadReceptor>471100</CodigoActividadReceptor>
  <NumeroConsecutivo>00100001010000000001</NumeroConsecutivo>
  <FechaEmision>2026-10-01T10:00:00-06:00</FechaEmision>
  <Emisor>
    <Nombre>SOFTWARE EJEMPLO S.A.</Nombre>
    <Identificacion><Tipo>02</Tipo><Numero>3101123456</Numero></Identificacion>
    <CorreoElectronico>facturacion@ejemplo.cr</CorreoElectronico>
  </Emisor>
  <Receptor>
    <Nombre>CLIENTE EJEMPLO S.A.</Nombre>
    <Identificacion><Tipo>02</Tipo><Numero>3101987654</Numero></Identificacion>
  </Receptor>
  <ResumenFactura>
    <CodigoTipoMoneda><CodigoMoneda>CRC</CodigoMoneda><TipoCambio>1</TipoCambio></CodigoTipoMoneda>
    <TotalImpuesto>13000.00000</TotalImpuesto>
    <TotalComprobante>113000.00000</TotalComprobante>
  </ResumenFactura>
  <ds:Signature xmlns:ds="http://www.w3.org/2000/09/xmldsig#" Id="Signature-1"></ds:Signature>
</FacturaElectronica>
"""#

private func mensajeDeAceptacion(_ c: ComprobanteRecibido) -> MensajeReceptorRequest {
    MensajeReceptorRequest(
        clave: c.clave, numeroCedulaEmisor: c.emisor.numeroIdentificacion, fechaEmisionDoc: "2026-10-05T09:00:00-06:00",
        mensaje: .aceptado, montoTotalImpuesto: 13000, codigoActividad: "471100", condicionImpuesto: "01",
        montoTotalImpuestoAcreditar: 13000, totalFactura: Monto(c.totalComprobante)!,
        numeroCedulaReceptor: c.receptor.numeroIdentificacion, numeroConsecutivoReceptor: "00100001050000000001",
        fechaEmisionComprobante: c.fechaEmision)
}

@Suite("mensaje receptor")
struct MensajeReceptorTests {
    @Test func leeLaFacturaDelProveedor() throws {
        let r = try FacturaElectronica.leerComprobante(xml: facturaRecibida)
        #expect(r.exito, "\(r.diagnosticos)")
        let c = try #require(r.comprobante)
        #expect(c.tipoDocumento == "01")
        #expect(c.nombreDocumento == "FacturaElectronica")
        #expect(c.version == "4.4")
        #expect(c.clave == "50601102600310112345600100001010000000001112345678")
        #expect(c.emisor.nombre == "SOFTWARE EJEMPLO S.A.")
        #expect(c.emisor.numeroIdentificacion == "3101123456")
        #expect(c.emisor.correo == "facturacion@ejemplo.cr")
        #expect(c.receptor.numeroIdentificacion == "3101987654")
        #expect(c.codigoMoneda == "CRC")
        #expect(c.totalComprobante == "113000.00000")
        #expect(c.firmado)
        #expect(c.plazoConfirmacion == "2026-11-11")
    }

    @Test func rechazaLoQueNoSePuedeConfirmar() throws {
        // XML sin firma: se lee, pero no se puede confirmar
        let sinFirma = facturaRecibida.replacingOccurrences(of: #"<ds:Signature xmlns:ds="http://www.w3.org/2000/09/xmldsig#" Id="Signature-1"></ds:Signature>"#, with: "")
        let r = try FacturaElectronica.leerComprobante(xml: sinFirma)
        #expect(!r.exito)
        #expect(r.diagnosticos.contains { $0.codigo == "MR_SIN_FIRMA" })
        #expect(r.comprobante?.firmado == false)

        // Texto que ni siquiera es XML
        let basura = try FacturaElectronica.leerComprobante(xml: "esto no es xml")
        #expect(!basura.exito)
        #expect(basura.comprobante == nil)
    }

    @Test func confirmaLaFactura() throws {
        let recibido = try #require(try FacturaElectronica.leerComprobante(xml: facturaRecibida).comprobante)
        let r = try FacturaElectronica.mensajeReceptor(mensajeDeAceptacion(recibido))
        #expect(r.exito, "\(r.diagnosticos)")
        let xml = try #require(r.xml)
        #expect(xml.contains("<MensajeReceptor"))
        #expect(xml.contains("<TotalFactura>113000.00000</TotalFactura>"))
        #expect(xml.contains("<NumeroConsecutivoReceptor>00100001050000000001</NumeroConsecutivoReceptor>"))
        #expect(r.plazoConfirmacion == "2026-11-11")
    }

    @Test func diagnosticosConLaRutaDelCampo() throws {
        let recibido = try #require(try FacturaElectronica.leerComprobante(xml: facturaRecibida).comprobante)
        var m = mensajeDeAceptacion(recibido)
        m.numeroConsecutivoReceptor = "00100001070000000001"   // el tipo 07 es de rechazo, no de aceptación
        let r = try FacturaElectronica.mensajeReceptor(m)
        #expect(!r.exito)
        #expect(r.xml == nil)
        let d = try #require(r.diagnosticos.first { $0.codigo == "RULE_MR_CONSECUTIVO_TIPO" })
        #expect(d.ruta == "numeroConsecutivoReceptor")
    }

    @Test func fueraDePlazoEsUnaAdvertenciaNoUnError() throws {
        let recibido = try #require(try FacturaElectronica.leerComprobante(xml: facturaRecibida).comprobante)
        var m = mensajeDeAceptacion(recibido)
        m.fechaEmisionDoc = "2026-12-30T09:00:00-06:00"
        let r = try FacturaElectronica.mensajeReceptor(m)
        #expect(r.exito, "\(r.diagnosticos)")
        let d = try #require(r.diagnosticos.first { $0.codigo == "RULE_MR_FUERA_DE_PLAZO" })
        #expect(d.severidad == .advertencia)
    }

    @Test func requestDesdeJson() throws {
        let json = #"""
        {"clave":"50601102600310112345600100001010000000001112345678","numeroCedulaEmisor":"3101123456",
         "fechaEmisionDoc":"2026-10-05T09:00:00-06:00","mensaje":"3","detalleMensaje":"Mercadería no recibida",
         "totalFactura":113000,"numeroCedulaReceptor":"3101987654","numeroConsecutivoReceptor":"00100001070000000001"}
        """#
        let m = try JSONDecoder().decode(MensajeReceptorRequest.self, from: Data(json.utf8))
        #expect(m.mensaje == .rechazado)
        #expect(m.totalFactura == 113000)
        let r = try FacturaElectronica.mensajeReceptor(m)
        #expect(r.exito, "\(r.diagnosticos)")
        #expect(r.xml?.contains("<Mensaje>3</Mensaje>") == true)
    }
}
