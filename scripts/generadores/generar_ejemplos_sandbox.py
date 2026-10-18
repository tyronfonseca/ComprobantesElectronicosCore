#!/usr/bin/env python3
"""Genera un juego de requests (.json) relacionados entre sí con datos reales de emisor y receptor, para probar
en el ambiente sandbox de Hacienda. Solo para pruebas.

Los datos se leen de un archivo JSON (por defecto datos_sandbox.json junto a este script, ignorado por Git;
ver datos_sandbox.example.json). Hacienda valida que el CABYS corresponda a la actividad económica, por eso todas
las líneas usan el CABYS del archivo y cada parte su propio código de actividad.

    FE, NC, ND, REP   emisor -> receptor
    TE                emisor, sin receptor
    FEC               el emisor del archivo es el receptor del comprobante (el comprador que la emite y la
                      envía) y el receptor del archivo es el vendedor
    FEE               emisor -> receptorExtranjero (opcional en el archivo; si falta se usa uno ficticio)

Las notas de crédito y débito referencian la factura de contado y el recibo de pago la factura a crédito, así
que se deben enviar en el orden de los archivos.

Uso:
    python3 scripts/generadores/generar_ejemplos_sandbox.py [--datos datos_sandbox.json] [--inicio N] [--salida DIR]

Los JSON se cargan en el ejemplo web (Cargar JSON), que los valida y firma; el XML firmado se envía con
scripts/enviar_comprobante.py. Solo usa la biblioteca estándar.

El .env (para HACIENDA_USUARIO) es el de la raíz del repositorio, el mismo de scripts/enviar_comprobante.py.
"""
from __future__ import annotations

import argparse
import json
import re
import sys
import time
from pathlib import Path

AQUI = Path(__file__).resolve().parent
RAIZ = AQUI.parents[1]
sys.path.insert(0, str(AQUI.parent))  # scripts/: leer_env de enviar_comprobante.py

from enviar_comprobante import leer_env  # noqa: E402
from numeracion import codigo_seguridad, fecha_emision, generar_clave, generar_consecutivo  # noqa: E402


class ErrorDatos(Exception):
    pass


def identificacion_del_usuario(usuario: str) -> str | None:
    """Número de identificación del usuario del API: cpf-01-0123-0456 -> 101230456; cpj-3-101-000101 -> 3101000101."""
    m = re.match(r"^(cp[fj])-([\d-]+)@", usuario, re.IGNORECASE)
    if not m:
        return None
    grupos = m.group(2).split("-")
    return str(int(grupos[0])) + "".join(grupos[1:]) if m.group(1).lower() == "cpf" else "".join(grupos)


def leer_datos(ruta: Path) -> dict:
    if not ruta.exists():
        raise ErrorDatos(f"No existe {ruta}. Copie datos_sandbox.example.json a datos_sandbox.json y complételo.")
    try:
        datos = json.loads(ruta.read_text(encoding="utf-8"))
    except json.JSONDecodeError as e:
        raise ErrorDatos(f"{ruta} no es un JSON válido: {e}") from e
    for nombre in ("emisor", "receptor"):
        p = datos.get(nombre) or {}
        ident = p.get("identificacion") or {}
        if not (p.get("nombre") and ident.get("tipo") and ident.get("numero") and p.get("codigoActividad")
                and p.get("correoElectronico")):
            raise ErrorDatos(f"Faltan datos de {nombre}: nombre, identificacion.tipo, identificacion.numero, "
                             "codigoActividad y correoElectronico.")
    if not re.fullmatch(r"\d{13}", datos.get("cabys", "")):
        raise ErrorDatos("cabys debe ser un código de 13 dígitos.")
    return datos


def parte(p: dict, como_emisor: bool, completa: bool = True) -> dict:
    """Parte en el formato del request: el emisor lleva una lista de correos y el receptor uno solo."""
    out = {"nombre": p["nombre"], "identificacion": p["identificacion"]}
    if completa:
        for campo in ("nombreComercial", "ubicacion", "telefono"):
            if p.get(campo):
                out[campo] = p[campo]
    out["correoElectronico"] = [p["correoElectronico"]] if como_emisor else p["correoElectronico"]
    return out


