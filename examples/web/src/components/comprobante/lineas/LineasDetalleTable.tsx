import React from "react";
import type { LineaDetalle } from "@cec-cr/core";
import { Table, TableHeader, TableBody, TableRow, TableHead, TableCell } from "@/components/ui/table";
import { Button } from "@/components/ui/button";
import { Badge } from "@/components/ui/badge";
import { Trash2, Pencil, CornerDownRight } from "lucide-react";
import { useErroresEn, useFacturacionStore, VACIO } from "@/store/comprobanteStore";
import { descuentoLinea, formatoMonto, impuestosLinea, lineasSalida, type LineaSalida } from "@/lib/core";

interface Props {
  editando: number | null;
  onEditar: (indice: number) => void;
  onQuitar: (indice: number) => void;
}

function Fila({ linea, calculada, indice, editando, onEditar, onQuitar }: Props & { linea: LineaDetalle; calculada?: LineaSalida; indice: number }) {
  const errores = useErroresEn(`detalleServicio[${indice}]`);
  const { iva, otros, exonerado } = impuestosLinea(calculada);
  const surtido = linea.detalleSurtido ?? [];
  const surtidoCalculado = calculada?.DetalleSurtido?.LineaDetalleSurtido ?? [];
  const monto = (v: number | string | undefined) => (calculada ? formatoMonto(v) : "—");

  return (
    <>
      <TableRow className={editando === indice ? "bg-amber-50/40" : errores ? "bg-red-50/40" : ""}>
        <TableCell className="text-center font-semibold text-xs text-slate-600">{indice + 1}</TableCell>
        <TableCell className="font-mono text-xs">{linea.codigoCABYS ?? "—"}</TableCell>
        <TableCell className="text-xs">
          <div className="font-medium text-slate-900">{linea.detalle}</div>
          <div className="flex flex-wrap gap-1 mt-0.5">
            {errores > 0 && <Badge variant="destructive" className="text-[10px] py-0">{errores} error(es)</Badge>}
            {linea.exoneracion && <Badge variant="warning" className="text-[10px] py-0">Exonerada</Badge>}
            {(linea.numerosVINoSerie?.length ?? 0) > 0 && (
              <Badge variant="outline" className="text-[10px] py-0 font-mono">VIN: {linea.numerosVINoSerie!.join(", ")}</Badge>
            )}
            {linea.registroMedicamento && <Badge variant="outline" className="text-[10px] py-0">Medicamento</Badge>}
            {surtido.length > 0 && <Badge variant="secondary" className="text-[10px] py-0">Surtido ({surtido.length})</Badge>}
          </div>
        </TableCell>
        <TableCell className="text-right text-xs font-mono">{linea.cantidad ?? "1"} {linea.unidadMedida ?? ""}</TableCell>
        <TableCell className="text-right text-xs font-mono">{monto(calculada?.MontoTotal)}</TableCell>
        <TableCell className="text-right text-xs font-mono text-emerald-600">{monto(-descuentoLinea(calculada))}</TableCell>
        <TableCell className="text-right text-xs font-mono">{monto(otros)}</TableCell>
        <TableCell className="text-right text-xs font-mono">{monto(iva - exonerado)}</TableCell>
        <TableCell className="text-right text-xs font-mono font-bold">{monto(calculada?.MontoTotalLinea)}</TableCell>
        <TableCell className="text-center whitespace-nowrap">
          <Button size="sm" variant="ghost" onClick={() => onEditar(indice)} className="h-7 w-7 p-0 text-blue-600">
            <Pencil className="w-3.5 h-3.5" />
          </Button>
          <Button size="sm" variant="ghost" onClick={() => onQuitar(indice)} className="h-7 w-7 p-0 text-red-600">
            <Trash2 className="w-3.5 h-3.5" />
          </Button>
        </TableCell>
      </TableRow>
      {surtido.map((s, j) => (
        <TableRow key={j} className="bg-slate-50/70">
          <TableCell className="text-center font-mono text-[11px] text-slate-400">
            <span className="flex items-center justify-center gap-0.5"><CornerDownRight className="w-3 h-3" />{indice + 1}.{j + 1}</span>
          </TableCell>
          <TableCell className="font-mono text-xs text-slate-600">{s.codigoCABYS}</TableCell>
          <TableCell className="text-xs text-slate-700">{s.detalle}</TableCell>
          <TableCell className="text-right text-xs font-mono">{s.cantidad} {s.unidadMedida}</TableCell>
          <TableCell colSpan={4} />
          <TableCell className="text-right text-xs font-mono">{monto(surtidoCalculado[j]?.SubTotalSurtido)}</TableCell>
          <TableCell />
        </TableRow>
      ))}
    </>
  );
}

export function LineasDetalleTable(props: Props) {
  const lineas = useFacturacionStore((s) => s.request.detalleServicio) ?? VACIO;
  const comprobante = useFacturacionStore((s) => s.vistaPrevia?.comprobante);
  const calculadas = lineasSalida(comprobante);

  return (
    <div className="border border-slate-200 rounded-xl overflow-hidden bg-white shadow-sm">
      <Table>
        <TableHeader className="bg-slate-100/90">
          <TableRow>
            <TableHead className="w-10 text-center text-xs">#</TableHead>
            <TableHead className="w-28 text-xs">CABYS</TableHead>
            <TableHead className="text-xs">Detalle</TableHead>
            <TableHead className="w-24 text-right text-xs">Cantidad</TableHead>
            <TableHead className="w-28 text-right text-xs">Monto total</TableHead>
            <TableHead className="w-24 text-right text-xs">Descuento</TableHead>
            <TableHead className="w-24 text-right text-xs">Otros imp.</TableHead>
            <TableHead className="w-24 text-right text-xs">IVA</TableHead>
            <TableHead className="w-28 text-right text-xs">Total línea</TableHead>
            <TableHead className="w-20" />
          </TableRow>
        </TableHeader>
        <TableBody>
          {lineas.length === 0 ? (
            <TableRow>
              <TableCell colSpan={10} className="text-center py-8 text-xs text-slate-400 italic">Sin líneas de detalle</TableCell>
            </TableRow>
          ) : (
            lineas.map((l, i) => <Fila key={i} {...props} linea={l} calculada={calculadas[i]} indice={i} />)
          )}
        </TableBody>
      </Table>
      {lineas.length > 0 && calculadas.length === 0 && (
        <p className="px-4 py-2 text-[11px] text-slate-500 border-t border-slate-100">
          Los montos aparecen cuando el comprobante no tiene errores de formato en los decimales.
        </p>
      )}
    </div>
  );
}
