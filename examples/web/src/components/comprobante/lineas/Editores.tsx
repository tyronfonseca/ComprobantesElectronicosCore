import React from "react";
import type {
  CodigoComercial,
  CodigoDescuento,
  CodigoImpuesto,
  CodigoTarifaIVA,
  DatosImpuestoEspecifico,
  Descuento,
  Exoneracion,
  Impuesto,
  InstitucionExoneracion,
  LineaDetalleSurtido,
  TipoCodigoComercial,
  TipoDocumentoExoneracion,
} from "@cec-cr/core";
import { Button } from "@/components/ui/button";
import { Badge } from "@/components/ui/badge";
import { Plus, Trash2 } from "lucide-react";
import { Campo, CampoCatalogo, decimalDeInput } from "@/components/common/Campo";
import { useCatalogos, useFacturacionStore } from "@/store/comprobanteStore";

/*
 * Editores de las partes de una línea de detalle. Reciben la ruta del request para mostrar los
 * diagnósticos del core en cada campo y se reutilizan en los componentes de surtido.
 */

interface ListaProps<T> {
  ruta: string;
  valor: T[];
  onChange: (valor: T[]) => void;
}

/** Copia la lista, aplica el cambio y la devuelve. */
function cambiar<T>(lista: T[], fn: (l: T[]) => void): T[] {
  const copia = lista.map((x) => (typeof x === "object" ? { ...x } : x)) as T[];
  fn(copia);
  return copia;
}

function FilaAcciones({ titulo, onQuitar }: { titulo: string; onQuitar: () => void }) {
  return (
    <div className="flex items-center justify-between">
      <span className="text-[11px] font-bold text-slate-600 uppercase tracking-wide">{titulo}</span>
      <Button type="button" variant="ghost" size="icon" className="h-7 w-7 text-slate-400 hover:text-red-600" onClick={onQuitar}>
        <Trash2 className="w-3.5 h-3.5" />
      </Button>
    </div>
  );
}

function BotonAgregar({ texto, onClick, disabled }: { texto: string; onClick: () => void; disabled?: boolean }) {
  return (
    <Button type="button" variant="outline" size="sm" className="h-8 text-xs gap-1 bg-white" onClick={onClick} disabled={disabled}>
      <Plus className="w-3.5 h-3.5" /> {texto}
    </Button>
  );
}

// ── Impuestos ────────────────────────────────────────────────────────────────────────────────────

const ESPECIFICOS = ["03", "04", "05", "06"];

