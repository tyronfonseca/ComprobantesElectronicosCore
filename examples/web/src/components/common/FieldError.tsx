import React from "react";
import { AlertTriangle } from "lucide-react";

interface FieldErrorProps {
  error?: string | null;
}

export function FieldError({ error }: FieldErrorProps) {
  if (!error) return null;
  return (
    <p className="text-[11px] text-red-600 mt-1 font-medium flex items-center gap-1 animate-fadeIn">
      <AlertTriangle className="w-3 h-3 inline shrink-0" />
      {error}
    </p>
  );
}

