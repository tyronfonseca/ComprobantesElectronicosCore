# ComprobantesElectronicosCore para Swift

Binding Swift de Comprobantes Electrónicos Core. Usa el core en C++ a través de los structs de la API de C (`cec_request_c`) y de sus funciones de acceso al resultado; no usa JSON.

- `procesar`: valida las reglas del Anexo 1 v4.4, calcula los montos y genera el XML sin firmar.
- `calcular`: vista previa de los totales aunque el comprobante esté incompleto.
- `firmar` y `leerCertificado`: firma XAdES-EPES con un certificado PKCS#12.
- `prepararFirma` y `completarFirma`: firma en dos pasos cuando la llave privada está fuera del proceso (keystore, HSM).
- `leerComprobante` y `mensajeReceptor`: Mensaje Receptor (aceptación, aceptación parcial o rechazo de un comprobante recibido).
- `catalogos` y `territorio`: los mismos catálogos y división territorial que usan las reglas.
- `generarClave`, `generarConsecutivo` y `fechaEmision`: numeración del comprobante.

## Requisitos

- Swift 5.9 o superior y macOS 13 o superior.
- OpenSSL 3 de Homebrew: `brew install openssl@3`. El paquete lo encuentra con pkg-config a través del target de sistema `COpenSSL`.

iOS no está soportado todavía: requiere empaquetar OpenSSL como XCFramework. Linux y Windows no se han verificado.

## Instalación

En `Package.swift`:

```swift
dependencies: [
    .package(url: "<url del repositorio>", branch: "main")
],
targets: [
    .target(name: "MiApp", dependencies: [
        .product(name: "ComprobantesElectronicosCore", package: "ComprobantesElectronicosCore")
    ])
]
```

En Xcode: File > Add Package Dependencies, con la URL o la ruta local del repositorio.

## Procesar un comprobante

```swift
import ComprobantesElectronicosCore

let fecha = fechaEmision()   // hora actual de Costa Rica, RFC 3339
let consecutivo = try generarConsecutivo(sucursal: 1, terminal: 1, tipoDocumento: .facturaElectronica, numero: 1)
let clave = try generarClave(fechaEmision: fecha, identificacion: "3101000101",
                             numeroConsecutivo: consecutivo, codigoSeguridad: "10000001")

let request = ComprobanteRequest(
    tipoDocumento: .facturaElectronica,
    clave: clave,
    proveedorSistemas: "3101000999",
    codigoActividadEmisor: "6201.0",
    numeroConsecutivo: consecutivo,
    fechaEmision: fecha,
    emisor: Emisor(
        nombre: "SOLUCIONES DE SOFTWARE EJEMPLO S.A.",
        identificacion: Identificacion(tipo: .juridica, numero: "3101000101"),
        ubicacion: Ubicacion(provincia: "1", canton: "01", distrito: "08", otrasSenas: "Avenida 10"),
        correoElectronico: ["facturacion@software.example.com"]),
    receptor: Receptor(
        nombre: "COMERCIALIZADORA CLIENTE EJEMPLO S.A.",
        identificacion: Identificacion(tipo: .juridica, numero: "3101000202"),
        correoElectronico: "cuentasporpagar@cliente.example.com"),
    condicionVenta: .contado,
    detalleServicio: [
        LineaDetalle(codigoCABYS: "8314100000200", cantidad: 40, unidadMedida: "h",
                     detalle: "Desarrollo de software a la medida (horas)", precioUnitario: 25000,
                     impuestos: [.iva(.general13)])
    ],
    resumenFactura: ResumenFacturaRequest(mediosPago: [MedioPago(tipoMedioPago: .transferencia)]))

let resultado = try FacturaElectronica.procesar(request)
if resultado.exito {
    print(resultado.xml!)
    print(resultado.totales!.totalComprobante)   // 1130000
} else {
    for d in resultado.diagnosticos {
        print(d.codigo, d.ruta, d.mensaje)
    }
}
```

Ejemplo completo: [`examples/swift/main.swift`](../../examples/swift/main.swift) (`swift run FacturaSwiftExample`).

### Tipos del request

Los tipos (`ComprobanteRequest`, `LineaDetalle`, `Impuesto`, `Exoneracion`, `OtrosCargos`, `InformacionReferencia`...) tienen los mismos nombres que el JSON Schema del request ([`schemas/comprobante_request.schema.json`](../../schemas/comprobante_request.schema.json)).

- Los catálogos principales son enums: `TipoDocumento`, `TipoIdentificacion`, `CondicionVenta`, `TipoMedioPago`, `CodigoImpuesto`, `CodigoTarifaIVA` y `Situacion`. El resto de códigos son `Codigo` (texto, ej. `"07"`).
- Los montos son `Monto`, un `Decimal` sin pérdida de precisión. Acepta literales enteros (`25000`) y de texto (`"1500.25"`).
- Los tipos son `Codable`, para guardar o leer requests en JSON desde la aplicación. El core no recibe JSON: el binding convierte el request a los structs de C.

### Resultado