/** Impuestos de la línea o de un componente de surtido (Nota 8). */
export function EditorImpuestos({ ruta, valor, onChange, esSurtido = false }: ListaProps<Impuesto> & { esSurtido?: boolean }) {
  const { tiposImpuesto, tarifasIVA } = useCatalogos();
  const tipoDocumento = useFacturacionStore((s) => s.request.tipoDocumento);
  const admiteExportacion = ["09", "02", "03"].includes(tipoDocumento);

  return (
    <div className="space-y-3">
      {valor.map((imp, i) => {
        const r = `${ruta}[${i}]`;
        const set = (fn: (x: Impuesto) => void) => onChange(cambiar(valor, (l) => fn(l[i])));
        const esIVA = imp.codigo === "01" || imp.codigo === "07";
        const especifico = ESPECIFICOS.includes(imp.codigo);
        const datos = imp.datosImpuestoEspecifico ?? {};
        const setDatos = (campo: keyof DatosImpuestoEspecifico, v: string) =>
          set((x) => { x.datosImpuestoEspecifico = { ...datos, [campo]: decimalDeInput(v) }; });
        return (
          <div key={i} className="p-3 border border-slate-200 rounded-lg bg-slate-50/60 space-y-2">
            <FilaAcciones titulo={`Impuesto ${i + 1}`} onQuitar={() => onChange(valor.filter((_, j) => j !== i))} />
            <div className="grid grid-cols-1 sm:grid-cols-3 gap-3">
              <CampoCatalogo label="Código (Nota 8)" ruta={`${r}.codigo`} value={imp.codigo} catalogo={tiposImpuesto}
                onChange={(v) => set((x) => {
                  x.codigo = v as CodigoImpuesto;
                  if (!(v === "01" || v === "07")) delete x.codigoTarifaIVA;
                  else x.codigoTarifaIVA ??= "08";
                  if (!["02", "12", "99"].includes(v)) delete x.tarifa;
                  if (v !== "08") delete x.factorCalculoIVA;
                  if (v !== "99") delete x.codigoImpuestoOTRO;
                  if (!ESPECIFICOS.includes(v)) delete x.datosImpuestoEspecifico;
                })} />
              {esIVA && (
                <CampoCatalogo label="Tarifa del IVA (Nota 8.1)" ruta={`${r}.codigoTarifaIVA`} value={imp.codigoTarifaIVA}
                  catalogo={tarifasIVA} onChange={(v) => set((x) => { x.codigoTarifaIVA = v as CodigoTarifaIVA; })} />
              )}
              {["02", "12", "99"].includes(imp.codigo) && (
                <Campo label="Tarifa (%)" ruta={`${r}.tarifa`} value={imp.tarifa} placeholder="Ej. 10"
                  onChange={(v) => set((x) => { x.tarifa = decimalDeInput(v); })} />
              )}
              {imp.codigo === "08" && (
                <Campo label="Factor de cálculo del IVA" ruta={`${r}.factorCalculoIVA`} value={imp.factorCalculoIVA}
                  placeholder="Ej. 0.1300" onChange={(v) => set((x) => { x.factorCalculoIVA = decimalDeInput(v); })} />
              )}
              {imp.codigo === "99" && (
                <Campo label="Descripción del impuesto" ruta={`${r}.codigoImpuestoOTRO`} value={imp.codigoImpuestoOTRO}
                  onChange={(v) => set((x) => { x.codigoImpuestoOTRO = v; })} />
              )}
              {admiteExportacion && !esSurtido && (
                <Campo label="Monto de exportación (opcional)" ruta={`${r}.montoExportacion`} value={imp.montoExportacion}
                  onChange={(v) => set((x) => { x.montoExportacion = decimalDeInput(v); })} />
              )}
            </div>
            {especifico && (
              <div className="grid grid-cols-1 sm:grid-cols-5 gap-3 pt-2 border-t border-slate-200">
                <Campo label={imp.codigo === "03" ? "Litros" : imp.codigo === "06" ? "Unidades" : "Contenido (ml)"}
                  ruta={`${r}.datosImpuestoEspecifico.cantidadUnidadMedida`} value={datos.cantidadUnidadMedida}
                  onChange={(v) => setDatos("cantidadUnidadMedida", v)} />
                {imp.codigo === "04" && (
                  <>
                    <Campo label="% alcohol" ruta={`${r}.datosImpuestoEspecifico.porcentaje`} value={datos.porcentaje}
                      onChange={(v) => setDatos("porcentaje", v)} />
                    <Campo label="Proporción (ml alcohol)" ruta={`${r}.datosImpuestoEspecifico.proporcion`} value={datos.proporcion}
                      onChange={(v) => setDatos("proporcion", v)} />
                  </>
                )}
                {imp.codigo === "05" && (
                  <Campo label="Volumen de referencia" ruta={`${r}.datosImpuestoEspecifico.volumenUnidadConsumo`}
                    value={datos.volumenUnidadConsumo} onChange={(v) => setDatos("volumenUnidadConsumo", v)} />
                )}
                <Campo label="Impuesto por unidad" ruta={`${r}.datosImpuestoEspecifico.impuestoUnidad`} value={datos.impuestoUnidad}
                  onChange={(v) => setDatos("impuestoUnidad", v)} />
              </div>
            )}
          </div>
        );
      })}
      <BotonAgregar texto="Agregar impuesto" onClick={() => onChange([...valor, { codigo: "01", codigoTarifaIVA: "08" }])} />
    </div>
  );
}

// ── Descuentos ───────────────────────────────────────────────────────────────────────────────────

/** Hasta 5 descuentos (Nota 20). */
export function EditorDescuentos({ ruta, valor, onChange }: ListaProps<Descuento>) {
  const { tiposDescuento } = useCatalogos();
  return (
    <div className="space-y-3">
      {valor.map((d, i) => {
        const r = `${ruta}[${i}]`;
        const set = (fn: (x: Descuento) => void) => onChange(cambiar(valor, (l) => fn(l[i])));
        return (
          <div key={i} className="p-3 border border-slate-200 rounded-lg bg-slate-50/60 space-y-2">
            <FilaAcciones titulo={`Descuento ${i + 1}`} onQuitar={() => onChange(valor.filter((_, j) => j !== i))} />
            <div className="grid grid-cols-1 sm:grid-cols-2 gap-3">
              <Campo label="Monto" ruta={`${r}.montoDescuento`} value={d.montoDescuento}
                onChange={(v) => set((x) => { x.montoDescuento = decimalDeInput(v); })} />
              <CampoCatalogo label="Código (Nota 20)" ruta={`${r}.codigoDescuento`} value={d.codigoDescuento} catalogo={tiposDescuento}
                onChange={(v) => set((x) => {
                  x.codigoDescuento = v as CodigoDescuento;
                  if (v !== "99") { delete x.codigoDescuentoOTRO; delete x.naturalezaDescuento; }
                })} />
              {d.codigoDescuento === "99" && (
                <>
                  <Campo label="Descripción del descuento" ruta={`${r}.codigoDescuentoOTRO`} value={d.codigoDescuentoOTRO}
                    onChange={(v) => set((x) => { x.codigoDescuentoOTRO = v; })} />
                  <Campo label="Naturaleza del descuento" ruta={`${r}.naturalezaDescuento`} value={d.naturalezaDescuento}
                    onChange={(v) => set((x) => { x.naturalezaDescuento = v; })} />
                </>
              )}
            </div>
          </div>
        );
      })}
      <BotonAgregar texto="Agregar descuento" disabled={valor.length >= 5}
        onClick={() => onChange([...valor, { montoDescuento: "", codigoDescuento: "07" }])} />
    </div>
  );
}

