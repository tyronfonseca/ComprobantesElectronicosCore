import React, { useMemo, useState } from "react";
import type { ComprobanteRequest, IvaCobradoFabrica, LineaDetalle, TipoTransaccion } from "@cec-cr/core";
import { Card, CardHeader, CardTitle, CardDescription, CardContent } from "@/components/ui/card";
import { Button } from "@/components/ui/button";
import { Badge } from "@/components/ui/badge";
import { Dialog } from "@/components/ui/dialog";
import { ShoppingBag, Plus, Check, RotateCcw } from "lucide-react";
import { Campo, CampoCatalogo, ErroresContext, decimalDeInput } from "@/components/common/Campo";
import { core, limpiar, useCatalogos, useFacturacionStore } from "@/store/comprobanteStore";
import { descuentoLinea, formatoMonto, impuestosLinea, lineasSalida } from "@/lib/core";
import { EditorCodigosComerciales, EditorDescuentos, EditorExoneracion, EditorImpuestos, EditorSurtido } from "./lineas/Editores";
import { LineasDetalleTable } from "./lineas/LineasDetalleTable";

type Panel = "impuestos" | "exoneracion" | "descuentos" | "codigos" | "surtido" | null;

function lineaNueva(esREP: boolean): LineaDetalle {
  return esREP
    ? { detalle: "", montoTotal: "", impuestos: [{ codigo: "01", codigoTarifaIVA: "08" }] }
    : { codigoCABYS: "", cantidad: "1", unidadMedida: "Unid", detalle: "", precioUnitario: "",
        impuestos: [{ codigo: "01", codigoTarifaIVA: "08" }] };
}

/** Vista previa del comprobante con la línea en edición: montos y diagnósticos de esa línea. */
function usePreviaLinea(request: ComprobanteRequest, borrador: LineaDetalle, indice: number) {
  return useMemo(() => {
    const lineas = [...(request.detalleServicio ?? [])];
    lineas[indice] = borrador;
    const r = core().calcular(limpiar({ ...request, detalleServicio: lineas }) as ComprobanteRequest);
    const prefijo = `detalleServicio[${indice}]`;
    const diagnosticos = r.diagnosticos.filter((d) => d.ruta === prefijo || d.ruta.startsWith(prefijo + "."));
    return {
      linea: lineasSalida(r.comprobante)[indice],
      diagnosticos,
      error: (ruta: string) => diagnosticos.find((d) => d.ruta === ruta)?.mensaje,
    };
  }, [request, borrador, indice]);
}

