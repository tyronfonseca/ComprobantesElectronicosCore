import React from "react";
import type { Emisor, Receptor, Ubicacion } from "@cec-cr/core";
import { Card, CardHeader, CardTitle, CardDescription, CardContent } from "@/components/ui/card";
import { Button } from "@/components/ui/button";
import { Badge } from "@/components/ui/badge";
import { Label } from "@/components/ui/label";
import { Mail, MapPin, Phone, Plus, Trash2 } from "lucide-react";
import { Campo, CampoCatalogo } from "@/components/common/Campo";
import { UbicacionEditor } from "@/components/common/UbicacionEditor";
import { FieldError } from "@/components/common/FieldError";
import { useCatalogos, useError, useErroresEn } from "@/store/comprobanteStore";

type Parte = Emisor | Receptor;

interface ParteFormProps {
  /** "emisor" o "receptor": prefijo de las rutas del request. */
  ruta: "emisor" | "receptor";
  titulo: React.ReactNode;
  descripcion: string;
  parte: Parte;
  onChange: (fn: (draft: Parte) => void) => void;
  /** Encabezado adicional (ej. el interruptor "sin receptor"). */
  acciones?: React.ReactNode;
}

const UBICACION_VACIA: Ubicacion = { provincia: "", canton: "", distrito: "", otrasSenas: "" };