// ── Códigos comerciales ──────────────────────────────────────────────────────────────────────────

/** Hasta 5 códigos comerciales (Nota 12). Con CABYS de surtido se exige el tipo 03 (fabricante). */
export function EditorCodigosComerciales({ ruta, valor, onChange }: ListaProps<CodigoComercial>) {
  const { tiposCodigoComercial } = useCatalogos();
  return (
    <div className="space-y-2">
      {valor.map((c, i) => {
        const r = `${ruta}[${i}]`;
        const set = (fn: (x: CodigoComercial) => void) => onChange(cambiar(valor, (l) => fn(l[i])));
        return (
          <div key={i} className="grid grid-cols-12 gap-2 items-end">
            <CampoCatalogo className="col-span-5" label={i === 0 ? "Tipo (Nota 12)" : ""} ruta={`${r}.tipo`} value={c.tipo}
              catalogo={tiposCodigoComercial} onChange={(v) => set((x) => { x.tipo = v as TipoCodigoComercial; })} />
            <Campo className="col-span-6" label={i === 0 ? "Código" : ""} ruta={`${r}.codigo`} value={c.codigo} maxLength={20}
              onChange={(v) => set((x) => { x.codigo = v; })} />
            <Button type="button" variant="ghost" size="icon" className="col-span-1 h-9 w-9 text-slate-400 hover:text-red-600"
              onClick={() => onChange(valor.filter((_, j) => j !== i))}>
              <Trash2 className="w-4 h-4" />
            </Button>
          </div>
        );
      })}
      <BotonAgregar texto="Agregar código" disabled={valor.length >= 5}
        onClick={() => onChange([...valor, { tipo: "01", codigo: "" }])} />
    </div>
  );
}

// ── Exoneración ──────────────────────────────────────────────────────────────────────────────────

const REQUIERE_ARTICULO = ["02", "03", "06", "07", "08"];

/** Exoneración del IVA de la línea (Nota 10.1 y 23). */
export function EditorExoneracion({ ruta, valor, onChange }: { ruta: string; valor?: Exoneracion; onChange: (v?: Exoneracion) => void }) {
  const { tiposExoneracion, institucionesExoneracion } = useCatalogos();
  if (!valor) {
    return (
      <BotonAgregar texto="Aplicar exoneración"
        onClick={() => onChange({ tipoDocumentoEX1: "04", numeroDocumento: "", nombreInstitucion: "01", fechaEmisionEX: "", tarifaExonerada: "13" })} />
    );
  }
  const set = (cambios: Partial<Exoneracion>) => onChange({ ...valor, ...cambios });
  const entero = (v: string) => (v ? Number.parseInt(v, 10) : undefined);
  return (
    <div className="p-3 border border-amber-200 rounded-lg bg-amber-50/60 space-y-3">
      <FilaAcciones titulo="Exoneración" onQuitar={() => onChange(undefined)} />
      <div className="grid grid-cols-1 sm:grid-cols-2 gap-3">
        <CampoCatalogo label="Tipo de documento (Nota 10.1)" ruta={`${ruta}.tipoDocumentoEX1`} value={valor.tipoDocumentoEX1}
          catalogo={tiposExoneracion} onChange={(v) => set({ tipoDocumentoEX1: v as TipoDocumentoExoneracion })} />
        <Campo label="Número de documento" ruta={`${ruta}.numeroDocumento`} value={valor.numeroDocumento}
          onChange={(v) => set({ numeroDocumento: v })} />
        {valor.tipoDocumentoEX1 === "99" && (
          <Campo className="sm:col-span-2" label="Descripción del tipo de documento" ruta={`${ruta}.tipoDocumentoOTRO`}
            value={valor.tipoDocumentoOTRO} onChange={(v) => set({ tipoDocumentoOTRO: v })} />
        )}
        {REQUIERE_ARTICULO.includes(valor.tipoDocumentoEX1) && (
          <>
            <Campo label="Artículo" type="number" ruta={`${ruta}.articulo`} value={valor.articulo}
              onChange={(v) => set({ articulo: entero(v) })} />
            <Campo label="Inciso (opcional)" type="number" ruta={`${ruta}.inciso`} value={valor.inciso}
              onChange={(v) => set({ inciso: entero(v) })} />
          </>
        )}
        <CampoCatalogo label="Institución (Nota 23)" ruta={`${ruta}.nombreInstitucion`} value={valor.nombreInstitucion}
          catalogo={institucionesExoneracion} onChange={(v) => set({ nombreInstitucion: v as InstitucionExoneracion })} />
        {valor.nombreInstitucion === "99" && (
          <Campo label="Nombre de la institución" ruta={`${ruta}.nombreInstitucionOtros`} value={valor.nombreInstitucionOtros}
            onChange={(v) => set({ nombreInstitucionOtros: v })} />
        )}
        <Campo label="Fecha de emisión del documento" type="datetime-local" ruta={`${ruta}.fechaEmisionEX`}
          value={valor.fechaEmisionEX.slice(0, 16)}
          onChange={(v) => set({ fechaEmisionEX: v ? `${v}:00-06:00` : "" })} />
        <Campo label="Tarifa exonerada (%)" ruta={`${ruta}.tarifaExonerada`} value={valor.tarifaExonerada}
          onChange={(v) => set({ tarifaExonerada: decimalDeInput(v) })} />
      </div>
    </div>
  );
}

