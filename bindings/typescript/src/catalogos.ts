/**
 * Catálogos del Anexo 1 y división territorial (Nota 14), leídos del core: son los mismos que validan las reglas.
 */
import { Memoria, type ModuloWasm } from "./wasm.ts";

export interface ElementoCatalogo {
    codigo: string;
    descripcion: string;
}

export interface Moneda extends ElementoCatalogo {
    simbolo: string;
}

export interface UnidadMedida extends ElementoCatalogo {
    simbolo: string;
}

/** Catálogos disponibles por nombre (Catalogos::GetPorNombre del core). */
export type NombreCatalogo =
    | "CondicionesVenta" | "MediosPago" | "TiposIdentificacion" | "TarifasIVA" | "TiposImpuesto"
    | "TiposDescuento" | "TiposTransaccion" | "TiposExoneracion" | "InstitucionesExoneracion"
    | "TiposOtrosCargos" | "TiposDocumentoReferencia" | "CodigosReferencia" | "TiposCodigoComercial"
    | "IVACobradoFabrica" | "TiposDocumento" | "FormasFarmaceuticas";

/** Recorre `total` elementos con una función del ABI que llena salidas por índice. */
function listar<T>(m: ModuloWasm, total: number, leer: (mem: Memoria, indice: number) => T | null): T[] {
    return Memoria.usar(m, (mem) => {
        const out: T[] = [];
        for (let i = 0; i < total; ++i) {
            const e = leer(mem, i);
            if (e !== null) out.push(e);
        }
        return out;
    });
}

export class Catalogos {
    readonly #m: ModuloWasm;
    readonly #cache = new Map<string, unknown>();

    constructor(m: ModuloWasm) {
        this.#m = m;
    }

    #memo<T>(clave: string, cargar: () => T): T {
        if (!this.#cache.has(clave)) this.#cache.set(clave, cargar());
        return this.#cache.get(clave) as T;
    }

    /** Nombres de los catálogos que se pueden pedir con porNombre. */
    nombres(): NombreCatalogo[] {
        return this.#memo("nombres", () => {
            const m = this.#m;
            const out: NombreCatalogo[] = [];
            for (let i = 0; i < m._cec_catalogo_total_nombres(); ++i) {
                out.push(m.UTF8ToString(m._cec_catalogo_nombre_por_indice(i)) as NombreCatalogo);
            }
            return out;
        });
    }

    porNombre(nombre: NombreCatalogo): ElementoCatalogo[] {
        return this.#memo(`catalogo:${nombre}`, () => Memoria.usar(this.#m, (mem) => {
            const m = this.#m;
            const ptrNombre = mem.cadena(nombre);
            const total = m._cec_catalogo_total_elementos(ptrNombre);
            if (total === 0) throw new RangeError(`Catálogo desconocido: ${nombre}.`);
            const out: ElementoCatalogo[] = [];
            const codigo = mem.salida();
            const descripcion = mem.salida();
            for (let i = 0; i < total; ++i) {
                if (m._cec_catalogo_elemento_por_indice(ptrNombre, i, codigo, descripcion)) {
                    out.push({ codigo: mem.leerCadena(codigo), descripcion: mem.leerCadena(descripcion) });
                }
            }
            return out;
        }));
    }

    /** Monedas ISO 4217. */
    monedas(): Moneda[] {
        return this.#memo("monedas", () => listar(this.#m, this.#m._cec_catalogo_total_monedas(), (mem, i) => {
            const [c, s, d] = [mem.salida(), mem.salida(), mem.salida()];
            if (!this.#m._cec_catalogo_moneda_por_indice(i, c, s, d)) return null;
            return { codigo: mem.leerCadena(c), simbolo: mem.leerCadena(s), descripcion: mem.leerCadena(d) };
        }));
    }

    /** Unidades de medida (UnidadMedida de la línea). */
    unidadesMedida(): UnidadMedida[] {
        return this.#memo("unidades", () => listar(this.#m, this.#m._cec_catalogo_total_unidades_medida(), (mem, i) => {
            const [c, s, d] = [mem.salida(), mem.salida(), mem.salida()];
            if (!this.#m._cec_catalogo_unidad_medida_por_indice(i, c, s, d)) return null;
            return { codigo: mem.leerCadena(c), simbolo: mem.leerCadena(s), descripcion: mem.leerCadena(d) };
        }));
    }

    /** Formas farmacéuticas (FormaFarmaceutica de la línea). */
    formasFarmaceuticas(): ElementoCatalogo[] {
        return this.#memo("formas", () => listar(this.#m, this.#m._cec_catalogo_total_formas_farmaceuticas(), (mem, i) => {
            const [c, d] = [mem.salida(), mem.salida()];
            if (!this.#m._cec_catalogo_forma_farmaceutica_por_indice(i, c, d)) return null;
            return { codigo: mem.leerCadena(c), descripcion: mem.leerCadena(d) };
        }));
    }
}

