/**
 * Punto de entrada del binding: carga el core (WebAssembly) y procesa, firma y consulta catálogos.
 */
import { Catalogos, DivisionTerritorial } from "./catalogos.ts";
import type {
    Certificado,
    ComprobanteRequest,
    ComprobanteSalida,
    Diagnostico,
    ResultadoCalculo,
    ResultadoCertificado,
    ResultadoComprobante,
    ResultadoFirma,
    ResultadoFirmaPreparada,
    ComprobanteRecibido,
    MensajeReceptorRequest,
    ResultadoLecturaComprobante,
    ResultadoMensajeReceptor,
} from "./tipos.ts";
import { conResultado, Memoria, VERSION_ABI, type FabricaModulo, type ModuloWasm } from "./wasm.ts";

type Json = Record<string, any>;

function diagnosticos(json: Json): Diagnostico[] {
    return (json.diagnostics ?? []).map((d: Json) => ({
        severidad: d.severity === "error" ? "error" : "advertencia",
        codigo: d.code,
        ruta: d.path,
        mensaje: d.message,
    }));
}

function certificado(json: Json): Certificado | undefined {
    return json.certificado ? { ...json.certificado } : undefined;
}

function bytes(datos: Uint8Array | ArrayBuffer): Uint8Array {
    return datos instanceof Uint8Array ? datos : new Uint8Array(datos);
}

export class FacturaElectronica {
    readonly #m: ModuloWasm;

    /** Catálogos del Anexo 1 (los mismos que validan las reglas). */
    readonly catalogos: Catalogos;

    /** Provincias, cantones, distritos y barrios (Nota 14). */
    readonly territorio: DivisionTerritorial;

    private constructor(m: ModuloWasm) {
        this.#m = m;
        this.catalogos = new Catalogos(m);
        this.territorio = new DivisionTerritorial(m);
    }

    /**
     * Carga el core. Recibe la factory de Emscripten (`createCecModule` de cec.js) o un módulo ya creado.
     *
     * ```ts
     * import createCecModule from "./cec.js";
     * const fe = await FacturaElectronica.cargar(createCecModule);
     * ```
     *
     * En el navegador, `opciones.locateFile` indica dónde está cec.wasm.
     */
    static async cargar(fuente: FabricaModulo | ModuloWasm, opciones?: Record<string, unknown>): Promise<FacturaElectronica> {
        const m = typeof fuente === "function" ? await fuente(opciones) : fuente;
        const version = m._cec_abi_version();
        if (version !== VERSION_ABI) {
            throw new Error(`El módulo WebAssembly tiene el ABI ${version}; este binding requiere el ${VERSION_ABI}. Recompile cec.wasm.`);
        }
        return new FacturaElectronica(m);
    }

    /** Versión del ABI de C del módulo cargado. */
    get versionAbi(): number {
        return this.#m._cec_abi_version();
    }

    /**
     * Valida las reglas del Anexo 1, calcula los montos y genera el XML sin firmar.
     * Un comprobante inválido no lanza: viene con exito = false y sus diagnósticos.
     */
    procesar(request: ComprobanteRequest): ResultadoComprobante {
        const m = this.#m;
        const json = Memoria.usar(m, (mem) => {
            const [ptr, len] = mem.texto(JSON.stringify(request));
            return conResultado(m, mem, "cec_build_json", (out) => m._cec_build_json(ptr, len, out));
        });
        return {
            exito: json.status === "ok",
            diagnosticos: diagnosticos(json),
            xml: json.xml as string | undefined,
            comprobante: json.comprobante as ComprobanteSalida | undefined,
        };
    }

    /**
     * Vista previa para edición en vivo: calcula montos y totales aunque las reglas reporten errores
     * (por ejemplo, un emisor todavía incompleto). No genera XML; para emitir use procesar.
     */
    calcular(request: ComprobanteRequest): ResultadoCalculo {
        const m = this.#m;
        const json = Memoria.usar(m, (mem) => {
            const [ptr, len] = mem.texto(JSON.stringify(request));
            return conResultado(m, mem, "cec_calcular_json", (out) => m._cec_calcular_json(ptr, len, out));
        });
        return {
            exito: json.status === "ok",
            diagnosticos: diagnosticos(json),
            comprobante: json.comprobante as ComprobanteSalida | undefined,
        };
    }

    /** Firma un XML con XAdES-EPES usando un certificado PKCS#12 (.p12/.pfx). */
    firmar(xml: string, p12: Uint8Array | ArrayBuffer, pin: string): ResultadoFirma {
        const m = this.#m;
        const json = Memoria.usar(m, (mem) => {
            const [ptrXml, lenXml] = mem.texto(xml);
            const [ptrP12, lenP12] = mem.bytes(bytes(p12));
            const ptrPin = mem.cadena(pin);
            return conResultado(m, mem, "cec_sign_xml", (out) => m._cec_sign_xml(ptrXml, lenXml, ptrP12, lenP12, ptrPin, out));
        });
        return {
            exito: json.status === "ok",
            diagnosticos: diagnosticos(json),
            xml: json.xml as string | undefined,
            certificado: certificado(json),
        };
    }

