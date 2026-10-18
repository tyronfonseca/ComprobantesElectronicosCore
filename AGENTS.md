# AGENTS.md

Guía para agentes de código y para quien contribuya por primera vez. El uso de la biblioteca está en [README.md](README.md); el diseño, en [docs/ARQUITECTURA.md](docs/ARQUITECTURA.md); lo que no hace o no está verificado, en [docs/LIMITACIONES.md](docs/LIMITACIONES.md).

## Qué es este proyecto

Comprobantes Electrónicos Core (CEC): biblioteca en C++20 que valida, calcula, serializa a XML y firma (XAdES-EPES) los comprobantes electrónicos de Costa Rica según el documento oficial del Ministerio de Hacienda "Anexos y Estructuras para la Emisión de Comprobantes Electrónicos" versión 4.4. Tiene también el Mensaje Receptor v4.4 (aceptación, aceptación parcial y rechazo de un comprobante recibido). Se usa desde C++, C (ABI versión 3), TypeScript (WebAssembly) y Swift. No es un producto oficial de Hacienda.

Comprobantes: 01 Factura, 02 Nota de Débito, 03 Nota de Crédito, 04 Tiquete, 08 Factura de Compra, 09 Factura de Exportación, 10 Recibo de Pago.

## Reglas que no se negocian

1. **La norma manda.** Solo el documento oficial de Hacienda (Anexos y Estructuras v4.4, con los XSD de `docs/hacienda_schemas/v4.4`) es fuente de verdad. Si el código, una plantilla, un test o un XSD contradicen la norma, está mal el código, la plantilla, el test o el XSD. Ante la duda, no inventar: dejar el caso sin cubrir y anotarlo en [docs/LIMITACIONES.md](docs/LIMITACIONES.md).
2. **No incluir el catálogo CABYS.** Cambia con frecuencia. Solo se guardan los prefijos y códigos puntuales que las reglas necesitan (`include/cec/common/CabysClasificacion.h`). La biblioteca valida el formato del CABYS, no su existencia.
3. **El dinero es entero.** `domain::Money` guarda `int64` escalado por 100000 (5 decimales) y redondea half-up. Nunca usar `double` ni `float` para montos. El request recibe los montos como texto decimal.
4. **Sin excepciones hacia afuera.** Un comprobante inválido no lanza: devuelve diagnósticos (`DiagnosticList`) con código estable (`RULE_...`), ruta en el request (`detalleServicio[0].codigoCABYS`), mensaje en español y severidad. Los códigos y las rutas son contrato: no renombrarlos sin avisarlo en el [CHANGELOG](CHANGELOG.md).
5. **El orden y la presencia de los nodos viven en un solo lugar:** `RecorridoComprobante`. No duplicar esa lógica en un escritor, en un binding ni en la interfaz.
6. **El request lleva solo datos de entrada.** Los montos calculados (subtotales, impuestos, totales) los calcula el core. Excepción: en el Recibo de Pago el Anexo 1 pide `montoTotal` y `subTotal`.
7. **Datos ficticios.** Ningún dato real (identificaciones, nombres, correos, teléfonos, claves de comprobantes reales) en código, tests, plantillas, ejemplos ni documentación. Correos en `example.com`/`example.cr`. Credenciales y datos reales van en archivos ignorados por Git (`.env`, `scripts/generadores/datos_sandbox.json`).
8. **Documentación en español, sin emojis.** Los README y las guías se actualizan en la misma tarea que cambia la API, los comandos o la estructura.

## Mapa: qué archivo tocar

