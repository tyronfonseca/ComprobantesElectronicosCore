import React from "react";
import { UserRound } from "lucide-react";
import { ParteForm } from "./ParteForm";
import { useFacturacionStore } from "@/store/comprobanteStore";

export function ReceptorForm() {
  const receptor = useFacturacionStore((s) => s.request.receptor);
  const tipo = useFacturacionStore((s) => s.request.tipoDocumento);
  const editar = useFacturacionStore((s) => s.editar);

  const interruptor = (
    <label className="flex items-center gap-2 text-xs font-semibold text-slate-700 cursor-pointer select-none">
      <input
        type="checkbox"
        checked={Boolean(receptor)}
        onChange={(e) => editar((d) => {
          d.receptor = e.target.checked ? { nombre: "", identificacion: { tipo: "02", numero: "" } } : undefined;
        })}
        className="w-4 h-4 rounded text-blue-600 border-slate-300"
      />
      Incluir receptor
    </label>
  );

  if (!receptor) {
    return (
      <div className="flex items-center justify-between rounded-xl border border-dashed border-slate-300 bg-white px-5 py-4">
        <span className="text-sm font-semibold text-slate-600 flex items-center gap-2">
          <UserRound className="w-4 h-4 text-blue-600" /> 3. Receptor (sin receptor)
        </span>
        {interruptor}
      </div>
    );
  }

  return (
    <ParteForm
      ruta="receptor"
      titulo={<><UserRound className="w-4 h-4 text-blue-600" /> 3. Receptor</>}
      descripcion={tipo === "08"
        ? "En la Factura Electrónica de Compra el receptor es quien compra y emite el comprobante."
        : "Cliente al que se le emite el comprobante."}
      parte={receptor}
      onChange={(fn) => editar((d) => { if (d.receptor) fn(d.receptor); })}
      acciones={interruptor}
    />
  );
}
