#!/usr/bin/env python3
"""
Envía un comprobante electrónico firmado al API de Recepción del Ministerio de Hacienda (Anexo 3)
y consulta su estado.

Uso:
    python3 scripts/enviar_comprobante.py comprobante_firmado.xml            # envía y espera la respuesta
    python3 scripts/enviar_comprobante.py comprobante_firmado.xml --simular  # muestra lo que enviaría
    python3 scripts/enviar_comprobante.py --consultar <clave>                # solo consulta el estado

Las credenciales se leen de un archivo .env (por defecto el de la raíz del repositorio; otro con --env):

    HACIENDA_USUARIO=cpj-3-101-000101@stag.comprobanteselectronicos.go.cr
    HACIENDA_CLAVE=contraseña generada en ATV
    HACIENDA_AMBIENTE=sandbox                 # sandbox (predeterminado) o produccion
    # Opcionales:
    HACIENDA_CALLBACK_URL=https://mi-servidor/callback
    HACIENDA_API_URL=...                      # reemplaza la URL del ambiente
    HACIENDA_TOKEN_URL=...
    HACIENDA_CLIENT_ID=...
    HACIENDA_CA_BUNDLE=/ruta/cacert.pem       # certificados raíz para HTTPS (ver contexto_ssl)

Solo usa la biblioteca estándar de Python 3.9 o superior.
"""
from __future__ import annotations

import argparse
import base64
import json
import os
import ssl
import sys
import time
import urllib.error
import urllib.parse
import urllib.request
import xml.etree.ElementTree as ET
from pathlib import Path

RAIZ = Path(__file__).resolve().parent.parent

AMBIENTES = {
    "sandbox": {
        "api": "https://api-sandbox.comprobanteselectronicos.go.cr/recepcion/v1",
        "token": "https://idp.comprobanteselectronicos.go.cr/auth/realms/rut-stag/protocol/openid-connect/token",
        "client_id": "api-stag",
    },
}

# Estados finales de la respuesta de Hacienda (ind-estado).
ESTADOS_FINALES = {"aceptado", "rechazado", "error"}


class ErrorEnvio(Exception):
    pass


# Contexto TLS de todas las peticiones; lo define contexto_ssl() al leer la configuración.
_SSL: ssl.SSLContext | None = None


# ── Configuración ────────────────────────────────────────────────────────────────────────────────

def leer_env(ruta: Path) -> dict[str, str]:
    """Lee un archivo .env (CLAVE=valor; admite comillas y comentarios). Las variables de entorno tienen prioridad."""
    valores: dict[str, str] = {}
    if ruta.exists():
        for linea in ruta.read_text(encoding="utf-8").splitlines():
            linea = linea.strip()
            if not linea or linea.startswith("#") or "=" not in linea:
                continue
            clave, valor = linea.split("=", 1)
            clave = clave.strip().removeprefix("export ").strip()
            valor = valor.strip()
            if len(valor) >= 2 and valor[0] == valor[-1] and valor[0] in "'\"":
                valor = valor[1:-1]
            elif " #" in valor:
                valor = valor.split(" #", 1)[0].rstrip()
            valores[clave] = valor
    for clave in list(valores) + [k for k in os.environ if k.startswith("HACIENDA_")]:
        if clave in os.environ:
            valores[clave] = os.environ[clave]
    return valores


def contexto_ssl(ca_bundle: str = "") -> ssl.SSLContext:
    """Contexto TLS con certificados raíz.

    El Python de python.org en macOS no trae certificados raíz hasta ejecutar "Install Certificates.command",
    y entonces toda conexión falla con CERTIFICATE_VERIFY_FAILED. Si el contexto predeterminado no tiene
    ninguno, se usa certifi (si está instalado) o el archivo del sistema. HACIENDA_CA_BUNDLE fuerza un archivo.
    """
    if ca_bundle:
        if not Path(ca_bundle).is_file():
            raise ErrorEnvio(f"HACIENDA_CA_BUNDLE no existe: {ca_bundle}")
        return ssl.create_default_context(cafile=ca_bundle)
    ctx = ssl.create_default_context()
    if ctx.cert_store_stats()["x509_ca"]:
        return ctx
    try:
        import certifi
        return ssl.create_default_context(cafile=certifi.where())
    except ImportError:
        pass
    for archivo in ("/etc/ssl/cert.pem", "/etc/ssl/certs/ca-certificates.crt", "/etc/pki/tls/certs/ca-bundle.crt"):
        if Path(archivo).is_file():
            return ssl.create_default_context(cafile=archivo)
    return ctx


