/**
 * Acceso de bajo nivel al módulo WebAssembly (cec.wasm, ABI de C v3).
 * El resto del binding usa estas funciones; no hace falta usarlas directamente.
 */

/** Versión del ABI de C que espera este binding. */
export const VERSION_ABI = 3;

/** Funciones exportadas por scripts/build_wasm.sh. Punteros de 32 bits (wasm32). */
export interface ModuloWasm {
    HEAPU8: Uint8Array;
    _malloc(bytes: number): number;
    _free(ptr: number): void;
    UTF8ToString(ptr: number, maxBytes?: number): string;
    getValue(ptr: number, tipo: "i8" | "i32" | "*"): number;

    _cec_abi_version(): number;
    _cec_build_json(json: number, len: number, outRes: number): number;
    _cec_calcular_json(json: number, len: number, outRes: number): number;
    _cec_sign_xml(xml: number, xmlLen: number, p12: number, p12Len: number, pin: number, outRes: number): number;
    _cec_leer_certificado(p12: number, p12Len: number, pin: number, outRes: number): number;
    /** Firma en dos pasos; ausentes en módulos compilados antes de agregarlas. */
    _cec_preparar_firma?(xml: number, xmlLen: number, certDer: number, certLen: number, outRes: number): number;
    _cec_completar_firma?(xml: number, xmlLen: number, signatureValue: number, outRes: number): number;
    /** Mensaje Receptor; ausentes en módulos compilados antes de agregarlas. */
    _cec_mensaje_receptor_json?(json: number, len: number, outRes: number): number;
    _cec_leer_comprobante_xml?(xml: number, xmlLen: number, outRes: number): number;
    _cec_result_data(res: number, outData: number, outLen: number): number;
    _cec_result_free(res: number): void;

    _cec_dta_total_provincias(): number;
    _cec_dta_provincia_por_indice(indice: number, outCodigo: number, outNombre: number, outTotal: number): number;
    _cec_dta_total_cantones(provincia: number): number;
    _cec_dta_canton_por_indice(provincia: number, indice: number, outCodigo: number, outNombre: number, outTotal: number): number;
    _cec_dta_total_distritos(provincia: number, canton: number): number;
    _cec_dta_distrito_por_indice(provincia: number, canton: number, indice: number, outCodigo: number, outNombre: number, outTotal: number): number;
    _cec_dta_total_barrios(provincia: number, canton: number, distrito: number): number;
    _cec_dta_barrio_por_indice(provincia: number, canton: number, distrito: number, indice: number, outCodigo: number, outNombre: number): number;
    _cec_dta_validar_ubicacion(provincia: number, canton: number, distrito: number): number;

    _cec_catalogo_total_monedas(): number;
    _cec_catalogo_moneda_por_indice(indice: number, outCodigo: number, outSimbolo: number, outDescripcion: number): number;
    _cec_catalogo_total_unidades_medida(): number;
    _cec_catalogo_unidad_medida_por_indice(indice: number, outCodigo: number, outSimbolo: number, outDescripcion: number): number;
    _cec_catalogo_total_formas_farmaceuticas(): number;
    _cec_catalogo_forma_farmaceutica_por_indice(indice: number, outCodigo: number, outDescripcion: number): number;
    _cec_catalogo_total_nombres(): number;
    _cec_catalogo_nombre_por_indice(indice: number): number;
    _cec_catalogo_total_elementos(nombre: number): number;
    _cec_catalogo_elemento_por_indice(nombre: number, indice: number, outCodigo: number, outDescripcion: number): number;
}

/** Factory que genera Emscripten (MODULARIZE): `createCecModule` de cec.js. */
export type FabricaModulo = (opciones?: Record<string, unknown>) => Promise<ModuloWasm>;

const ESTADOS: Record<number, string> = {
    1: "puntero nulo",
    2: "tamaño fuera de límite (máximo 10 MB)",
    3: "error de decodificación",
    4: "error interno del core",
    5: "índice fuera de rango",
};