    /**
     * Firma en dos pasos, para cuando la llave privada está fuera del navegador (keystore de un servidor, HSM).
     * Paso 1: arma la firma con el certificado público (X.509 DER) y devuelve `signedInfo`, que se firma con
     * RSA-SHA256 (SHA256withRSA) usando la llave del certificado.
     */
    prepararFirma(xml: string, certificadoDer: Uint8Array | ArrayBuffer): ResultadoFirmaPreparada {
        const m = this.#m;
        const fn = this.#requerir("_cec_preparar_firma");
        const json = Memoria.usar(m, (mem) => {
            const [ptrXml, lenXml] = mem.texto(xml);
            const [ptrCert, lenCert] = mem.bytes(bytes(certificadoDer));
            return conResultado(m, mem, "cec_preparar_firma", (out) => fn(ptrXml, lenXml, ptrCert, lenCert, out));
        });
        return {
            exito: json.status === "ok",
            diagnosticos: diagnosticos(json),
            xml: json.xml as string | undefined,
            signedInfo: json.signedInfo as string | undefined,
            certificado: certificado(json),
        };
    }

    /**
     * Paso 2: inserta el SignatureValue (Base64) en el XML preparado y verifica la firma con el certificado de
     * KeyInfo. Si no corresponde, devuelve el diagnóstico SIGN_VERIFY_FAILED.
     */
    completarFirma(xmlPreparado: string, signatureValueBase64: string): ResultadoFirma {
        const m = this.#m;
        const fn = this.#requerir("_cec_completar_firma");
        const json = Memoria.usar(m, (mem) => {
            const [ptrXml, lenXml] = mem.texto(xmlPreparado);
            const ptrFirma = mem.cadena(signatureValueBase64);
            return conResultado(m, mem, "cec_completar_firma", (out) => fn(ptrXml, lenXml, ptrFirma, out));
        });
        return {
            exito: json.status === "ok",
            diagnosticos: diagnosticos(json),
            xml: json.xml as string | undefined,
        };
    }

    /**
     * Mensaje Receptor v4.4: valida la confirmación o el rechazo de un comprobante recibido y genera el XML sin
     * firmar (se firma igual que un comprobante: firmar o prepararFirma/completarFirma).
     */
    mensajeReceptor(request: MensajeReceptorRequest): ResultadoMensajeReceptor {
        const m = this.#m;
        const fn = this.#requerir("_cec_mensaje_receptor_json");
        const json = Memoria.usar(m, (mem) => {
            const [ptr, len] = mem.texto(JSON.stringify(request));
            return conResultado(m, mem, "cec_mensaje_receptor_json", (out) => fn(ptr, len, out));
        });
        return {
            exito: json.status === "ok",
            diagnosticos: diagnosticos(json),
            xml: json.xml as string | undefined,
            plazoConfirmacion: json.plazoConfirmacion as string | undefined,
        };
    }

    /** Lee el XML de un comprobante recibido (el del proveedor) para confirmarlo o rechazarlo. */
    leerComprobante(xml: string): ResultadoLecturaComprobante {
        const m = this.#m;
        const fn = this.#requerir("_cec_leer_comprobante_xml");
        const json = Memoria.usar(m, (mem) => {
            const [ptr, len] = mem.texto(xml);
            return conResultado(m, mem, "cec_leer_comprobante_xml", (out) => fn(ptr, len, out));
        });
        return {
            exito: json.status === "ok",
            diagnosticos: diagnosticos(json),
            comprobante: json.comprobante as ComprobanteRecibido | undefined,
        };
    }

    #requerir<K extends "_cec_preparar_firma" | "_cec_completar_firma" | "_cec_mensaje_receptor_json" | "_cec_leer_comprobante_xml">(nombre: K): NonNullable<ModuloWasm[K]> {
        const fn = this.#m[nombre];
        if (typeof fn !== "function") {
            throw new Error(`El módulo WebAssembly no exporta ${nombre}. Recompile cec.wasm (make wasm).`);
        }
        return fn.bind(this.#m) as NonNullable<ModuloWasm[K]>;
    }

    /** Lee titular, emisor, vigencia y huella de un certificado PKCS#12 sin firmar nada. */
    leerCertificado(p12: Uint8Array | ArrayBuffer, pin: string): ResultadoCertificado {
        const m = this.#m;
        const json = Memoria.usar(m, (mem) => {
            const [ptrP12, lenP12] = mem.bytes(bytes(p12));
            const ptrPin = mem.cadena(pin);
            return conResultado(m, mem, "cec_leer_certificado", (out) => m._cec_leer_certificado(ptrP12, lenP12, ptrPin, out));
        });
        return {
            exito: json.status === "ok",
            diagnosticos: diagnosticos(json),
            certificado: certificado(json),
        };
    }
}
