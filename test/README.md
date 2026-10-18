# Pruebas

Tests de C++ con GoogleTest. Se ejecutan con:

```bash
make test                          # compila y ejecuta build/CECTests
make test FILTER='Reglas*'         # solo un grupo (filtro de GoogleTest)
./build/CECTests --gtest_list_tests
```

Con CMake: `ctest --test-dir build_cmake --output-on-failure`.

Con ThreadSanitizer:

```bash
make tsan                # compila en build_tsan/ y ejecuta el grupo Hilos*
make tsan FILTER='*'     # toda la suite con ThreadSanitizer (más lento)
```

En modo debug (el predeterminado) el Makefile activa las verificaciones de libc++ (`_LIBCPP_HARDENING_MODE_DEBUG`), que detectan comparadores inválidos, accesos fuera de rango y otros errores que en release pasarían desapercibidos.

## Organización

```
test/
├── TestMain.cpp
├── support/Fixtures.h      Requests base, motor con reloj fijo y utilidades de verificación
├── support/CertificadoPrueba.h  Certificado PKCS#12 temporal en memoria para los tests de firma
├── golden/                 Casos de referencia: request.json, expected.xml y expected_comprobante.json
│   └── TestGolden.cpp
└── unit/
    ├── TestReglas.cpp      Reglas del Anexo 1, una por test
    ├── TestCalculo.cpp     Montos de línea y del resumen verificados a mano
    ├── TestRecorrido.cpp   Presencia, orden y formato de los nodos del XML por tipo de comprobante
    ├── TestJsonCodec.cpp   Lectura del request JSON y errores con ruta
    ├── TestCApi.cpp        API de C: structs, JSON, diagnósticos, totales
    ├── TestFirma.cpp       PKCS#12, canonicalización C14N, firma XAdES-EPES y firma en dos pasos
    ├── TestMensajeReceptor.cpp  Mensaje Receptor: lectura del comprobante recibido, reglas y XML
    ├── TestHilos.cpp       Uso concurrente de la biblioteca (ver ThreadSanitizer)
    ├── TestCatalogos.cpp   Catálogos y división territorial
    └── TestMoney.cpp       Aritmética de montos
```

## Casos de referencia (golden)

Cada carpeta de `test/golden/<caso>/` es un comprobante válido de un tipo:

| Caso | Comprobante |
|---|---|
| `fe_servicios` | Factura Electrónica de servicios |
| `fe_completa` | Factura con descuentos, exoneración, varias líneas y otros cargos |
| `fe_surtido` | Factura con surtido (componentes con distintas tarifas) |
| `fe_usd` | Factura en dólares |
| `fe_vehiculo_vin` | Factura de un vehículo con VIN |
| `te`, `nc`, `nd`, `fec`, `fee`, `rep` | Tiquete, notas de crédito y débito, compra, exportación y recibo de pago |

Estos casos cumplen dos funciones:

1. **Golden:** `TestGolden.cpp` procesa cada `request.json` y compara el XML y el comprobante en JSON con `expected.xml` y `expected_comprobante.json`, carácter por carácter.
2. **Bases válidas:** los tests de reglas parten de uno de estos requests (`fixtures::request("fe_completa")`), cambian un solo dato y verifican el diagnóstico y su ruta. Así cada test muestra exactamente qué dato provoca qué error.

Los bindings de TypeScript y Swift usan los mismos casos y deben producir exactamente el mismo XML. Los `expected.xml` también cumplen los XSD oficiales: `python3 scripts/validar_xsd.py test/golden/*/expected.xml` (lo ejecuta el CI; necesita `xmllint`).

### Regenerar los archivos esperados

Después de un cambio intencional en la salida:

```bash
GOLDEN_UPDATE=1 ./build/CECTests --gtest_filter='Golden*'
git diff test/golden    # revisar que solo cambió lo esperado
```

## Fecha de emisión

La regla de fecha de emisión rechaza fechas futuras. Para que los tests no dependan del día en que se ejecutan, `fixtures::procesar` usa un motor de reglas con un reloj fijo (2026-10-15 12:00, hora de Costa Rica).

## Datos

Todos los datos de prueba son ficticios: identificaciones inventadas, correos en el dominio reservado `example.com` y teléfonos genéricos.
