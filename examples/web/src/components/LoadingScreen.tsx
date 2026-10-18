import React from "react";
import { FileCheck, RefreshCw } from "lucide-react";

export function LoadingScreen() {
  return (
    <div className="min-h-screen flex flex-col items-center justify-center bg-slate-50 text-slate-700 p-6">
      <div className="p-8 bg-white rounded-2xl shadow-lg border border-slate-200 flex flex-col items-center gap-4 max-w-md w-full text-center">
        <div className="p-4 bg-blue-50 text-blue-600 rounded-full">
          <FileCheck className="w-8 h-8 animate-pulse" />
        </div>
        <h2 className="text-lg font-bold text-slate-900">Iniciando Motor WebAssembly</h2>
        <p className="text-sm text-slate-500">Cargando biblioteca nativa de Comprobantes Electrónicos Core...</p>
        <div className="flex items-center gap-2 text-xs text-blue-600 font-medium">
          <RefreshCw className="w-4 h-4 animate-spin" />
          Cargando catálogos DTA y estructuras fiscales
        </div>
      </div>
    </div>
  );
}