def configuracion(env: dict[str, str], requiere_credenciales: bool) -> dict[str, str]:
    global _SSL
    _SSL = contexto_ssl(env.get("HACIENDA_CA_BUNDLE", ""))
    ambiente = env.get("HACIENDA_AMBIENTE", "sandbox").lower()
    if ambiente not in AMBIENTES:
        raise ErrorEnvio(f"HACIENDA_AMBIENTE debe ser 'sandbox' o 'produccion', no '{ambiente}'.")
    base = AMBIENTES[ambiente]
    conf = {
        "ambiente": ambiente,
        "api": env.get("HACIENDA_API_URL", base["api"]).rstrip("/"),
        "token": env.get("HACIENDA_TOKEN_URL", base["token"]),
        "client_id": env.get("HACIENDA_CLIENT_ID", base["client_id"]),
        "usuario": env.get("HACIENDA_USUARIO", ""),
        "clave": env.get("HACIENDA_CLAVE", ""),
        "callback": env.get("HACIENDA_CALLBACK_URL", ""),
    }
    if requiere_credenciales and (not conf["usuario"] or not conf["clave"]):
        raise ErrorEnvio("Faltan HACIENDA_USUARIO o HACIENDA_CLAVE en el .env.")
    return conf


# ── Comprobante ──────────────────────────────────────────────────────────────────────────────────

def _hijo(nodo: ET.Element, nombre: str) -> ET.Element | None:
    """Hijo directo por nombre local, sin importar el namespace del tipo de comprobante."""
    for h in nodo:
        if h.tag.rsplit("}", 1)[-1] == nombre:
            return h
    return None


def _identificacion(parte: ET.Element | None) -> dict[str, str] | None:
    ident = _hijo(parte, "Identificacion") if parte is not None else None
    if ident is None:
        return None
    tipo, numero = _hijo(ident, "Tipo"), _hijo(ident, "Numero")
    if tipo is None or numero is None:
        return None
    return {"tipoIdentificacion": (tipo.text or "").strip(), "numeroIdentificacion": (numero.text or "").strip()}


def datos_envio(xml: bytes, callback: str) -> dict:
    """Cuerpo del POST /recepcion a partir del XML firmado."""
    try:
        raiz = ET.fromstring(xml)
    except ET.ParseError as e:
        raise ErrorEnvio(f"El archivo no es un XML válido: {e}") from e

    tipo = raiz.tag.rsplit("}", 1)[-1]
    if not any(h.tag.rsplit("}", 1)[-1] == "Signature" for h in raiz):
        raise ErrorEnvio("El XML no está firmado (falta ds:Signature). Fírmelo antes de enviarlo.")

    clave, fecha = _hijo(raiz, "Clave"), _hijo(raiz, "FechaEmision")
    if clave is None or fecha is None:
        raise ErrorEnvio(f"El XML ({tipo}) no tiene Clave o FechaEmision.")

    emisor = _identificacion(_hijo(raiz, "Emisor"))
    receptor = _identificacion(_hijo(raiz, "Receptor"))
    # En la Factura Electrónica de Compra quien emite el comprobante es el receptor (el comprador);
    # su identificación es la de la clave y es quien lo envía.
    if tipo == "FacturaElectronicaCompra":
        emisor, receptor = receptor, emisor
    if emisor is None:
        raise ErrorEnvio("No se encontró la identificación de quien emite el comprobante.")

    datos = {
        "clave": clave.text.strip(),
        "fecha": fecha.text.strip(),
        "emisor": emisor,
        "comprobanteXml": base64.b64encode(xml).decode("ascii"),
    }
    if receptor is not None:
        datos["receptor"] = receptor
    if callback:
        datos["callbackUrl"] = callback
    return datos


# ── HTTP ─────────────────────────────────────────────────────────────────────────────────────────

