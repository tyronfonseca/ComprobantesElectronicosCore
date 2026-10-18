# Reglas y condiciones por campo

Tablas con la "Condición del Campo" de cada nodo de los comprobantes electrónicos v4.4 para los siete tipos de comprobante (FE, FEE, FEC, TE, NC, ND y REP):

| Valor | Significado |
|:---:|---|
| 1 | Obligatorio |
| 2 | Condicional: depende de otros datos del comprobante |
| 3 | Opcional |
| 4 | Inexistente: el nodo no existe en ese tipo de comprobante |

Son transcripciones y resúmenes del documento oficial "Anexos y Estructuras para la Emisión de Comprobantes Electrónicos" del Ministerio de Hacienda, organizados para consultarlos junto al código; no lo sustituyen y no están cubiertos por la licencia MIT del proyecto (ver el [aviso sobre documentos públicos de Hacienda](../../README.md#aviso-legal)). La fuente vigente y vinculante es la del portal oficial.

- [Encabezado.md](Encabezado.md): clave, actividad, consecutivo, fecha, emisor, receptor y condición de venta.
- [Detalle.md](Detalle.md): líneas de detalle, impuestos, exoneración, surtidos, otros cargos y resumen.
- [Informacion_Referencia.md](Informacion_Referencia.md): información de referencia.
- [Otros_Seguridad.md](Otros_Seguridad.md): nodo "Otros" y firma.

El orden y las condiciones que genera la biblioteca están en `include/cec/visitor/RecorridoComprobante.h`. Los XSD de [`../hacienda_schemas/v4.4`](../hacienda_schemas/v4.4) son más permisivos que el Anexo en algunos campos y van por detrás en otros; las diferencias conocidas están en [docs/LIMITACIONES.md](../LIMITACIONES.md#diferencias-entre-el-anexo-y-los-xsd-publicados).
