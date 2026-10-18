import React, { useMemo } from "react";
import type { Ubicacion } from "@cec-cr/core";
import { Campo, CampoCatalogo } from "./Campo";
import { core } from "@/store/comprobanteStore";

interface UbicacionEditorProps {
  /** Ruta de la ubicación en el request (ej. "emisor.ubicacion"). */
  ruta: string;
  ubicacion: Ubicacion;
  onChange: (ubicacion: Ubicacion) => void;
}

/** Provincia, cantón y distrito en cascada desde la división territorial del core (Nota 14). */
export function UbicacionEditor({ ruta, ubicacion, onChange }: UbicacionEditorProps) {
  const territorio = core().territorio;
  const provincias = useMemo(() => territorio.provincias(), [territorio]);
  const cantones = useMemo(() => territorio.cantones(ubicacion.provincia), [territorio, ubicacion.provincia]);
  const distritos = useMemo(
    () => territorio.distritos(ubicacion.provincia, ubicacion.canton),
    [territorio, ubicacion.provincia, ubicacion.canton]
  );
  const barrios = useMemo(
    () => territorio.barrios(ubicacion.provincia, ubicacion.canton, ubicacion.distrito),
    [territorio, ubicacion.provincia, ubicacion.canton, ubicacion.distrito]
  );

  return (
    <div className="grid grid-cols-1 sm:grid-cols-4 gap-4">
      <CampoCatalogo
        label="Provincia"
        ruta={`${ruta}.provincia`}
        value={ubicacion.provincia}
        catalogo={provincias.map((p) => ({ codigo: p.codigo, descripcion: p.nombre }))}
        placeholder="Seleccione"
        onChange={(provincia) => onChange({ ...ubicacion, provincia, canton: "", distrito: "", barrio: "" })}
      />
      <CampoCatalogo
        label="Cantón"
        ruta={`${ruta}.canton`}
        value={ubicacion.canton}
        catalogo={cantones.map((c) => ({ codigo: c.codigo, descripcion: c.nombre }))}
        placeholder="Seleccione"
        onChange={(canton) => onChange({ ...ubicacion, canton, distrito: "", barrio: "" })}
      />
      <CampoCatalogo
        label="Distrito"
        ruta={`${ruta}.distrito`}
        value={ubicacion.distrito}
        catalogo={distritos.map((d) => ({ codigo: d.codigo, descripcion: d.nombre }))}
        placeholder="Seleccione"
        onChange={(distrito) => onChange({ ...ubicacion, distrito, barrio: "" })}
      />
      <CampoCatalogo
        label="Barrio (opcional)"
        ruta={`${ruta}.barrio`}
        value={ubicacion.barrio}
        catalogo={barrios.map((b) => ({ codigo: b.nombre, descripcion: b.nombre }))}
        placeholder="Sin barrio"
        onChange={(barrio) => onChange({ ...ubicacion, barrio })}
      />
      <Campo
        className="sm:col-span-4"
        label="Otras señas"
        ruta={`${ruta}.otrasSenas`}
        value={ubicacion.otrasSenas}
        placeholder="Dirección exacta"
        onChange={(otrasSenas) => onChange({ ...ubicacion, otrasSenas })}
      />
    </div>
  );
}
