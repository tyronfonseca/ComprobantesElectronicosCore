import React from "react";
import { Button } from "@/components/ui/button";
import { AlertTriangle, RefreshCw } from "lucide-react";

interface ErrorScreenProps {
  errorMessage: string | null;
}

export function ErrorScreen({ errorMessage }: ErrorScreenProps) {
  return (
    <div className="min-h-screen flex flex-col items-center justify-center bg-slate-50 text-slate-700 p-6">
      <div className="p-8 bg-white rounded-2xl shadow-lg border border-red-200 flex flex-col items-center gap-4 max-w-md w-full text-center">
        <div className="p-4 bg-red-50 text-red-600 rounded-full">
          <AlertTriangle className="w-8 h-8" />
        </div>
        <h2 className="text-lg font-bold text-slate-900">Error al Cargar WebAssembly</h2>
        <p className="text-sm text-red-600 font-mono">{errorMessage || "No se pudo inicializar el motor WebAssembly."}</p>
        <Button onClick={() => window.location.reload()} variant="outline" className="gap-2 mt-2">
          <RefreshCw className="w-4 h-4" /> Reintentar
        </Button>
      </div>
    </div>
  );
}