def generar(datos: dict, inicio: int) -> list[tuple[str, dict]]:
    E, R = datos["emisor"], datos["receptor"]
    X = datos.get("receptorExtranjero") or {
        "nombre": "EXAMPLE TEST CUSTOMER LLC", "identificacion": {"tipo": "05", "numero": "900000001"},
        "correoElectronico": E["correoElectronico"],
    }
    tarifa = datos.get("codigoTarifaIVA", "08")
    cabys = datos["cabys"]
    fecha = fecha_emision()

    def numerar(req: dict, secuencia: int) -> dict:
        # La clave lleva la identificación de quien emite: el receptor en la FEC.
        quien_emite = req["receptor"] if req["tipoDocumento"] == "08" else req["emisor"]
        req["numeroConsecutivo"] = generar_consecutivo(1, 1, req["tipoDocumento"], secuencia)
        req["clave"] = generar_clave(fecha, quien_emite["identificacion"]["numero"], req["numeroConsecutivo"],
                                     codigo_seguridad())
        return req

    def base(tipo_documento: str, **extra) -> dict:
        return {"tipoDocumento": tipo_documento, "clave": "", "proveedorSistemas": E["identificacion"]["numero"],
                "numeroConsecutivo": "", "fechaEmision": fecha, **extra}

    def iva(codigo_tarifa: str = tarifa) -> list[dict]:
        return [{"codigo": "01", "codigoTarifaIVA": codigo_tarifa}]

    def crc(tipo_medio_pago: str = "01") -> dict:
        return {"codigoTipoMoneda": {"codigoMoneda": "CRC"}, "mediosPago": [{"tipoMedioPago": tipo_medio_pago}]}

    def linea(detalle: str, cantidad: str, precio: str, **extra) -> dict:
        return {"codigoCABYS": cabys, "cantidad": cantidad, "unidadMedida": "Unid", "detalle": detalle,
                "precioUnitario": precio, "impuestos": iva(), **extra}

    def referencia(doc: dict, codigo: str, razon: str) -> list[dict]:
        return [{"tipoDocIR": doc["tipoDocumento"], "numero": doc["clave"], "fechaEmisionIR": doc["fechaEmision"],
                 "codigo": codigo, "razon": razon}]

    def entre_emisor_y_receptor() -> dict:
        return {"codigoActividadEmisor": E["codigoActividad"], "codigoActividadReceptor": R["codigoActividad"],
                "emisor": parte(E, True), "receptor": parte(R, False)}

    # ── Ejemplos, en el orden de envío ────────────────────────────────────────
    fe_contado = numerar(base(
        "01", **entre_emisor_y_receptor(), condicionVenta="01",
        detalleServicio=[
            linea("Artículo de prueba A", "2", "10000"),
            linea("Artículo de prueba B", "1", "25000", descuentos=[{"montoDescuento": "2500", "codigoDescuento": "07"}]),
        ],
        resumenFactura=crc("01"),
    ), inicio)

    cantidad_credito, precio_credito = 5, 20000
    fe_credito = numerar(base(
        "01", **entre_emisor_y_receptor(), condicionVenta="02", plazoCredito=30,
        detalleServicio=[linea("Artículo de prueba a crédito", str(cantidad_credito), str(precio_credito))],
        resumenFactura=crc("04"),
    ), inicio + 1)

    nc = numerar(base(
        "03", **entre_emisor_y_receptor(), condicionVenta="01",
        detalleServicio=[linea("Devolución del artículo de prueba A", "1", "10000")],
        resumenFactura=crc("01"),
        informacionReferencia=referencia(fe_contado, "02", "Corrige monto: devolución de una unidad"),
    ), inicio)

    nd = numerar(base(
        "02", **entre_emisor_y_receptor(), condicionVenta="01",
        detalleServicio=[linea("Cargo adicional no facturado", "1", "5000")],
        resumenFactura=crc("01"),
        informacionReferencia=referencia(fe_contado, "02", "Corrige monto: cargo no facturado"),
    ), inicio)

    te = numerar(base(
        "04", codigoActividadEmisor=E["codigoActividad"], emisor=parte(E, True), condicionVenta="01",
        detalleServicio=[linea("Venta de mostrador de prueba", "3", "1500")],
        resumenFactura=crc("01"),
    ), inicio)

    fec = numerar(base(
        "08", codigoActividadReceptor=E["codigoActividad"],
        emisor=parte(R, True), receptor=parte(E, False), condicionVenta="01",
        detalleServicio=[linea("Compra de prueba", "10", "3000")],
        resumenFactura=crc("01"),
        informacionReferencia=[{"tipoDocIR": "14", "numero": "REG-PRUEBA-0001", "fechaEmisionIR": fecha, "codigo": "04",
                                "razon": "Compra a proveedor no inscrito (pruebas)"}],
    ), inicio)

    fee = numerar(base(
        "09", codigoActividadEmisor=E["codigoActividad"], emisor=parte(E, True),
        receptor={**parte(X, False), "otrasSenasExtranjero": X.get("otrasSenasExtranjero", "Dirección de pruebas en el extranjero")},
        condicionVenta="01",
        detalleServicio=[{**linea("Exportación de prueba", "2", "100"), "impuestos": iva("10")}],  # tarifa 0 %
        resumenFactura={"codigoTipoMoneda": {"codigoMoneda": "USD", "tipoCambio": "505.25"},
                        "mediosPago": [{"tipoMedioPago": "04"}]},
    ), inicio)

    # El recibo registra el pago de la factura a crédito: el monto sin impuestos (venta neta, sin descuentos)
    # y el core calcula el IVA de ese monto.
    pagado = str(cantidad_credito * precio_credito)
    rep = numerar(base(
        "10", emisor=parte(E, True, completa=False), receptor=parte(R, False, completa=False), condicionVenta="09",
        detalleServicio=[{"detalle": "Pago de la factura a crédito de prueba", "montoTotal": pagado, "subTotal": pagado,
                          "impuestos": iva()}],
        resumenFactura=crc("04"),
        # Código 04 (referencia a otro documento): el 17 del Anexo no está en el XSD v4.4 publicado y fallaría la validación de esquema.
        informacionReferencia=referencia(fe_credito, "04", "Pago de la factura a crédito de prueba"),
    ), inicio)

    return [("01_fe_contado", fe_contado), ("02_fe_credito", fe_credito), ("03_nc_fe_contado", nc),
            ("04_nd_fe_contado", nd), ("05_te", te), ("06_fec", fec), ("07_fee", fee), ("08_rep_fe_credito", rep)]