const codificador = new TextEncoder();
const decodificador = new TextDecoder();

/**
 * Memoria del módulo con liberación agrupada: todo lo reservado en `usar` se libera al terminar.
 * HEAPU8 se lee en cada acceso porque cambia cuando la memoria crece.
 */
export class Memoria {
    readonly #m: ModuloWasm;
    readonly #reservas: number[] = [];

    private constructor(m: ModuloWasm) {
        this.#m = m;
    }

    static usar<T>(m: ModuloWasm, fn: (mem: Memoria) => T): T {
        const mem = new Memoria(m);
        try {
            return fn(mem);
        } finally {
            for (const ptr of mem.#reservas) m._free(ptr);
        }
    }

    reservar(bytes: number): number {
        const ptr = this.#m._malloc(Math.max(bytes, 1));
        if (ptr === 0) throw new Error("Sin memoria en el módulo WebAssembly.");
        this.#reservas.push(ptr);
        this.#m.HEAPU8.fill(0, ptr, ptr + Math.max(bytes, 1));
        return ptr;
    }

    /** Copia bytes al módulo; devuelve puntero y largo. */
    bytes(datos: Uint8Array): [number, number] {
        const ptr = this.reservar(datos.length);
        this.#m.HEAPU8.set(datos, ptr);
        return [ptr, datos.length];
    }

    /** Texto UTF-8 sin terminador (para funciones que reciben puntero + largo). */
    texto(valor: string): [number, number] {
        return this.bytes(codificador.encode(valor));
    }

    /** Texto UTF-8 terminado en cero. */
    cadena(valor: string): number {
        const datos = codificador.encode(valor);
        const ptr = this.reservar(datos.length + 1);
        this.#m.HEAPU8.set(datos, ptr);
        return ptr;
    }

    /** Espacio para un valor de salida de 32 bits (puntero, uint32 o uint8). */
    salida(): number {
        return this.reservar(4);
    }

    leerPuntero(ptr: number): number {
        return this.#m.getValue(ptr, "*") >>> 0;
    }

    leerUint32(ptr: number): number {
        return this.#m.getValue(ptr, "i32") >>> 0;
    }

    leerUint8(ptr: number): number {
        return this.#m.getValue(ptr, "i8") & 0xff;
    }

    /** Cadena terminada en cero a la que apunta el puntero guardado en `ptrSalida`. */
    leerCadena(ptrSalida: number): string {
        const ptr = this.leerPuntero(ptrSalida);
        return ptr === 0 ? "" : this.#m.UTF8ToString(ptr);
    }

    leerBytesUtf8(ptr: number, len: number): string {
        return len === 0 ? "" : decodificador.decode(this.#m.HEAPU8.subarray(ptr, ptr + len));
    }
}

/** Lanza si una función del ABI devolvió un estado distinto de CEC_STATUS_OK. */
export function verificarEstado(estado: number, funcion: string): void {
    if (estado !== 0) {
        throw new Error(`${funcion}: ${ESTADOS[estado] ?? `estado ${estado}`}.`);
    }
}

/**
 * Ejecuta una función del ABI que produce un cec_result, lee su JSON (cec_result_data) y lo libera.
 */
export function conResultado(
    m: ModuloWasm,
    mem: Memoria,
    funcion: string,
    llamar: (outRes: number) => number,
): Record<string, unknown> {
    const outRes = mem.salida();
    verificarEstado(llamar(outRes), funcion);
    const res = mem.leerPuntero(outRes);
    try {
        const outData = mem.salida();
        const outLen = mem.salida();
        verificarEstado(m._cec_result_data(res, outData, outLen), "cec_result_data");
        return JSON.parse(mem.leerBytesUtf8(mem.leerPuntero(outData), mem.leerUint32(outLen)));
    } finally {
        m._cec_result_free(res);
    }
}
