import React from "react";
import type { OtrosCargos, TipoIdentificacion, TipoOtrosCargos } from "@cec-cr/core";
import { Card, CardHeader, CardTitle, CardDescription, CardContent } from "@/components/ui/card";
import { Button } from "@/components/ui/button";
import { Badge } from "@/components/ui/badge";
import { Calculator, Plus, Receipt, Trash2 } from "lucide-react";
import { Campo, CampoCatalogo, decimalDeInput } from "@/components/common/Campo";
import { useCatalogos, useErroresEn, useFacturacionStore, VACIO } from "@/store/comprobanteStore";
import { formatoMonto } from "@/lib/core";

/** Cobros de terceros (04) y otros cargos que identifican al tercero. */
const CON_TERCERO = ["04"];

export function OtrosCargosForm() {
  const c = useCatalogos();
  const cargos = useFacturacionStore((s) => s.request.otrosCargos) ?? VACIO;
  const ventaNeta = useFacturacionStore((s) => Number(s.vistaPrevia?.comprobante?.ResumenFactura.TotalVentaNeta ?? 0));
  const total = useFacturacionStore((s) => s.vistaPrevia?.comprobante?.ResumenFactura.TotalOtrosCargos);
  const editar = useFacturacionStore((s) => s.editar);
  const errores = useErroresEn("otrosCargos");

  const set = (i: number, fn: (c: OtrosCargos) => void) =>
    editar((d) => {
      const lista = (d.otrosCargos ?? []).map((x) => ({ ...x }));
      fn(lista[i]);
      d.otrosCargos = lista;
    });

  return (
    <Card>
      <CardHeader className="pb-3">
        <div className="flex items-center justify-between gap-2">
          <div>
            <CardTitle className="text-base flex items-center gap-2">
              <Receipt className="w-4 h-4 text-blue-600" /> 5. Otros cargos
              {errores > 0 && <Badge variant="destructive">{errores} error(es)</Badge>}
            </CardTitle>
            <CardDescription>Nota 16 · Total: {formatoMonto(total)}</CardDescription>
          </div>
          <Button type="button" variant="outline" size="sm" className="text-xs h-8 gap-1" disabled={cargos.length >= 15}
            onClick={() => editar((d) => {
              d.otrosCargos = [...(d.otrosCargos ?? []), { tipoDocumentoOC: "06", detalle: "Impuesto de servicio 10%", porcentajeOC: "10", montoCargo: "" }];
            })}>
            <Plus className="w-3.5 h-3.5" /> Agregar
          </Button>
        </div>
      </CardHeader>
      <CardContent className="space-y-3">
        {cargos.length === 0 && <p className="text-xs text-slate-400 italic">Sin otros cargos</p>}
        {cargos.map((cargo, i) => {
          const r = `otrosCargos[${i}]`;
          return (
            <div key={i} className="p-3 border border-slate-200 rounded-lg space-y-3">
              <div className="flex items-center justify-between">
                <span className="text-[11px] font-bold text-slate-600 uppercase">Cargo {i + 1}</span>
                <Button type="button" variant="ghost" size="icon" className="h-7 w-7 text-slate-400 hover:text-red-600"
                  onClick={() => editar((d) => { d.otrosCargos = (d.otrosCargos ?? []).filter((_, j) => j !== i); })}>
                  <Trash2 className="w-3.5 h-3.5" />
                </Button>
              </div>
              <div className="grid grid-cols-1 sm:grid-cols-2 gap-3">
                <CampoCatalogo label="Tipo (Nota 16)" ruta={`${r}.tipoDocumentoOC`} value={cargo.tipoDocumentoOC} catalogo={c.tiposOtrosCargos}
                  onChange={(v) => set(i, (x) => { x.tipoDocumentoOC = v as TipoOtrosCargos; })} />
                <Campo label="Detalle" ruta={`${r}.detalle`} value={cargo.detalle}
                  onChange={(v) => set(i, (x) => { x.detalle = v; })} />
                {cargo.tipoDocumentoOC === "99" && (
                  <Campo className="sm:col-span-2" label="Descripción del tipo de cargo" ruta={`${r}.tipoDocumentoOTROS`}
                    value={cargo.tipoDocumentoOTROS} onChange={(v) => set(i, (x) => { x.tipoDocumentoOTROS = v; })} />
                )}
                {(CON_TERCERO.includes(cargo.tipoDocumentoOC) || cargo.identificacionTercero) && (
                  <>
                    <CampoCatalogo label="Tipo de identificación del tercero" ruta={`${r}.identificacionTercero.tipo`}
                      value={cargo.identificacionTercero?.tipo} catalogo={c.tiposIdentificacion} placeholder="Seleccione"
                      onChange={(v) => set(i, (x) => { x.identificacionTercero = { numero: x.identificacionTercero?.numero ?? "", tipo: v as TipoIdentificacion }; })} />
                    <Campo label="Identificación del tercero" ruta={`${r}.identificacionTercero.numero`}
                      value={cargo.identificacionTercero?.numero}
                      onChange={(v) => set(i, (x) => { x.identificacionTercero = { tipo: x.identificacionTercero?.tipo ?? "02", numero: v }; })} />
                    <Campo className="sm:col-span-2" label="Nombre del tercero" ruta={`${r}.nombreTercero`} value={cargo.nombreTercero}
                      onChange={(v) => set(i, (x) => { x.nombreTercero = v; })} />
                  </>
                )}
                <Campo label="Porcentaje (opcional)" ruta={`${r}.porcentajeOC`} value={cargo.porcentajeOC}
                  onChange={(v) => set(i, (x) => { x.porcentajeOC = decimalDeInput(v); })} />
                <div className="flex items-end gap-2">
                  <Campo className="flex-1" label="Monto del cargo" ruta={`${r}.montoCargo`} value={cargo.montoCargo}
                    onChange={(v) => set(i, (x) => { x.montoCargo = decimalDeInput(v); })} />
                  {cargo.porcentajeOC && ventaNeta > 0 && (
                    <Button type="button" variant="outline" size="icon" className="h-9 w-9 shrink-0"
                      title="Calcular con el porcentaje sobre la venta neta"
                      onClick={() => set(i, (x) => { x.montoCargo = (ventaNeta * Number(x.porcentajeOC) / 100).toFixed(5); })}>
                      <Calculator className="w-4 h-4" />
                    </Button>
                  )}
                </div>
              </div>
            </div>
          );
        })}
      </CardContent>
    </Card>
  );
}