export function LineasDetalleForm() {
  const c = useCatalogos();
  const request = useFacturacionStore((s) => s.request);
  const editar = useFacturacionStore((s) => s.editar);
  const lineas = request.detalleServicio ?? [];
  const tipo = request.tipoDocumento;
  const esREP = tipo === "10";
  const moneda = request.resumenFactura.codigoTipoMoneda.codigoMoneda;

  const [borrador, setBorrador] = useState<LineaDetalle>(() => lineaNueva(esREP));
  const [editando, setEditando] = useState<number | null>(null);
  const [panel, setPanel] = useState<Panel>(null);
  const [mostrarErrores, setMostrarErrores] = useState(false);

  const indice = editando ?? lineas.length;
  const ruta = `detalleServicio[${indice}]`;
  const previa = usePreviaLinea(request, borrador, indice);
  const { iva, otros, exonerado } = impuestosLinea(previa.linea);

  const set = (fn: (l: LineaDetalle) => void) =>
    setBorrador((prev) => {
      const copia = structuredClone(prev);
      fn(copia);
      return copia;
    });

  const limpiarFormulario = () => {
    setBorrador(lineaNueva(esREP));
    setEditando(null);
    setMostrarErrores(false);
  };

  const guardar = () => {
    if (previa.diagnosticos.some((d) => d.severidad === "error")) {
      setMostrarErrores(true);
      return;
    }
    editar((d) => {
      const lista = [...(d.detalleServicio ?? [])];
      lista[indice] = borrador;
      d.detalleServicio = lista;
    });
    limpiarFormulario();
  };

  const editarFila = (i: number) => {
    setBorrador(structuredClone(lineas[i]));
    setEditando(i);
    setMostrarErrores(true);
  };

  const quitarFila = (i: number) => {
    editar((d) => {
      // Sin numeroLinea explícito el core numera según la posición.
      d.detalleServicio = (d.detalleServicio ?? []).filter((_, j) => j !== i).map(({ numeroLinea, ...l }) => l);
    });
    if (editando === i) limpiarFormulario();
  };

  const tieneSurtido = (borrador.detalleSurtido?.length ?? 0) > 0;
  const admitePartida = ["09", "02", "03"].includes(tipo);

  return (
    <Card className="overflow-hidden">
      <CardHeader className="bg-slate-50/70 border-b border-slate-200/80 py-4">
        <div className="flex items-center justify-between">
          <div className="flex items-center gap-2">
            <ShoppingBag className="w-5 h-5 text-blue-600" />
            <div>
              <CardTitle className="text-base font-bold">4. Detalle del servicio o mercancía</CardTitle>
              <CardDescription className="text-xs">Los montos los calcula el core en cada cambio</CardDescription>
            </div>
          </div>
          <Badge variant="secondary" className="font-mono text-xs">{lineas.length} {lineas.length === 1 ? "línea" : "líneas"}</Badge>
        </div>
      </CardHeader>

      <CardContent className="p-4 sm:p-6 space-y-6">
        <ErroresContext.Provider value={mostrarErrores ? previa.error : () => undefined}>
          <div className="p-4 bg-slate-50 border border-slate-200 rounded-xl space-y-4">
            <div className="flex items-center justify-between">
              <span className="text-xs font-bold text-slate-800 uppercase tracking-wider">
                {editando !== null ? `Editando línea ${editando + 1}` : `Nueva línea ${lineas.length + 1}`}
              </span>
              {editando !== null && <Badge variant="warning" className="text-xs">Modo edición</Badge>}
            </div>

            {esREP ? (
              <div className="grid grid-cols-1 sm:grid-cols-4 gap-3">
                <Campo className="sm:col-span-2" label="Detalle del pago" ruta={`${ruta}.detalle`} value={borrador.detalle}
                  onChange={(v) => set((l) => { l.detalle = v; })} />
                <Campo label={`Monto pagado sin impuestos (${moneda})`} ruta={`${ruta}.montoTotal`} value={borrador.montoTotal}
                  onChange={(v) => set((l) => { l.montoTotal = decimalDeInput(v); })} />
                <Campo label="Monto para el IVA (opcional)" ruta={`${ruta}.subTotal`} value={borrador.subTotal}
                  onChange={(v) => set((l) => { l.subTotal = decimalDeInput(v); })} />
              </div>
            ) : (
              <>
                <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-4 gap-3">
                  <Campo label="CABYS (13 dígitos)" ruta={`${ruta}.codigoCABYS`} value={borrador.codigoCABYS} maxLength={13}
                    className="font-mono" onChange={(v) => set((l) => { l.codigoCABYS = v; })} />
                  <CampoCatalogo label="Unidad de medida" ruta={`${ruta}.unidadMedida`} value={borrador.unidadMedida}
                    catalogo={c.unidadesMedida} onChange={(v) => set((l) => { l.unidadMedida = v; })} />
                  <Campo label="Cantidad" ruta={`${ruta}.cantidad`} value={borrador.cantidad}
                    onChange={(v) => set((l) => { l.cantidad = decimalDeInput(v); })} />
                  <Campo label={`Precio unitario (${moneda})`} ruta={`${ruta}.precioUnitario`} value={borrador.precioUnitario}
                    placeholder={tieneSurtido ? "Suma de los subtotales de los componentes" : ""}
                    onChange={(v) => set((l) => { l.precioUnitario = decimalDeInput(v); })} />
                </div>
                <div className="grid grid-cols-1 sm:grid-cols-4 gap-3">
                  <Campo className="sm:col-span-2" label="Detalle" ruta={`${ruta}.detalle`} value={borrador.detalle}
                    onChange={(v) => set((l) => { l.detalle = v; })} />
                  <CampoCatalogo label="Tipo de transacción (opcional)" ruta={`${ruta}.tipoTransaccion`} value={borrador.tipoTransaccion}
                    catalogo={c.tiposTransaccion} placeholder="Sin especificar"
                    onChange={(v) => set((l) => { l.tipoTransaccion = (v || undefined) as TipoTransaccion | undefined; })} />
                  <Campo label="Unidad comercial (opcional)" ruta={`${ruta}.unidadMedidaComercial`} value={borrador.unidadMedidaComercial}
                    onChange={(v) => set((l) => { l.unidadMedidaComercial = v; })} />
                </div>

                <details className="rounded-lg border border-slate-200 bg-white p-3">
                  <summary className="cursor-pointer text-xs font-semibold text-slate-700">
                    Bienes especiales y opciones avanzadas
                  </summary>
                  <div className="grid grid-cols-1 sm:grid-cols-3 gap-3 mt-3">
                    {admitePartida && (
                      <Campo label="Partida arancelaria (12 dígitos)" ruta={`${ruta}.partidaArancelaria`}
                        value={borrador.partidaArancelaria} maxLength={12}
                        onChange={(v) => set((l) => { l.partidaArancelaria = v; })} />
                    )}
                    <Campo className="sm:col-span-2" label="VIN o número de serie (separados por coma)"
                      ruta={`${ruta}.numerosVINoSerie`} value={(borrador.numerosVINoSerie ?? []).join(", ")}
                      onChange={(v) => set((l) => { l.numerosVINoSerie = v.split(",").map((x) => x.trim()).filter(Boolean); })} />
                    <Campo label="Registro de medicamento" ruta={`${ruta}.registroMedicamento`} value={borrador.registroMedicamento}
                      onChange={(v) => set((l) => { l.registroMedicamento = v; })} />
                    <CampoCatalogo label="Forma farmacéutica" ruta={`${ruta}.formaFarmaceutica`} value={borrador.formaFarmaceutica}
                      catalogo={c.formasFarmaceuticas} placeholder="No aplica"
                      onChange={(v) => set((l) => { l.formaFarmaceutica = v; })} />
                    <CampoCatalogo label="IVA cobrado a nivel de fábrica (Nota 21)" ruta={`${ruta}.ivaCobradoFabrica`}
                      value={borrador.ivaCobradoFabrica} catalogo={c.ivaCobradoFabrica} placeholder="No aplica"
                      onChange={(v) => set((l) => { l.ivaCobradoFabrica = (v || undefined) as IvaCobradoFabrica | undefined; })} />
                    <Campo label="Base imponible (solo si es editable)" ruta={`${ruta}.baseImponible`} value={borrador.baseImponible}
                      onChange={(v) => set((l) => { l.baseImponible = decimalDeInput(v); })} />
                    <Campo label="Impuesto asumido por emisor o fábrica" ruta={`${ruta}.impuestoAsumidoEmisorFabrica`}
                      value={borrador.impuestoAsumidoEmisorFabrica}
                      onChange={(v) => set((l) => { l.impuestoAsumidoEmisorFabrica = decimalDeInput(v); })} />
                  </div>
                </details>
              </>
            )}

            <div className="flex flex-wrap gap-2">
              <Button type="button" variant="outline" size="sm" className="text-xs h-8 bg-white" onClick={() => setPanel("impuestos")}>
                Impuestos <Badge variant="secondary" className="ml-1.5 text-[10px] py-0">{borrador.impuestos?.length ?? 0}</Badge>
              </Button>
              {!esREP && (
                <>
                  <Button type="button" variant="outline" size="sm" className="text-xs h-8 bg-white" onClick={() => setPanel("exoneracion")}>
                    Exoneración {borrador.exoneracion && <Badge variant="warning" className="ml-1.5 text-[10px] py-0">Sí</Badge>}
                  </Button>
                  <Button type="button" variant="outline" size="sm" className="text-xs h-8 bg-white" onClick={() => setPanel("descuentos")}>
                    Descuentos <Badge variant="secondary" className="ml-1.5 text-[10px] py-0">{borrador.descuentos?.length ?? 0}</Badge>
                  </Button>
                  <Button type="button" variant="outline" size="sm" className="text-xs h-8 bg-white" onClick={() => setPanel("codigos")}>
                    Códigos comerciales <Badge variant="secondary" className="ml-1.5 text-[10px] py-0">{borrador.codigosComerciales?.length ?? 0}</Badge>
                  </Button>
                  <Button type="button" variant="outline" size="sm" className="text-xs h-8 bg-white" onClick={() => setPanel("surtido")}>
                    Combo o surtido <Badge variant="secondary" className="ml-1.5 text-[10px] py-0">{borrador.detalleSurtido?.length ?? 0}</Badge>
                  </Button>
                </>
              )}
            </div>

            <div className="grid grid-cols-2 sm:grid-cols-5 gap-3 text-xs font-mono">
              {[
                ["Monto total", previa.linea?.MontoTotal],
                ["Descuentos", previa.linea ? -descuentoLinea(previa.linea) : undefined],
                ["IVA", previa.linea ? iva - exonerado : undefined],
                ["Otros impuestos", previa.linea ? otros : undefined],
              ].map(([titulo, valor]) => (
                <div key={titulo as string} className="rounded-md border border-slate-200 bg-white px-3 py-2">
                  <div className="text-[10px] font-sans font-semibold text-slate-500 uppercase">{titulo}</div>
                  <div>{valor === undefined ? "—" : formatoMonto(valor)}</div>
                </div>
              ))}
              <div className="rounded-md border border-blue-200 bg-blue-50 px-3 py-2 font-bold text-blue-800">
                <div className="text-[10px] font-sans font-semibold uppercase">Total línea</div>
                <div>{previa.linea ? formatoMonto(previa.linea.MontoTotalLinea) : "—"}</div>
              </div>
            </div>
            {mostrarErrores && previa.diagnosticos.some((d) => d.severidad === "error") && (
              <ul className="text-[11px] text-red-600 list-disc pl-5">
                {previa.diagnosticos.filter((d) => d.severidad === "error").map((d, i) => (
                  <li key={i}>{d.mensaje} <span className="font-mono text-red-400">({d.ruta.replace(ruta + ".", "")})</span></li>
                ))}
              </ul>
            )}

            <div className="flex items-center justify-end gap-3 pt-2 border-t border-slate-200">
              <Button type="button" variant="outline" size="sm" onClick={limpiarFormulario} className="text-xs h-9">
                <RotateCcw className="w-3.5 h-3.5 mr-1.5" /> Limpiar
              </Button>
              <Button type="button" size="sm" onClick={guardar} className="text-xs h-9 bg-blue-600 hover:bg-blue-700 gap-1.5 px-4">
                {editando !== null ? <><Check className="w-3.5 h-3.5" /> Actualizar línea</> : <><Plus className="w-3.5 h-3.5" /> Agregar línea</>}
              </Button>
            </div>
          </div>

          <Dialog open={panel !== null} onOpenChange={(o) => !o && setPanel(null)} maxWidth="2xl"
            title={{
              impuestos: "Impuestos de la línea", exoneracion: "Exoneración del IVA", descuentos: "Descuentos (hasta 5)",
              codigos: "Códigos comerciales (hasta 5)", surtido: "Componentes del combo o surtido (hasta 20)",
            }[panel ?? "impuestos"]}
            footer={<Button size="sm" className="bg-blue-600 hover:bg-blue-700" onClick={() => setPanel(null)}>Listo</Button>}>
            {panel === "impuestos" && (
              <EditorImpuestos ruta={`${ruta}.impuestos`} valor={borrador.impuestos ?? []}
                onChange={(v) => set((l) => { l.impuestos = v; })} />
            )}
            {panel === "exoneracion" && (
              <EditorExoneracion ruta={`${ruta}.exoneracion`} valor={borrador.exoneracion}
                onChange={(v) => set((l) => { l.exoneracion = v; })} />
            )}
            {panel === "descuentos" && (
              <EditorDescuentos ruta={`${ruta}.descuentos`} valor={borrador.descuentos ?? []}
                onChange={(v) => set((l) => { l.descuentos = v; })} />
            )}
            {panel === "codigos" && (
              <EditorCodigosComerciales ruta={`${ruta}.codigosComerciales`} valor={borrador.codigosComerciales ?? []}
                onChange={(v) => set((l) => { l.codigosComerciales = v; })} />
            )}
            {panel === "surtido" && (
              <EditorSurtido ruta={`${ruta}.detalleSurtido`} valor={borrador.detalleSurtido ?? []}
                onChange={(v) => set((l) => { l.detalleSurtido = v; })} />
            )}
          </Dialog>
        </ErroresContext.Provider>

        <LineasDetalleTable editando={editando} onEditar={editarFila} onQuitar={quitarFila} />
      </CardContent>
    </Card>
  );
}