| Propiedad | Contenido |
|---|---|
| `exito` | `true` si no hubo errores (puede haber advertencias) |
| `diagnosticos` | `Diagnostico(severidad, codigo, ruta, mensaje)`; `ruta` usa los nombres del request, ej. `detalleServicio[0].codigoCABYS` |
| `xml` | XML sin firmar (solo con `exito`) |
| `totales` | `Totales` del ResumenFactura: `totalVenta`, `totalImpuesto`, `totalComprobante`, etc. (solo con `exito`) |

Un comprobante inválido no lanza: viene con `exito == false`. Solo se lanza `ErrorCEC` ante un error de uso de la API de C o un ABI incompatible.

## Vista previa

`calcular` devuelve los totales aunque las reglas reporten errores; solo no los devuelve ante errores de formato. Nunca genera XML.

```swift
let previa = try FacturaElectronica.calcular(requestIncompleto)
previa.totales?.totalComprobante
```

## Firma

```swift
let p12 = try Data(contentsOf: urlCertificado)

let lectura = try FacturaElectronica.leerCertificado(p12: p12, pin: pin)
lectura.certificado?.sujeto
lectura.certificado?.vigente

let firmado = try FacturaElectronica.firmar(xml: resultado.xml!, p12: p12, pin: pin)
if firmado.exito { enviarAHacienda(firmado.xml!) }
```

### Firma en dos pasos (llave fuera del proceso)

Cuando la llave privada vive en un keystore de servidor o un HSM, el core arma la firma con el certificado público y solo la operación RSA se hace afuera:

```swift
let prep = try FacturaElectronica.prepararFirma(xml: resultado.xml!, certificadoDer: certificadoDer)   // X.509 DER, sin llave
let firma = try await servidor.firmar(prep.signedInfo!)                                              // SHA256withRSA, en Base64
let firmado = try FacturaElectronica.completarFirma(xmlPreparado: prep.xml!, signatureValueBase64: firma)
```

`completarFirma` verifica la firma con el certificado de `KeyInfo` y devuelve `SIGN_VERIFY_FAILED` si la llave no corresponde.

## Mensaje Receptor

Para aceptar (total o parcialmente) o rechazar una factura o nota recibida de un proveedor. `leerComprobante` extrae los datos del XML recibido y `mensajeReceptor` valida el mensaje y genera su XML sin firmar, que se firma igual que un comprobante. Solo se confirman facturas y notas de débito o crédito.

```swift
let leido = try FacturaElectronica.leerComprobante(xml: xmlDelProveedor)
guard leido.exito, let c = leido.comprobante else { /* leido.diagnosticos dice por qué no se puede confirmar */ return }

let mensaje = MensajeReceptorRequest(
    clave: c.clave,
    numeroCedulaEmisor: c.emisor.numeroIdentificacion,
    fechaEmisionDoc: fechaEmision(),
    mensaje: .aceptado,                                  // .aceptadoParcialmente, .rechazado
    totalFactura: Monto(c.totalComprobante)!,
    numeroCedulaReceptor: c.receptor.numeroIdentificacion,
    numeroConsecutivoReceptor: "00100001050000000001",   // tipo 05, 06 o 07 según el mensaje
    fechaEmisionComprobante: c.fechaEmision)             // opcional: revisa el plazo del art. 10

let r = try FacturaElectronica.mensajeReceptor(mensaje)
if r.exito { let firmado = try FacturaElectronica.firmar(xml: r.xml!, p12: p12, pin: pin) }
```

El plazo vencido del art. 10 es una advertencia (`RULE_MR_FUERA_DE_PLAZO`) y `plazoConfirmacion` trae el último día del plazo. Cuando la factura lleva impuesto, el mensaje de aceptación también lleva `montoTotalImpuesto`, `codigoActividad`, `condicionImpuesto` y `montoTotalImpuestoAcreditar`; el rechazo no lleva condición ni montos.

## Catálogos y división territorial

```swift
FacturaElectronica.catalogos.porNombre(.codigosReferencia)   // [ElementoCatalogo(codigo:descripcion:)]
FacturaElectronica.catalogos.monedas()
FacturaElectronica.catalogos.unidadesMedida()
FacturaElectronica.catalogos.formasFarmaceuticas()

FacturaElectronica.territorio.provincias()                   // [Territorio(codigo: "1", nombre: "San José"), ...]
FacturaElectronica.territorio.cantones(provincia: "1")       // códigos de dos dígitos, como van en el request
FacturaElectronica.territorio.distritos(provincia: "1", canton: "08")
FacturaElectronica.territorio.barrios(provincia: "1", canton: "08", distrito: "01")
FacturaElectronica.territorio.esUbicacionValida(provincia: "1", canton: "08", distrito: "01")
```

## Tests

```bash
swift test
```

Los tests procesan los casos de referencia del core ([`test/golden`](../../test/golden)) y comparan el XML con el esperado, así que también verifican la conversión del request a los structs de C. Los tests de firma generan un certificado temporal con el comando `openssl` y se omiten si no está instalado.
