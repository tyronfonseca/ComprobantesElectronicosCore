import React, { useMemo } from "react";
import { Card, CardHeader, CardTitle, CardDescription, CardContent } from "@/components/ui/card";
import { Badge } from "@/components/ui/badge";
import { MapPin } from "lucide-react";
import { core } from "@/store/comprobanteStore";

/** Cantones y distritos se consultan al core cuando se abre cada nivel. */
function Canton({ provincia, codigo, nombre }: { provincia: string; codigo: string; nombre: string }) {
  const [abierto, setAbierto] = React.useState(false);
  const distritos = useMemo(() => (abierto ? core().territorio.distritos(provincia, codigo) : []), [abierto, provincia, codigo]);
  return (
    <details className="border-l-2 border-blue-200 pl-3 py-1" onToggle={(e) => setAbierto((e.target as HTMLDetailsElement).open)}>
      <summary className="font-medium text-slate-700 cursor-pointer">Cantón {codigo}: {nombre}</summary>
      <div className="pl-3 mt-2 text-xs text-slate-600 space-y-1">
        {distritos.map((d) => (
          <div key={d.codigo}>
            Distrito {d.codigo}: <span className="font-semibold">{d.nombre}</span>{" "}
            ({core().territorio.barrios(provincia, codigo, d.codigo).length} barrios)
          </div>
        ))}
      </div>
    </details>
  );
}

export function DtaExplorerCard() {
  const territorio = core().territorio;
  const provincias = useMemo(() => territorio.provincias().map((p) => ({ ...p, cantones: territorio.cantones(p.codigo) })), [territorio]);

  return (
    <Card>
      <CardHeader>
        <CardTitle className="text-base flex items-center gap-2">
          <MapPin className="w-4 h-4 text-blue-600" /> División territorial (Nota 14)
        </CardTitle>
        <CardDescription>Provincias, cantones, distritos y barrios del core</CardDescription>
      </CardHeader>
      <CardContent>
        <div className="max-h-137.5 overflow-y-auto space-y-2 pr-2">
          {provincias.map((p) => (
            <details key={p.codigo} className="border border-slate-200 rounded-lg p-3 bg-white text-sm">
              <summary className="font-bold text-slate-900 cursor-pointer flex justify-between items-center">
                <span>Provincia {p.codigo}: {p.nombre}</span>
                <Badge variant="secondary">{p.cantones.length} cantones</Badge>
              </summary>
              <div className="pl-4 mt-3 space-y-2">
                {p.cantones.map((c) => <Canton key={c.codigo} provincia={p.codigo} codigo={c.codigo} nombre={c.nombre} />)}
              </div>
            </details>
          ))}
        </div>
      </CardContent>
    </Card>
  );
}
