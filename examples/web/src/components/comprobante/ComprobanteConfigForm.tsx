import React, { useState } from "react";
import type { CondicionVenta, MedioPago, TipoDocumento, TipoMedioPago } from "@cec-cr/core";
import { Card, CardHeader, CardTitle, CardDescription, CardContent } from "@/components/ui/card";
import { Badge } from "@/components/ui/badge";
import { Button } from "@/components/ui/button";
import { Label } from "@/components/ui/label";
import { FileText, FileCode, Plus, Trash2 } from "lucide-react";
import { Campo, CampoCatalogo, decimalDeInput } from "@/components/common/Campo";
import { FieldError } from "@/components/common/FieldError";
import { CargarFacturaModal } from "./CargarFacturaModal";
import { useCatalogos, useError, useFacturacionStore, VACIO } from "@/store/comprobanteStore";

const SITUACIONES = [
  { codigo: "1", descripcion: "Normal" },
  { codigo: "2", descripcion: "Contingencia" },
  { codigo: "3", descripcion: "Sin internet" },
];

function MediosPagoEditor() {
  const { mediosPago } = useCatalogos();
  const lista = useFacturacionStore((s) => s.request.resumenFactura.mediosPago) ?? VACIO;
  const editar = useFacturacionStore((s) => s.editar);
  const errorLista = useError("resumenFactura.mediosPago");
  const varios = lista.length > 1;

  const cambiar = (fn: (lista: MedioPago[]) => void) =>
    editar((d) => {
      const nueva = [...(d.resumenFactura.mediosPago ?? [])].map((m) => ({ ...m }));
      fn(nueva);
      d.resumenFactura.mediosPago = nueva;
    });

  return (
    <div className="space-y-2">
      <div className="flex items-center justify-between">
        <Label className="text-xs font-semibold text-slate-700">Medios de pago ({lista.length}/4)</Label>
        {lista.length < 4 && (
          <Button type="button" variant="ghost" size="sm" className="h-6 text-xs text-blue-600 px-2"
            onClick={() => cambiar((l) => l.push({ tipoMedioPago: "01" }))}>
            <Plus className="w-3 h-3 mr-1" /> Agregar
          </Button>
        )}
      </div>
      {lista.map((m, i) => (
        <div key={i} className="grid grid-cols-12 gap-2 items-end">
          <CampoCatalogo
            className={varios ? "col-span-5" : "col-span-11"}
            label=""
            ruta={`resumenFactura.mediosPago[${i}].tipoMedioPago`}
            value={m.tipoMedioPago}
            catalogo={mediosPago}
            onChange={(v) => cambiar((l) => { l[i].tipoMedioPago = v as TipoMedioPago; })}
          />
          {m.tipoMedioPago === "99" && (
            <Campo className="col-span-12 order-last" label="Detalle del medio de pago"
              ruta={`resumenFactura.mediosPago[${i}].medioPagoOtros`} value={m.medioPagoOtros}
              onChange={(v) => cambiar((l) => { l[i].medioPagoOtros = v; })} />
          )}
          {varios && (
            <Campo className="col-span-6" label="" placeholder="Monto"
              ruta={`resumenFactura.mediosPago[${i}].totalMedioPago`} value={m.totalMedioPago}
              onChange={(v) => cambiar((l) => { l[i].totalMedioPago = decimalDeInput(v); })} />
          )}
          {varios && (
            <Button type="button" variant="ghost" size="icon" className="col-span-1 h-9 w-9 text-slate-400 hover:text-red-600"
              onClick={() => cambiar((l) => l.splice(i, 1))}>
              <Trash2 className="w-4 h-4" />
            </Button>
          )}
        </div>
      ))}
      {varios && <p className="text-[11px] text-slate-500">Con dos o más medios, los montos deben sumar el total del comprobante.</p>}
      <FieldError error={errorLista} />
    </div>
  );
}

