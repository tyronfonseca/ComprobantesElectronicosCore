import { clsx, type ClassValue } from "clsx";
import { twMerge } from "tailwind-merge";

export function cn(...inputs: ClassValue[]): string {
  return twMerge(clsx(inputs));
}

/**
 * Formatea cantidad con exactamente 3 decimales según normativa de Hacienda v4.4
 */
export function formatCantidad(val: number): string {
  return (val || 0).toLocaleString("es-CR", {
    minimumFractionDigits: 3,
    maximumFractionDigits: 3
  });
}

/**
 * Formatea montos monetarios y subtotales con 5 decimales (o los decimales especificados)
 */
export function formatMonto(val: number, decimals: number = 5): string {
  return (val || 0).toLocaleString("es-CR", {
    minimumFractionDigits: decimals,
    maximumFractionDigits: decimals
  });
}

/**
 * Formatea monto con símbolo de moneda y decimales
 */
export function formatMoneda(val: number, moneda: string = "CRC", decimals: number = 2): string {
  const symbol = moneda === "CRC" ? "₡" : moneda;
  return `${symbol} ${formatMonto(val, decimals)}`;
}

