import { create } from "zustand";
import { immer } from "zustand/middleware/immer";
import {
  fechaEmision,
  generarClave,
  generarConsecutivo,
  type Certificado,
  type ComprobanteRequest,
  type Diagnostico,
  type ElementoCatalogo,
  type FacturaElectronica,
  type Moneda,
  type ResultadoCalculo,
  type Situacion,
  type TipoDocumento,
  type UnidadMedida,
} from "@cec-cr/core";
import { cargarCore, descargar } from "@/lib/core";

/** Instancia del core: fuera del estado de React porque no es serializable. */
let fe: FacturaElectronica | null = null;

export function core(): FacturaElectronica {
  if (!fe) throw new Error("El core todavía no está cargado.");
  return fe;
}

export interface CatalogosUI {
  tiposDocumento: ElementoCatalogo[];
  condicionesVenta: ElementoCatalogo[];
  mediosPago: ElementoCatalogo[];
  tiposIdentificacion: ElementoCatalogo[];
  tarifasIVA: ElementoCatalogo[];
  tiposImpuesto: ElementoCatalogo[];
  tiposDescuento: ElementoCatalogo[];
  tiposTransaccion: ElementoCatalogo[];
  tiposExoneracion: ElementoCatalogo[];
  institucionesExoneracion: ElementoCatalogo[];
  tiposOtrosCargos: ElementoCatalogo[];
  tiposDocumentoReferencia: ElementoCatalogo[];
  codigosReferencia: ElementoCatalogo[];
  tiposCodigoComercial: ElementoCatalogo[];
  ivaCobradoFabrica: ElementoCatalogo[];
  formasFarmaceuticas: ElementoCatalogo[];
  monedas: Moneda[];
  unidadesMedida: UnidadMedida[];
}

export interface Numeracion {
  sucursal: number;
  terminal: number;
  secuencia: number;
  codigoSeguridad: string;
  situacion: Situacion;
}

export interface Mensaje {
  tipo: "exito" | "error" | "info";
  texto: string;
}

interface FacturacionState {
  estado: "cargando" | "listo" | "error";
  errorCarga: string | null;
  catalogos: CatalogosUI | null;

  request: ComprobanteRequest;
  numeracion: Numeracion;
  /** Última vista previa del core (se recalcula en cada edición). */
  vistaPrevia: ResultadoCalculo | null;
  /** Los errores por campo se muestran después de validar o generar. */
  mostrarErrores: boolean;

  xml: string | null;
  xmlFirmado: boolean;
  certificado: Certificado | null;
  mensaje: Mensaje | null;

  inicializar: () => Promise<void>;
  editar: (fn: (draft: ComprobanteRequest) => void) => void;
  setNumeracion: (cambios: Partial<Numeracion>) => void;
  nuevo: (tipoDocumento?: TipoDocumento) => void;
  cargarJson: (texto: string) => Mensaje;
  exportarJson: () => string;
  descargarJson: () => void;
  validar: () => void;
  generarXml: () => void;
  leerCertificado: (p12: ArrayBuffer, pin: string) => { certificado?: Certificado; error?: string };
  firmar: (p12: ArrayBuffer, pin: string) => Mensaje;
}

const TIPOS_SOPORTADOS = ["01", "02", "03", "04", "08", "09", "10"];

function requestNuevo(tipoDocumento: TipoDocumento = "01"): ComprobanteRequest {
  return {
    tipoDocumento,
    clave: "",
    proveedorSistemas: "",
    numeroConsecutivo: "",
    fechaEmision: fechaEmision(),
    emisor: { nombre: "", identificacion: { tipo: "02", numero: "" }, correoElectronico: [""] },
    receptor: { nombre: "", identificacion: { tipo: "02", numero: "" } },
    condicionVenta: tipoDocumento === "10" ? "09" : "01",
    detalleServicio: [],
    resumenFactura: { codigoTipoMoneda: { codigoMoneda: "CRC" }, mediosPago: [{ tipoMedioPago: "01" }] },
  };
}

