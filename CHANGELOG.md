# Historial de cambios

Formato basado en [Keep a Changelog](https://keepachangelog.com/es-ES/1.1.0/). El proyecto es pre-1.0: la API de C++, la ABI de C y los bindings pueden cambiar entre versiones, y esos cambios se anotan aquí.

## Primera versión (sin publicar)

Primera versión de Comprobantes Electrónicos Core (CEC), según el documento oficial del Ministerio de Hacienda "Anexos y Estructuras para la Emisión de Comprobantes Electrónicos" v4.4 y sus XSD. Qué cubre y qué no: [docs/LIMITACIONES.md](docs/LIMITACIONES.md).

### Comprobantes

- Factura Electrónica (01), Nota de Débito (02), Nota de Crédito (03), Tiquete Electrónico (04), Factura Electrónica de Compra (08), Factura Electrónica de Exportación (09) y Recibo Electrónico de Pago (10).
- Mensaje Receptor (05, 06 y 07): aceptación, aceptación parcial y rechazo de una factura o nota recibida, con lectura del XML del proveedor y cálculo del plazo del art. 10.

### Núcleo

- Validación de las reglas del Anexo 1 con diagnósticos: código estable (`RULE_...`), ruta del campo en el request, mensaje en español y severidad (error o advertencia).
- Cálculo de montos de línea y del resumen: descuentos, impuestos tarifarios y específicos, exoneraciones, surtidos, otros cargos e IVA devuelto. Aritmética entera de punto fijo con 5 decimales, sin `double` y sin `__int128` (`Aritmetica128.h`).
- Vista previa del cálculo (`calcular`) para editar en vivo, aunque el comprobante todavía tenga errores de reglas.
- Generación del XML v4.4 y del mismo árbol en JSON con el patrón Visitor: `RecorridoComprobante` concentra el orden y la presencia de los nodos, y los escritores solo escriben.
- Firma XAdES-EPES con un certificado PKCS#12, y firma en dos pasos (`prepararFirma` y `completarFirma`) para llaves que no pueden entrar al proceso (keystore, HSM).
- Catálogos de las notas del Anexo 1 (16 catálogos, 168 monedas, 101 unidades de medida, 229 formas farmacéuticas) y división territorial (7 provincias, 84 cantones, 492 distritos y 7 087 barrios). No incluye el catálogo CABYS: solo los prefijos y códigos que las reglas necesitan.
- Request de entrada en JSON con las etiquetas del Anexo 1 en camelCase, descrito en `schemas/comprobante_request.schema.json`.
- Advertencia `RULE_REF_CODIGO_FUERA_DEL_XSD` y `RULE_REF_TIPO_DOC_FUERA_DEL_XSD` cuando la información de referencia usa un valor del Anexo que el XSD publicado todavía no enumera (códigos 13 a 17, tipos 19 y 20).

### Interfaces

- C++: cabecera maestra `cec/CEC.h` y `ComprobanteService`.
- C: ABI versión 3. El request se pasa con structs (`cec_request_c`) y el resultado se lee con funciones de acceso, sin depender de JSON; también hay variantes `_json`. Incluye el Mensaje Receptor por struct (`cec_mensaje_receptor`) y por JSON.
- TypeScript / JavaScript (`@cec-cr/core`) sobre WebAssembly.
- Swift (Swift Package Manager) sobre los structs de la API de C, sin JSON.
- Ejemplos de C, C++ y Swift, y una aplicación web de demostración (React) con plantillas de cada tipo de comprobante.

### Herramientas

- `scripts/enviar_comprobante.py`: envía un XML firmado al API de Recepción de Hacienda (sandbox o producción) con credenciales de un `.env`.
- `scripts/generadores/`: genera las plantillas del ejemplo web y un juego de comprobantes relacionados con datos reales para el sandbox.
- `scripts/validar_xsd.py`: valida comprobantes contra los XSD oficiales (con `xmllint`).

### Pruebas y CI

- Tests de C++ (unitarios y casos de referencia por tipo de comprobante), de TypeScript y de Swift; los tres producen el mismo XML. Los casos de referencia cumplen los XSD oficiales.
- `make tsan` ejecuta el test de uso concurrente con ThreadSanitizer.
- Integración continua en Linux (GCC), macOS (Clang y Swift) y Windows (MSVC, con OpenSSL de vcpkg), más un job de Python que valida los scripts, las plantillas y los casos de referencia contra los XSD.

### Documentación

- README, `docs/ARQUITECTURA.md`, `docs/LIMITACIONES.md`, `CONTRIBUTING.md`, `SECURITY.md`, los README de cada binding y las tablas de condiciones de cada campo (`docs/reglas_condiciones_campos`).
- `AGENTS.md` y `CLAUDE.md`: guía para agentes de código (reglas, mapa de archivos, comandos y trampas conocidas).
- Avisos: los documentos de Hacienda de `docs/` son información pública que no está cubierta por la licencia MIT, y el proyecto se desarrolló con la asistencia de Claude (Anthropic).

### Problemas conocidos

- Los XSD publicados no enumeran los códigos de referencia 13 a 17 ni los tipos 19 y 20 que sí están en el Anexo; la biblioteca los acepta con una advertencia (ver `docs/LIMITACIONES.md`).
