# @cec-cr/core

Binding TypeScript/JavaScript de Comprobantes Electrónicos Core. Ejecuta el core en C++ compilado a WebAssembly, en el navegador o en Node.js, sin servidor.

- `procesar`: valida las reglas del Anexo 1 v4.4, calcula los montos y genera el XML sin firmar.
- `calcular`: vista previa de los montos aunque el comprobante esté incompleto.
- `firmar` y `leerCertificado`: firma XAdES-EPES con un certificado PKCS#12.
- `catalogos` y `territorio`: los mismos catálogos y división territorial que usan las reglas.
- `generarClave`, `generarConsecutivo` y `fechaEmision`: numeración del comprobante (TypeScript puro).

## Requisitos

- El módulo WebAssembly del core: `cec.js` y `cec.wasm`, generados desde la raíz del repositorio con `make wasm` (o `scripts/build_wasm.sh`) en `dist/wasm/`.
- Node.js 22.18 o superior para los tests y el uso en Node.

## Compilar el paquete

```bash
cd bindings/typescript
npm install            # TypeScript (devDependency)
npm run build          # dist/ con JavaScript y declaraciones .d.ts
npm run typecheck
npm test               # tests contra dist/wasm/cec.js
```

El código fuente usa imports con extensión `.ts`; Node 22.18 o superior lo ejecuta directamente y `tsc` reescribe las extensiones a `.js` al compilar.

## Cargar el core

`FacturaElectronica.cargar` recibe la factory que genera Emscripten (`createCecModule`, exportada por `cec.js`) o un módulo ya creado, y verifica que el ABI del módulo sea el que espera el binding.

En Node.js:

```ts
import { createRequire } from "node:module";
import { FacturaElectronica } from "@cec-cr/core";

const require = createRequire(import.meta.url);
const fe = await FacturaElectronica.cargar(require("./dist/wasm/cec.js"));
```

En el navegador, con `cec.js` y `cec.wasm` publicados en la raíz del sitio:

```ts
await new Promise((ok, error) => {
  const script = Object.assign(document.createElement("script"), { src: "/cec.js", onload: ok, onerror: error });
  document.head.appendChild(script);
});
const fe = await FacturaElectronica.cargar((globalThis as any).createCecModule, {
  locateFile: (archivo: string) => `/${archivo}`,
});
```

## Procesar un comprobante

```ts
import { fechaEmision, generarClave, generarConsecutivo, type ComprobanteRequest } from "@cec-cr/core";

const fecha = fechaEmision();   // hora actual de Costa Rica, RFC 3339
const numeroConsecutivo = generarConsecutivo({ sucursal: 1, terminal: 1, tipoDocumento: "01", numero: 1 });
const clave = generarClave({ fechaEmision: fecha, identificacion: "3101000101", numeroConsecutivo, codigoSeguridad: "10000001" });

const request: ComprobanteRequest = {
  tipoDocumento: "01",
  clave,
  proveedorSistemas: "3101000999",
  codigoActividadEmisor: "6201.0",
  numeroConsecutivo,
  fechaEmision: fecha,
  emisor: {
    nombre: "SOLUCIONES DE SOFTWARE EJEMPLO S.A.",
    identificacion: { tipo: "02", numero: "3101000101" },
    ubicacion: { provincia: "1", canton: "01", distrito: "08", otrasSenas: "Avenida 10" },
    correoElectronico: ["facturacion@software.example.com"],
  },
  receptor: {
    nombre: "COMERCIALIZADORA CLIENTE EJEMPLO S.A.",
    identificacion: { tipo: "02", numero: "3101000202" },
    correoElectronico: "cuentasporpagar@cliente.example.com",
  },
  condicionVenta: "01",
  detalleServicio: [{
    codigoCABYS: "8314100000200", cantidad: "40", unidadMedida: "h",
    detalle: "Desarrollo de software a la medida (horas)", precioUnitario: "25000",
    impuestos: [{ codigo: "01", codigoTarifaIVA: "08" }],
  }],
  resumenFactura: { codigoTipoMoneda: { codigoMoneda: "CRC" }, mediosPago: [{ tipoMedioPago: "04" }] },
};

const resultado = fe.procesar(request);
if (resultado.exito) {
  console.log(resultado.xml);
  console.log(resultado.comprobante?.ResumenFactura.TotalComprobante);   // "1130000.00000"
} else {
  for (const d of resultado.diagnosticos) console.log(d.codigo, d.ruta, d.mensaje);
}
```

`ComprobanteRequest` refleja el JSON Schema del request ([`schemas/comprobante_request.schema.json`](../../schemas/comprobante_request.schema.json)). Los códigos de catálogo son tipos unión (`"01" | "02" | ...`) y los montos aceptan texto o número; se recomienda texto para no perder precisión.

### Resultado

| Campo | Contenido |
|---|---|
| `exito` | `true` si no hubo errores (puede haber advertencias) |
| `diagnosticos` | `{ severidad, codigo, ruta, mensaje }`; `ruta` usa los nombres del request, ej. `detalleServicio[0].impuestos[1].codigo` |
| `xml` | XML sin firmar (solo con `exito`) |
| `comprobante` | Comprobante calculado con las etiquetas del XML v4.4 (solo con `exito`); los decimales vienen como texto |

