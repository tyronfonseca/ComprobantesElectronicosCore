import React, { useMemo, useState } from "react";
import type { NombreCatalogo } from "@cec-cr/core";
import { Card, CardHeader, CardTitle, CardDescription, CardContent } from "@/components/ui/card";
import { SelectCatalog } from "@/components/inputs/SelectCatalog";
import { Label } from "@/components/ui/label";
import { Table, TableHeader, TableBody, TableRow, TableHead, TableCell } from "@/components/ui/table";
import { Database } from "lucide-react";
import { core } from "@/store/comprobanteStore";

export function CatalogosOficialesCard() {
  const catalogos = core().catalogos;
  const nombresCatalogos = useMemo(() => catalogos.nombres(), [catalogos]);
  const [catalogoSeleccionado, setCatalogo] = useState<NombreCatalogo>(nombresCatalogos[0]);
  const elementosCatalogo = useMemo(() => catalogos.porNombre(catalogoSeleccionado), [catalogos, catalogoSeleccionado]);

  return (
    <Card>
      <CardHeader>
        <CardTitle className="text-base flex items-center gap-2">
          <Database className="w-4 h-4 text-blue-600" />
          Catálogos del Anexo 1 v4.4
        </CardTitle>
        <CardDescription>
          Los mismos catálogos que valida el core
        </CardDescription>
      </CardHeader>
      <CardContent className="space-y-4">
        <div>
          <Label>Seleccionar Catálogo Oficial</Label>
          <SelectCatalog
            catalog={nombresCatalogos}
            value={catalogoSeleccionado}
            onChange={val => setCatalogo(String(val) as NombreCatalogo)}
          />
        </div>

        <div className="max-h-115 overflow-y-auto border rounded-lg">
          <Table>
            <TableHeader>
              <TableRow>
                <TableHead className="w-24">Código</TableHead>
                <TableHead>Descripción Oficial</TableHead>
              </TableRow>
            </TableHeader>
            <TableBody>
              {elementosCatalogo.map(elem => (
                <TableRow key={elem.codigo}>
                  <TableCell className="font-mono font-semibold">{elem.codigo}</TableCell>
                  <TableCell>{elem.descripcion}</TableCell>
                </TableRow>
              ))}
            </TableBody>
          </Table>
        </div>
      </CardContent>
    </Card>
  );
}

