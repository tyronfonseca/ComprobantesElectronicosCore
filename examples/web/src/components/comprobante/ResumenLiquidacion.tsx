import React from "react";
import type { ResumenFacturaSalida } from "@cec-cr/core";
import { Card, CardHeader, CardTitle, CardContent, CardFooter } from "@/components/ui/card";
import { Badge } from "@/components/ui/badge";
import { Button } from "@/components/ui/button";
import { Separator } from "@/components/ui/separator";
import { CheckCircle2, Code2, Cpu } from "lucide-react";
import { useFacturacionStore } from "@/store/comprobanteStore";
import { formatoMonto } from "@/lib/core";

/** Filas del ResumenFactura en el orden del Anexo; los totales condicionales en cero no vienen del core. */
const FILAS: [keyof ResumenFacturaSalida, string][] = [
  ["TotalServGravados", "Servicios gravados"],
  ["TotalServExentos", "Servicios exentos"],
  ["TotalServExonerado", "Servicios exonerados"],
  ["TotalServNoSujeto", "Servicios no sujetos"],
  ["TotalMercanciasGravadas", "Mercancías gravadas"],
  ["TotalMercanciasExentas", "Mercancías exentas"],
  ["TotalMercExonerada", "Mercancías exoneradas"],
  ["TotalMercNoSujeta", "Mercancías no sujetas"],
  ["TotalVenta", "Total venta"],
  ["TotalDescuentos", "Descuentos"],
  ["TotalVentaNeta", "Venta neta"],
  ["TotalImpuesto", "Impuestos"],
  ["TotalImpAsumEmisorFabrica", "Impuesto asumido por emisor o fábrica"],
  ["TotalIVADevuelto", "IVA devuelto"],
  ["TotalOtrosCargos", "Otros cargos"],
];

export function ResumenLiquidacion() {
  const resumen = useFacturacionStore((s) => s.vistaPrevia?.comprobante?.ResumenFactura);
  const moneda = useFacturacionStore((s) => s.request.resumenFactura.codigoTipoMoneda.codigoMoneda);
  const validar = useFacturacionStore((s) => s.validar);
  const generarXml = useFacturacionStore((s) => s.generarXml);

  return (
    <Card className="border-blue-200 bg-linear-to-br from-white to-blue-50/40 shadow-md">
      <CardHeader className="pb-3">
        <CardTitle className="text-base flex items-center justify-between">
          <span className="flex items-center gap-1.5">
            Resumen del comprobante
            <span className="inline-flex items-center gap-0.5 text-[10px] font-semibold text-blue-600 bg-blue-100/70 px-1.5 py-0.5 rounded border border-blue-200">
              <Cpu className="w-3 h-3" /> core C++
            </span>
          </span>
          <Badge variant="outline" className="font-mono bg-white">{moneda}</Badge>
        </CardTitle>
      </CardHeader>
      <CardContent className="space-y-2 text-sm">
        {!resumen && <p className="text-xs text-slate-500">Complete el detalle para ver los totales.</p>}
        {resumen && FILAS.filter(([k]) => resumen[k] !== undefined).map(([k, titulo]) => (
          <div key={k} className="flex justify-between text-slate-600">
            <span>{titulo}</span>
            <span className="font-mono">{formatoMonto(resumen[k] as string)}</span>
          </div>
        ))}
        {(resumen?.TotalDesgloseImpuesto ?? []).map((d, i) => (
          <div key={i} className="flex justify-between text-xs text-slate-500 pl-3 border-l-2 border-blue-200">
            <span>Impuesto {d.Codigo}{d.CodigoTarifaIVA ? ` · tarifa ${d.CodigoTarifaIVA}` : ""}</span>
            <span className="font-mono">{formatoMonto(d.TotalMontoImpuesto)}</span>
          </div>
        ))}
        <Separator />
        <div className="flex justify-between items-baseline pt-1">
          <span className="text-base font-bold text-slate-900">Total comprobante</span>
          <span className="text-2xl font-extrabold font-mono text-blue-700">{formatoMonto(resumen?.TotalComprobante)}</span>
        </div>
      </CardContent>
      <CardFooter className="grid grid-cols-2 gap-2 pt-2">
        <Button variant="outline" onClick={validar} className="w-full gap-1.5">
          <CheckCircle2 className="w-4 h-4 text-emerald-600" /> Validar
        </Button>
        <Button onClick={generarXml} className="w-full gap-1.5 bg-blue-600 hover:bg-blue-700">
          <Code2 className="w-4 h-4" /> Generar XML
        </Button>
      </CardFooter>
    </Card>
  );
}