Un comprobante inválido no lanza excepción. Un JSON con tipos incorrectos devuelve el diagnóstico `JSON_DECODE_ERROR` con la ruta del campo. Solo se lanza `Error` ante un error de uso del módulo (por ejemplo, un ABI incompatible).

## Vista previa

`calcular` devuelve `comprobante` (con los montos) aunque las reglas reporten errores; solo no lo devuelve ante errores de formato. Nunca genera XML. Es útil para mostrar totales mientras el usuario completa el formulario.

```ts
const previa = fe.calcular(requestIncompleto);
previa.comprobante?.ResumenFactura.TotalComprobante;
```

## Firma

```ts
const p12 = new Uint8Array(await archivo.arrayBuffer());

const lectura = fe.leerCertificado(p12, pin);
lectura.certificado?.sujeto;
lectura.certificado?.vigente;

const firmado = fe.firmar(resultado.xml!, p12, pin);
if (firmado.exito) enviarAHacienda(firmado.xml!);
```

El certificado y el PIN no salen del proceso: la firma se hace dentro del módulo WebAssembly.

### Firma en dos pasos (llave fuera del navegador)

Cuando la llave privada vive en un keystore de servidor o un HSM, el módulo arma la firma con el certificado público y deja afuera solo la operación RSA:

```ts
const prep = fe.prepararFirma(resultado.xml!, certificadoDer);       // X.509 DER, sin llave
const firma = await servidor.firmar(prep.signedInfo!);               // SHA256withRSA, en Base64
const firmado = fe.completarFirma(prep.xml!, firma);                 // inserta SignatureValue y verifica
```

`completarFirma` verifica la firma con el certificado de KeyInfo y devuelve `SIGN_VERIFY_FAILED` si la llave no corresponde.

## Mensaje Receptor

Para aceptar (total o parcialmente) o rechazar un comprobante recibido de un proveedor. `leerComprobante` extrae los datos del XML recibido y `mensajeReceptor` valida el mensaje y genera su XML sin firmar, que se firma igual que un comprobante (`firmar`, o `prepararFirma` y `completarFirma`).

```ts
const recibido = fe.leerComprobante(xmlDelProveedor);
if (!recibido.exito) throw new Error(recibido.diagnosticos[0].mensaje);   // por qué no se puede confirmar
const c = recibido.comprobante!;                                           // clave, emisor, receptor, totales, plazo

const r = fe.mensajeReceptor({
    clave: c.clave,
    numeroCedulaEmisor: c.emisor.numeroIdentificacion,
    numeroCedulaReceptor: c.receptor.numeroIdentificacion,
    fechaEmisionDoc: fechaEmision(),
    mensaje: "1",                                    // 1 aceptado, 2 aceptado parcialmente, 3 rechazado
    totalFactura: c.totalComprobante,
    numeroConsecutivoReceptor: "00100001050000000001", // tipo 05, 06 o 07 según el mensaje
    fechaEmisionComprobante: c.fechaEmision,         // opcional: revisa el plazo del art. 10
});
if (r.exito) {
    const firmado = fe.firmar(r.xml!, p12, pin);
}
```

Solo se confirman facturas y notas de débito o crédito. El plazo vencido del art. 10 es una advertencia (`RULE_MR_FUERA_DE_PLAZO`) y `plazoConfirmacion` trae el último día del plazo.

## Catálogos y división territorial

```ts
fe.catalogos.nombres();                         // ["CondicionesVenta", "MediosPago", ...]
fe.catalogos.porNombre("CodigosReferencia");    // [{ codigo: "01", descripcion: "Anula documento de referencia" }, ...]
fe.catalogos.monedas();                         // [{ codigo: "CRC", simbolo: "₡", descripcion: ... }, ...]
fe.catalogos.unidadesMedida();
fe.catalogos.formasFarmaceuticas();

fe.territorio.provincias();                     // [{ codigo: "1", nombre: "San José" }, ...]
fe.territorio.cantones("1");                    // códigos de dos dígitos, como van en el request ("01")
fe.territorio.distritos("1", "08");
fe.territorio.barrios("1", "08", "01");         // en el request, Barrio es el nombre
fe.territorio.esUbicacionValida("1", "08", "01");
```

## Numeración

```ts
generarConsecutivo({ sucursal: 1, terminal: 1, tipoDocumento: "01", numero: 1 });   // 20 dígitos
generarClave({ fechaEmision, identificacion, numeroConsecutivo, situacion: "1", codigoSeguridad });   // 50 posiciones
fechaEmision(new Date());                                                            // "2026-10-15T09:30:00-06:00"
```

En la Factura Electrónica de Compra, la identificación de la clave es la del receptor (quien emite el comprobante). Las funciones lanzan `RangeError` si un dato está fuera de rango.

## Tests

`npm test` carga el módulo real (`dist/wasm/cec.js`) y procesa los casos de referencia del core ([`test/golden`](../../test/golden)): el XML debe ser idéntico al esperado. Los tests de firma generan un certificado temporal con el comando `openssl` y se omiten si no está instalado.
