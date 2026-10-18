#!/usr/bin/env python3
"""Genera las plantillas de examples/web/src/templates (JSON e index.ts) con datos ficticios.

Uso: python3 scripts/generadores/generar_plantillas.py

Los montos que dependen del cálculo (el pago con tarjeta del restaurante) se calculan aquí con las mismas
reglas del core. Al cargar una plantilla en el ejemplo web, el core la valida.
"""
from __future__ import annotations

import copy
import json
from pathlib import Path

from numeracion import generar_clave, generar_consecutivo

RAIZ = Path(__file__).resolve().parents[2]
SALIDA = RAIZ / "examples/web/src/templates"

FECHA = "2026-10-01T10:00:00-06:00"
PROVEEDOR = "3101000999"  # Proveedor de sistemas ficticio

# ── Partes ficticias ──────────────────────────────────────────────────────────
SOFTWARE = {
    "nombre": "SOLUCIONES DE SOFTWARE EJEMPLO S.A.",
    "identificacion": {"tipo": "02", "numero": "3101000101"},
    "nombreComercial": "Software Ejemplo",
    "ubicacion": {"provincia": "1", "canton": "01", "distrito": "08", "otrasSenas": "Avenida 10, edificio azul, piso 2"},
    "telefono": {"codigoPais": "506", "numTelefono": "22000101"},
    "correoElectronico": ["facturacion@software.example.com"],
}
CLIENTE = {
    "nombre": "COMERCIALIZADORA CLIENTE EJEMPLO S.A.",
    "identificacion": {"tipo": "02", "numero": "3101000202"},
    "ubicacion": {"provincia": "4", "canton": "01", "distrito": "01", "otrasSenas": "100 metros norte del parque central"},
    "telefono": {"codigoPais": "506", "numTelefono": "22000202"},
    "correoElectronico": "cuentasporpagar@cliente.example.com",
}


def con(base: dict, **cambios) -> dict:
    """Copia de una parte con campos reemplazados."""
    return {**copy.deepcopy(base), **cambios}


def numerar(req: dict, numero: int, identificacion: str, seguridad: str = "00000000") -> dict:
    req["numeroConsecutivo"] = generar_consecutivo(1, 1, req["tipoDocumento"], numero)
    req["clave"] = generar_clave(req["fechaEmision"], identificacion, req["numeroConsecutivo"], seguridad)
    return req


def base(tipo_documento: str, **extra) -> dict:
    return {"tipoDocumento": tipo_documento, "clave": "", "proveedorSistemas": PROVEEDOR,
            "numeroConsecutivo": "", "fechaEmision": FECHA, **extra}


def iva(codigo_tarifa: str = "08") -> list[dict]:
    return [{"codigo": "01", "codigoTarifaIVA": codigo_tarifa}]


def crc(medios_pago: list[dict] | None = None) -> dict:
    return {"codigoTipoMoneda": {"codigoMoneda": "CRC"}, "mediosPago": medios_pago or [{"tipoMedioPago": "04"}]}


plantillas: list[dict] = []


def agregar(archivo: str, codigo: str, badge: str, titulo: str, descripcion: str, req: dict) -> None:
    plantillas.append({"archivo": archivo, "codigo": codigo, "badge": badge, "titulo": titulo,
                       "descripcion": descripcion, "req": req})


# ── Plantillas ────────────────────────────────────────────────────────────────
fe_servicios = numerar(base(
    "01", codigoActividadEmisor="6201.0", codigoActividadReceptor="4690.0",
    emisor=SOFTWARE, receptor=CLIENTE, condicionVenta="02", plazoCredito=30,
    detalleServicio=[
        {"codigoCABYS": "8314100000200", "codigosComerciales": [{"tipo": "01", "codigo": "DEV-HORA"}], "cantidad": "40",
         "unidadMedida": "h", "detalle": "Desarrollo de software a la medida (horas)", "precioUnitario": "25000", "impuestos": iva()},
        {"codigoCABYS": "8314100000200", "cantidad": "1", "unidadMedida": "Sp", "detalle": "Capacitación de usuarios",
         "precioUnitario": "150000", "descuentos": [{"montoDescuento": "15000", "codigoDescuento": "07"}], "impuestos": iva()},
    ],
    resumenFactura=crc([{"tipoMedioPago": "04"}]),
), 1, SOFTWARE["identificacion"]["numero"], "10000001")
agregar("01_factura_electronica", "01", "01 - FE", "Factura Electrónica (01)",
        "Servicios profesionales a crédito 30 días: dos líneas al 13%, una con descuento comercial", fe_servicios)

