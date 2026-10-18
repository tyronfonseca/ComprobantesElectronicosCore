import React from "react";
import type { CodigoReferencia, InformacionReferencia, TipoDocumentoReferencia } from "@cec-cr/core";
import { fechaEmision } from "@cec-cr/core";
import { Card, CardHeader, CardTitle, CardDescription, CardContent } from "@/components/ui/card";
import { Button } from "@/components/ui/button";
import { Badge } from "@/components/ui/badge";
import { Link2, Plus, Trash2 } from "lucide-react";
import { Campo, CampoCatalogo } from "@/components/common/Campo";
import { useCatalogos, useErroresEn, useFacturacionStore, VACIO } from "@/store/comprobanteStore";

/** "2026-10-01T10:00:00-06:00" <-> valor de un input datetime-local. */
const aInput = (f?: string) => (f ?? "").slice(0, 16);
const deInput = (v: string) => (v ? `${v}:00-06:00` : "");

export function ReferenciasForm() {
  const c = useCatalogos();
  const referencias = useFacturacionStore((s) => s.request.informacionReferencia) ?? VACIO;
  const editar = useFacturacionStore((s) => s.editar);
  const errores = useErroresEn("informacionReferencia");

  const set = (i: number, fn: (r: InformacionReferencia) => void) =>
    editar((d) => {
      const lista = (d.informacionReferencia ?? []).map((x) => ({ ...x }));
      fn(lista[i]);
      d.informacionReferencia = lista;
    });

  return (
    <Card>
      <CardHeader className="pb-3">
        <div className="flex items-center justify-between gap-2">
          <div>
            <CardTitle className="text-base flex items-center gap-2">
              <Link2 className="w-4 h-4 text-blue-600" /> 6. Información de referencia
              {errores > 0 && <Badge variant="destructive">{errores} error(es)</Badge>}
            </CardTitle>
            <CardDescription>Notas 9 y 10 · obligatoria en NC, ND, FEC y REP</CardDescription>
          </div>
          <Button type="button" variant="outline" size="sm" className="text-xs h-8 gap-1" disabled={referencias.length >= 10}
            onClick={() => editar((d) => {
              d.informacionReferencia = [...(d.informacionReferencia ?? []),
                { tipoDocIR: "01", numero: "", fechaEmisionIR: fechaEmision(), codigo: "01", razon: "" }];
            })}>
            <Plus className="w-3.5 h-3.5" /> Agregar
          </Button>
        </div>
      </CardHeader>
      <CardContent className="space-y-3">
        {referencias.length === 0 && <p className="text-xs text-slate-400 italic">Sin referencias</p>}
        {referencias.map((ref, i) => {
          const r = `informacionReferencia[${i}]`;
          return (
            <div key={i} className="p-3 border border-slate-200 rounded-lg space-y-3">
              <div className="flex items-center justify-between">
                <span className="text-[11px] font-bold text-slate-600 uppercase">Referencia {i + 1}</span>
                <Button type="button" variant="ghost" size="icon" className="h-7 w-7 text-slate-400 hover:text-red-600"
                  onClick={() => editar((d) => { d.informacionReferencia = (d.informacionReferencia ?? []).filter((_, j) => j !== i); })}>
                  <Trash2 className="w-3.5 h-3.5" />
                </Button>
              </div>
              <div className="grid grid-cols-1 sm:grid-cols-2 gap-3">
                <CampoCatalogo label="Tipo de documento (Nota 10)" ruta={`${r}.tipoDocIR`} value={ref.tipoDocIR}
                  catalogo={c.tiposDocumentoReferencia} onChange={(v) => set(i, (x) => { x.tipoDocIR = v as TipoDocumentoReferencia; })} />
                <CampoCatalogo label="Código de referencia (Nota 9)" ruta={`${r}.codigo`} value={ref.codigo}
                  catalogo={c.codigosReferencia} placeholder="Sin código"
                  onChange={(v) => set(i, (x) => { x.codigo = (v || undefined) as CodigoReferencia | undefined; })} />
                {ref.tipoDocIR === "99" && (
                  <Campo label="Descripción del tipo de documento" ruta={`${r}.tipoDocRefOTRO`} value={ref.tipoDocRefOTRO}
                    onChange={(v) => set(i, (x) => { x.tipoDocRefOTRO = v; })} />
                )}
                {ref.codigo === "99" && (
                  <Campo label="Descripción del código" ruta={`${r}.codigoReferenciaOTRO`} value={ref.codigoReferenciaOTRO}
                    onChange={(v) => set(i, (x) => { x.codigoReferenciaOTRO = v; })} />
                )}
                <Campo className="sm:col-span-2 font-mono" label="Número o clave del documento" ruta={`${r}.numero`}
                  value={ref.numero} maxLength={50} onChange={(v) => set(i, (x) => { x.numero = v; })} />
                <Campo label="Fecha de emisión del documento" type="datetime-local" ruta={`${r}.fechaEmisionIR`}
                  value={aInput(ref.fechaEmisionIR)} onChange={(v) => set(i, (x) => { x.fechaEmisionIR = deInput(v); })} />
                <Campo label="Razón" ruta={`${r}.razon`} value={ref.razon}
                  onChange={(v) => set(i, (x) => { x.razon = v; })} />
              </div>
            </div>
          );
        })}
      </CardContent>
    </Card>
  );
}