// ── División territorial ─────────────────────────────────────────────────────────────────────────

/** Elemento territorial con el código tal como va en Ubicacion del request (provincia "1", cantón "08"). */
export interface Territorio {
    codigo: string;
    nombre: string;
}

const dosDigitos = (n: number) => String(n).padStart(2, "0");

/** Código del request ("08") al número que usa el core; NaN si no es numérico. */
function numero(codigo: string): number {
    return /^\d{1,2}$/.test(codigo) ? Number(codigo) : Number.NaN;
}

export class DivisionTerritorial {
    readonly #m: ModuloWasm;

    constructor(m: ModuloWasm) {
        this.#m = m;
    }

    provincias(): Territorio[] {
        const m = this.#m;
        return listar(m, m._cec_dta_total_provincias(), (mem, i) => {
            const [c, n, t] = [mem.salida(), mem.salida(), mem.salida()];
            if (!m._cec_dta_provincia_por_indice(i, c, n, t)) return null;
            return { codigo: String(mem.leerUint8(c)), nombre: mem.leerCadena(n) };
        });
    }

    cantones(provincia: string): Territorio[] {
        const m = this.#m;
        const p = numero(provincia);
        if (Number.isNaN(p)) return [];
        return listar(m, m._cec_dta_total_cantones(p), (mem, i) => {
            const [c, n, t] = [mem.salida(), mem.salida(), mem.salida()];
            if (!m._cec_dta_canton_por_indice(p, i, c, n, t)) return null;
            return { codigo: dosDigitos(mem.leerUint8(c)), nombre: mem.leerCadena(n) };
        });
    }

    distritos(provincia: string, canton: string): Territorio[] {
        const m = this.#m;
        const [p, c] = [numero(provincia), numero(canton)];
        if (Number.isNaN(p) || Number.isNaN(c)) return [];
        return listar(m, m._cec_dta_total_distritos(p, c), (mem, i) => {
            const [cod, n, t] = [mem.salida(), mem.salida(), mem.salida()];
            if (!m._cec_dta_distrito_por_indice(p, c, i, cod, n, t)) return null;
            return { codigo: dosDigitos(mem.leerUint8(cod)), nombre: mem.leerCadena(n) };
        });
    }

    /** Barrios del distrito. En el request, Barrio es el nombre (texto), no el código. */
    barrios(provincia: string, canton: string, distrito: string): Territorio[] {
        const m = this.#m;
        const [p, c, d] = [numero(provincia), numero(canton), numero(distrito)];
        if ([p, c, d].some(Number.isNaN)) return [];
        return listar(m, m._cec_dta_total_barrios(p, c, d), (mem, i) => {
            const [cod, n] = [mem.salida(), mem.salida()];
            if (!m._cec_dta_barrio_por_indice(p, c, d, i, cod, n)) return null;
            return { codigo: dosDigitos(mem.leerUint8(cod)), nombre: mem.leerCadena(n) };
        });
    }

    /** true si provincia, cantón y distrito existen y son coherentes entre sí. */
    esUbicacionValida(provincia: string, canton: string, distrito: string): boolean {
        const [p, c, d] = [numero(provincia), numero(canton), numero(distrito)];
        if ([p, c, d].some(Number.isNaN)) return false;
        return this.#m._cec_dta_validar_ubicacion(p, c, d) === 1;
    }
}
