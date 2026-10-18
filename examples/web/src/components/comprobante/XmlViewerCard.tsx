import React, { useState } from "react";
import { Card, CardHeader, CardTitle, CardContent } from "@/components/ui/card";
import { Button } from "@/components/ui/button";
import { Alert, AlertTitle, AlertDescription } from "@/components/ui/alert";
import { Badge } from "@/components/ui/badge";
import { Code2, Copy, Download, CheckCircle2, AlertTriangle, FileCode, ShieldCheck, Info } from "lucide-react";
import { useDiagnosticos, useFacturacionStore } from "@/store/comprobanteStore";
import { descargar } from "@/lib/core";
import { FirmarXmlModal } from "./FirmarXmlModal";

export function XmlViewerCard() {
  const [copiado, setCopiado] = useState(false);
  const mensaje = useFacturacionStore((s) => s.mensaje);
  const mostrarErrores = useFacturacionStore((s) => s.mostrarErrores);
  const xml = useFacturacionStore((s) => s.xml);
  const firmado = useFacturacionStore((s) => s.xmlFirmado);
  const clave = useFacturacionStore((s) => s.request.clave);
  const descargarJson = useFacturacionStore((s) => s.descargarJson);
  const diagnosticos = useDiagnosticos();

  const copiar = async () => {
    if (!xml) return;
    await navigator.clipboard.writeText(xml);
    setCopiado(true);
    setTimeout(() => setCopiado(false), 2000);
  };

  return (
    <div className="space-y-6">
      {mensaje && (
        <Alert variant={mensaje.tipo === "exito" ? "success" : mensaje.tipo === "error" ? "destructive" : "default"}>
          {mensaje.tipo === "exito" ? <CheckCircle2 className="w-4 h-4" /> : mensaje.tipo === "error" ? <AlertTriangle className="w-4 h-4" /> : <Info className="w-4 h-4" />}
          <AlertTitle>{mensaje.texto}</AlertTitle>
        </Alert>
      )}

      {mostrarErrores && diagnosticos.length > 0 && (
        <Card>
          <CardHeader className="pb-2">
            <CardTitle className="text-sm">Diagnósticos del core ({diagnosticos.length})</CardTitle>
          </CardHeader>
          <CardContent>
            <ul className="space-y-1.5 text-xs">
              {diagnosticos.map((d, i) => (
                <li key={i} className={d.severidad === "error" ? "text-red-700" : "text-amber-700"}>
                  <span className="font-semibold">{d.severidad === "error" ? "Error" : "Advertencia"}:</span> {d.mensaje}
                  <div className="font-mono text-[10px] text-slate-400">{d.codigo}{d.ruta ? ` · ${d.ruta}` : ""}</div>
                </li>
              ))}
            </ul>
          </CardContent>
        </Card>
      )}

      <div className="flex justify-end">
        <Button size="sm" variant="outline" onClick={descargarJson} className="h-7 text-xs gap-1">
          <FileCode className="w-3.5 h-3.5 text-blue-600" /> Descargar request JSON
        </Button>
      </div>

      {xml && (
        <Card className="border-slate-300">
          <CardHeader className="pb-2 flex flex-row items-center justify-between flex-wrap gap-2">
            <div className="flex items-center gap-2">
              <CardTitle className="text-sm flex items-center gap-2">
                <Code2 className="w-4 h-4 text-blue-600" /> XML v4.4
              </CardTitle>
              {firmado
                ? <Badge variant="success" className="text-[10px] gap-1"><ShieldCheck className="w-3 h-3" /> Firmado XAdES-EPES</Badge>
                : <Badge variant="secondary" className="text-[10px]">Sin firma</Badge>}
            </div>
            <div className="flex items-center gap-2 flex-wrap">
              {!firmado && <FirmarXmlModal />}
              <Button size="sm" variant="outline" onClick={copiar} className="h-7 text-xs gap-1">
                <Copy className="w-3.5 h-3.5" /> {copiado ? "Copiado" : "Copiar"}
              </Button>
              <Button size="sm" onClick={() => descargar(`${clave || "comprobante"}${firmado ? "_firmado" : ""}.xml`, xml, "application/xml")}
                className="h-7 text-xs gap-1 bg-emerald-600 hover:bg-emerald-700">
                <Download className="w-3.5 h-3.5" /> XML
              </Button>
            </div>
          </CardHeader>
          <CardContent>
            <pre className="p-3 bg-slate-900 text-slate-100 rounded-md font-mono text-xs max-h-96 overflow-auto leading-tight">{xml}</pre>
          </CardContent>
        </Card>
      )}
    </div>
  );
}
