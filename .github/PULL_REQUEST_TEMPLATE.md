## Qué cambia y por qué

<!-- Resuma el cambio. Enlace el issue si existe. -->

## Respaldo normativo

<!-- Sección o nota del Anexo 1 v4.4 en que se basa una regla o un cálculo nuevo o corregido. -->

## Pruebas

- [ ] `make test` pasa
- [ ] `npm test` en `bindings/typescript` (después de `make wasm`), si toca el core
- [ ] `swift test`, si toca la API de C o el binding Swift
- [ ] El cambio tiene su test (y un error corregido tiene un test que falla sin la corrección)

## Lista de verificación

- [ ] Sin datos reales, credenciales ni archivos generados (`build/`, `dist/`, `ejemplos_sandbox/`)
- [ ] README, `docs/`, esquema JSON y [CHANGELOG](../CHANGELOG.md) actualizados
- [ ] Indico lo que no pude verificar (otra plataforma, sandbox de Hacienda)
