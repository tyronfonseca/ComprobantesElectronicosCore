import { FacturaElectronica, type ComprobanteSalida, type FabricaModulo } from "@cec-cr/core";

/**
 * Carga el core (public/cec.js + cec.wasm, generados por scripts/build_wasm.sh).
 * cec.js define la factory global createCecModule.
 */
export async function cargarCore(): Promise<FacturaElectronica> {
  const global = globalThis as { createCecModule?: FabricaModulo };
  if (!global.createCecModule) {
    await new Promise<void>((resolve, reject) => {
      const script = document.createElement("script");
      script.src = "/cec.js";
      script.onload = () => resolve();
      script.onerror = () => reject(new Error("No se pudo cargar /cec.js. Ejecute scripts/build_wasm.sh."));
      document.head.appendChild(script);
    });
  }
  if (!global.createCecModule) {
    throw new Error("cec.js no definió createCecModule.");
  }
  return FacturaElectronica.cargar(global.createCecModule, {
    locateFile: (archivo: string) => `/${archivo}`,
  });
}

/** Descarga un texto como archivo. */
export function descargar(nombre: string, contenido: string, tipo: string) {
  const url = URL.createObjectURL(new Blob([contenido], { type: `${tipo};charset=utf-8` }));
  const a = document.createElement("a");
  a.href = url;
  a.download = nombre;
  document.body.appendChild(a);
  a.click();
  a.remove();
  URL.revokeObjectURL(url);
}

/** Monto decimal del core ("1500.00000") para mostrar. */
export function formatoMonto(valor: string | number | undefined): string {
  const n = (typeof valor === "number" ? valor : Number(valor ?? 0)) || 0; // || 0 evita "-0,00"
  return (Number.isFinite(n) ? n : 0).toLocaleString("es-CR", { minimumFractionDigits: 2, maximumFractionDigits: 2 });
}

/** LineaDetalle calculada (estructura del XML v4.4, decimales en texto). */
export interface LineaSalida {
  MontoTotal: string;
  SubTotal: string;
  BaseImponible?: string;
  Descuento?: { MontoDescuento: string }[];
  Impuesto?: { Codigo: string; Monto: string; Exoneracion?: { MontoExoneracion: string } }[];
  ImpuestoNeto?: string;
  MontoTotalLinea: string;
  DetalleSurtido?: { LineaDetalleSurtido: { SubTotalSurtido: string }[] };
}

/** Líneas calculadas del comprobante, en el mismo orden que detalleServicio del request. */
export function lineasSalida(comprobante?: ComprobanteSalida): LineaSalida[] {
  const detalle = comprobante?.DetalleServicio as { LineaDetalle?: LineaSalida[] } | undefined;
  return detalle?.LineaDetalle ?? [];
}

/** Suma de descuentos de una línea calculada. */
export function descuentoLinea(l?: LineaSalida): number {
  return (l?.Descuento ?? []).reduce((t, d) => t + Number(d.MontoDescuento), 0);
}

/** Suma de impuestos de una línea calculada, separando el IVA (01, 07, 08) de los demás. */
export function impuestosLinea(l?: LineaSalida): { iva: number; otros: number; exonerado: number } {
  let iva = 0, otros = 0, exonerado = 0;
  for (const i of l?.Impuesto ?? []) {
    if (["01", "07", "08"].includes(i.Codigo)) iva += Number(i.Monto);
    else otros += Number(i.Monto);
    exonerado += Number(i.Exoneracion?.MontoExoneracion ?? 0);
  }
  return { iva, otros, exonerado };
}