def _peticion(metodo: str, url: str, *, datos: bytes | None = None, encabezados: dict[str, str] | None = None):
    # Cloudflare, delante del IdP y del API, rechaza el User-Agent predeterminado "Python-urllib" (error 1010).
    encabezados = {"User-Agent": "cec-enviar-comprobante/1.0", "Accept": "application/json", **(encabezados or {})}
    req = urllib.request.Request(url, data=datos, method=metodo, headers=encabezados)
    try:
        with urllib.request.urlopen(req, timeout=60, context=_SSL) as r:
            return r.status, dict(r.headers), r.read()
    except urllib.error.HTTPError as e:
        return e.code, dict(e.headers), e.read()
    except urllib.error.URLError as e:
        detalle = f"No se pudo conectar con {url}: {e.reason}"
        if isinstance(e.reason, ssl.SSLCertVerificationError):
            detalle += ("\nNo se pudo verificar el certificado del servidor. En macOS con el Python de python.org ejecute "
                        "\"Install Certificates.command\" (carpeta /Applications/Python 3.x) o defina HACIENDA_CA_BUNDLE.")
        raise ErrorEnvio(detalle) from e


def obtener_token(conf: dict[str, str]) -> dict:
    """Token OAuth2 'Resource Owner Password Credential' del IdP (Anexo 3). Vence en 5 minutos."""
    cuerpo = urllib.parse.urlencode({
        "grant_type": "password",
        "client_id": conf["client_id"],
        "username": conf["usuario"],
        "password": conf["clave"],
    }).encode()
    estado, _, respuesta = _peticion("POST", conf["token"], datos=cuerpo,
                                     encabezados={"Content-Type": "application/x-www-form-urlencoded"})
    if estado != 200:
        detalle = respuesta.decode("utf-8", "replace")
        raise ErrorEnvio(f"El IdP rechazó las credenciales (HTTP {estado}): {detalle}")
    return json.loads(respuesta)


def cerrar_sesion(conf: dict[str, str], token: dict) -> None:
    """Cierra la sesión en el IdP (el Anexo 3 pide cerrar la sesión al terminar)."""
    if "refresh_token" not in token or not conf["token"].endswith("/token"):
        return
    cuerpo = urllib.parse.urlencode({"client_id": conf["client_id"], "refresh_token": token["refresh_token"]}).encode()
    try:
        _peticion("POST", conf["token"][: -len("/token")] + "/logout", datos=cuerpo,
                  encabezados={"Content-Type": "application/x-www-form-urlencoded"})
    except ErrorEnvio:
        pass


def _autorizacion(token: dict) -> dict[str, str]:
    return {"Authorization": f"bearer {token['access_token']}", "Content-Type": "application/json"}


def _causa(encabezados: dict[str, str], cuerpo: bytes) -> str:
    """Hacienda informa el motivo de un rechazo en el encabezado X-Error-Cause."""
    causa = next((v for k, v in encabezados.items() if k.lower() == "x-error-cause"), "")
    texto = cuerpo.decode("utf-8", "replace").strip()
    return "; ".join(x for x in (causa, texto) if x) or "sin detalle"


def enviar(conf: dict[str, str], token: dict, datos: dict) -> None:
    estado, encabezados, cuerpo = _peticion("POST", f"{conf['api']}/recepcion",
                                            datos=json.dumps(datos).encode("utf-8"),
                                            encabezados=_autorizacion(token))
    if estado not in (200, 201, 202):
        raise ErrorEnvio(f"Hacienda rechazó el envío (HTTP {estado}): {_causa(encabezados, cuerpo)}")


def consultar(conf: dict[str, str], token: dict, clave: str) -> dict | None:
    """Estado del comprobante; None si Hacienda todavía no lo registra."""
    estado, encabezados, cuerpo = _peticion("GET", f"{conf['api']}/recepcion/{clave}", encabezados=_autorizacion(token))
    if estado == 404:
        return None
    if estado != 200:
        raise ErrorEnvio(f"No se pudo consultar el estado (HTTP {estado}): {_causa(encabezados, cuerpo)}")
    return json.loads(cuerpo)


def esperar_respuesta(conf: dict[str, str], token: dict, clave: str, intentos: int, espera: float) -> dict | None:
    for intento in range(1, intentos + 1):
        respuesta = consultar(conf, token, clave)
        estado = _estado(respuesta)
        print(f"  Consulta {intento}/{intentos}: {estado or 'aún no registrado'}")
        if estado in ESTADOS_FINALES:
            return respuesta
        if intento < intentos:
            time.sleep(espera)
    return respuesta


