import React from "react";
import { Building2 } from "lucide-react";
import { ParteForm } from "./ParteForm";
import { useFacturacionStore } from "@/store/comprobanteStore";

export function EmisorForm() {
  const emisor = useFacturacionStore((s) => s.request.emisor);
  const esCompra = useFacturacionStore((s) => s.request.tipoDocumento === "08");
  const editar = useFacturacionStore((s) => s.editar);

  return (
    <ParteForm
      ruta="emisor"
      titulo={<><Building2 className="w-4 h-4 text-blue-600" /> 2. Emisor</>}
      descripcion={esCompra
        ? "En la Factura Electrónica de Compra el emisor es el proveedor que vende."
        : "Obligado tributario que emite el comprobante."}
      parte={emisor}
      onChange={(fn) => editar((d) => fn(d.emisor))}
    />
  );
}