def main() -> int:
    p = argparse.ArgumentParser(description="Genera comprobantes relacionados con datos reales para el sandbox de Hacienda.")
    p.add_argument("--datos", type=Path, default=AQUI / "datos_sandbox.json", help="datos del emisor, receptor y CABYS")
    p.add_argument("--inicio", type=int, default=int(time.time()) % 10**10,
                   help="primer número de secuencia del consecutivo (por defecto, derivado de la hora)")
    p.add_argument("--salida", type=Path, default=RAIZ / "ejemplos_sandbox", help="carpeta de salida (ignorada por Git)")
    p.add_argument("--env", type=Path, default=RAIZ / ".env", help="archivo .env con HACIENDA_USUARIO")
    args = p.parse_args()

    try:
        if not 1 <= args.inicio <= 9_999_999_998:
            raise ErrorDatos("--inicio debe ser un número entre 1 y 9999999998.")
        datos = leer_datos(args.datos)
        E, R = datos["emisor"], datos["receptor"]
        usuario = identificacion_del_usuario(leer_env(args.env).get("HACIENDA_USUARIO", ""))
        if usuario and usuario != E["identificacion"]["numero"]:
            print(f"Aviso: el emisor ({E['identificacion']['numero']}) no es el dueño de HACIENDA_USUARIO ({usuario}); "
                  "Hacienda solo acepta comprobantes propios.", file=sys.stderr)
        ejemplos = generar(datos, args.inicio)
    except (ErrorDatos, ValueError) as e:
        print(f"Error: {e}", file=sys.stderr)
        return 1

    print(f"Emisor:   {E['nombre']} ({E['identificacion']['tipo']} {E['identificacion']['numero']}), actividad {E['codigoActividad']}")
    print(f"Receptor: {R['nombre']} ({R['identificacion']['tipo']} {R['identificacion']['numero']}), actividad {R['codigoActividad']}")
    print(f"CABYS {datos['cabys']}, tarifa {datos.get('codigoTarifaIVA', '08')}, secuencia {args.inicio}")
    args.salida.mkdir(parents=True, exist_ok=True)
    for archivo, req in ejemplos:
        (args.salida / f"{archivo}.json").write_text(json.dumps(req, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
        print(f"  {archivo}.json  {req['clave']}")
    print(f"\nEscritos en {args.salida}. Cárguelos en el ejemplo web en este orden, fírmelos y envíelos con "
          "scripts/enviar_comprobante.py.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
