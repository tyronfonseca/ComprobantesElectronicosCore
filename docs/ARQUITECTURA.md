# Arquitectura

Cómo está organizado el core y por qué. Para usarlo, ver el [README](../README.md); para trabajar en él, [AGENTS.md](../AGENTS.md).

## Flujo de un comprobante

```
ComprobanteRequestDTO (texto)            JSON ──► JsonCodec::decodeRequest ─┐
        │                                                                   │
        ▼  parseDto                                                         │
ComprobanteCanonico (tipos fuertes, Money)  ◄───────────────────────────────┘
        │
        ▼  RuleEngine::validate           ──► errores de reglas: se detiene
        ▼  CalculationEngine::calculate   ──► montos de línea y del resumen
        ▼  validateInvariants             ──► coherencia de los totales
        ▼  RecorridoComprobante + escritor (XML, JSON)
XML sin firmar ──► XadesSigner (PKCS#12) ──► XML firmado
```

`ComprobanteService` (en `include/cec/service/ComprobanteService.h`) encadena las etapas:

- `process(request[, engine])`: ejecuta todo el flujo. Se detiene en la primera etapa con errores y solo en el caso exitoso produce `xml` y `comprobante`.
- `calcular(request[, engine])`: vista previa para editar en vivo. Corre las reglas solo para informar, calcula aunque haya errores de reglas y nunca genera XML.
- `signXml`, `prepararFirma`, `completarFirma`, `leerCertificado`: firma y lectura de certificados.

Todas las funciones devuelven un resultado con `diagnostics`; ninguna lanza por datos inválidos.

## Capas y namespaces

| Namespace | Carpeta | Contenido |
|---|---|---|
| `cec::domain` | `include/cec/domain` | `ComprobanteCanonico` (el comprobante ya tipado), `Money`, `Diagnostic`/`DiagnosticList`, enums de los catálogos |
| `cec::common` | `include/cec/common`, `src/common` | Catálogos de las notas del Anexo 1, división territorial, monedas, unidades de medida, formas farmacéuticas, prefijos CABYS |
| `cec::parsing` | `include/cec/parsing` | `ComprobanteRequestDTO`: la forma del request, todo en texto |
| `cec::rules` | `include/cec/rules` | `RuleEngine` y una regla por tema (`IValidationRule`) |
| `cec::calculation` | `include/cec/calculation` | `CalculationEngine` |
| `cec::visitor` | `include/cec/visitor` | `RecorridoComprobante`, `IEscritorArbol`, `IFormatoSalida`, `EscritorXml` |
| `cec::codec` | `include/cec/codec`, `src/codec` | Lector del request JSON y escritor JSON del comprobante |
| `cec::crypto` | `include/cec/crypto`, `src/crypto` | PKCS#12, C14N, firma XAdES-EPES |
| `cec::mensaje` | `include/cec/mensaje` | Mensaje Receptor |
| `cec::service` | `include/cec/service` | Punto de entrada `ComprobanteService` |

Las reglas, el cálculo, el recorrido y el servicio son cabeceras. Lo que se compila aparte es la API de C, el JSON, la criptografía y los catálogos.

## Dinero

`domain::Money` guarda un `int64` escalado por 100000 (5 decimales, el máximo del Anexo 1). Las sumas y restas detectan desbordamiento y la multiplicación por cantidades o porcentajes usa un producto intermedio de 128 bits con redondeo half-up. Ese producto se calcula con dos palabras de 64 bits en C++ estándar (`Aritmetica128.h`), sin `__int128`, para que compile igual en GCC, Clang y MSVC. Cantidades y tarifas también viajan escaladas. El request y el XML usan texto decimal; no hay `double` en la ruta de cálculo.

## Reglas y diagnósticos

`RuleEngine` ejecuta, en este orden, la clave numérica, la fecha de emisión, el consecutivo, la actividad económica, el emisor, el receptor, la condición de venta, los medios de pago, la moneda, las líneas de detalle, la información de referencia y los otros cargos. Cada regla recibe el comprobante y agrega diagnósticos; ninguna lanza.

Un `Diagnostic` tiene severidad (error o advertencia), categoría (`Structural`, `Schema`, `Catalog`, `BusinessRule`, `Calculation`, `PostCondition`), un código estable, la ruta del campo en el request y un mensaje orientativo en español. Quien consuma el resultado decide por código y ruta, nunca por el texto del mensaje.

La regla de fecha de emisión usa un reloj inyectable y una tolerancia de 5 minutos por defecto: `RuleEngine(reloj, tolerancia)` permite cambiarlos (los tests usan un reloj fijo) y `process` y `calcular` aceptan ese motor.

## Cálculo

`CalculationEngine` completa en el comprobante: monto total de la línea, descuentos, subtotal, base imponible, impuestos (IVA, tarifarios y específicos), exoneraciones, surtidos, otros cargos, IVA devuelto y el resumen con su desglose. El Recibo de Pago es un caso aparte: no tiene cantidad ni precio, el monto es `montoTotal` y el impuesto se calcula sobre `subTotal`. `validateInvariants` comprueba después que el total del comprobante cuadre con sus componentes y que los medios de pago sumen el total.

## Recorrido y escritores (patrón Visitor)

`RecorridoComprobante` es el único código que sabe qué nodo va en qué comprobante, en qué orden y con cuántos decimales. Cada campo declara su "Condición del Campo" del Anexo 1 como una cadena de 7 dígitos, una por tipo en el orden FE FEE FEC TE NC ND REP:

| Dígito | Significado |
|:---:|---|
| 1 | Obligatorio: se emite siempre |
| 2, 3 | Condicional u opcional: se emite si el dato está |
| 4 | Inexistente: nunca se emite |