// ── Surtido ──────────────────────────────────────────────────────────────────────────────────────

/** Componentes de un surtido, combo o paquete (hasta 20). */
export function EditorSurtido({ ruta, valor, onChange }: ListaProps<LineaDetalleSurtido>) {
  const { unidadesMedida } = useCatalogos();
  return (
    <div className="space-y-3">
      {valor.map((comp, i) => {
        const r = `${ruta}[${i}]`;
        const set = (fn: (x: LineaDetalleSurtido) => void) => onChange(cambiar(valor, (l) => fn(l[i])));
        return (
          <details key={i} className="p-3 border border-purple-200 rounded-lg bg-purple-50/40" open={i === valor.length - 1}>
            <summary className="cursor-pointer text-xs font-semibold text-slate-700 flex items-center gap-2">
              Componente {i + 1}: {comp.detalle || "sin detalle"}
              <Badge variant="outline" className="text-[10px] font-mono">{comp.codigoCABYS || "sin CABYS"}</Badge>
            </summary>
            <div className="space-y-3 mt-3">
              <FilaAcciones titulo="Datos del componente" onQuitar={() => onChange(valor.filter((_, j) => j !== i))} />
              <div className="grid grid-cols-1 sm:grid-cols-4 gap-3">
                <Campo label="CABYS" ruta={`${r}.codigoCABYS`} value={comp.codigoCABYS} maxLength={13}
                  onChange={(v) => set((x) => { x.codigoCABYS = v; })} />
                <Campo label="Cantidad" ruta={`${r}.cantidad`} value={comp.cantidad}
                  onChange={(v) => set((x) => { x.cantidad = decimalDeInput(v); })} />
                <CampoCatalogo label="Unidad" ruta={`${r}.unidadMedida`} value={comp.unidadMedida} catalogo={unidadesMedida}
                  onChange={(v) => set((x) => { x.unidadMedida = v; })} />
                <Campo label="Precio unitario" ruta={`${r}.precioUnitario`} value={comp.precioUnitario}
                  onChange={(v) => set((x) => { x.precioUnitario = decimalDeInput(v); })} />
                <Campo className="sm:col-span-4" label="Detalle" ruta={`${r}.detalle`} value={comp.detalle}
                  onChange={(v) => set((x) => { x.detalle = v; })} />
              </div>
              <EditorCodigosComerciales ruta={`${r}.codigosComerciales`} valor={comp.codigosComerciales ?? []}
                onChange={(v) => set((x) => { x.codigosComerciales = v; })} />
              <EditorDescuentos ruta={`${r}.descuentos`} valor={comp.descuentos ?? []}
                onChange={(v) => set((x) => { x.descuentos = v; })} />
              <EditorImpuestos esSurtido ruta={`${r}.impuestos`} valor={comp.impuestos}
                onChange={(v) => set((x) => { x.impuestos = v; })} />
            </div>
          </details>
        );
      })}
      <BotonAgregar texto="Agregar componente" disabled={valor.length >= 20}
        onClick={() => onChange([...valor, {
          codigoCABYS: "", cantidad: "1", unidadMedida: "Unid", detalle: "", precioUnitario: "",
          impuestos: [{ codigo: "01", codigoTarifaIVA: "08" }],
        }])} />
    </div>
  );
}
