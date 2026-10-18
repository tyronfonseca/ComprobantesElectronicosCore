# Requisitos, alcance y limitaciones

Lo que la biblioteca necesita, lo que cubre y lo que no. Está escrito para decidir si sirve para un caso y para no suponer garantías que no existen. Si encuentra una diferencia con el comportamiento real, es un error de esta página: avise o corríjala.

## Estado del proyecto

Pre-1.0. La API de C++, la ABI de C (versión 3) y los bindings pueden cambiar entre versiones; los cambios se anotan en el [CHANGELOG](../CHANGELOG.md). No es un producto oficial del Ministerio de Hacienda y no sustituye la validación de Hacienda ni la asesoría tributaria. Quien emite comprobantes es responsable de su contenido: pruebe siempre en el ambiente `sandbox` antes de usar producción.

## Norma que implementa

- "Anexos y Estructuras para la Emisión de Comprobantes Electrónicos" versión 4.4, con la bitácora de ajustes al 22/04/2026.
- Esquemas XSD v4.4 de [`docs/hacienda_schemas/v4.4`](hacienda_schemas/v4.4).
- Solo se genera la versión 4.4. No se generan versiones anteriores.
- Si la norma oficial y esta biblioteca difieren, vale la norma.

## Requisitos

| Para | Requiere |
|---|---|
| Biblioteca y tests de C++ | Compilador C++20 con `<chrono>` completo (se probó con Apple Clang 21; el CI usa GCC 13; no se han probado versiones anteriores); OpenSSL 3 |
| Tests | Submódulo `libs/googletest` (`git submodule update --init`) |
| CMake (opcional) | CMake 3.20 o superior |
| WebAssembly | Emscripten (`em++`); el script compila OpenSSL para wasm la primera vez |
| Binding TypeScript | Node.js 22.18 o superior (importa archivos `.ts` directamente en los tests) |
| Binding Swift | Swift 5.9 o superior, macOS 13 o superior, OpenSSL 3 (Homebrew) |
| Scripts de Python | Python 3.9 o superior, sin paquetes adicionales |
| Ejemplo web | Node.js y los paquetes de `examples/web/package.json` |

## Plataformas y qué se ha verificado

| Plataforma | Estado |
|---|---|
| macOS arm64 (Apple Clang 21, OpenSSL 3.6) | Verificada: tests de C++, binding TypeScript y binding Swift (Swift 6.4), ejemplos de C y de C++, WebAssembly con Emscripten 6 |
| Linux (GCC 13) | Verificada en el CI: compilación con CMake y tests de C++ (197) |
| Windows con MSVC | Verificada en el CI: MSVC 14.51 (Visual Studio 18) con CMake, Ninja y OpenSSL de vcpkg; los tests de C++ pasan (196: falta solo la comparación de la aritmética de 128 bits con `__int128`, que MSVC no tiene). El dinero usa `Aritmetica128.h`, en C++ estándar, y no `__int128`. Los casos de referencia se guardan con LF (`.gitattributes`) para que el checkout de Windows no los convierta a CRLF |
| macOS (CI) | Verificada en el CI: Make, CMake, tests de C++ y de Swift |
| CMake | Verificado en el CI (Linux, macOS y Windows); no estaba instalado en la máquina de desarrollo |
| Swift en Linux | Sin verificar |
| WebAssembly en el CI | No se compila en el CI; se compila a mano con `make wasm` |

## Qué valida la biblioteca y qué no

La biblioteca valida lo que se puede validar con los datos del propio comprobante y con los catálogos de las notas del Anexo 1: obligatoriedad y prohibición de campos por tipo de comprobante, formatos y longitudes, catálogos, coherencia de clave, consecutivo y fecha, combinaciones permitidas, y la coherencia de los montos calculados.

Hay verificaciones que solo puede hacer el Ministerio de Hacienda porque dependen de información que la biblioteca no tiene. **Un comprobante que la biblioteca da por válido todavía puede ser rechazado por Hacienda**, por ejemplo cuando:

