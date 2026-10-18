# Demostración web de Comprobantes Electrónicos Core

Aplicación en React 19, TypeScript y Tailwind CSS para crear, validar, firmar y descargar comprobantes electrónicos v4.4. Usa el core en C++ compilado a WebAssembly a través del binding [`@cec-cr/core`](../../bindings/typescript): las reglas, los cálculos, el XML y la firma se ejecutan en el navegador, sin servidor.

## Qué muestra

- Los siete tipos de comprobante (FE, ND, NC, TE, FEC, FEE y REP).
- Edición en vivo: en cada cambio el core recalcula los montos con `calcular`, así que la tabla de líneas y el resumen se actualizan aunque el comprobante todavía esté incompleto.
- Validación con los diagnósticos del core: al validar o generar el XML, cada error aparece debajo de su campo, ubicado por la ruta que devuelve el core (`detalleServicio[0].codigoCABYS`).
- Líneas de detalle con impuestos (IVA, impuestos tarifarios y específicos), exoneración, hasta 5 descuentos, códigos comerciales, surtidos, VIN, medicamentos y opciones avanzadas (partida arancelaria, IVA cobrado a nivel de fábrica, base imponible editable).
- Otros cargos, información de referencia y varios medios de pago.
- Numeración: el consecutivo y la clave se generan a partir de la sucursal, la terminal, el número, la situación y el código de seguridad. Al generar el XML, la fecha de emisión se actualiza a la hora actual de Costa Rica.
- Firma XAdES-EPES con un certificado `.p12`/`.pfx`, mostrando el titular y la vigencia del certificado antes de firmar.
- Carga y descarga del request en JSON, con 12 ejemplos válidos de todos los tipos de comprobante.
- Explorador de catálogos y de la división territorial.

## Requisitos

- Node.js 20 o superior.
- El módulo WebAssembly en `public/` (`cec.js` y `cec.wasm`). Se genera desde la raíz del repositorio con:

```bash
make wasm        # o ./scripts/build_wasm.sh; copia cec.js y cec.wasm a examples/web/public/
```

## Ejecución

```bash
cd examples/web
npm install
npm run dev        # servidor de desarrollo (Vite)
npm run build      # verificación de tipos y build de producción en dist/
npm run preview    # sirve el build de producción
```

El paquete `@cec-cr/core` se resuelve directamente desde `bindings/typescript/src` (alias en `vite.config.ts` y `tsconfig.json`), así que los cambios en el binding se reflejan sin publicarlo.

## Estructura

```
src/
├── lib/core.ts                 Carga del módulo WebAssembly y utilidades de formato
├── store/comprobanteStore.ts   Estado (zustand + immer): request, numeración, vista previa, XML y firma
├── templates/                  Ejemplos de request (generados por scripts/generadores/generar_plantillas.py)
├── components/
│   ├── common/                 Campo con error por ruta, editor de ubicación
│   ├── comprobante/            Formularios: comprobante, emisor/receptor, detalle, otros cargos,
│   │   │                       referencias, resumen, XML, firma y carga de JSON
│   │   └── lineas/             Tabla de líneas y editores de impuestos, descuentos, exoneración,
│   │                           códigos comerciales y surtido
│   ├── catalogos/              Explorador de catálogos y división territorial
│   ├── inputs/                 Selector de catálogos
│   └── ui/                     Componentes base (Card, Dialog, Button...)
└── App.tsx
```

## Cómo usa el binding

El store guarda únicamente el request (`ComprobanteRequest`), sin montos calculados. Los montos, totales y diagnósticos salen del core:

```ts
import { FacturaElectronica, generarClave, generarConsecutivo } from "@cec-cr/core";

const fe = await FacturaElectronica.cargar(createCecModule, { locateFile: (f) => `/${f}` });

// En cada edición: vista previa con montos y diagnósticos
const previa = fe.calcular(request);

// Al generar: XML sin firmar
const resultado = fe.procesar(request);

// Firma con el certificado del usuario (no sale del navegador)
const firmado = fe.firmar(resultado.xml!, p12, pin);
```

Los errores se muestran por campo comparando la ruta de cada diagnóstico con la ruta del campo en el request (ver `components/common/Campo.tsx`).

## Ejemplos de comprobantes

Los ejemplos de `src/templates` usan datos ficticios; el core los valida al cargarlos en la página. Se generan con:

```bash
python3 scripts/generadores/generar_plantillas.py   # escribe los JSON y templates/index.ts
```

## Licencia

Este ejemplo forma parte de Comprobantes Electrónicos Core y está sujeto a la [licencia del repositorio](../../LICENSE.txt).
