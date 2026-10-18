import React, { useState, useRef } from "react";
import { Dialog } from "@/components/ui/dialog";
import { Button } from "@/components/ui/button";
import { Badge } from "@/components/ui/badge";
import { Upload, FileCode, CheckCircle2, AlertCircle, Sparkles, Download, RefreshCw, FileText } from "lucide-react";
import { useFacturacionStore } from "@/store/comprobanteStore";

interface CargarFacturaModalProps {
  open: boolean;
  onOpenChange: (open: boolean) => void;
}

import { EJEMPLOS_COMPROBANTES, type EjemploDocumento } from "@/templates";

export function CargarFacturaModal({ open, onOpenChange }: CargarFacturaModalProps) {
  const cargarJson = useFacturacionStore((state) => state.cargarJson);
  const exportarJson = useFacturacionStore((state) => state.exportarJson);
  const handleDescargarJson = useFacturacionStore((state) => state.descargarJson);
  
  const [jsonText, setJsonText] = useState("");
  const [feedback, setFeedback] = useState<{ isSuccess?: boolean; message?: string } | null>(null);
  const [dragActive, setDragActive] = useState(false);
  const [selectedTemplateIndex, setSelectedTemplateIndex] = useState<number | null>(null);
  const fileInputRef = useRef<HTMLInputElement>(null);

  const handleProcessFile = (file: File) => {
    if (!file.name.endsWith(".json") && file.type !== "application/json") {
      setFeedback({ isSuccess: false, message: "Por favor seleccione un archivo con extensión .json" });
      return;
    }
    const reader = new FileReader();
    reader.onload = (e) => {
      const content = e.target?.result as string;
      if (content) {
        setJsonText(content);
        setSelectedTemplateIndex(null);
        setFeedback(null);
      }
    };
    reader.onerror = () => {
      setFeedback({ isSuccess: false, message: "Error al leer el archivo seleccionado." });
    };
    reader.readAsText(file);
  };

  const handleFileChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    const files = e.target.files;
    if (files && files.length > 0) {
      handleProcessFile(files[0]);
    }
  };

  const handleDrop = (e: React.DragEvent) => {
    e.preventDefault();
    setDragActive(false);
    if (e.dataTransfer.files && e.dataTransfer.files.length > 0) {
      handleProcessFile(e.dataTransfer.files[0]);
    }
  };

  const handleSelectTemplate = (ej: EjemploDocumento, idx: number) => {
    setJsonText(ej.json);
    setSelectedTemplateIndex(idx);
    setFeedback(null);
  };

  const handleCargar = () => {
    if (!jsonText.trim()) {
      setFeedback({ isSuccess: false, message: "Por favor ingrese o cargue un documento JSON." });
      return;
    }

    const res = cargarJson(jsonText);
    setFeedback({ isSuccess: res.tipo !== "error", message: res.texto });

    if (res.tipo !== "error") {
      setTimeout(() => {
        onOpenChange(false);
      }, 1200);
    }
  };

  const handleCargarActual = () => {
    const currentJson = exportarJson();
    if (currentJson) {
      setJsonText(currentJson);
      setSelectedTemplateIndex(null);
      setFeedback({ isSuccess: true, message: "JSON del comprobante actual generado y cargado en el editor." });
    }
  };

  return (
    <Dialog
      open={open}
      onOpenChange={onOpenChange}
      title="Cargar Comprobante Electrónico (.json)"
      description="Cargue un request de comprobante (comprobante_request.schema.json) o un ejemplo. El core lo valida al cargarlo."
      maxWidth="xl"
      footer={
        <div className="flex items-center justify-between w-full">
          <div className="flex items-center gap-2">
            <Button
              type="button"
              variant="outline"
              size="sm"
              onClick={handleCargarActual}
              className="text-xs gap-1.5"
            >
              <RefreshCw className="w-3.5 h-3.5" /> Ver JSON Actual
            </Button>
            <Button
              type="button"
              variant="outline"
              size="sm"
              onClick={handleDescargarJson}
              className="text-xs gap-1.5"
            >
              <Download className="w-3.5 h-3.5" /> Descargar .json
            </Button>
          </div>
          <div className="flex items-center gap-2">
            <Button
              type="button"
              variant="outline"
              onClick={() => onOpenChange(false)}
            >
              Cancelar
            </Button>
            <Button
              type="button"
              className="bg-blue-600 hover:bg-blue-700 text-white gap-1.5"
              onClick={handleCargar}
            >
              <CheckCircle2 className="w-4 h-4" /> Cargar
            </Button>
          </div>
        </div>
      }
    >
      <div className="space-y-4">
        {/* Zona de Drop y Carga de Archivo */}
        <div
          onDragOver={(e) => { e.preventDefault(); setDragActive(true); }}
          onDragLeave={() => setDragActive(false)}
          onDrop={handleDrop}
          onClick={() => fileInputRef.current?.click()}
          className={`border-2 border-dashed rounded-xl p-5 text-center cursor-pointer transition-colors ${dragActive
              ? "border-blue-500 bg-blue-50/50"
              : "border-slate-300 hover:border-blue-400 bg-slate-50/60 hover:bg-blue-50/20"
            }`}
        >
          <input
            ref={fileInputRef}
            type="file"
            accept=".json,application/json"
            className="hidden"
            onChange={handleFileChange}
          />
          <div className="flex flex-col items-center justify-center gap-1.5">
            <div className="w-10 h-10 rounded-full bg-blue-100 flex items-center justify-center text-blue-600 mb-1">
              <Upload className="w-5 h-5" />
            </div>
            <p className="text-sm font-semibold text-slate-800">
              Arrastre y suelte un archivo <span className="text-blue-600 font-mono">.json</span> aquí o haga clic para examinar
            </p>
            <p className="text-xs text-slate-500">
              Compatible con esquemas oficiales de ComprobanteElectronicoCore (Hacienda v4.4)
            </p>
          </div>
        </div>

        {/* Plantillas de Ejemplo Oficiales para los 7 Comprobantes */}
        <div className="space-y-2">
          <div className="flex items-center justify-between">
            <span className="text-xs font-bold text-slate-700 uppercase tracking-wide flex items-center gap-1.5">
              <Sparkles className="w-3.5 h-3.5 text-blue-600" />
              Plantillas de Ejemplo Oficiales (Comprobantes 01 - 10):
            </span>
            <span className="text-[11px] text-slate-500">7 tipos disponibles</span>
          </div>

          <div className="grid grid-cols-1 sm:grid-cols-2 md:grid-cols-3 gap-2">
            {EJEMPLOS_COMPROBANTES.map((ej, idx) => {
              const isSelected = selectedTemplateIndex === idx;
              return (
                <button
                  key={ej.codigo}
                  type="button"
                  onClick={() => handleSelectTemplate(ej, idx)}
                  className={`text-left p-2.5 rounded-lg border transition-all ${isSelected
                      ? "border-blue-500 bg-blue-50/60 ring-2 ring-blue-500/20 shadow-sm"
                      : "border-slate-200 bg-white hover:border-blue-300 hover:bg-blue-50/30"
                    }`}
                >
                  <div className="flex items-center justify-between gap-1 mb-1">
                    <Badge
                      variant="outline"
                      className={`text-[10px] font-mono font-bold ${isSelected ? "bg-blue-600 text-white border-blue-600" : "bg-slate-100 text-slate-700"
                        }`}
                    >
                      {ej.badge}
                    </Badge>
                    <FileText className={`w-3.5 h-3.5 ${isSelected ? "text-blue-600" : "text-slate-400"}`} />
                  </div>
                  <div className="text-xs font-bold text-slate-800 line-clamp-1">
                    {ej.titulo}
                  </div>
                  <div className="text-[11px] text-slate-500 line-clamp-2 mt-0.5 leading-snug">
                    {ej.descripcion}
                  </div>
                </button>
              );
            })}
          </div>
        </div>

        {/* Editor de Texto JSON */}
        <div className="space-y-1.5">
          <div className="flex items-center justify-between">
            <label className="text-xs font-semibold text-slate-700 flex items-center gap-1.5">
              <FileCode className="w-3.5 h-3.5 text-blue-600" />
              Contenido JSON del Comprobante:
            </label>
            {jsonText && (
              <Badge variant="outline" className="text-[11px] font-mono">
                {jsonText.length} caracteres
              </Badge>
            )}
          </div>
          <textarea
            value={jsonText}
            onChange={(e) => {
              setJsonText(e.target.value);
              setSelectedTemplateIndex(null);
              setFeedback(null);
            }}
            placeholder="Pegue aquí el contenido JSON del comprobante..."
            rows={9}
            className="w-full font-mono text-xs p-3 rounded-lg border border-slate-300 focus:outline-none focus:ring-2 focus:ring-blue-500 focus:border-transparent bg-slate-950 text-emerald-400 leading-relaxed"
            spellCheck={false}
          />
        </div>

        {/* Feedback Alert */}
        {feedback && (
          <div
            className={`p-3 rounded-lg flex items-start gap-2 text-xs border ${feedback.isSuccess
                ? "bg-emerald-50 text-emerald-800 border-emerald-200"
                : "bg-red-50 text-red-800 border-red-200"
              }`}
          >
            {feedback.isSuccess ? (
              <CheckCircle2 className="w-4 h-4 text-emerald-600 shrink-0 mt-0.5" />
            ) : (
              <AlertCircle className="w-4 h-4 text-red-600 shrink-0 mt-0.5" />
            )}
            <div className="flex-1 font-medium">{feedback.message}</div>
          </div>
        )}
      </div>
    </Dialog>
  );
}