# Restaurante: 2 x 6500 + 2 x 1800 = 16600; IVA 13 % = 2158; servicio 10 % = 1660; total 20418.
# Se pagan 10000 en efectivo y el resto con tarjeta.
restaurante = numerar(base(
    "01", codigoActividadEmisor="5610.1",
    emisor=con(SOFTWARE, nombre="RESTAURANTE EJEMPLO LIMITADA", nombreComercial="Restaurante Ejemplo",
               identificacion={"tipo": "02", "numero": "3102000303"}, correoElectronico=["facturas@restaurante.example.com"]),
    receptor={"nombre": "MARIA EJEMPLO EJEMPLO", "identificacion": {"tipo": "01", "numero": "100000303"},
              "correoElectronico": "maria@correo.example.com"},
    condicionVenta="01",
    detalleServicio=[
        {"codigoCABYS": "6331100000100", "cantidad": "2", "unidadMedida": "Unid", "detalle": "Casado con pescado",
         "precioUnitario": "6500", "impuestos": iva()},
        {"codigoCABYS": "6331100000100", "cantidad": "2", "unidadMedida": "Unid", "detalle": "Refresco natural",
         "precioUnitario": "1800", "impuestos": iva()},
    ],
    otrosCargos=[{"tipoDocumentoOC": "06", "detalle": "Impuesto de servicio 10%", "porcentajeOC": "10", "montoCargo": "1660"}],
    resumenFactura=crc([{"tipoMedioPago": "01", "totalMedioPago": "10000"}, {"tipoMedioPago": "02", "totalMedioPago": "0"}]),
), 2, "3102000303", "10000002")
venta = 2 * 6500 + 2 * 1800
restaurante["resumenFactura"]["mediosPago"][1]["totalMedioPago"] = f"{venta * 1.13 + venta * 0.10 - 10000:.2f}"
agregar("01_factura_restaurante", "01-restaurante", "01 - Servicio 10%", "Factura de Restaurante (01)",
        "Consumo con impuesto de servicio 10% (otros cargos, Nota 16) pagado en efectivo y tarjeta", restaurante)

exoneracion = numerar(base(
    "01", codigoActividadEmisor="4651.0", codigoActividadReceptor="8510.0",
    emisor=con(SOFTWARE, nombre="DISTRIBUIDORA DE EQUIPO EJEMPLO S.A.", nombreComercial="Equipo Ejemplo",
               identificacion={"tipo": "02", "numero": "3101000404"}, correoElectronico=["ventas@equipo.example.com"]),
    receptor=con(CLIENTE, nombre="ASOCIACION EDUCATIVA EJEMPLO", identificacion={"tipo": "02", "numero": "3002000404"}),
    condicionVenta="01",
    detalleServicio=[
        {"codigoCABYS": "4523000000000", "codigosComerciales": [{"tipo": "04", "codigo": "LAP-14"}], "cantidad": "10",
         "unidadMedida": "Unid", "detalle": "Computadora portátil de 14 pulgadas", "precioUnitario": "420000", "impuestos": iva(),
         "exoneracion": {"tipoDocumentoEX1": "04", "numeroDocumento": "AL-00000001-26", "nombreInstitucion": "01",
                         "fechaEmisionEX": "2026-09-15T08:00:00-06:00", "tarifaExonerada": "13"}},
    ],
    resumenFactura=crc(),
), 3, "3101000404", "10000003")
agregar("01_factura_exoneracion", "01-exoneracion", "01 - Exonerada", "Factura con Exoneración (01)",
        "Venta de equipo con exoneración total del IVA (autorización local genérica, Nota 10.1)", exoneracion)

