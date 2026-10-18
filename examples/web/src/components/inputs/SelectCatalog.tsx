import React from "react";
import type { ElementoCatalogo, Moneda, UnidadMedida } from "@cec-cr/core";
import { Label } from "../ui/label";
import { Select } from "../ui/select";

export interface SelectCatalogOption {
  value: string | number;
  label: React.ReactNode;
}

export interface SelectCatalogItem {
  codigo?: string | number;
  descripcion?: string;
  nombre?: string;
  simbolo?: string;
  value?: string | number;
  label?: string;
  [key: string]: any;
}

export type CatalogInputItem =
  | ElementoCatalogo
  | Moneda
  | UnidadMedida
  | SelectCatalogItem
  | SelectCatalogOption
  | string;

export interface SelectCatalogProps<T = any> {
  label?: string;
  value: T;
  setter?: (val: T) => void;
  onChange?: (val: T) => void;
  catalog?: readonly CatalogInputItem[] | CatalogInputItem[];
  options?: readonly SelectCatalogOption[];
  valueAsNumber?: boolean;
  disabled?: boolean;
  className?: string;
  selectClassName?: string;
  placeholder?: string;
  renderOption?: (item: CatalogInputItem) => React.ReactNode;
}

export function SelectCatalog<T = any>({
  label,
  value,
  setter,
  onChange,
  catalog = [],
  options,
  valueAsNumber,
  disabled = false,
  className = "",
  selectClassName = "",
  placeholder,
  renderOption
}: SelectCatalogProps<T>) {
  const isNumeric = valueAsNumber ?? (typeof value === "number");

  const notifyChange = (val: T) => {
    if (onChange) onChange(val);
    if (setter) setter(val);
  };

  const effectiveItems = options || catalog;

  const getItemCode = (rawItem: CatalogInputItem, idx: number): string => {
    if (typeof rawItem === "string") return rawItem;
    const item = rawItem as SelectCatalogItem;
    if (item.value !== undefined) return String(item.value);
    if (item.codigo !== undefined) return String(item.codigo);
    if (item.simbolo !== undefined) return String(item.simbolo);
    return String(idx);
  };

  // Determinar el código exacto que debe seleccionar el elemento <select>
  const selectedCode = React.useMemo(() => {
    if (value === undefined || value === null || !effectiveItems || effectiveItems.length === 0) {
      return value !== undefined && value !== null ? String(value) : "";
    }

    const valStr = String(value);

    // 1. Búsqueda por coincidencia exacta de string
    for (let i = 0; i < effectiveItems.length; i++) {
      const code = getItemCode(effectiveItems[i], i);
      if (code === valStr) {
        return code;
      }
    }

    // 2. Búsqueda por coincidencia numérica (ej. enum 1 con código 1 de Hacienda)
    if (typeof value === "number" || (!isNaN(Number(value)) && valStr.trim() !== "")) {
      const numVal = Number(value);
      for (let i = 0; i < effectiveItems.length; i++) {
        const raw = effectiveItems[i];
        if (typeof raw !== "string") {
          const item = raw as SelectCatalogItem;
          const candidate = item.value ?? item.codigo;
          if (candidate !== undefined && !isNaN(Number(candidate)) && Number(candidate) === numVal) {
            return getItemCode(raw, i);
          }
        }
      }
    }

    return valStr;
  }, [value, effectiveItems]);

  const handleChange = (e: React.ChangeEvent<HTMLSelectElement>) => {
    const rawVal = e.target.value;
    if (rawVal === "" && placeholder) {
      notifyChange(undefined as unknown as T);
      return;
    }

    // Buscar el item original seleccionado
    let foundItem: CatalogInputItem | undefined;
    if (effectiveItems) {
      for (let i = 0; i < effectiveItems.length; i++) {
        if (getItemCode(effectiveItems[i], i) === rawVal) {
          foundItem = effectiveItems[i];
          break;
        }
      }
    }

    if (foundItem && typeof foundItem !== "string") {
      const item = foundItem as SelectCatalogItem;
      if (item.value !== undefined) {
        notifyChange(item.value as unknown as T);
        return;
      }
    }

    if (isNumeric) {
      const numVal = Number(rawVal);
      notifyChange((isNaN(numVal) ? rawVal : numVal) as unknown as T);
    } else {
      notifyChange(rawVal as unknown as T);
    }
  };

  return (
    <div className={className}>
      {label && <Label className="text-xs font-semibold text-slate-700 mb-1 block">{label}</Label>}
      <Select
        value={selectedCode}
        onChange={handleChange}
        disabled={disabled}
        className={selectClassName}
      >
        {placeholder && (
          <option value="">
            {placeholder}
          </option>
        )}
        {effectiveItems?.map((rawItem, idx) => {
          if (typeof rawItem === "string") {
            return (
              <option key={`${rawItem}-${idx}`} value={rawItem}>
                {rawItem}
              </option>
            );
          }

          const item = rawItem as SelectCatalogItem;
          const itemCode = getItemCode(rawItem, idx);
          const itemDesc = item.label || item.descripcion || item.nombre || "";
          const itemSimbolo = item.simbolo;

          let optionContent: React.ReactNode;
          if (renderOption) {
            optionContent = renderOption(rawItem);
          } else if (item.label !== undefined) {
            optionContent = item.label;
          } else if (itemSimbolo && item.codigo !== undefined) {
            optionContent = `${item.codigo} (${itemSimbolo}) - ${itemDesc}`;
          } else if (itemSimbolo && item.codigo === undefined) {
            optionContent = `${itemSimbolo} - ${itemDesc}`;
          } else if (itemDesc && itemDesc !== itemCode) {
            optionContent = `${itemCode} - ${itemDesc}`;
          } else {
            optionContent = itemCode || itemDesc;
          }

          return (
            <option key={`${itemCode}-${idx}`} value={itemCode}>
              {optionContent}
            </option>
          );
        })}
      </Select>
    </div>
  );
}