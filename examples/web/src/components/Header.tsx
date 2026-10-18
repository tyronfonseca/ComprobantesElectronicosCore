import React from "react";
import { Badge } from "@/components/ui/badge";
import { Cpu } from "lucide-react";

export function Header() {
  return (
    <header className="sticky top-0 z-50 bg-white/90 backdrop-blur border-b border-slate-200 px-6 py-4 shadow-sm">
      <div className="max-w-7xl mx-auto flex flex-col sm:flex-row sm:items-center justify-between gap-4">
        <div className="flex items-center gap-3">
          <div className="p-2.5 bg-blue-600 text-white rounded-xl shadow-md">
            <Cpu className="w-6 h-6" />
          </div>
          <div>
            <div className="flex items-center gap-2">
              <h1 className="text-xl font-bold tracking-tight text-slate-900">Comprobantes Electrónicos Core - Demo (WASM + Typescript)</h1>
              <Badge variant="outline" className="bg-blue-50 text-blue-700 border-blue-200">v0.0.1</Badge>
            </div>
            <p className="text-xs text-slate-500">Motor C++20 compilado a WebAssembly en el navegador</p>
          </div>
        </div>

        <div className="flex items-center gap-3">
          <Badge variant="success" className="gap-1.5 py-1 px-3">
            <span className="w-2 h-2 rounded-full bg-white animate-pulse" />
            Motor WASM Activo
          </Badge>
        </div>
      </div>
    </header>
  );
}

