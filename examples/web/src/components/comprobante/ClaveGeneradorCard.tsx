import React from "react";
import { Card, CardHeader, CardTitle, CardContent } from "@/components/ui/card";
import { KeyRound } from "lucide-react";
import { useFacturacionStore } from "@/store/comprobanteStore";

export function ClaveGeneradorCard() {
  const clave = useFacturacionStore((s) => s.request.clave);
  const fecha = useFacturacionStore((s) => s.request.fechaEmision);

  return (
    <Card>
      <CardHeader className="pb-3">
        <CardTitle className="text-sm flex items-center gap-2">
          <KeyRound className="w-4 h-4 text-amber-500" /> Clave numérica
        </CardTitle>
      </CardHeader>
      <CardContent className="space-y-3">
        <div className="p-3 bg-slate-900 text-emerald-400 font-mono text-xs rounded-md break-all tracking-wider select-all">
          {clave || "Complete la identificación y la numeración para generar la clave."}
        </div>
        <p className="text-[11px] text-slate-500">
          506 · fecha {fecha.slice(0, 10)} · identificación (12) · consecutivo (20) · situación · código de seguridad (8).
          Al generar el XML la fecha de emisión se actualiza a la hora actual.
        </p>
      </CardContent>
    </Card>
  );
}