El recorrido emite eventos a un `IEscritorArbol` (abrir y cerrar documento, nodo y lista, y `campo` con su tipo de valor). `EscritorXml` produce el XML v4.4 y `EscritorJson` el mismo árbol en JSON. Un formato nuevo en árbol (por ejemplo proto) implementa `IEscritorArbol`; uno que no sea árbol (por ejemplo PDF) implementa `IFormatoSalida` y trabaja sobre el comprobante calculado. En ningún caso se duplican las condiciones.

## Firma XAdES-EPES

`XadesSigner::signXml` agrega al XML un `ds:Signature` envuelta (enveloped) con RSA-SHA256, la política de firma de Hacienda y tres referencias: el documento completo (con las transformaciones enveloped-signature y C14N), `SignedProperties` y `KeyInfo`.

Detalle que importa: los digests de `KeyInfo` y de `SignedProperties` se calculan con el nodo ya insertado en el documento, porque el C14N inclusivo de un subárbol incluye los namespaces declarados en sus ancestros (`XmlCanonicalizer` lo hace). Calcularlos sobre una copia suelta produce una firma que parece válida y que Hacienda rechaza con "El XML fue modificado luego de haber sido firmado". El XML final se imprime sin formato; cualquier espacio agregado después de firmar la invalida.

Firma en dos pasos, para cuando la llave privada no puede entrar al proceso (keystore de servidor, HSM): `prepararFirma` recibe el certificado público (X.509 DER) y devuelve el XML con `SignatureValue` vacío y el `SignedInfo` canonicalizado; el cliente lo firma con RSA-SHA256 donde esté la llave; `completarFirma` inserta el valor y verifica la firma con el certificado de `KeyInfo`.

## Mensaje Receptor

`cec::mensaje` implementa `MensajeReceptor_V4.4.xsd`: confirma (total o parcialmente) o rechaza una factura o nota recibida.

- `leerComprobante`: lee el XML del proveedor y extrae clave, emisor, receptor, moneda, totales, si está firmado y el plazo para confirmarlo (8 días hábiles desde el primer día del mes siguiente, art. 10).
- `validar` y `generar`: reglas del mensaje (códigos `RULE_MR_*`) y XML sin firmar, que se firma igual que un comprobante.
- Solo se confirman facturas, notas de débito y notas de crédito (tipos 01, 02 y 03). El plazo vencido es una advertencia, no un error.

## API de C (ABI versión 3)

`include/cec/c_api/cec_c_api.h` está pensada para cualquier lenguaje con FFI, así que no depende de JSON:

- Entrada por structs (`cec_request_c` y sus partes). Un `const char*` en `NULL` significa "no informado"; los enteros opcionales usan `CEC_SIN_VALOR` (-1); las listas son puntero más cantidad.
- Resultado opaco (`cec_result*`) que se libera con `cec_result_free`. Se lee con funciones de acceso (`cec_result_is_success`, `_xml`, `_totales`, `_total_diagnosticos`, `_diagnostico`, `_certificado`). `cec_result_data` entrega además un JSON con todo el resultado.
- Hay variantes `_json` (`cec_build_json`, `cec_calcular_json`, `cec_mensaje_receptor_json`) para quien prefiere JSON, como el binding de TypeScript.
- Las funciones devuelven un `cec_status`; un comprobante inválido no es un error de estado: el estado es `CEC_STATUS_OK` y `cec_result_is_success` indica el resultado.
- Los macros `CEC_API`, `cec_STATIC` y `cec_EXPORTS` controlan la visibilidad en la biblioteca estática y dinámica.

## Bindings

| Binding | Cómo llama al core | Notas |
|---|---|---|
| TypeScript (`@cec-cr/core`) | WebAssembly (`dist/wasm/cec.js`, función `createCecModule`). Envía el request como JSON a las funciones `_json` y lee `cec_result_data` | Incluye firma en dos pasos y Mensaje Receptor |
| Swift | Structs de C y funciones de acceso, sin JSON; un `Arena` reserva y libera la memoria de los structs | OpenSSL por un `systemLibrary` (`COpenSSL`); incluye firma en dos pasos y Mensaje Receptor |

La lista de funciones exportadas al WebAssembly está escrita a mano en `scripts/build_wasm.sh`; una función de C nueva hay que agregarla ahí y a `ModuloWasm` en `bindings/typescript/src/wasm.ts`.

## Pruebas

- **Unitarias** (`test/unit`): una por tema; las reglas se prueban con un caso válido al que se le cambia un dato.
- **Golden** (`test/golden/<caso>`): `request.json`, `expected.xml` y `expected_comprobante.json` de un comprobante válido por cada tipo y variante. Los bindings de TypeScript y Swift comparan contra los mismos casos, así que los tres producen el mismo XML.
- **Firma**: además de la verificación criptográfica, un test recalcula los digests de cada referencia sobre el XML firmado ya reparseado, como lo hace Hacienda.
- La firma generada también se verificó con una implementación independiente de XMLDSig (libxml2 vía lxml) durante el desarrollo; ese verificador no forma parte del repositorio.

## Scripts y ejemplos

- `scripts/generadores/`: genera las plantillas del ejemplo web y un juego de comprobantes relacionados con datos reales para el sandbox (ver el [README](../README.md#envío-a-hacienda)).
- `scripts/enviar_comprobante.py`: envía un XML firmado al API de Recepción y espera la respuesta. Solo usa la biblioteca estándar de Python.
- `examples/cpp`, `examples/c_api`, `examples/swift`: ejemplos mínimos de cada API.
- `examples/web`: aplicación React que usa el binding de TypeScript; el core se ejecuta en el navegador, sin servidor.