| Tarea | Archivos |
|---|---|
| Regla de validación nueva o ajustada | `include/cec/rules/*Rule.h` (implementan `IValidationRule::validate`), registrar en el constructor de `RuleEngine.h`; test en `test/unit/TestReglas.cpp` |
| Cálculo de montos | `include/cec/calculation/CalculationEngine.h`; test en `test/unit/TestCalculo.cpp` |
| Orden, presencia o formato de un nodo del XML | `include/cec/visitor/RecorridoComprobante.h` (condición de 7 dígitos por campo); `EscritorXml.h` y `codec/EscritorJson.h` solo escriben |
| Otro formato de salida (PDF, proto) | Implementar `visitor::IEscritorArbol` o `IFormatoSalida`; no se toca el recorrido |
| Campo nuevo en el request | `parsing/ComprobanteRequestDTO.h`, `codec/JsonCodec.cpp`, `parseDto` en `service/ComprobanteService.h`, `domain/Comprobante.h`, `RecorridoComprobante.h`, struct y conversión en `c_api/cec_c_api.h` y `src/c_api/cec_c_api.cpp`, `schemas/comprobante_request.schema.json`, `bindings/typescript/src/tipos.ts`, `bindings/swift/Sources/.../Tipos.swift` y `Marshaling.swift`, tests y goldens |
| Catálogo o código de una nota del Anexo 1 | `include/cec/common/Codigos.h`, `Catalogos.h`, `src/common/` |
| Firma XAdES, C14N, PKCS#12 | `src/crypto/*.cpp`, `include/cec/crypto/`; test en `test/unit/TestFirma.cpp` |
| Mensaje Receptor | `include/cec/mensaje/MensajeReceptor.h`; test en `test/unit/TestMensajeReceptor.cpp` |
| API de C | `include/cec/c_api/cec_c_api.h`, `src/c_api/cec_c_api.cpp`; si la API de C cambia de forma incompatible, subir `cec_abi_version` |
| Función de C nueva usada desde WebAssembly | Agregarla a `EXPORTED_FUNCS` en `scripts/build_wasm.sh` y a `ModuloWasm` en `bindings/typescript/src/wasm.ts` |
| Binding TypeScript / Swift | `bindings/typescript/src/`, `bindings/swift/Sources/` |
| Ejemplo web y sus plantillas | `examples/web/src/`; las plantillas se generan con `scripts/generadores/generar_plantillas.py`, no se editan a mano |
| Envío al API de Hacienda | `scripts/enviar_comprobante.py` |
| Actualizar los XSD de Hacienda | Reemplazar los de `docs/hacienda_schemas/v4.4`; luego comparar catálogos y restricciones con el código (unidades, monedas, longitudes, ocurrencias), actualizar `docs/reglas_condiciones_campos` y correr `python3 scripts/validar_xsd.py test/golden/*/expected.xml` |

## Comandos

```bash
git submodule update --init          # GoogleTest en libs/googletest (una vez)
make test                            # compila y ejecuta los tests de C++ (debug, con verificaciones de libc++)
make test FILTER='Reglas*'           # un grupo
make examples                        # ejemplos de C++ y de C
make BUILD_MODE=release              # -O3
make wasm                            # dist/wasm/cec.js + cec.wasm (copia también a examples/web/public); requiere Emscripten
cmake -B build_cmake && cmake --build build_cmake && ctest --test-dir build_cmake
swift test                           # binding Swift (macOS)
cd bindings/typescript && npm test   # binding TypeScript; necesita dist/wasm/cec.js ya compilado
cd examples/web && npm install && npm run dev
python3 scripts/generadores/generar_plantillas.py   # regenera las plantillas del ejemplo web
python3 scripts/validar_xsd.py test/golden/*/expected.xml   # valida XML contra los XSD oficiales (necesita xmllint)
```

Después de cambiar el core hay que recompilar el WebAssembly (`make wasm`) antes de probar el binding TypeScript o el ejemplo web; un `cec.wasm` viejo produce síntomas confusos (por ejemplo, una firma que Hacienda rechaza). Los goldens se regeneran con `GOLDEN_UPDATE=1 ./build/CECTests --gtest_filter='Golden*'` y el diff de `test/golden` se revisa a mano.

Estado esperado de la suite en un árbol limpio: todos los tests de C++ (`make test`), de TypeScript (`npm test`) y de Swift (`swift test`) pasan. Si alguno falla antes de tu cambio, dilo en vez de ocultarlo.

## Convenciones

- Prefijo `cec` en todo lo público: namespaces `cec::domain`, `cec::common`, `cec::parsing`, `cec::rules`, `cec::calculation`, `cec::visitor`, `cec::codec`, `cec::crypto`, `cec::mensaje`, `cec::service`; funciones de C `cec_*`; tipos de C `cec_*_c`.
- Las reglas, el cálculo, el recorrido y el servicio son solo cabeceras (`include/cec/`); lo que se compila está en `src/` (API de C, JSON, criptografía, catálogos).
- Los nombres del request son las etiquetas del Anexo 1 en camelCase; las listas van en plural; los códigos de catálogo son texto de dos dígitos (`"01"`); los montos, texto decimal.
- Seguir el estilo del archivo que se edita: comentarios y mensajes en español, sin sobre-documentar.
- Incluir directamente las cabeceras estándar que se usan (`<cstdint>`, `<cstdio>`, `<algorithm>`...). Compilar solo con Clang esconde faltantes que GCC sí reporta.
- Los tests parten de un caso válido de `test/golden/<caso>/request.json` (`fixtures::request("fe_completa")`), cambian un solo dato y verifican el código y la ruta del diagnóstico. El reloj de los tests es fijo (2026-10-15 12:00 -06:00).