surtido = numerar(base(
    "01", codigoActividadEmisor="4711.0",
    emisor=con(SOFTWARE, nombre="SUPERMERCADO EJEMPLO S.A.", nombreComercial="Super Ejemplo",
               identificacion={"tipo": "02", "numero": "3101000505"}, correoElectronico=["facturacion@super.example.com"]),
    receptor=CLIENTE, condicionVenta="01",
    detalleServicio=[{
        "codigoCABYS": "2399999002200", "codigosComerciales": [{"tipo": "03", "codigo": "7400000000017"}], "cantidad": "2",
        "unidadMedida": "Unid", "detalle": "Canasta de productos alimenticios", "precioUnitario": "6500", "impuestos": iva(),
        "detalleSurtido": [
            {"codigoCABYS": "2111101000000", "cantidad": "1", "unidadMedida": "Unid", "detalle": "Galletas finas 300 g",
             "precioUnitario": "2500", "impuestos": iva()},
            {"codigoCABYS": "0111101000000", "cantidad": "4", "unidadMedida": "Unid", "detalle": "Manzanas",
             "precioUnitario": "250", "impuestos": iva("02")},
            {"codigoCABYS": "0113101000000", "cantidad": "1", "unidadMedida": "Kg", "detalle": "Café molido",
             "precioUnitario": "3000", "impuestos": iva("10")},
        ],
    }],
    resumenFactura=crc([{"tipoMedioPago": "02"}]),
), 4, "3101000505", "10000004")
agregar("01_factura_surtidos_combo", "01-surtido", "01 - Surtido", "Factura con Surtido (01)",
        "Canasta (CABYS de surtido habilitado) con componentes al 13%, 1% y exento; código del fabricante tipo 03", surtido)

medicamento = numerar(base(
    "01", codigoActividadEmisor="4772.0",
    emisor=con(SOFTWARE, nombre="FARMACIA EJEMPLO S.A.", nombreComercial="Farmacia Ejemplo",
               identificacion={"tipo": "02", "numero": "3101000606"}, correoElectronico=["ventas@farmacia.example.com"]),
    receptor={"nombre": "JUAN EJEMPLO EJEMPLO", "identificacion": {"tipo": "01", "numero": "100000606"},
              "correoElectronico": "juan@correo.example.com"},
    condicionVenta="01",
    detalleServicio=[{"codigoCABYS": "3562100000000", "cantidad": "2", "unidadMedida": "Unid",
                      "detalle": "Acetaminofén 500 mg, caja de 10 tabletas", "precioUnitario": "1500",
                      "registroMedicamento": "MS-0000001",
                      "formaFarmaceutica": "199",  # Tableta (Nota Técnica 9)
                      "impuestos": iva("03")}],
    resumenFactura=crc([{"tipoMedioPago": "06"}]),
), 5, "3101000606", "10000005")
agregar("01_factura_medicamento", "01-medicamento", "01 - Medicamento", "Factura de Medicamento (01)",
        "Medicamento con registro sanitario y forma farmacéutica (Nota 19) al 2%, pagado por SINPE Móvil", medicamento)

vehiculo = numerar(base(
    "01", codigoActividadEmisor="4510.0",
    emisor=con(SOFTWARE, nombre="AGENCIA DE VEHICULOS EJEMPLO S.A.", nombreComercial="Autos Ejemplo",
               identificacion={"tipo": "02", "numero": "3101000707"}, correoElectronico=["ventas@autos.example.com"]),
    receptor=CLIENTE, condicionVenta="01",
    detalleServicio=[{"codigoCABYS": "4911300000100", "cantidad": "1", "unidadMedida": "Unid", "detalle": "Automóvil sedán 0 km",
                      "precioUnitario": "15000000", "numerosVINoSerie": ["JN1AA0000AA000001"], "impuestos": iva()}],
    resumenFactura=crc(),
), 6, "3101000707", "10000006")
agregar("01_factura_vehiculo_vin", "01-vin", "01 - VIN", "Factura de Vehículo con VIN (01)",
        "Venta de vehículo nuevo con número VIN (NumeroVINoSerie)", vehiculo)

