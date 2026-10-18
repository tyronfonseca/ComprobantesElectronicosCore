/**
 * Numeración del comprobante (Notas 3 y 4.1 del Anexo 1). Funciones puras: no requieren el módulo WebAssembly.
 */
import type { Situacion, TipoDocumento } from "./tipos.ts";

/** Tipos del consecutivo (Nota 4.1): los comprobantes y los Mensajes Receptor (05 aceptación, 06 parcial, 07 rechazo). */
export type TipoConsecutivo = TipoDocumento | "05" | "06" | "07";

export interface DatosConsecutivo {
    /** Casa matriz o sucursal: 1..999. */
    sucursal: number;
    /** Terminal o punto de venta: 1..99999. */
    terminal: number;
    tipoDocumento: TipoConsecutivo;
    /** Número del comprobante: 1..9999999999. */
    numero: number | bigint;
}

export interface DatosClave {
    /** RFC 3339; la fecha DDMMYY de la clave es la de este texto (ver fechaEmision). */
    fechaEmision: string;
    /**
     * Identificación del obligado tributario: el emisor, o el receptor en la Factura Electrónica de Compra.
     * Se rellena con ceros a la izquierda hasta 12 posiciones; admite letras para personas jurídicas.
     */
    identificacion: string;
    /** 20 dígitos (ver generarConsecutivo). */
    numeroConsecutivo: string;
    /** 1 normal (por defecto), 2 contingencia, 3 sin internet. */
    situacion?: Situacion;
    /** 8 dígitos generados por el emisor. */
    codigoSeguridad: string | number;
}

function entero(nombre: string, valor: number | bigint, maximo: bigint, digitos: number): string {
    const n = BigInt(valor);
    if (n < 1n || n > maximo) throw new RangeError(`${nombre} debe estar entre 1 y ${maximo}.`);
    return n.toString().padStart(digitos, "0");
}

/**
 * NumeroConsecutivo de 20 dígitos: sucursal (3) + terminal (5) + tipo de comprobante (2) + número (10).
 */
export function generarConsecutivo(d: DatosConsecutivo): string {
    return entero("La sucursal", d.sucursal, 999n, 3) +
        entero("La terminal", d.terminal, 99999n, 5) +
        d.tipoDocumento +
        entero("El número", d.numero, 9_999_999_999n, 10);
}

/**
 * Clave de 50 posiciones: 506 + fecha DDMMYY + identificación (12) + consecutivo (20) + situación (1)
 * + código de seguridad (8).
 */
export function generarClave(d: DatosClave): string {
    const fecha = /^(\d{2})(\d{2})-(\d{2})-(\d{2})T/.exec(d.fechaEmision);
    if (!fecha) throw new RangeError("fechaEmision debe tener formato RFC 3339 (YYYY-MM-DDThh:mm:ss±hh:mm).");
    const [, , anio, mes, dia] = fecha;

    if (!/^[0-9A-Za-z]{1,12}$/.test(d.identificacion)) {
        throw new RangeError("La identificación debe tener de 1 a 12 caracteres alfanuméricos, sin guiones.");
    }
    if (!/^\d{20}$/.test(d.numeroConsecutivo)) {
        throw new RangeError("El número consecutivo debe tener 20 dígitos.");
    }
    const seguridad = String(d.codigoSeguridad).padStart(8, "0");
    if (!/^\d{8}$/.test(seguridad)) throw new RangeError("El código de seguridad debe tener hasta 8 dígitos.");

    return "506" + dia + mes + anio + d.identificacion.padStart(12, "0") + d.numeroConsecutivo +
        (d.situacion ?? "1") + seguridad;
}

/**
 * Fecha de emisión RFC 3339 en hora de Costa Rica (UTC-06:00, sin horario de verano), sin milisegundos.
 */
export function fechaEmision(fecha: Date = new Date()): string {
    const cr = new Date(fecha.getTime() - 6 * 3600 * 1000);
    return cr.toISOString().slice(0, 19) + "-06:00";
}
