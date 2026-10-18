// Ejemplos de request (schemas/comprobante_request.schema.json) con datos ficticios.
// Generados por scripts/generadores/generar_plantillas.py; el core los valida al cargarlos.
import p0 from "./01_factura_electronica.json";
import p1 from "./01_factura_restaurante.json";
import p2 from "./01_factura_exoneracion.json";
import p3 from "./01_factura_surtidos_combo.json";
import p4 from "./01_factura_medicamento.json";
import p5 from "./01_factura_vehiculo_vin.json";
import p6 from "./02_nota_debito.json";
import p7 from "./03_nota_credito.json";
import p8 from "./04_tiquete_electronico.json";
import p9 from "./08_factura_compra.json";
import p10 from "./09_factura_exportacion.json";
import p11 from "./10_recibo_pago.json";

export interface EjemploDocumento {
  codigo: string;
  titulo: string;
  descripcion: string;
  badge: string;
  json: string;
}

export const EJEMPLOS_COMPROBANTES: EjemploDocumento[] = [
  {
    codigo: "01",
    titulo: "Factura Electrónica (01)",
    descripcion: "Servicios profesionales a crédito 30 días: dos líneas al 13%, una con descuento comercial",
    badge: "01 - FE",
    json: JSON.stringify(p0, null, 2),
  },
  {
    codigo: "01-restaurante",
    titulo: "Factura de Restaurante (01)",
    descripcion: "Consumo con impuesto de servicio 10% (otros cargos, Nota 16) pagado en efectivo y tarjeta",
    badge: "01 - Servicio 10%",
    json: JSON.stringify(p1, null, 2),
  },
  {
    codigo: "01-exoneracion",
    titulo: "Factura con Exoneración (01)",
    descripcion: "Venta de equipo con exoneración total del IVA (autorización local genérica, Nota 10.1)",
    badge: "01 - Exonerada",
    json: JSON.stringify(p2, null, 2),
  },
  {
    codigo: "01-surtido",
    titulo: "Factura con Surtido (01)",
    descripcion: "Canasta (CABYS de surtido habilitado) con componentes al 13%, 1% y exento; código del fabricante tipo 03",
    badge: "01 - Surtido",
    json: JSON.stringify(p3, null, 2),
  },
  {
    codigo: "01-medicamento",
    titulo: "Factura de Medicamento (01)",
    descripcion: "Medicamento con registro sanitario y forma farmacéutica (Nota 19) al 2%, pagado por SINPE Móvil",
    badge: "01 - Medicamento",
    json: JSON.stringify(p4, null, 2),
  },
  {
    codigo: "01-vin",
    titulo: "Factura de Vehículo con VIN (01)",
    descripcion: "Venta de vehículo nuevo con número VIN (NumeroVINoSerie)",
    badge: "01 - VIN",
    json: JSON.stringify(p5, null, 2),
  },
  {
    codigo: "02",
    titulo: "Nota de Débito Electrónica (02)",
    descripcion: "Aumenta el monto de la factura de servicios por horas no facturadas (código 02)",
    badge: "02 - ND",
    json: JSON.stringify(p6, null, 2),
  },
  {
    codigo: "03",
    titulo: "Nota de Crédito Electrónica (03)",
    descripcion: "Rebaja parcial de la factura de servicios (código 02, corrige monto)",
    badge: "03 - NC",
    json: JSON.stringify(p7, null, 2),
  },
  {
    codigo: "04",
    titulo: "Tiquete Electrónico (04)",
    descripcion: "Venta de mostrador sin receptor: producto al 13% y canasta básica al 1%, pago en efectivo",
    badge: "04 - TE",
    json: JSON.stringify(p8, null, 2),
  },
  {
    codigo: "08",
    titulo: "Factura Electrónica de Compra (08)",
    descripcion: "El comprador (receptor) emite la factura por la compra a un productor no inscrito; la clave lleva su cédula",
    badge: "08 - FEC",
    json: JSON.stringify(p9, null, 2),
  },
  {
    codigo: "09",
    titulo: "Factura Electrónica de Exportación (09)",
    descripcion: "Exportación en USD a cliente extranjero: servicios y mercancía con partida arancelaria, tarifa 0%",
    badge: "09 - FEE",
    json: JSON.stringify(p10, null, 2),
  },
  {
    codigo: "10",
    titulo: "Recibo Electrónico de Pago (10)",
    descripcion: "Registra el pago de una factura de servicios prestados al Estado (código de referencia 04; el 17 del Anexo no está en el XSD v4.4 publicado)",
    badge: "10 - REP",
    json: JSON.stringify(p11, null, 2),
  },
];