nd = numerar(base(
    "02", codigoActividadEmisor="6201.0", emisor=SOFTWARE, receptor=CLIENTE, condicionVenta="01",
    detalleServicio=[{"codigoCABYS": "8314100000200", "cantidad": "1", "unidadMedida": "Sp", "detalle": "Horas adicionales no facturadas",
                      "precioUnitario": "50000", "impuestos": iva()}],
    resumenFactura=crc(),
    informacionReferencia=[{"tipoDocIR": "01", "numero": fe_servicios["clave"], "fechaEmisionIR": FECHA, "codigo": "02",
                            "razon": "Corrige monto: horas adicionales"}],
), 1, SOFTWARE["identificacion"]["numero"], "20000001")
agregar("02_nota_debito", "02", "02 - ND", "Nota de Débito Electrónica (02)",
        "Aumenta el monto de la factura de servicios por horas no facturadas (código 02)", nd)

nc = numerar(base(
    "03", codigoActividadEmisor="6201.0", emisor=SOFTWARE, receptor=CLIENTE, condicionVenta="01",
    detalleServicio=[{"codigoCABYS": "8314100000200", "cantidad": "1", "unidadMedida": "Sp", "detalle": "Capacitación de usuarios no impartida",
                      "precioUnitario": "135000", "impuestos": iva()}],
    resumenFactura=crc(),
    informacionReferencia=[{"tipoDocIR": "01", "numero": fe_servicios["clave"], "fechaEmisionIR": FECHA, "codigo": "02",
                            "razon": "Corrige monto: se elimina la capacitación"}],
), 1, SOFTWARE["identificacion"]["numero"], "30000001")
agregar("03_nota_credito", "03", "03 - NC", "Nota de Crédito Electrónica (03)",
        "Rebaja parcial de la factura de servicios (código 02, corrige monto)", nc)

te = numerar(base(
    "04", codigoActividadEmisor="4711.0",
    emisor=con(SOFTWARE, nombre="MINISUPER EJEMPLO S.A.", nombreComercial="Minisuper Ejemplo",
               identificacion={"tipo": "02", "numero": "3101000808"}, correoElectronico=["caja@minisuper.example.com"]),
    condicionVenta="01",
    detalleServicio=[
        {"codigoCABYS": "2399999000000", "cantidad": "2", "unidadMedida": "Unid", "detalle": "Galletas surtidas",
         "precioUnitario": "950", "impuestos": iva()},
        {"codigoCABYS": "0113101000000", "cantidad": "1", "unidadMedida": "Kg", "detalle": "Arroz",
         "precioUnitario": "1200", "impuestos": iva("02")},
    ],
    resumenFactura=crc([{"tipoMedioPago": "01"}]),
), 1, "3101000808", "40000001")
agregar("04_tiquete_electronico", "04", "04 - TE", "Tiquete Electrónico (04)",
        "Venta de mostrador sin receptor: producto al 13% y canasta básica al 1%, pago en efectivo", te)

fec = numerar(base(
    "08", codigoActividadReceptor="1079.0",
    emisor={"nombre": "PEDRO PRODUCTOR EJEMPLO", "identificacion": {"tipo": "01", "numero": "100000909"},
            "ubicacion": {"provincia": "1", "canton": "19", "distrito": "01", "otrasSenas": "Finca Ejemplo, 2 km al este de la escuela"},
            "correoElectronico": ["pedro@correo.example.com"]},
    receptor=con(CLIENTE, nombre="BENEFICIO DE CAFE EJEMPLO S.A.", identificacion={"tipo": "02", "numero": "3101000909"}),
    condicionVenta="01",
    detalleServicio=[{"codigoCABYS": "0161100000100", "cantidad": "50", "unidadMedida": "Unid", "detalle": "Café en fruta (fanegas)",
                      "precioUnitario": "45000", "impuestos": iva("02")}],
    resumenFactura=crc(),
    informacionReferencia=[{"tipoDocIR": "14", "numero": "REG-ESP-0001", "fechaEmisionIR": FECHA, "codigo": "04",
                            "razon": "Compra a productor del régimen especial agropecuario"}],
), 1, "3101000909", "80000001")
agregar("08_factura_compra", "08", "08 - FEC", "Factura Electrónica de Compra (08)",
        "El comprador (receptor) emite la factura por la compra a un productor no inscrito; la clave lleva su cédula", fec)

