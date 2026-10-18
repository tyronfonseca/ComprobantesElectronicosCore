#!/usr/bin/env python3
"""Valida comprobantes XML contra los XSD oficiales de Hacienda (docs/hacienda_schemas/v4.4).

Uso:
    python3 scripts/validar_xsd.py test/golden/*/expected.xml
    python3 scripts/validar_xsd.py comprobante_firmado.xml

Elige el XSD por el elemento raíz (FacturaElectronica, TiqueteElectronico, MensajeReceptor...). Si el XML no está
firmado se le agrega una ds:Signature de relleno, solo para la validación, porque el XSD la exige; el archivo no se
modifica. Requiere xmllint (libxml2): en macOS ya viene, en Debian/Ubuntu `apt install libxml2-utils`.

Los XSD oficiales importan el esquema de xmldsig con una ruta de la CDN de Hacienda (../../xmldsig-core-schema.xsd) que
no existe en este repositorio; el script trabaja sobre una copia temporal con la ruta corregida y no toca los originales.
"""
from __future__ import annotations

import argparse
import re
import shutil
import subprocess
import sys
import tempfile
import xml.etree.ElementTree as ET
from pathlib import Path

RAIZ = Path(__file__).resolve().parent.parent
XSD_DIR = RAIZ / "docs" / "hacienda_schemas" / "v4.4"

XSD_POR_RAIZ = {
    "FacturaElectronica": "FacturaElectronica_V4.4.xsd",
    "FacturaElectronicaExportacion": "FacturaElectronicaExportacion_V4.4.xsd",
    "FacturaElectronicaCompra": "FacturaElectronicaCompra_V4.4.xsd",
    "TiqueteElectronico": "TiqueteElectronico_V4.4.xsd",
    "NotaCreditoElectronica": "NotaCreditoElectronica_V4.4.xsd",
    "NotaDebitoElectronica": "NotaDebitoElectronica_V4.4.xsd",
    "ReciboElectronicoPago": "ReciboElectronicoPago_V4.4.xsd",
    "MensajeReceptor": "MensajeReceptor_V4.4.xsd",
    "MensajeHacienda": "MensajeHacienda_V4.4.xsd",
}

FIRMA_DE_RELLENO = (
    '<ds:Signature xmlns:ds="http://www.w3.org/2000/09/xmldsig#"><ds:SignedInfo>'
    '<ds:CanonicalizationMethod Algorithm="http://www.w3.org/TR/2001/REC-xml-c14n-20010315"/>'
    '<ds:SignatureMethod Algorithm="http://www.w3.org/2001/04/xmldsig-more#rsa-sha256"/>'
    '<ds:Reference URI=""><ds:DigestMethod Algorithm="http://www.w3.org/2001/04/xmlenc#sha256"/>'
    '<ds:DigestValue>AAAA</ds:DigestValue></ds:Reference></ds:SignedInfo>'
    '<ds:SignatureValue>AAAA</ds:SignatureValue></ds:Signature>'
)


def local(tag: str) -> str:
    return tag.rsplit("}", 1)[-1]


def preparar_xsd(destino: Path) -> None:
    """Copia los XSD y apunta el import de xmldsig al archivo que está al lado."""
    destino.mkdir(parents=True, exist_ok=True)
    for xsd in XSD_DIR.glob("*.xsd"):
        texto = xsd.read_text(encoding="utf-8")
        texto = re.sub(r'schemaLocation="[^"]*xmldsig-core-schema\.xsd"', 'schemaLocation="xmldsig-core-schema.xsd"', texto)
        (destino / xsd.name).write_text(texto, encoding="utf-8")


def validar(xml: Path, xsd_dir: Path, tmp: Path) -> tuple[bool, str]:
    datos = xml.read_bytes()
    try:
        raiz = ET.fromstring(datos)
    except ET.ParseError as e:
        return False, f"XML mal formado: {e}"
    nombre = local(raiz.tag)
    if nombre not in XSD_POR_RAIZ:
        return False, f"elemento raíz sin XSD conocido: {nombre}"

    nota = ""
    if not any(local(h.tag) == "Signature" for h in raiz):
        texto = datos.decode("utf-8")
        cierre = texto.rindex(f"</{nombre}>")
        datos = (texto[:cierre] + FIRMA_DE_RELLENO + texto[cierre:]).encode("utf-8")
        nota = " (sin firma: se validó con una firma de relleno)"
    copia = tmp / "validar.xml"
    copia.write_bytes(datos)

    r = subprocess.run(["xmllint", "--noout", "--schema", str(xsd_dir / XSD_POR_RAIZ[nombre]), str(copia)],
                       capture_output=True, text=True)
    if r.returncode == 0:
        return True, f"{nombre}{nota}"
    errores = [l.split("Schemas validity error : ", 1)[-1] for l in r.stderr.splitlines() if "validity error" in l or "parser error" in l]
    return False, f"{nombre}{nota}\n      " + "\n      ".join(e[:300] for e in (errores or [r.stderr.strip()]))


def main() -> int:
    p = argparse.ArgumentParser(description="Valida comprobantes XML contra los XSD oficiales de Hacienda.")
    p.add_argument("xml", nargs="+", type=Path)
    args = p.parse_args()
    if shutil.which("xmllint") is None:
        print("Error: no se encontró xmllint (en Debian/Ubuntu: apt install libxml2-utils).", file=sys.stderr)
        return 2

    fallos = 0
    with tempfile.TemporaryDirectory() as t:
        tmp = Path(t)
        xsd_dir = tmp / "xsd"
        preparar_xsd(xsd_dir)
        for xml in args.xml:
            ok, detalle = validar(xml, xsd_dir, tmp)
            print(f"{'OK    ' if ok else 'ERROR '}{xml}: {detalle}")
            fallos += not ok
    print(f"\n{len(args.xml) - fallos} de {len(args.xml)} válidos")
    return 1 if fallos else 0


if __name__ == "__main__":
    sys.exit(main())
