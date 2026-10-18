import React from "react";
import { Input } from "@/components/ui/input";
import { Label } from "@/components/ui/label";
import { FieldError } from "@/components/common/FieldError";
import { SelectCatalog, type CatalogInputItem } from "@/components/inputs/SelectCatalog";
import { useError } from "@/store/comprobanteStore";

const CLASE_ERROR = "border-red-500 focus-visible:ring-red-500 bg-red-50/10";

/**
 * Fuente de errores alternativa al store: la usa el editor de líneas para mostrar los diagnósticos
 * de la vista previa de la línea en edición antes de guardarla.
 */
export const ErroresContext = React.createContext<((ruta: string) => string | undefined) | null>(null);

function useErrorCampo(ruta: string | undefined): string | undefined {
  const alternativa = React.useContext(ErroresContext);
  const delStore = useError(ruta ?? "");
  if (!ruta) return undefined;
  return alternativa ? alternativa(ruta) : delStore;
}

interface CampoProps extends Omit<React.InputHTMLAttributes<HTMLInputElement>, "onChange" | "value"> {
  label: React.ReactNode;
  /** Ruta del campo en el request; muestra el diagnóstico del core con esa ruta. */
  ruta?: string;
  value: string | number | undefined;
  onChange: (valor: string) => void;
}

/** Campo de texto con el error del core asociado a su ruta. */
export function Campo({ label, ruta, value, onChange, className = "", ...props }: CampoProps) {
  const error = useErrorCampo(ruta);
  return (
    <div className={className}>
      {label !== "" && <Label className="text-xs font-semibold text-slate-700 mb-1 block">{label}</Label>}
      <Input
        {...props}
        value={value ?? ""}
        className={`text-xs h-9 bg-white ${error ? CLASE_ERROR : ""}`}
        onChange={(e) => onChange(e.target.value)}
      />
      <FieldError error={error} />
    </div>
  );
}

interface CampoCatalogoProps {
  label: string;
  ruta?: string;
  value: string | undefined;
  onChange: (valor: string) => void;
  catalogo: readonly CatalogInputItem[];
  placeholder?: string;
  disabled?: boolean;
  className?: string;
}

/** Selector de catálogo con el error del core asociado a su ruta. */
export function CampoCatalogo({ label, ruta, value, onChange, catalogo, placeholder, disabled, className }: CampoCatalogoProps) {
  const error = useErrorCampo(ruta);
  return (
    <div className={className}>
      <SelectCatalog
        label={label}
        value={value ?? ""}
        onChange={(v) => onChange(v === undefined ? "" : String(v))}
        catalog={catalogo}
        valueAsNumber={false}
        placeholder={placeholder}
        disabled={disabled}
        selectClassName={error ? CLASE_ERROR : ""}
      />
      <FieldError error={error} />
    </div>
  );
}

/** Número decimal como texto para el request ("" si está vacío). */
export function decimalDeInput(valor: string): string {
  return valor.trim().replace(",", ".");
}
