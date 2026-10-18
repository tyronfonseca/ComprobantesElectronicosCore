# Cómo contribuir

Gracias por querer mejorar Comprobantes Electrónicos Core (CEC). Este documento resume lo necesario para proponer un cambio. La guía técnica detallada (qué archivo tocar para cada tarea, comandos y trampas conocidas) está en [AGENTS.md](AGENTS.md); el diseño, en [docs/ARQUITECTURA.md](docs/ARQUITECTURA.md).

## Antes de empezar

- Para errores y propuestas pequeñas, abra un issue o un pull request directamente. Para cambios grandes (una API nueva, otro tipo de comprobante, otra plataforma), abra primero un issue para acordar el enfoque.
- Revise [docs/LIMITACIONES.md](docs/LIMITACIONES.md): ahí está lo que la biblioteca no hace a propósito y lo que todavía no cubre.
- Toda regla de negocio debe poder justificarse con el documento oficial de Hacienda "Anexos y Estructuras para la Emisión de Comprobantes Electrónicos" v4.4. Al proponer una regla o corregir una, cite la sección o la nota del Anexo (por ejemplo, "Anexo 1, nota 16"). Cuando el Anexo y un XSD no coinciden, vale el Anexo; deje constancia de la diferencia.

## Preparar el entorno

```bash
git clone --recurse-submodules <url-del-repositorio>
cd ComprobantesElectronicosCore
make test                # compila y ejecuta los tests de C++
```

Requisitos y versiones probadas: [docs/LIMITACIONES.md](docs/LIMITACIONES.md#requisitos). Para los bindings: `cd bindings/typescript && npm test` (necesita `make wasm`) y `swift test` (macOS).

## Qué debe cumplir un cambio

1. **Tests.** Un cambio de comportamiento lleva su test. Las reglas se prueban partiendo de un caso válido de `test/golden` y cambiando un solo dato (ver [test/README.md](test/README.md)). Un error corregido lleva un test que falla sin la corrección.
2. **Toda la suite pasa:** `make test`, y si toca el core, también `npm test` (después de `make wasm`) y `swift test`.
3. **Sin datos reales.** Identificaciones, nombres, correos, teléfonos, claves, certificados y credenciales de personas o empresas reales no entran al repositorio, ni siquiera en un test o en un issue. Use datos ficticios (`example.com`, cédulas inventadas).
4. **Documentación al día.** Si cambia la API, los comandos o la estructura, actualice el README que corresponda, `docs/` y el [CHANGELOG](CHANGELOG.md) en el mismo pull request. Los textos van en español y sin emojis.
5. **Estilo.** Siga el estilo del archivo que edita. Incluya directamente las cabeceras estándar que use. Los códigos de diagnóstico (`RULE_...`) y las rutas son parte de la API: no los renombre sin avisarlo en el CHANGELOG.
6. **Cambios pequeños y enfocados.** Un pull request, un tema. No mezcle reformateos con cambios de comportamiento.

## Cambiar la API de C

La ABI está versionada (`cec_abi_version`). Un cambio compatible agrega funciones o campos al final; uno incompatible sube la versión y se anota en el CHANGELOG. Si agrega una función que debe usarse desde WebAssembly, agréguela también a `EXPORTED_FUNCS` en `scripts/build_wasm.sh` y al tipo `ModuloWasm` del binding de TypeScript.

## Regenerar archivos de referencia

- Plantillas del ejemplo web: `python3 scripts/generadores/generar_plantillas.py`.
- Resultados esperados de los casos golden, solo después de un cambio intencional: `GOLDEN_UPDATE=1 ./build/CECTests --gtest_filter='Golden*'`, y revise el diff de `test/golden` antes de confirmar.

## Mensajes de commit y pull requests

Mensajes en español, en imperativo y con un resumen corto (por ejemplo, "Valida el plazo del Mensaje Receptor"). En la descripción del pull request explique qué cambia y por qué, y qué pruebas ejecutó. Indique también lo que no pudo verificar (otra plataforma, el sandbox de Hacienda).

## Contribuciones hechas con asistencia de IA

Se aceptan aportes escritos con ayuda de herramientas de IA, igual que los demás: quien los envía es responsable de entender el cambio, de que pase las pruebas y de que respete las reglas de arriba (norma oficial, sin datos reales). Indíquelo en la descripción del pull request si una parte importante fue generada. El proyecto mismo se desarrolló con la asistencia de Claude (ver el aviso en el [README](README.md#aviso-legal)).

## Licencia

Al contribuir acepta que su aporte se distribuya bajo la [licencia MIT](LICENSE.txt) del proyecto.