- el código CABYS no existe en el catálogo vigente o no corresponde a la actividad económica del emisor;
- la actividad económica no está inscrita en el RUT, o el emisor, el receptor o el proveedor de sistemas no están inscritos;
- el consecutivo no sigue la secuencia que Hacienda espera;
- la fecha es anterior a la permitida en la situación "sin internet" (la biblioteca solo rechaza fechas futuras);
- Hacienda cambia una regla o un catálogo después de esta versión.

**CABYS.** El catálogo completo no se incluye, porque cambia con frecuencia. Solo se usan los prefijos y códigos puntuales que las reglas necesitan (bebidas, jabón de tocador, surtidos habilitados, servicios de salud, medicamentos y medios de transporte). El CABYS se valida por formato (13 dígitos).

## Funcionalidad que no incluye

- El nodo `Otros` (`OtroTexto`, `OtroContenido`) de los comprobantes no se genera.
- No interpreta el `MensajeHacienda` (la respuesta de Hacienda). El script `scripts/enviar_comprobante.py` lo guarda y muestra el detalle.
- No genera la representación gráfica (PDF) ni el código QR. El patrón Visitor permite agregar formatos, pero no hay ninguno de ese tipo.
- No verifica la firma de un comprobante recibido: `leerComprobante` solo comprueba que tenga un nodo de firma. La firma que la propia biblioteca genera sí se verifica en `completarFirma`.
- No se comunica con Hacienda desde C++, C, TypeScript ni Swift. El envío está en el script de Python.
- No administra consecutivos, certificados, claves de seguridad ni almacenamiento de comprobantes: el sistema que la use debe encargarse.
- El Mensaje Receptor solo confirma facturas (01) y notas de débito y crédito (02 y 03). No tiene esquema JSON propio (`schemas/` cubre el request de los comprobantes).
- Los mensajes de diagnóstico están en español; no hay otros idiomas.

## Firma electrónica

- XAdES-EPES envuelta (enveloped), RSA-SHA256, con la política de firma que usa Hacienda y las referencias al documento, a `SignedProperties` y a `KeyInfo`.
- Solo contenedores PKCS#12 (`.p12`, `.pfx`). No se validó con tarjetas inteligentes ni con otros tipos de llave. El algoritmo declarado es RSA-SHA256 y la biblioteca no comprueba que la llave del certificado sea RSA.
- Sin sellado de tiempo (no es XAdES-T ni superior).
- Hay firma en dos pasos para llaves que no pueden entrar al proceso (keystore de servidor, HSM): `prepararFirma` y `completarFirma`.
- Cualquier cambio de bytes después de firmar invalida la firma, incluidos saltos de línea y espacios. No reformatee el XML firmado.
- La firma generada se verificó con una implementación independiente de XMLDSig (libxml2 vía lxml) durante el desarrollo; esa verificación no forma parte de los tests del repositorio. El XML firmado cumple el XSD (ver "Validación contra los XSD").
- No hay una validación sistemática contra el sandbox de Hacienda registrada en el repositorio. El flujo de envío (token, envío, consulta) sí se ejecutó contra el sandbox.

## Diferencias entre el Anexo y los XSD publicados

La biblioteca sigue el Anexo y genera XML que cumple los XSD de [`docs/hacienda_schemas/v4.4`](hacienda_schemas/v4.4). La excepción son valores que el Anexo (con la bitácora del 22/04/2026) admite y que los XSD publicados todavía no enumeran:

| Campo | Anexo | XSD publicado |
|---|---|---|
| Código de referencia (`informacionReferencia[].codigo`, nota 9) | 01 a 17 y 99 | 01 a 12 y 99 |
| Tipo de documento de referencia (`informacionReferencia[].tipoDocIR`, nota 10) | 01 a 20 y 99 | 01 a 18 y 99 |