function codigoSeguridadAleatorio(): string {
  return String(Math.floor(Math.random() * 1e8)).padStart(8, "0");
}

/**
 * Quita textos vacíos, listas vacías y objetos sin datos para que el request lleve solo lo informado.
 * En listas de objetos no elimina elementos (las rutas de los diagnósticos usan su índice).
 */
export function limpiar<T>(valor: T): T {
  if (Array.isArray(valor)) {
    const lista = valor
      .filter((v) => !(typeof v === "string" && v === ""))
      .map((v) => (v && typeof v === "object" ? limpiar(v) ?? {} : v));
    return (lista.length ? lista : undefined) as T;
  }
  if (valor && typeof valor === "object") {
    const out: Record<string, unknown> = {};
    for (const [k, v] of Object.entries(valor)) {
      const limpio = limpiar(v);
      if (limpio !== undefined && limpio !== "") out[k] = limpio;
    }
    return (Object.keys(out).length ? out : undefined) as T;
  }
  return valor;
}

/** Consecutivo y clave derivados de la numeración (Notas 3 y 4.1); vacíos si faltan datos. */
function derivarNumeracion(req: ComprobanteRequest, n: Numeracion) {
  try {
    req.numeroConsecutivo = generarConsecutivo({
      sucursal: n.sucursal, terminal: n.terminal, tipoDocumento: req.tipoDocumento, numero: n.secuencia,
    });
  } catch {
    req.numeroConsecutivo = "";
  }
  // En la Factura Electrónica de Compra la clave lleva la identificación del receptor (quien la emite).
  const identificacion = req.tipoDocumento === "08"
    ? req.receptor?.identificacion?.numero
    : req.emisor.identificacion?.numero;
  try {
    req.clave = generarClave({
      fechaEmision: req.fechaEmision,
      identificacion: identificacion ?? "",
      numeroConsecutivo: req.numeroConsecutivo,
      situacion: n.situacion,
      codigoSeguridad: n.codigoSeguridad || "0",
    });
  } catch {
    req.clave = "";
  }
}