def _estado(respuesta: dict | None) -> str:
    if not respuesta:
        return ""
    # El GET devuelve "ind-estado"; el ejemplo de callback del Anexo 3 usa "indEstado".
    return str(respuesta.get("ind-estado") or respuesta.get("indEstado") or "").lower()


def guardar_respuesta(respuesta: dict, destino: Path) -> Path | None:
    """Guarda el MensajeHacienda (respuesta-xml, en Base64) y devuelve la ruta."""
    contenido = respuesta.get("respuesta-xml") or respuesta.get("respuestaXml")
    if not contenido:
        return None
    destino.write_bytes(base64.b64decode(contenido))
    return destino


def mostrar_respuesta(respuesta: dict | None, carpeta: Path) -> int:
    if not respuesta:
        print("Hacienda todavía no registra el comprobante; vuelva a consultar más tarde con --consultar.")
        return 2
    clave = respuesta.get("clave", "")
    estado = _estado(respuesta)
    print(f"Clave:  {clave}")
    print(f"Estado: {estado}")
    ruta = guardar_respuesta(respuesta, carpeta / f"{clave}_respuesta.xml")
    if ruta:
        print(f"Respuesta de Hacienda guardada en {ruta}")
        try:
            mensaje = _hijo(ET.fromstring(ruta.read_bytes()), "DetalleMensaje")
            if mensaje is not None and mensaje.text:
                print(f"Detalle: {mensaje.text.strip()}")
        except ET.ParseError:
            pass
    if estado == "aceptado":
        return 0
    return 1 if estado in ("rechazado", "error") else 2


# ── Programa ─────────────────────────────────────────────────────────────────────────────────────

def main() -> int:
    sys.stdout.reconfigure(line_buffering=True)   # el progreso y los errores salen en orden aunque se redirija
    p = argparse.ArgumentParser(description="Envía un comprobante firmado al API de Recepción de Hacienda.")
    p.add_argument("xml", nargs="?", type=Path, help="XML firmado (XAdES-EPES)")
    p.add_argument("--consultar", metavar="CLAVE", help="solo consulta el estado de un comprobante ya enviado")
    p.add_argument("--simular", action="store_true", help="muestra el cuerpo del envío sin conectarse")
    p.add_argument("--env", type=Path, default=RAIZ / ".env", help="archivo de credenciales (predeterminado: .env)")
    p.add_argument("--intentos", type=int, default=10, help="consultas de estado después de enviar (predeterminado: 10)")
    p.add_argument("--espera", type=float, default=3.0, help="segundos entre consultas (predeterminado: 3)")
    args = p.parse_args()

    if not args.xml and not args.consultar:
        p.error("indique el XML firmado o --consultar CLAVE")

    try:
        conf = configuracion(leer_env(args.env), requiere_credenciales=not args.simular)

        if args.consultar:
            token = obtener_token(conf)
            try:
                return mostrar_respuesta(consultar(conf, token, args.consultar), Path.cwd())
            finally:
                cerrar_sesion(conf, token)

        datos = datos_envio(args.xml.read_bytes(), conf["callback"])
        if args.simular:
            resumen = {**datos, "comprobanteXml": f"<{len(datos['comprobanteXml'])} caracteres en Base64>"}
            print(f"POST {conf['api']}/recepcion ({conf['ambiente']})")
            print(json.dumps(resumen, ensure_ascii=False, indent=2))
            return 0

        print(f"Ambiente: {conf['ambiente']} ({conf['api']})")
        token = obtener_token(conf)
        try:
            enviar(conf, token, datos)
            print(f"Comprobante {datos['clave']} recibido por Hacienda. Esperando la respuesta...")
            respuesta = esperar_respuesta(conf, token, datos["clave"], args.intentos, args.espera)
            return mostrar_respuesta(respuesta, args.xml.resolve().parent)
        finally:
            cerrar_sesion(conf, token)
    except ErrorEnvio as e:
        print(f"Error: {e}", file=sys.stderr)
        return 3
    except FileNotFoundError as e:
        print(f"Error: no existe {e.filename}", file=sys.stderr)
        return 3


if __name__ == "__main__":
    sys.exit(main())
