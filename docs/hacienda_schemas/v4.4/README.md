# Esquemas XSD y Catálogos Oficiales v4.4 - Ministerio de Hacienda de Costa Rica

Este directorio contiene la especificación técnica oficial, esquemas XML Schema Definition (**XSD**), anexos y catálogos de referencia correspondientes a la versión **v4.4** del sistema de Comprobantes Electrónicos de la Dirección General de Tributación (DGT) del Ministerio de Hacienda de Costa Rica.

---

## Aviso Legal e Importante

> **Origen de los esquemas**: Los documentos y esquemas XSD/XML alojados en este directorio fueron recopilados a partir del portal oficial de la Dirección General de Tributación:  
> **[Ministerio de Hacienda - Anexos y Estructuras de Comprobantes Electrónicos](https://atv.hacienda.go.cr/ATV/ComprobanteElectronico/frmAnexosyEstructuras.aspx#)**
>
> **Información pública y licencia**: Los documentos de este directorio son información pública publicada por el Ministerio de Hacienda. Se incluyen sin cambios de contenido por parte de este proyecto, solo como referencia técnica para desarrollar y probar la biblioteca. Su autoría y sus derechos pertenecen al Ministerio de Hacienda y no están cubiertos por la licencia MIT del repositorio. Verifique siempre la versión vigente en el portal oficial.
>
> **Fuente de verdad y no afiliación**: Este proyecto no tiene afiliación oficial con el Ministerio de Hacienda ni con la Dirección General de Tributación. La única fuente normativa y legalmente vinculante son los documentos publicados en sus canales oficiales.

> **Versión de los XSD**: Los esquemas XSD de este directorio son anteriores a la bitácora de ajustes del 22/04/2026 del documento "Anexos y Estructuras para la Emisión de Comprobantes Electrónicos v4.4". Por ejemplo, no incluyen los códigos 13 a 17 de la Nota 9 ni los códigos 19 y 20 de la Nota 10 (esto sigue así en la última versión de los XSD, actualizada el 18/10/2026). Las reglas de la biblioteca siguen el documento oficial con esos ajustes; ante una diferencia entre un XSD y el documento, prevalece el documento.

---

## Estructura y Detalle de Esquemas XSD

| Archivo XSD | Tipo de Comprobante / Estructura | Código `TipoDocumento` | Descripción |
|---|---|:---:|---|
| [`FacturaElectronica_V4.4.xsd`](/docs/hacienda_schemas/v4.4/FacturaElectronica_V4.4.xsd) | Factura Electrónica | `01` | Comprobante estándar de venta de bienes o prestación de servicios en el mercado local. |
| [`NotaDebitoElectronica_V4.4.xsd`](/docs/hacienda_schemas/v4.4/NotaDebitoElectronica_V4.4.xsd) | Nota de Débito Electrónica | `02` | Ajuste al alza de montos previamente facturados, cargos financieros o intereses. |
| [`NotaCreditoElectronica_V4.4.xsd`](/docs/hacienda_schemas/v4.4/NotaCreditoElectronica_V4.4.xsd) | Nota de Crédito Electrónica | `03` | Anulaciones parciales o totales, devoluciones de mercadería o descuentos posteriores. |
| [`TiqueteElectronico_V4.4.xsd`](/docs/hacienda_schemas/v4.4/TiqueteElectronico_V4.4.xsd) | Tiquete Electrónico | `04` | Comprobante emitido al consumidor final (receptor no obligado o sin requerimiento de crédito fiscal). |
| [`FacturaElectronicaCompra_V4.4.xsd`](/docs/hacienda_schemas/v4.4/FacturaElectronicaCompra_V4.4.xsd) | Factura Electrónica de Compra | `08` | Emitida por el comprador cuando adquiere bienes/servicios a proveedores no obligados o del Régimen Simplificado. |
| [`FacturaElectronicaExportacion_V4.4.xsd`](/docs/hacienda_schemas/v4.4/FacturaElectronicaExportacion_V4.4.xsd) | Factura Electrónica de Exportación | `09` | Comprobante para ventas de mercancías o servicios destinados al exterior (operaciones exentas de IVA local). |
| [`ReciboElectronicoPago_V4.4.xsd`](/docs/hacienda_schemas/v4.4/ReciboElectronicoPago_V4.4.xsd) | Recibo Electrónico de Pago | `10` | Documento electrónico emitido para respaldar pagos de facturas a crédito o recepciones parciales. |
| [`MensajeReceptor_V4.4.xsd`](/docs/hacienda_schemas/v4.4/MensajeReceptor_V4.4.xsd) | Mensaje Receptor | - | Mensajes de Aceptación (`05`), Aceptación Parcial (`06`) o Rechazo (`07`) transmitidos por el receptor a Hacienda. |
| [`MensajeHacienda_V4.4.xsd`](/docs/hacienda_schemas/v4.4/MensajeHacienda_V4.4.xsd) | Mensaje Hacienda | - | Respuesta oficial de validación generada por el Ministerio de Hacienda (Aceptado, Rechazado o Aceptado Parcialmente). |
| [`xmldsig-core-schema.xsd`](/docs/hacienda_schemas/v4.4/xmldsig-core-schema.xsd) | Firma Digital W3C XMLDSig | - | Esquema base del estándar W3C para la firma digital de documentos XML (utilizado en firmas XAdES-EPES). |

---

## Catálogos Oficiales y Anexos Técnicos

Además de los esquemas XML, este directorio incluye catálogos oficiales descargados de Hacienda y digitalizados para el motor C++:

### 1. Codificación Territorial (Provincias, Cantones, Distritos y Barrios)
- **Archivo**: [`Codificacion,canton,provincia,distritoybarrio.pdf`](/docs/hacienda_schemas/v4.4/Codificacion,canton,provincia,distritoybarrio.pdf)
- **Uso**: Define la codificación oficial de ubicación geográfica de Costa Rica:
  - **7 Provincias** (1 a 7).
  - **84 Cantones** (incluyendo los cantones más recientes: Monteverde [06-12] y Puerto Jiménez [06-13]).
  - **492 Distritos**.
  - **7 087 barrios**.
- **Implementación C++**: Clase [`DivisionTerritorial`](/include/cec/common/DivisionTerritorial.h), expuesta también en la API de C (`cec_dta_*`) y en los bindings.

### 2. Catálogo de Formas Farmacéuticas (Nota 19)
- **Archivo**: [`Nota_9_Codigo_Forma_Farmaceutica.csv`](/docs/hacienda_schemas/v4.4/Nota_9_Codigo_Forma_Farmaceutica.csv)
- **Uso**: Catálogo oficial de 229 códigos y descripciones para medicamentos, productos farmacéuticos y sanitarios en comprobantes electrónicos según la Nota 19.
- **Implementación C++**: Clase [`FormasFarmaceuticas`](/include/cec/common/FormasFarmaceuticas.h), usada por la regla de medicamentos.

---

## Validar un XML contra los XSD

```bash
python3 scripts/validar_xsd.py comprobante.xml          # o varios archivos; necesita xmllint
python3 scripts/validar_xsd.py test/golden/*/expected.xml
```

Elige el XSD por el elemento raíz. Los XSD oficiales importan `xmldsig-core-schema.xsd` con la ruta `../../xmldsig-core-schema.xsd` (la de la CDN de Hacienda), que no resuelve desde este directorio; el script trabaja sobre una copia con la ruta corregida y no modifica estos archivos. Más detalle y las diferencias conocidas con el Anexo: [docs/LIMITACIONES.md](/docs/LIMITACIONES.md).

---

## Documentación relacionada

- [Reglas y condiciones por campo](/docs/reglas_condiciones_campos): resumen de las condiciones de cada campo por tipo de comprobante.
- [JSON Schema del request](/schemas/comprobante_request.schema.json): datos de entrada que recibe la biblioteca.
- [README principal](/README.md): arquitectura, compilación y uso de la API.