export const useFacturacionStore = create<FacturacionState>()(
  immer((set, get) => {
    /** Recalcula consecutivo, clave y vista previa después de cada cambio. */
    const actualizar = () =>
      set((s) => {
        derivarNumeracion(s.request, s.numeracion);
        if (fe) s.vistaPrevia = fe.calcular(limpiar(s.request) as ComprobanteRequest);
        s.xml = null;
        s.xmlFirmado = false;
      });

    return {
      estado: "cargando",
      errorCarga: null,
      catalogos: null,
      request: requestNuevo(),
      numeracion: { sucursal: 1, terminal: 1, secuencia: 1, codigoSeguridad: codigoSeguridadAleatorio(), situacion: "1" },
      vistaPrevia: null,
      mostrarErrores: false,
      xml: null,
      xmlFirmado: false,
      certificado: null,
      mensaje: null,

      inicializar: async () => {
        try {
          fe = await cargarCore();
          const c = fe.catalogos;
          const catalogos: CatalogosUI = {
            tiposDocumento: c.porNombre("TiposDocumento").filter((e) => TIPOS_SOPORTADOS.includes(e.codigo)),
            condicionesVenta: c.porNombre("CondicionesVenta"),
            mediosPago: c.porNombre("MediosPago"),
            tiposIdentificacion: c.porNombre("TiposIdentificacion"),
            tarifasIVA: c.porNombre("TarifasIVA"),
            tiposImpuesto: c.porNombre("TiposImpuesto"),
            tiposDescuento: c.porNombre("TiposDescuento"),
            tiposTransaccion: c.porNombre("TiposTransaccion"),
            tiposExoneracion: c.porNombre("TiposExoneracion"),
            institucionesExoneracion: c.porNombre("InstitucionesExoneracion"),
            tiposOtrosCargos: c.porNombre("TiposOtrosCargos"),
            tiposDocumentoReferencia: c.porNombre("TiposDocumentoReferencia"),
            codigosReferencia: c.porNombre("CodigosReferencia"),
            tiposCodigoComercial: c.porNombre("TiposCodigoComercial"),
            ivaCobradoFabrica: c.porNombre("IVACobradoFabrica"),
            formasFarmaceuticas: c.formasFarmaceuticas(),
            monedas: c.monedas(),
            unidadesMedida: c.unidadesMedida(),
          };
          set((s) => {
            s.catalogos = catalogos;
            s.estado = "listo";
          });
          actualizar();
        } catch (e) {
          set((s) => {
            s.estado = "error";
            s.errorCarga = e instanceof Error ? e.message : String(e);
          });
        }
      },

      editar: (fn) => {
        set((s) => { fn(s.request); });
        actualizar();
      },

      setNumeracion: (cambios) => {
        set((s) => { Object.assign(s.numeracion, cambios); });   // el producer de Immer no debe devolver nada
        actualizar();
      },

      nuevo: (tipoDocumento = "01") => {
        set((s) => {
          s.request = requestNuevo(tipoDocumento);
          s.numeracion.codigoSeguridad = codigoSeguridadAleatorio();
          s.mostrarErrores = false;
          s.mensaje = null;
          s.certificado = null;
        });
        actualizar();
      },

      cargarJson: (texto) => {
        let request: ComprobanteRequest;
        try {
          request = JSON.parse(texto);
        } catch (e) {
          return { tipo: "error", texto: `JSON inválido: ${e instanceof Error ? e.message : e}` };
        }
        if (!request || typeof request !== "object" || !TIPOS_SOPORTADOS.includes(request.tipoDocumento)) {
          return { tipo: "error", texto: "El JSON no es un request de comprobante (falta tipoDocumento válido)." };
        }
        // La numeración se toma del consecutivo y la clave del documento.
        const consecutivo = request.numeroConsecutivo ?? "";
        const clave = request.clave ?? "";
        set((s) => {
          // El request se usa tal cual: completarlo con valores por defecto agregaría datos que no trae
          // (por ejemplo, un receptor vacío en un tiquete).
          s.request = {
            ...request,
            emisor: request.emisor ?? requestNuevo().emisor,
            resumenFactura: request.resumenFactura ?? requestNuevo().resumenFactura,
          };
          if (/^\d{20}$/.test(consecutivo)) {
            s.numeracion.sucursal = Number(consecutivo.slice(0, 3));
            s.numeracion.terminal = Number(consecutivo.slice(3, 8));
            s.numeracion.secuencia = Number(consecutivo.slice(10));
          }
          if (clave.length === 50) {
            s.numeracion.situacion = (["1", "2", "3"].includes(clave[41]) ? clave[41] : "1") as Situacion;
            s.numeracion.codigoSeguridad = clave.slice(42);
          }
          s.mostrarErrores = true;
          s.certificado = null;
        });
        actualizar();
        const errores = get().vistaPrevia?.diagnosticos.filter((d) => d.severidad === "error").length ?? 0;
        const mensaje: Mensaje = errores === 0
          ? { tipo: "exito", texto: "Comprobante cargado y válido." }
          : { tipo: "info", texto: `Comprobante cargado con ${errores} error(es) de validación.` };
        set((s) => { s.mensaje = mensaje; });
        return mensaje;
      },

      exportarJson: () => JSON.stringify(limpiar(get().request), null, 2),

      descargarJson: () => {
        const { request } = get();
        descargar(`${request.clave || "comprobante"}.json`, get().exportarJson(), "application/json");
      },

      validar: () => {
        set((s) => {
          s.mostrarErrores = true;
          const errores = s.vistaPrevia?.diagnosticos.filter((d) => d.severidad === "error").length ?? 0;
          s.mensaje = errores === 0
            ? { tipo: "exito", texto: "El comprobante cumple las reglas del Anexo 1 v4.4." }
            : { tipo: "error", texto: `El comprobante tiene ${errores} error(es).` };
        });
      },

      generarXml: () => {
        // La fecha de emisión es la de generación (no puede ser futura ni anterior, salvo sin internet).
        set((s) => { s.request.fechaEmision = fechaEmision(); });
        actualizar();
        const resultado = core().procesar(limpiar(get().request) as ComprobanteRequest);
        set((s) => {
          s.mostrarErrores = true;
          s.vistaPrevia = { exito: resultado.exito, diagnosticos: resultado.diagnosticos, comprobante: resultado.comprobante };
          s.xml = resultado.xml ?? null;
          s.xmlFirmado = false;
          const errores = resultado.diagnosticos.filter((d) => d.severidad === "error").length;
          s.mensaje = resultado.exito
            ? { tipo: "exito", texto: "XML generado según el Anexo 1 v4.4." }
            : { tipo: "error", texto: `No se generó el XML: el comprobante tiene ${errores} error(es).` };
        });
      },

      leerCertificado: (p12, pin) => {
        const r = core().leerCertificado(p12, pin);
        set((s) => { s.certificado = r.certificado ?? null; });
        return r.exito ? { certificado: r.certificado } : { error: r.diagnosticos[0]?.mensaje ?? "No se pudo leer el certificado." };
      },

      firmar: (p12, pin) => {
        const { xml, xmlFirmado } = get();
        if (!xml || xmlFirmado) return { tipo: "error", texto: "Primero genere el XML sin firmar." };
        const r = core().firmar(xml, p12, pin);
        const mensaje: Mensaje = r.exito
          ? { tipo: "exito", texto: "Comprobante firmado con XAdES-EPES." }
          : { tipo: "error", texto: r.diagnosticos[0]?.mensaje ?? "No se pudo firmar." };
        set((s) => {
          if (r.exito) {
            s.xml = r.xml ?? null;
            s.xmlFirmado = true;
            s.certificado = r.certificado ?? s.certificado;
          }
          s.mensaje = mensaje;
        });
        return mensaje;
      },
    };
  })
);

