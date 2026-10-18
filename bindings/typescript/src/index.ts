/**
 * Binding TypeScript del core de Facturación Electrónica de Costa Rica (comprobantes v4.4).
 */
export { FacturaElectronica } from "./core.ts";
export { Catalogos, DivisionTerritorial } from "./catalogos.ts";
export type { ElementoCatalogo, Moneda, NombreCatalogo, Territorio, UnidadMedida } from "./catalogos.ts";
export { fechaEmision, generarClave, generarConsecutivo } from "./clave.ts";
export type { DatosClave, DatosConsecutivo, TipoConsecutivo } from "./clave.ts";
export { VERSION_ABI } from "./wasm.ts";
export type { FabricaModulo, ModuloWasm } from "./wasm.ts";
export type * from "./tipos.ts";