fee = numerar(base(
    "09", codigoActividadEmisor="6201.0", emisor=SOFTWARE,
    receptor={"nombre": "EXAMPLE IMPORTS LLC", "identificacion": {"tipo": "05", "numero": "900000001"},
              "otrasSenasExtranjero": "100 Example Street, Springfield, USA", "correoElectronico": "ap@imports.example.com"},
    condicionVenta="01",
    detalleServicio=[
        {"codigoCABYS": "8314100000200", "cantidad": "80", "unidadMedida": "h", "detalle": "Software development services",
         "precioUnitario": "55", "impuestos": iva("10")},
        {"codigoCABYS": "4523000000000", "partidaArancelaria": "847130000000", "cantidad": "2", "unidadMedida": "Unid",
         "detalle": "Laptop computer", "precioUnitario": "900", "impuestos": iva("10")},
    ],
    resumenFactura={"codigoTipoMoneda": {"codigoMoneda": "USD", "tipoCambio": "505.25"}, "mediosPago": [{"tipoMedioPago": "04"}]},
), 1, SOFTWARE["identificacion"]["numero"], "90000001")
agregar("09_factura_exportacion", "09", "09 - FEE", "Factura Electrónica de Exportación (09)",
        "Exportación en USD a cliente extranjero: servicios y mercancía con partida arancelaria, tarifa 0%", fee)

rep = numerar(base(
    "10",
    emisor={"nombre": SOFTWARE["nombre"], "identificacion": SOFTWARE["identificacion"], "correoElectronico": SOFTWARE["correoElectronico"]},
    receptor={"nombre": "INSTITUCION PUBLICA EJEMPLO", "identificacion": {"tipo": "02", "numero": "3007000001"},
              "correoElectronico": "pagos@institucion.example.com"},
    condicionVenta="09",
    detalleServicio=[{"detalle": "Pago de la factura de servicios prestados al Estado", "montoTotal": "1000000",
                      "subTotal": "1000000", "impuestos": iva()}],
    resumenFactura=crc(),
    informacionReferencia=[{"tipoDocIR": "01", "numero": fe_servicios["clave"], "fechaEmisionIR": FECHA, "codigo": "04",
                            "razon": "Pago a comprobante electrónico"}],
), 1, SOFTWARE["identificacion"]["numero"], "11000001")
agregar("10_recibo_pago", "10", "10 - REP", "Recibo Electrónico de Pago (10)",
        "Registra el pago de una factura de servicios prestados al Estado (código de referencia 04; el 17 del Anexo no está en el XSD v4.4 publicado)", rep)


# ── Escribir ──────────────────────────────────────────────────────────────────
def a_json(valor) -> str:
    return json.dumps(valor, indent=2, ensure_ascii=False)


def main() -> None:
    for p in plantillas:
        (SALIDA / f"{p['archivo']}.json").write_text(a_json(p["req"]) + "\n", encoding="utf-8")

    imports = "\n".join(f'import p{i} from "./{p["archivo"]}.json";' for i, p in enumerate(plantillas))
    lista = "\n".join(f"""  {{
    codigo: {a_json(p["codigo"])},
    titulo: {a_json(p["titulo"])},
    descripcion: {a_json(p["descripcion"])},
    badge: {a_json(p["badge"])},
    json: JSON.stringify(p{i}, null, 2),
  }},""" for i, p in enumerate(plantillas))
    (SALIDA / "index.ts").write_text(f"""// Ejemplos de request (schemas/comprobante_request.schema.json) con datos ficticios.
// Generados por scripts/generadores/generar_plantillas.py; el core los valida al cargarlos.
{imports}

export interface EjemploDocumento {{
  codigo: string;
  titulo: string;
  descripcion: string;
  badge: string;
  json: string;
}}

export const EJEMPLOS_COMPROBANTES: EjemploDocumento[] = [
{lista}
];
""", encoding="utf-8")
    print(f"{len(plantillas)} plantillas escritas en {SALIDA.relative_to(RAIZ)}.")


if __name__ == "__main__":
    main()