// ── Selectores ───────────────────────────────────────────────────────────────────────────────────

const SIN_DIAGNOSTICOS: Diagnostico[] = [];

export function useDiagnosticos(): Diagnostico[] {
  return useFacturacionStore((s) => s.vistaPrevia?.diagnosticos ?? SIN_DIAGNOSTICOS);
}

/** Mensaje del primer diagnóstico con esa ruta exacta del request (solo después de validar). */
export function useError(ruta: string): string | undefined {
  return useFacturacionStore((s) =>
    s.mostrarErrores ? s.vistaPrevia?.diagnosticos.find((d) => d.ruta === ruta)?.mensaje : undefined
  );
}

/** Cantidad de errores bajo una ruta (ej. "detalleServicio[2]"), para marcar filas y secciones. */
export function useErroresEn(prefijo: string): number {
  return useFacturacionStore((s) =>
    s.mostrarErrores
      ? (s.vistaPrevia?.diagnosticos ?? []).filter(
          (d) => d.severidad === "error" && (d.ruta === prefijo || d.ruta.startsWith(prefijo + ".") || d.ruta.startsWith(prefijo + "["))
        ).length
      : 0
  );
}

export function useCatalogos(): CatalogosUI {
  const c = useFacturacionStore((s) => s.catalogos);
  if (!c) throw new Error("Catálogos no cargados.");
  return c;
}

/**
 * Lista vacía compartida para selectores (`useFacturacionStore(s => s.x) ?? VACIO`): un `[]` nuevo
 * dentro del selector cambia en cada llamada y provoca renders infinitos.
 */
export const VACIO: never[] = [];
