import React, { useEffect, useState } from "react";
import { Tabs, TabsList, TabsTrigger, TabsContent } from "@/components/ui/tabs";
import { FileText, Database } from "lucide-react";

// Store
import { useFacturacionStore } from "./store/comprobanteStore";

// Componentes Globales
import { Header } from "@/components/Header";
import { LoadingScreen } from "@/components/LoadingScreen";
import { ErrorScreen } from "@/components/ErrorScreen";

// Componentes Modulares por Sección
import { ComprobanteConfigForm } from "@/components/comprobante/ComprobanteConfigForm";
import { EmisorForm } from "@/components/comprobante/EmisorForm";
import { ReceptorForm } from "@/components/comprobante/ReceptorForm";
import { LineasDetalleForm } from "@/components/comprobante/LineasDetalleForm";
import { OtrosCargosForm } from "@/components/comprobante/OtrosCargosForm";
import { ReferenciasForm } from "@/components/comprobante/ReferenciasForm";
import { ResumenDrawer } from "@/components/comprobante/ResumenDrawer";

import { DtaExplorerCard } from "@/components/catalogos/DtaExplorerCard";
import { CatalogosOficialesCard } from "@/components/catalogos/CatalogosOficialesCard";


function FacturacionAppContent() {
  const inicializar = useFacturacionStore((state) => state.inicializar);
  const estado = useFacturacionStore((state) => state.estado);
  const errorMessage = useFacturacionStore((state) => state.errorCarga);
  
  const [activeTab, setActiveTab] = useState<"comprobante" | "catalogos">("comprobante");

  useEffect(() => {
    inicializar();
  }, [inicializar]);

  if (estado === "cargando") {
    return <LoadingScreen />;
  }

  if (estado === "error") {
    return <ErrorScreen errorMessage={errorMessage} />;
  }

  return (
    <div className="min-h-screen bg-slate-50 text-slate-900 pb-24">
      <Header />

      <main className="w-full max-w-7xl mx-auto px-4 sm:px-6 pt-6">
        <Tabs value={activeTab} onValueChange={v => setActiveTab(v as any)}>
          <TabsList className="grid grid-cols-2 w-full max-w-md mb-6 bg-slate-200/70 p-1 rounded-xl">
            <TabsTrigger value="comprobante" className="gap-2">
              <FileText className="w-4 h-4" /> Comprobantes (01-10)
            </TabsTrigger>
            <TabsTrigger value="catalogos" className="gap-2">
              <Database className="w-4 h-4" /> Catálogos & DTA
            </TabsTrigger>
          </TabsList>

          {/* TAB 1: Emisión de Comprobantes Electrónicos */}
          <TabsContent value="comprobante">
            <div className="space-y-6">
              <ComprobanteConfigForm />
              <EmisorForm />
              <ReceptorForm />
              <LineasDetalleForm />
             <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
                <OtrosCargosForm />
                <ReferenciasForm />
              </div>
            </div>
          </TabsContent>

          {/* TAB 2: División Territorial Administrativa & Catálogos Oficiales */}
          <TabsContent value="catalogos">
            <div className="grid grid-cols-1 md:grid-cols-12 gap-6">
              <div className="md:col-span-6 space-y-6">
                <DtaExplorerCard />
              </div>
              <div className="md:col-span-6 space-y-6">
                <CatalogosOficialesCard />
              </div>
            </div>
          </TabsContent>
        </Tabs>
      </main>

      {/* Right Navbar Menu (Drawer) y Botón Flotante con Validación Inmediata */}
      <ResumenDrawer />
    </div>
  );
}

export function App() {
  return (
      <FacturacionAppContent />
  );
}
export default App;