export function ComprobanteConfigForm() {
  const c = useCatalogos();
  const req = useFacturacionStore((s) => s.request);
  const numeracion = useFacturacionStore((s) => s.numeracion);
  const editar = useFacturacionStore((s) => s.editar);
  const setNumeracion = useFacturacionStore((s) => s.setNumeracion);
  const nuevo = useFacturacionStore((s) => s.nuevo);
  const [cargarAbierto, setCargarAbierto] = useState(false);

  const moneda = req.resumenFactura.codigoTipoMoneda.codigoMoneda;
  const esREP = req.tipoDocumento === "10";

  const entero = (v: string) => Number.parseInt(v, 10) || 0;

  return (
    <>
      <Card>
        <CardHeader className="pb-3">
          <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-3">
            <div>
              <CardTitle className="text-base flex items-center gap-2">
                <FileText className="w-4 h-4 text-blue-600" /> 1. Comprobante
              </CardTitle>
              <CardDescription>Tipo, condición de venta, moneda y numeración (Notas 3 y 4.1)</CardDescription>
            </div>
            <Button type="button" variant="outline" size="sm" onClick={() => setCargarAbierto(true)}
              className="bg-blue-50 text-blue-700 border-blue-200 hover:bg-blue-100 gap-1.5 font-semibold text-xs h-9 px-3">
              <FileCode className="w-4 h-4" /> Cargar JSON o ejemplo
            </Button>
          </div>
        </CardHeader>
        <CardContent className="space-y-4">
          <div className="grid grid-cols-1 sm:grid-cols-4 gap-4">
            <CampoCatalogo
              label="Tipo de comprobante"
              ruta="tipoDocumento"
              value={req.tipoDocumento}
              catalogo={c.tiposDocumento}
              onChange={(v) => nuevo(v as TipoDocumento)}
            />
            <Campo label="Proveedor de sistemas" ruta="proveedorSistemas" value={req.proveedorSistemas}
              placeholder="Identificación del proveedor"
              onChange={(v) => editar((d) => { d.proveedorSistemas = v; })} />
            <Campo label="Actividad económica del emisor" ruta="codigoActividadEmisor" value={req.codigoActividadEmisor}
              maxLength={6} placeholder="6 caracteres"
              onChange={(v) => editar((d) => { d.codigoActividadEmisor = v; })} />
            <Campo label="Actividad económica del receptor" ruta="codigoActividadReceptor" value={req.codigoActividadReceptor}
              maxLength={6} placeholder="6 caracteres"
              onChange={(v) => editar((d) => { d.codigoActividadReceptor = v; })} />
          </div>
          <p className="text-[11px] text-slate-500">Cambiar el tipo de comprobante empieza uno nuevo.</p>

          <div className="grid grid-cols-1 sm:grid-cols-4 gap-4 pt-2 border-t border-slate-100">
            <CampoCatalogo
              className="sm:col-span-2"
              label="Condición de venta"
              ruta="condicionVenta"
              value={req.condicionVenta}
              catalogo={c.condicionesVenta}
              onChange={(v) => editar((d) => {
                d.condicionVenta = v as CondicionVenta;
                if (v !== "02") delete d.plazoCredito;
                if (v !== "99") delete d.condicionVentaOtros;
              })}
            />
            {req.condicionVenta === "02" && (
              <Campo label="Plazo del crédito (días)" ruta="plazoCredito" type="number" min={1}
                value={req.plazoCredito}
                onChange={(v) => editar((d) => { d.plazoCredito = v ? entero(v) : undefined; })} />
            )}
            {req.condicionVenta === "99" && (
              <Campo className="sm:col-span-2" label="Detalle de la condición de venta" ruta="condicionVentaOtros"
                value={req.condicionVentaOtros}
                onChange={(v) => editar((d) => { d.condicionVentaOtros = v; })} />
            )}
          </div>

          <div className="grid grid-cols-1 sm:grid-cols-4 gap-4 pt-2 border-t border-slate-100">
            <CampoCatalogo
              label="Moneda"
              ruta="resumenFactura.codigoTipoMoneda.codigoMoneda"
              value={moneda}
              catalogo={c.monedas}
              onChange={(v) => editar((d) => {
                d.resumenFactura.codigoTipoMoneda = { codigoMoneda: v, tipoCambio: v === "CRC" ? undefined : d.resumenFactura.codigoTipoMoneda.tipoCambio };
              })}
            />
            {moneda !== "CRC" && (
              <Campo label="Tipo de cambio" ruta="resumenFactura.codigoTipoMoneda.tipoCambio"
                value={req.resumenFactura.codigoTipoMoneda.tipoCambio} placeholder="Ej. 512.37"
                onChange={(v) => editar((d) => { d.resumenFactura.codigoTipoMoneda.tipoCambio = decimalDeInput(v); })} />
            )}
            <div className="sm:col-span-2">
              <MediosPagoEditor />
            </div>
          </div>

          <div className="pt-2 border-t border-slate-100 space-y-3">
            <Badge variant="outline" className="font-mono text-xs bg-slate-50 border-slate-300">
              Consecutivo: {req.numeroConsecutivo || "—"}
            </Badge>
            <div className="grid grid-cols-1 sm:grid-cols-5 gap-4">
              <Campo label="Sucursal (1-999)" type="number" value={numeracion.sucursal}
                onChange={(v) => setNumeracion({ sucursal: entero(v) })} />
              <Campo label="Terminal (1-99999)" type="number" value={numeracion.terminal}
                onChange={(v) => setNumeracion({ terminal: entero(v) })} />
              <Campo label="Número" type="number" value={numeracion.secuencia}
                onChange={(v) => setNumeracion({ secuencia: entero(v) })} />
              <Campo label="Código de seguridad" ruta="clave" maxLength={8} value={numeracion.codigoSeguridad}
                onChange={(v) => setNumeracion({ codigoSeguridad: v.replace(/\D/g, "").slice(0, 8) })} />
              <CampoCatalogo label="Situación" value={numeracion.situacion} catalogo={SITUACIONES}
                onChange={(v) => setNumeracion({ situacion: v as "1" | "2" | "3" })} />
            </div>
            {esREP && (
              <p className="text-[11px] text-slate-500">
                Recibo Electrónico de Pago: el detalle registra el monto pagado de un comprobante a crédito o al Estado.
              </p>
            )}
          </div>
        </CardContent>
      </Card>
      <CargarFacturaModal open={cargarAbierto} onOpenChange={setCargarAbierto} />
    </>
  );
}
