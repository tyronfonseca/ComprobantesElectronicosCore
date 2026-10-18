"""Numeración del comprobante (Notas 3 y 4.1 del Anexo 1) para los generadores de ejemplos.

Misma lógica que bindings/typescript/src/clave.ts. Solo usa la biblioteca estándar.
"""
from __future__ import annotations

import re
import secrets
from datetime import datetime, timedelta, timezone

CR = timezone(timedelta(hours=-6))  # Costa Rica no usa horario de verano


def _entero(nombre: str, valor: int, maximo: int, digitos: int) -> str:
    if not 1 <= valor <= maximo:
        raise ValueError(f"{nombre} debe estar entre 1 y {maximo}.")
    return str(valor).zfill(digitos)


def generar_consecutivo(sucursal: int, terminal: int, tipo_documento: str, numero: int) -> str:
    """NumeroConsecutivo de 20 dígitos: sucursal (3) + terminal (5) + tipo de comprobante (2) + número (10)."""
    return (_entero("La sucursal", sucursal, 999, 3) + _entero("La terminal", terminal, 99999, 5)
            + tipo_documento + _entero("El número", numero, 9_999_999_999, 10))


def generar_clave(fecha_emision: str, identificacion: str, numero_consecutivo: str,
                  codigo_seguridad: str, situacion: str = "1") -> str:
    """Clave de 50 posiciones: 506 + DDMMYY + identificación (12) + consecutivo (20) + situación (1) + seguridad (8)."""
    fecha = re.match(r"^\d{2}(\d{2})-(\d{2})-(\d{2})T", fecha_emision)
    if not fecha:
        raise ValueError("fecha_emision debe tener formato RFC 3339 (YYYY-MM-DDThh:mm:ss±hh:mm).")
    anio, mes, dia = fecha.groups()
    if not re.fullmatch(r"[0-9A-Za-z]{1,12}", identificacion):
        raise ValueError("La identificación debe tener de 1 a 12 caracteres alfanuméricos, sin guiones.")
    if not re.fullmatch(r"\d{20}", numero_consecutivo):
        raise ValueError("El número consecutivo debe tener 20 dígitos.")
    seguridad = str(codigo_seguridad).zfill(8)
    if not re.fullmatch(r"\d{8}", seguridad):
        raise ValueError("El código de seguridad debe tener hasta 8 dígitos.")
    return "506" + dia + mes + anio + identificacion.zfill(12) + numero_consecutivo + situacion + seguridad


def fecha_emision(fecha: datetime | None = None) -> str:
    """Fecha de emisión RFC 3339 en hora de Costa Rica, sin fracciones de segundo."""
    return (fecha or datetime.now(CR)).astimezone(CR).replace(microsecond=0).isoformat()


def codigo_seguridad() -> str:
    return str(secrets.randbelow(10**8)).zfill(8)
