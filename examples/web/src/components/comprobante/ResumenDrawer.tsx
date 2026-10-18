import React, { useEffect, useState } from "react";
import { Button } from "@/components/ui/button";
import { ResumenLiquidacion } from "./ResumenLiquidacion";
import { ClaveGeneradorCard } from "./ClaveGeneradorCard";
import { XmlViewerCard } from "./XmlViewerCard";
import { useFacturacionStore } from "@/store/comprobanteStore";
import {
  Receipt,
  X,
  CheckCircle2
  ,
  AlertCircle,
  FileCode2,
  ChevronRight
} from "lucide-react";

export function ResumenDrawer() {
  const [isOpen, setIsOpen] = useState(false);
  const moneda = useFacturacionStore((state) => state.request.resumenFactura.codigoTipoMoneda.codigoMoneda);
  const totalComprobante = useFacturacionStore((state) => Number(state.vistaPrevia?.comprobante?.ResumenFactura.TotalComprobante ?? 0));
  const valido = useFacturacionStore((state) => state.mostrarErrores ? state.vistaPrevia?.exito : undefined);

  const handleOpenDrawer = () => setIsOpen(true);

  const handleCloseDrawer = () => {
    setIsOpen(false);
  };

  // Cerrar con la tecla Escape
  useEffect(() => {
    const handleKeyDown = (e: KeyboardEvent) => {
      if (e.key === "Escape" && isOpen) {
        setIsOpen(false);
      }
    };
    window.addEventListener("keydown", handleKeyDown);
    return () => window.removeEventListener("keydown", handleKeyDown);
  }, [isOpen]);

  // Prevenir scroll en body cuando está abierto en pantallas pequeñas
  useEffect(() => {
    if (isOpen) {
      document.body.style.overflow = "hidden";
    } else {
      document.body.style.overflow = "unset";
    }
    return () => {
      document.body.style.overflow = "unset";
    };
  }, [isOpen]);

  const currencySymbol = moneda === "CRC" ? "₡" : moneda;
  const formattedTotal = totalComprobante.toLocaleString("es-CR", {
    minimumFractionDigits: 2,
    maximumFractionDigits: 2
  });

  return (
    <>
      {/* Botón Flotante (Floating Action Button - FAB) */}
      <div className="fixed bottom-6 right-6 z-40">
        <button
          onClick={handleOpenDrawer}
          className="group flex items-center gap-3 bg-linear-to-r from-blue-600 to-indigo-600 hover:from-blue-700 hover:to-indigo-700 text-white px-5 py-3.5 rounded-full shadow-xl hover:shadow-2xl transition-all duration-300 transform hover:-translate-y-0.5 active:translate-y-0 focus:outline-none focus:ring-4 focus:ring-blue-400/50 cursor-pointer border border-blue-400/30"
          title="Abrir resumen de liquidación y validar comprobante"
        >
          <div className="flex items-center justify-center w-8 h-8 rounded-full bg-white/20 text-white">
            <Receipt className="w-5 h-5" />
          </div>
          <div className="flex flex-col text-left">
            <span className="text-[11px] font-medium text-blue-100 uppercase tracking-wider">
              Resumen & Validación
            </span>
            <span className="text-sm font-extrabold font-mono text-white">
              {currencySymbol} {formattedTotal}
            </span>
          </div>

          {valido !== undefined && (
            <div className="ml-1">
              {valido ? (
                <CheckCircle2 className="w-5 h-5 text-emerald-300" />
              ) : (
                <AlertCircle className="w-5 h-5 text-amber-300 animate-pulse" />
              )}
            </div>
          )}

          <ChevronRight className="w-4 h-4 text-blue-200 group-hover:translate-x-0.5 transition-transform" />
        </button>
      </div>

      {/* Backdrop Overlay */}
      {isOpen && (
        <div
          className="fixed inset-0 bg-slate-900/50 backdrop-blur-sm z-50 transition-opacity duration-300"
          onClick={handleCloseDrawer}
          aria-hidden="true"
        />
      )}

      {/* Right Drawer Panel */}
      <div
        className={`fixed top-0 right-0 bottom-0 w-full sm:w-125 md:w-135 bg-white shadow-2xl z-50 transform transition-transform duration-300 ease-in-out flex flex-col border-l border-slate-200 ${
          isOpen ? "translate-x-0" : "translate-x-full"
        }`}
      >
        {/* Header del Drawer */}
        <div className="px-5 py-4 border-b border-slate-200 bg-slate-50/80 flex items-center justify-between shrink-0">
          <div className="flex items-center gap-2.5">
            <div className="p-2 rounded-lg bg-blue-100/80 text-blue-700">
              <FileCode2 className="w-5 h-5" />
            </div>
            <div>
              <h3 className="text-base font-bold text-slate-900">
                Liquidación & Validación Fiscal
              </h3>
              <p className="text-xs text-slate-500">
                Motor nativo C++ WebAssembly
              </p>
            </div>
          </div>
          <Button
            variant="ghost"
            size="icon"
            onClick={handleCloseDrawer}
            className="h-8 w-8 text-slate-500 hover:text-slate-800 rounded-full"
            aria-label="Cerrar panel"
          >
            <X className="w-5 h-5" />
          </Button>
        </div>

        {/* Contenido Scrollable */}
        <div className="flex-1 overflow-y-auto p-5 space-y-6">
          {/* Resumen de Liquidación con Botones de Validación y Generación */}
          <ResumenLiquidacion />

          {/* Clave de 50 Dígitos */}
          <ClaveGeneradorCard />

          {/* Diagnóstico de Validación y Visor XML */}
          <XmlViewerCard />
        </div>
      </div>
    </>
  );
}
