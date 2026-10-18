# Comprobantes Electrónicos Core (CEC)

[![CI](https://github.com/tyronfonseca/ComprobantesElectronicosCore/actions/workflows/ci.yml/badge.svg)](https://github.com/tyronfonseca/ComprobantesElectronicosCore/actions/workflows/ci.yml)

Biblioteca en C++20 para emitir comprobantes electrónicos de Costa Rica según el documento oficial del Ministerio de Hacienda **"Anexos y Estructuras para la Emisión de Comprobantes Electrónicos" versión 4.4** (incluye la bitácora de ajustes al 22/04/2026).

A partir de los datos de entrada de un comprobante, la biblioteca:

- valida las reglas del Anexo 1 (obligatoriedad por tipo de comprobante, catálogos, formatos, clave y consecutivo, combinaciones permitidas),
- calcula todos los montos de línea y del resumen (descuentos, impuestos tarifarios y específicos, exoneraciones, surtidos, otros cargos, IVA devuelto),
- genera el XML sin firmar, en el orden y con las condiciones del Anexo 1,
- firma el XML con XAdES-EPES (Anexo 2) usando un certificado PKCS#12, o en dos pasos si la llave privada no puede entrar al proceso,
- genera el Mensaje Receptor (aceptación, aceptación parcial o rechazo de un comprobante recibido),
- expone los catálogos de las notas del Anexo 1 y la división territorial de Costa Rica.

El mismo núcleo se usa desde C++, desde C (y cualquier lenguaje con FFI), desde TypeScript/JavaScript (WebAssembly) y desde Swift.

**Estado:** pre-1.0; la API puede cambiar entre versiones (ver el [CHANGELOG](CHANGELOG.md)). No es un producto oficial del Ministerio de Hacienda. Un comprobante que la biblioteca da por válido todavía puede ser rechazado por Hacienda (por ejemplo, por un CABYS que no corresponde a la actividad del emisor): pruebe siempre en el sandbox. Qué hace, qué no y qué está verificado: [docs/LIMITACIONES.md](docs/LIMITACIONES.md).

---

## Comprobantes soportados

| Código | Comprobante |
|:---:|---|
| 01 | Factura Electrónica (FE) |
| 02 | Nota de Débito Electrónica (ND) |
| 03 | Nota de Crédito Electrónica (NC) |
| 04 | Tiquete Electrónico (TE) |
| 08 | Factura Electrónica de Compra (FEC) |
| 09 | Factura Electrónica de Exportación (FEE) |
| 10 | Recibo Electrónico de Pago (REP) |
| 05, 06, 07 | Mensaje Receptor: aceptación, aceptación parcial y rechazo (de una FE, ND o NC recibida) |

---

## Cómo funciona

```
request (datos de entrada)
   │
   ├─ parseo ──────────► comprobante canónico (montos como enteros escalados x100000)
   ├─ reglas ──────────► diagnósticos (error / advertencia) con la ruta del campo
   ├─ cálculo ─────────► montos de línea, desglose de impuestos y resumen
   └─ recorrido ───────► XML v4.4 (y el mismo árbol en JSON)

XML sin firmar ──► firma XAdES-EPES con el certificado .p12
```

- **Solo datos de entrada.** El request no lleva montos calculados (subtotales, impuestos, totales): los calcula el core. En el Recibo Electrónico de Pago se informan `montoTotal` y `subTotal`, porque así lo define el Anexo 1.
- **Diagnósticos en lugar de excepciones.** Un comprobante inválido no lanza: el resultado indica `exito = false` y trae cada diagnóstico con un código estable (`RULE_CABYS_FORMAT`), la ruta del campo en el request (`detalleServicio[0].codigoCABYS`) y un mensaje en español.
- **Un solo lugar para el orden y la presencia de los nodos.** `RecorridoComprobante` contiene la "Condición del Campo" del Anexo 1 por tipo de comprobante y emite eventos a un escritor (patrón visitor). El XML y el JSON son escritores de ese recorrido; agregar otro formato en árbol no duplica reglas.
- **Vista previa.** `calcular` devuelve los montos aunque el comprobante todavía tenga errores de reglas (por ejemplo, un emisor incompleto), sin generar XML. Sirve para interfaces que muestran totales mientras se edita.

### Validaciones que hace Hacienda y no la biblioteca

Algunas verificaciones requieren información que solo tiene el Ministerio de Hacienda: que el código CABYS exista en el catálogo vigente, que la actividad económica esté inscrita en el RUT, que el emisor esté inscrito, la secuencia del consecutivo, y la fecha anterior permitida en la situación "sin internet". La biblioteca valida el formato de esos datos, no su existencia. El catálogo CABYS no se incluye: solo se usan los prefijos que las reglas necesitan (bebidas, jabón de tocador, surtidos habilitados, servicios de salud, medicamentos y medios de transporte). La lista completa de lo que no cubre está en [docs/LIMITACIONES.md](docs/LIMITACIONES.md).

---

## Estructura del proyecto

```
.
├── include/cec/              Headers públicos
│   ├── CEC.h                 Cabecera maestra de la API de C++
│   ├── c_api/                API de C (cec_c_api.h) y module.modulemap
│   ├── common/               Catálogos, división territorial, clasificación CABYS
│   ├── domain/               Comprobante canónico, montos (Money), diagnósticos
│   ├── parsing/              Request de entrada (ComprobanteRequestDTO)
│   ├── rules/                Motor de reglas y reglas del Anexo 1
│   ├── calculation/          Motor de cálculo
│   ├── visitor/              Recorrido del comprobante y escritor XML
│   ├── codec/                Lector del request JSON y escritor JSON
│   ├── crypto/               PKCS#12, canonicalización C14N y firma XAdES-EPES
│   ├── mensaje/              Mensaje Receptor (confirmación o rechazo)
│   └── service/              ComprobanteService: procesar, calcular, firmar
├── src/                      Implementaciones (.cpp)
├── test/                     Tests de C++ (unit, golden) y casos de prueba
├── schemas/                  JSON Schema del request
├── bindings/
│   ├── typescript/           Binding TypeScript/WebAssembly (@cec-cr/core)
│   └── swift/                Binding Swift (Swift Package Manager)
├── examples/
│   ├── cpp/                  Ejemplo de la API de C++
│   ├── c_api/                Ejemplo de la API de C
│   ├── swift/                Ejemplo de Swift
│   └── web/                  Aplicación web de demostración
├── scripts/
│   ├── build_wasm.sh         Compila el core a WebAssembly
│   ├── enviar_comprobante.py Envía un XML firmado al API de Recepción de Hacienda
│   ├── generadores/          Generadores de plantillas y de ejemplos para el sandbox (Python)
│   ├── validar_xsd.py        Valida XML contra los XSD oficiales (xmllint)
│   └── run_tests_mac.sh
├── docs/                     Arquitectura, limitaciones, XSD oficiales y reglas por campo
├── libs/                     Dependencias incluidas (pugixml, nlohmann/json, GoogleTest)
├── AGENTS.md, CLAUDE.md      Guía para agentes de código
└── CONTRIBUTING.md, SECURITY.md, CHANGELOG.md
```

---

## Requisitos

| Para | Requiere |
|---|---|
| Biblioteca y tests | Compilador C++20 (probado con Apple Clang 21; el CI usa GCC 13), OpenSSL 3. MSVC también está verificado en el CI (14.51; no se han probado versiones anteriores) |
| Tests | Submódulo `libs/googletest` (`git submodule update --init`) |
| WebAssembly | Emscripten (`em++`); el script compila OpenSSL para wasm la primera vez |
| Binding TypeScript | Node.js 22.18 o superior |
| Binding Swift | Swift 5.9 o superior, macOS 13 o superior, OpenSSL 3 de Homebrew |

En macOS: `brew install openssl@3`. En Debian/Ubuntu: `apt install libssl-dev`. Plataformas verificadas y límites conocidos: [docs/LIMITACIONES.md](docs/LIMITACIONES.md).

---

## Compilación

### Make

```bash
make                     # librería estática y dinámica (build/libComprobantesElectronicosCore.*) y tests
make test                # compila y ejecuta los tests de C++
make test FILTER='Reglas*'
make examples            # compila y ejecuta los ejemplos de C++ y de C
make BUILD_MODE=release  # -O3 (por defecto: -O0 -g con verificaciones de libc++)
make tsan                # test de hilos con ThreadSanitizer (build_tsan/)
make wasm                # dist/wasm/cec.js y cec.wasm (también los copia al ejemplo web)
make clean
```

### CMake

```bash
cmake -B build_cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build_cmake --parallel
ctest --test-dir build_cmake --output-on-failure
```

El target es `ComprobantesElectronicosCore`, con el alias `cec::core`. Use `-DBUILD_SHARED_LIBS=ON` para la librería dinámica y `-DBUILD_TESTING=OFF` para omitir los tests.

### Swift

```bash
swift build
swift test
swift run FacturaSwiftExample
```

---

## El request

El request es el mismo en todos los lenguajes y está descrito en [`schemas/comprobante_request.schema.json`](schemas/comprobante_request.schema.json). Cada campo se llama como su etiqueta del Anexo 1 en camelCase y las listas van en plural. Los códigos de catálogo son texto de dos dígitos (`"01"`) y los montos son decimales en texto (`"1500.50"`) para no perder precisión.

```json
{
  "tipoDocumento": "01",
  "clave": "50601102600310100010100100001010000000001110000001",
  "proveedorSistemas": "3101000999",
  "codigoActividadEmisor": "6201.0",
  "numeroConsecutivo": "00100001010000000001",
  "fechaEmision": "2026-10-01T10:00:00-06:00",
  "emisor": {
    "nombre": "SOLUCIONES DE SOFTWARE EJEMPLO S.A.",
    "identificacion": { "tipo": "02", "numero": "3101000101" },
    "ubicacion": { "provincia": "1", "canton": "01", "distrito": "08", "otrasSenas": "Avenida 10" },
    "correoElectronico": ["facturacion@software.example.com"]
  },
  "receptor": {
    "nombre": "COMERCIALIZADORA CLIENTE EJEMPLO S.A.",
    "identificacion": { "tipo": "02", "numero": "3101000202" },
    "correoElectronico": "cuentasporpagar@cliente.example.com"
  },
  "condicionVenta": "01",
  "detalleServicio": [
    {
      "codigoCABYS": "8314100000200",
      "cantidad": "40",
      "unidadMedida": "h",
      "detalle": "Desarrollo de software a la medida (horas)",
      "precioUnitario": "25000",
      "impuestos": [{ "codigo": "01", "codigoTarifaIVA": "08" }]
    }
  ],
  "resumenFactura": {
    "codigoTipoMoneda": { "codigoMoneda": "CRC" },
    "mediosPago": [{ "tipoMedioPago": "04" }]
  }
}
```

Hay ejemplos completos y válidos de cada tipo de comprobante en [`examples/web/src/templates`](examples/web/src/templates) (se generan con `python3 scripts/generadores/generar_plantillas.py`) y en [`test/golden`](test/golden).

---

## Uso

### C++

```cpp
#include "cec/CEC.h"

cec::parsing::ComprobanteRequestDTO req;
req.tipoDocumento = "01";
// ... clave, consecutivo, fecha, emisor, receptor, líneas (ver examples/cpp)

auto resultado = cec::service::ComprobanteService::process(req);
if (resultado.isSuccess) {
    std::cout << resultado.xml;
    std::cout << resultado.comprobante->resumen.totalComprobante.toString(5);
} else {
    for (const auto& d : resultado.diagnostics.items())
        std::cout << d.code << " [" << d.path << "]: " << d.message << "\n";
}

// Firma XAdES-EPES
auto firma = cec::service::ComprobanteService::signXml(resultado.xml, bytesP12, pin);
```

El request también se puede leer desde JSON con `cec::codec::JsonCodec::decodeRequest(texto)`. Ejemplo completo: [`examples/cpp/FacturaCppExample.cpp`](examples/cpp/FacturaCppExample.cpp).

### C

La API de C ([`include/cec/c_api/cec_c_api.h`](include/cec/c_api/cec_c_api.h)) recibe el request como structs (`cec_request_c` y sus partes, con los mismos nombres del JSON) o como JSON, y expone el resultado con funciones de acceso, sin necesidad de leer JSON.

```c
#include "cec/c_api/cec_c_api.h"

cec_request_c req = { 0 };
req.tipoDocumento = "01";
/* ... ver examples/c_api/FacturaCApiExample.c */

cec_result* res = NULL;
if (cec_build(&req, &res) == CEC_STATUS_OK) {
    int32_t exito = 0;
    cec_result_is_success(res, &exito);
    if (exito) {
        const uint8_t* xml; size_t len;
        cec_result_xml(res, &xml, &len);
    } else {
        size_t total = 0;
        cec_result_total_diagnosticos(res, &total);
        for (size_t i = 0; i < total; ++i) {
            const char *codigo, *ruta, *mensaje;
            cec_result_diagnostico(res, i, NULL, &codigo, &ruta, &mensaje);
        }
    }
    cec_result_free(res);
}
```

| Función | Uso |
|---|---|
| `cec_build` / `cec_build_json` | Validar, calcular y generar el XML |
| `cec_calcular` / `cec_calcular_json` | Vista previa de montos sin XML |
| `cec_sign_xml` | Firmar un XML con XAdES-EPES |
| `cec_preparar_firma` / `cec_completar_firma` | Firma en dos pasos cuando la llave privada está en otro lugar (keystore, HSM): preparar con el certificado público, firmar `signedInfo` afuera con RSA-SHA256 y completar (verifica la firma) |
| `cec_leer_certificado` | Leer titular y vigencia de un .p12 |
| `cec_mensaje_receptor` / `cec_mensaje_receptor_json` | Validar y generar el XML del Mensaje Receptor (struct o JSON) |
| `cec_leer_comprobante_xml` | Leer el XML de un comprobante recibido para confirmarlo (`cec_result_comprobante_recibido`, `cec_result_plazo_confirmacion`) |
| `cec_result_is_success`, `cec_result_xml`, `cec_result_totales`, `cec_result_diagnostico`, `cec_result_certificado`, `cec_result_data` | Leer el resultado |
| `cec_catalogo_*`, `cec_dta_*` | Catálogos y división territorial |
| `cec_abi_version` | Versión del ABI (actualmente 3) |

Las convenciones de la API (textos `NULL` = no informado, `CEC_SIN_VALOR` para enteros opcionales, listas como puntero más cantidad) están documentadas en el header. Ejemplo completo: [`examples/c_api/FacturaCApiExample.c`](examples/c_api/FacturaCApiExample.c).

### TypeScript / JavaScript (WebAssembly)

Ver [`bindings/typescript/README.md`](bindings/typescript/README.md) y la aplicación de demostración en [`examples/web`](examples/web).

### Swift

Ver [`bindings/swift/README.md`](bindings/swift/README.md).

### Mensaje Receptor

Para confirmar (total o parcialmente) o rechazar una factura o nota recibida de un proveedor: se lee el XML recibido, se arma el mensaje con los nodos de `MensajeReceptor_V4.4.xsd` y el XML resultante se firma igual que un comprobante.

```cpp
#include "cec/CEC.h"

auto [recibido, diags] = cec::mensaje::leerComprobante(xmlDelProveedor);   // clave, emisor, receptor, totales, plazo
if (diags.hasErrors()) { /* por qué no se puede confirmar */ }

cec::mensaje::MensajeReceptorRequest m;
m.clave = recibido.clave;
m.numeroCedulaEmisor = recibido.emisorNumeroIdentificacion;
m.numeroCedulaReceptor = recibido.receptorNumeroIdentificacion;
m.fechaEmisionDoc = "2026-10-16T10:00:00-06:00";
m.mensaje = "1";                                      // 1 aceptado, 2 aceptado parcialmente, 3 rechazado
m.totalFactura = recibido.totalComprobante;
m.numeroConsecutivoReceptor = "00100001050000000001"; // tipo 05, 06 o 07 según el mensaje
m.fechaEmisionComprobante = recibido.fechaEmision;    // opcional: revisa el plazo del art. 10

auto r = cec::mensaje::procesar(m);                   // r.isSuccess, r.xml, r.diagnostics, r.plazoConfirmacion
```

Solo se confirman facturas y notas de débito o crédito. Un mensaje fuera del plazo del art. 10 (8 días hábiles desde el primer día del mes siguiente) produce la advertencia `RULE_MR_FUERA_DE_PLAZO`, no un error. Desde C: `cec_leer_comprobante_xml` con `cec_result_comprobante_recibido`, y `cec_mensaje_receptor` (struct) o `cec_mensaje_receptor_json`; desde TypeScript y Swift: `leerComprobante` y `mensajeReceptor`.

---

## Envío a Hacienda

[`scripts/enviar_comprobante.py`](scripts/enviar_comprobante.py) envía un XML firmado al API de Recepción (Anexo 3) y espera la respuesta. Usa solo la biblioteca estándar de Python 3.9 o superior y lee las credenciales de un archivo `.env` (ver [`.env.example`](.env.example); el `.env` no se versiona).

```bash
cp .env.example .env     # completar HACIENDA_USUARIO y HACIENDA_CLAVE (generados en ATV)
python3 scripts/enviar_comprobante.py comprobante_firmado.xml --simular   # muestra el envío sin conectarse
python3 scripts/enviar_comprobante.py comprobante_firmado.xml             # envía y consulta el estado
python3 scripts/enviar_comprobante.py --consultar <clave>                 # consulta un comprobante enviado
```

Para probar en el sandbox con datos reales, [`scripts/generadores/generar_ejemplos_sandbox.py`](scripts/generadores/generar_ejemplos_sandbox.py) genera un juego de comprobantes relacionados (FE de contado y a crédito, NC y ND que referencian la FE de contado, TE, FEC, FEE y un REP que paga la FE a crédito). Toma el emisor, el receptor, sus códigos de actividad y el CABYS de las líneas de `scripts/generadores/datos_sandbox.json` (ver [`datos_sandbox.example.json`](scripts/generadores/datos_sandbox.example.json); el archivo y la carpeta de salida `ejemplos_sandbox/` no se versionan). El CABYS debe corresponder a la actividad del emisor, o Hacienda rechaza el comprobante.

```bash
cp scripts/generadores/datos_sandbox.example.json scripts/generadores/datos_sandbox.json   # completar con los datos reales
python3 scripts/generadores/generar_ejemplos_sandbox.py   # escribe los JSON en ejemplos_sandbox/
```

Los JSON se cargan en el ejemplo web (Cargar JSON), que los valida y firma, y el XML firmado se envía con `enviar_comprobante.py`. Se deben enviar en el orden de los archivos, porque las notas y el recibo referencian las facturas.

`HACIENDA_AMBIENTE` elige `sandbox` (predeterminado) o `produccion`. La respuesta de Hacienda (MensajeHacienda) se guarda junto al XML como `<clave>_respuesta.xml`. El script termina con 0 si el comprobante fue aceptado, 1 si fue rechazado, 2 si sigue en proceso y 3 ante un error.

---

## Catálogos incluidos

- Los 16 catálogos de las notas del Anexo 1 que usan las reglas (condiciones de venta, medios de pago, tipos de identificación, impuestos, tarifas de IVA, descuentos, exoneraciones, instituciones, otros cargos, documentos y códigos de referencia, entre otros).
- División territorial: 7 provincias, 84 cantones, 492 distritos y 7 087 barrios.
- 168 monedas (ISO 4217, sin los códigos que el XSD v4.4 ya no admite), 101 unidades de medida (el XSD distingue mayúsculas: `Kg`, `M`, `Min`, `kWh`) y 229 formas farmacéuticas (Nota 19).

---

## Pruebas

Ver [`test/README.md`](test/README.md). Resumen: `make test` ejecuta los tests de C++; los bindings tienen sus propios tests (`node --test` en `bindings/typescript` y `swift test`) que comparan su salida con los mismos casos de referencia. `make tsan` ejecuta el test de uso concurrente con ThreadSanitizer. `python3 scripts/validar_xsd.py test/golden/*/expected.xml` valida los casos de referencia contra los XSD oficiales (necesita `xmllint`).

---

## Documentación

| Documento | Contenido |
|---|---|
| [docs/ARQUITECTURA.md](docs/ARQUITECTURA.md) | Flujo, capas, dinero, reglas, recorrido, firma, API de C y bindings |
| [docs/LIMITACIONES.md](docs/LIMITACIONES.md) | Requisitos, plataformas verificadas, qué valida y qué no, funcionalidad ausente |
| [CONTRIBUTING.md](CONTRIBUTING.md) | Cómo proponer cambios |
| [SECURITY.md](SECURITY.md) | Cómo reportar una vulnerabilidad |
| [CHANGELOG.md](CHANGELOG.md) | Cambios entre versiones |
| [AGENTS.md](AGENTS.md) | Guía para agentes de código: reglas, mapa de archivos, comandos y trampas conocidas (`CLAUDE.md` la importa) |
| [docs/reglas_condiciones_campos](docs/reglas_condiciones_campos) | Condición de cada campo del Anexo 1 por tipo de comprobante |
| [docs/hacienda_schemas/v4.4](docs/hacienda_schemas/v4.4) | XSD oficiales y catálogos de Hacienda |

---

## Aviso legal

**Documentos públicos de Hacienda.** Los esquemas XSD, el documento de codificación territorial y el catálogo de formas farmacéuticas de `docs/hacienda_schemas` son información pública publicada por la Dirección General de Tributación en su portal oficial. Se incluyen aquí solo como referencia técnica para desarrollar y probar la biblioteca; este proyecto no los modifica, su autoría y sus derechos pertenecen al Ministerio de Hacienda y **no están cubiertos por la licencia MIT** de este proyecto. Las tablas de `docs/reglas_condiciones_campos` y los catálogos del código son transcripciones o derivados del documento "Anexos y Estructuras para la Emisión de Comprobantes Electrónicos". La versión vigente y vinculante es siempre la del portal oficial: verifíquela allí antes de emitir comprobantes.

**Origen de los documentos.** Los esquemas y anexos de [`docs/hacienda_schemas`](docs/hacienda_schemas/v4.4) se obtuvieron del portal oficial de la Dirección General de Tributación: [Ministerio de Hacienda - Anexos y Estructuras de Comprobantes Electrónicos](https://atv.hacienda.go.cr/ATV/ComprobanteElectronico/frmAnexosyEstructuras.aspx).

**No afiliación.** Este proyecto no tiene relación oficial con el Ministerio de Hacienda ni con la Dirección General de Tributación. La única fuente normativa y legalmente vinculante es la publicada en sus canales oficiales.

**Desarrollo con asistencia de IA.** Una parte importante del código, las pruebas, los scripts y la documentación de este proyecto fue escrita con la asistencia de Claude, un modelo de inteligencia artificial de Anthropic, usado a través de Claude Code y bajo la dirección del autor. Como con cualquier software que genera documentos fiscales, revise el código y pruebe en el sandbox de Hacienda antes de usarlo en producción; que haya pruebas automáticas no garantiza que no haya errores.

**Datos de prueba.** Todos los datos de los ejemplos y las pruebas (nombres, identificaciones, correos, teléfonos, direcciones y montos) son ficticios. Cualquier similitud con personas o empresas reales es coincidencia.

---

## Licencia

El código se distribuye bajo la [Licencia MIT](LICENSE.txt). Copyright (c) 2025-2026 Tyron Fonseca.

Las dependencias de terceros y sus licencias se describen en [`libs/README.md`](libs/README.md) y [`THIRD_PARTY_LICENSES.md`](THIRD_PARTY_LICENSES.md).