La biblioteca acepta esos valores y emite una advertencia (`RULE_REF_CODIGO_FUERA_DEL_XSD`, `RULE_REF_TIPO_DOC_FUERA_DEL_XSD`), porque Hacienda podría rechazar el XML por esquema. Si necesita que el XML cumpla el XSD tal como está publicado, evite esos valores (para un Recibo de Pago, por ejemplo, use el código de referencia 04 en lugar del 17).

En sentido contrario, el XSD es más permisivo que el Anexo en varios campos (por ejemplo, que `Numero`, `Codigo` y `Razon` de la información de referencia sean opcionales en las notas, que `NombreTercero` exista en la exportación, o que las condiciones de venta 09 y 11 se admitan en NC y ND). Ahí la biblioteca aplica el Anexo, que es más estricto, y el XML sigue cumpliendo el XSD.

## Validación contra los XSD

`python3 scripts/validar_xsd.py archivo.xml...` valida comprobantes contra los XSD oficiales con `xmllint`; si el XML no está firmado agrega una firma de relleno solo para la validación. El CI lo ejecuta sobre los casos de referencia (`test/golden/*/expected.xml`). Las 12 plantillas del ejemplo web y los 8 comprobantes del generador del sandbox, firmados, también se validaron, generados con el core de WebAssembly. Los XSD oficiales importan el esquema de xmldsig con una ruta de la CDN de Hacienda que no existe en este repositorio; el script trabaja sobre una copia temporal con la ruta corregida.

## Números

- Los montos son enteros de 64 bits con 5 decimales (`Money`), con redondeo half-up. El máximo representable es de unos 92 billones (9,2 x 10^13) en la moneda del comprobante; la suma y la resta detectan desbordamiento.
- Una factura admite hasta 1000 líneas de detalle y hasta 1000 números VIN o de serie por línea, como indica el Anexo 1.
- Tarifas y cantidades se escalan igual que los montos, por lo que tienen 5 decimales de precisión.

## Hilos y memoria

- Se puede llamar desde varios hilos a la vez a `process`, `calcular`, `signXml`, el Mensaje Receptor y las consultas de catálogos y división territorial. El test `Hilos` los ejecuta con 8 hilos y `make tsan` lo corre con ThreadSanitizer. Los objetos que usted cree (un `RuleEngine` propio, un `Pkcs12Signer`) no se deben modificar mientras otro hilo los usa.
- En la API de C, el resultado (`cec_result*`) lo libera quien lo pide con `cec_result_free`; los textos que entrega viven hasta ese momento.
- Las consultas de elementos de un catálogo por nombre (`cec_catalogo_elemento_por_indice`) usan una caché por hilo: los punteros que devuelven valen hasta que ese mismo hilo consulte otro catálogo. Copie los textos si los necesita después.
- El módulo de WebAssembly es de un solo hilo.

## Bindings

| Capacidad | C++ | C | TypeScript | Swift |
|---|:---:|:---:|:---:|:---:|
| Procesar y calcular un comprobante | Sí | Sí | Sí | Sí |
| Firmar con un `.p12` y leer certificados | Sí | Sí | Sí | Sí |
| Firma en dos pasos | Sí | Sí | Sí | Sí |
| Mensaje Receptor y lectura de comprobantes recibidos | Sí | Sí | Sí | Sí |
| Catálogos y división territorial | Sí | Sí | Sí | Sí |
| Numeración (clave y consecutivo) | No | No | Sí | Sí |

El binding de Swift solo se ha verificado en macOS. El binding de TypeScript se usa desde este repositorio (`bindings/typescript`) y necesita el `cec.wasm` compilado con `make wasm`.

## Seguridad de los datos del usuario

- La biblioteca no guarda ni registra certificados, PIN ni contraseñas. En el ejemplo web, el certificado y el PIN se procesan en el navegador y no se envían a ningún servidor.
- Las credenciales del API de Hacienda las lee `scripts/enviar_comprobante.py` de un archivo `.env` local, que Git ignora. Use el ambiente `sandbox` para pruebas.
- Para reportar un problema de seguridad, ver [SECURITY.md](../SECURITY.md).