## Trampas conocidas

- **Comparadores estrictos.** El modo debug de libc++ aborta ante un comparador que no sea un orden débil estricto (ya pasó en `XmlCanonicalizer`). Por eso `make test` compila con `_LIBCPP_HARDENING_MODE_DEBUG`.
- **Firma XAdES.** Los digests de `KeyInfo` y `SignedProperties` se calculan con el nodo ya insertado en el documento, porque el C14N inclusivo arrastra los namespaces de los ancestros. El XML firmado se escribe sin formato (`format_raw`): cualquier espacio agregado después de firmar invalida la firma. El test `DigestsDeLasReferenciasCoincidenConElXmlFirmado` protege esto.
- **`calcular` frente a `procesar`.** `calcular` (vista previa) calcula aunque las reglas reporten errores y no genera XML; `procesar` se detiene en el primer grupo de errores y es el único que produce el XML para emitir.
- **Condiciones del campo.** Cada campo lleva 7 dígitos en el orden FE FEE FEC TE NC ND REP: `1` se emite siempre, `2` y `3` solo si el dato está, `4` nunca. Que falte un dato obligatorio o sobre uno inexistente lo reportan las reglas, no el recorrido.
- **Factura de Compra.** El emisor del XML es el vendedor, pero la clave y el envío llevan la identificación del receptor (el comprador que la emite).
- **`.gitignore` agresivo.** Ignora `*.js`, `*.wasm`, `*.map`, `dist/`, `build/` y `deps/`. Un archivo JavaScript de configuración nuevo hay que agregarlo con `git add -f`. Los artefactos de WebAssembly no se versionan.
- **Ejemplo web.** Las funciones que se pasan a `set` de Immer no deben devolver valor (usar llaves), y los selectores de Zustand no deben crear arreglos nuevos en cada llamada (usar una constante).
- **Anexo frente a XSD.** Los XSD publicados van por detrás del Anexo en algunos valores (códigos de referencia 13 a 17, tipos 19 y 20) y son más permisivos en otros. La biblioteca sigue el Anexo y avisa con una advertencia cuando un valor del Anexo no está en el XSD; el detalle está en [docs/LIMITACIONES.md](docs/LIMITACIONES.md). Las unidades de medida distinguen mayúsculas (`Kg`, no `kg`).
- **Dependencias de plataforma.** Se necesita OpenSSL 3. No usar `__int128`, `gmtime_r` ni otras extensiones de GCC o POSIX: el dinero usa `Aritmetica128.h` y el proyecto compila con MSVC y el CI de Windows lo comprueba; ver [docs/LIMITACIONES.md](docs/LIMITACIONES.md).

## Seguridad y datos

- Nunca subir `.env`, certificados (`.p12`, `.pfx`), PIN, contraseñas ni salidas de `ejemplos_sandbox/`. Ya están ignorados; no los muevas a una ruta que no lo esté.
- Usar siempre el ambiente `sandbox` de Hacienda para pruebas. No enviar comprobantes de prueba a producción.
- Los problemas de seguridad se reportan como indica [SECURITY.md](SECURITY.md).

## Antes de dar una tarea por terminada

1. `make test` pasa; si tocaste el binding o el WebAssembly, también `npm test`; si tocaste la API de C o el binding Swift, `swift test`.
2. La documentación afectada está actualizada (README, README del binding, `docs/`, esquema JSON, [CHANGELOG](CHANGELOG.md)).
3. Si cambió el XML que se genera, `python3 scripts/validar_xsd.py test/golden/*/expected.xml` sigue pasando.
4. No quedaron datos reales, credenciales ni archivos generados (`build/`, `dist/`, `ejemplos_sandbox/`) en el cambio.
5. Si algo no se pudo verificar en esta máquina (Linux, Windows, WebAssembly), decirlo.