/** Datos de una parte del comprobante (Emisor o Receptor del Anexo 1). */
export function ParteForm({ ruta, titulo, descripcion, parte, onChange, acciones }: ParteFormProps) {
  const { tiposIdentificacion } = useCatalogos();
  const errores = useErroresEn(ruta);
  const errorCorreo = useError(`${ruta}.correoElectronico`);
  const esEmisor = ruta === "emisor";
  const correos: string[] = esEmisor
    ? (parte as Emisor).correoElectronico ?? []
    : [(parte as Receptor).correoElectronico ?? ""];

  return (
    <Card>
      <CardHeader className="pb-3">
        <div className="flex items-center justify-between gap-3">
          <div>
            <CardTitle className="text-base flex items-center gap-2">
              {titulo}
              {errores > 0 && <Badge variant="destructive">{errores} error(es)</Badge>}
            </CardTitle>
            <CardDescription>{descripcion}</CardDescription>
          </div>
          {acciones}
        </div>
      </CardHeader>
      <CardContent className="space-y-4">
        <div className="grid grid-cols-1 sm:grid-cols-4 gap-4">
          <CampoCatalogo
            label="Tipo de identificación"
            ruta={`${ruta}.identificacion.tipo`}
            value={parte.identificacion?.tipo}
            catalogo={tiposIdentificacion}
            onChange={(tipo) => onChange((d) => {
              d.identificacion = { numero: d.identificacion?.numero ?? "", tipo: tipo as never };
            })}
          />
          <Campo
            label="Número de identificación"
            ruta={`${ruta}.identificacion.numero`}
            value={parte.identificacion?.numero}
            placeholder="Sin guiones"
            onChange={(numero) => onChange((d) => {
              d.identificacion = { tipo: d.identificacion?.tipo ?? "02", numero };
            })}
          />
          <Campo
            className="sm:col-span-2"
            label="Nombre o razón social"
            ruta={`${ruta}.nombre`}
            value={parte.nombre}
            onChange={(nombre) => onChange((d) => { d.nombre = nombre; })}
          />
          <Campo
            className="sm:col-span-2"
            label="Nombre comercial (opcional)"
            ruta={`${ruta}.nombreComercial`}
            value={parte.nombreComercial}
            onChange={(nombreComercial) => onChange((d) => { d.nombreComercial = nombreComercial; })}
          />
          {esEmisor && (
            <Campo
              label="Registro fiscal Ley 8707 (bebidas alcohólicas)"
              ruta="emisor.registroFiscal8707"
              value={(parte as Emisor).registroFiscal8707}
              maxLength={12}
              onChange={(v) => onChange((d) => { (d as Emisor).registroFiscal8707 = v; })}
            />
          )}
          <Campo
            label="Otras señas en el extranjero"
            ruta={`${ruta}.otrasSenasExtranjero`}
            value={parte.otrasSenasExtranjero}
            onChange={(v) => onChange((d) => { d.otrasSenasExtranjero = v; })}
          />
        </div>

        <div className="pt-2 border-t border-slate-100 space-y-3">
          <label className="flex items-center gap-2 cursor-pointer select-none">
            <input
              type="checkbox"
              checked={Boolean(parte.ubicacion)}
              onChange={(e) => onChange((d) => { d.ubicacion = e.target.checked ? { ...UBICACION_VACIA } : undefined; })}
              className="w-4 h-4 rounded text-blue-600 border-slate-300"
            />
            <span className="text-xs font-semibold text-slate-700 flex items-center gap-1.5">
              <MapPin className="w-3.5 h-3.5 text-blue-600" /> Ubicación en Costa Rica
            </span>
          </label>
          {parte.ubicacion && (
            <UbicacionEditor
              ruta={`${ruta}.ubicacion`}
              ubicacion={parte.ubicacion}
              onChange={(u) => onChange((d) => { d.ubicacion = u; })}
            />
          )}
        </div>

        <div className="pt-2 border-t border-slate-100 grid grid-cols-1 sm:grid-cols-2 gap-4">
          <div>
            <Label className="text-xs font-semibold text-slate-700 mb-1 flex items-center gap-1.5">
              <Phone className="w-3.5 h-3.5 text-slate-500" /> Teléfono (opcional)
            </Label>
            <div className="flex gap-2">
              <Campo
                className="w-24 shrink-0"
                label="País"
                ruta={`${ruta}.telefono.codigoPais`}
                value={parte.telefono?.codigoPais}
                placeholder="506"
                onChange={(codigoPais) => onChange((d) => {
                  d.telefono = codigoPais || d.telefono?.numTelefono ? { codigoPais, numTelefono: d.telefono?.numTelefono ?? "" } : undefined;
                })}
              />
              <Campo
                className="flex-1"
                label="Número"
                ruta={`${ruta}.telefono.numTelefono`}
                value={parte.telefono?.numTelefono}
                placeholder="22223333"
                onChange={(numTelefono) => onChange((d) => {
                  d.telefono = numTelefono || d.telefono?.codigoPais ? { codigoPais: d.telefono?.codigoPais || "506", numTelefono } : undefined;
                })}
              />
            </div>
          </div>

          <div>
            <div className="flex items-center justify-between mb-1">
              <Label className="text-xs font-semibold text-slate-700 flex items-center gap-1.5">
                <Mail className="w-3.5 h-3.5 text-slate-500" />
                {esEmisor ? `Correos electrónicos (${correos.length}/4)` : "Correo electrónico"}
              </Label>
              {esEmisor && correos.length < 4 && (
                <Button
                  type="button" variant="ghost" size="sm" className="h-6 text-xs text-blue-600 px-2"
                  onClick={() => onChange((d) => { (d as Emisor).correoElectronico = [...correos, ""]; })}
                >
                  <Plus className="w-3 h-3 mr-1" /> Agregar
                </Button>
              )}
            </div>
            <div className="space-y-2">
              {correos.map((correo, i) => (
                <div key={i} className="flex gap-2 items-center">
                  <Campo
                    className="flex-1"
                    label=""
                    ruta={esEmisor ? `${ruta}.correoElectronico[${i}]` : `${ruta}.correoElectronico`}
                    type="email"
                    value={correo}
                    placeholder="correo@ejemplo.com"
                    onChange={(v) => onChange((d) => {
                      if (esEmisor) {
                        const lista = [...correos];
                        lista[i] = v;
                        (d as Emisor).correoElectronico = lista;
                      } else {
                        (d as Receptor).correoElectronico = v;
                      }
                    })}
                  />
                  {esEmisor && correos.length > 1 && (
                    <Button
                      type="button" variant="ghost" size="icon" className="h-8 w-8 text-slate-400 hover:text-red-600"
                      onClick={() => onChange((d) => { (d as Emisor).correoElectronico = correos.filter((_, j) => j !== i); })}
                    >
                      <Trash2 className="w-4 h-4" />
                    </Button>
                  )}
                </div>
              ))}
            </div>
            {esEmisor && <FieldError error={errorCorreo} />}
          </div>
        </div>
      </CardContent>
    </Card>
  );
}
