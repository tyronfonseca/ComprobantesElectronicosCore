import React, { useState, useRef } from "react";
import { Dialog } from "@/components/ui/dialog";
import { Button } from "@/components/ui/button";
import { Input } from "@/components/ui/input";
import { Label } from "@/components/ui/label";
import { Alert, AlertTitle, AlertDescription } from "@/components/ui/alert";
import { Badge } from "@/components/ui/badge";
import { Card, CardContent } from "@/components/ui/card";
import {
  KeyRound,
  ShieldCheck,
  Lock,
  Upload,
  AlertTriangle,
  CheckCircle2,
  FileCheck2,
  Eye,
  EyeOff
} from "lucide-react";
import { useFacturacionStore } from "@/store/comprobanteStore";
import type { Certificado } from "@cec-cr/core";

export function FirmarXmlModal() {
  const [open, setOpen] = useState(false);
  const [pin, setPin] = useState("");
  const [showPin, setShowPin] = useState(false);
  const [p12File, setP12File] = useState<File | null>(null);
  const [p12Buffer, setP12Buffer] = useState<ArrayBuffer | null>(null);
  const [certInfo, setCertInfo] = useState<Certificado | null>(null);
  const [errorMsg, setErrorMsg] = useState<string | null>(null);
  const [successMsg, setSuccessMsg] = useState<string | null>(null);
  const [isProcessing, setIsProcessing] = useState(false);
  const fileInputRef = useRef<HTMLInputElement>(null);

  const esXmlFirmado = useFacturacionStore((state) => state.xmlFirmado);
  const firmar = useFacturacionStore((state) => state.firmar);
  const leerCertificado = useFacturacionStore((state) => state.leerCertificado);

  const handleFileChange = async (e: React.ChangeEvent<HTMLInputElement>) => {
    setErrorMsg(null);
    setSuccessMsg(null);
    setCertInfo(null);
    const file = e.target.files?.[0];
    if (!file) return;

    setP12File(file);
    try {
      const buffer = await file.arrayBuffer();
      setP12Buffer(buffer);
      if (pin.length >= 4) {
        inspectCertificate(buffer, pin);
      }
    } catch {
      setErrorMsg("Error al leer el archivo de llave criptográfica (.p12/.pfx).");
    }
  };

  const handlePinChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    const val = e.target.value;
    setPin(val);
    setErrorMsg(null);
    setSuccessMsg(null);
    if (p12Buffer && val.length >= 4) {
      inspectCertificate(p12Buffer, val);
    }
  };

  const inspectCertificate = (buffer: ArrayBuffer, pinVal: string) => {
    const res = leerCertificado(buffer, pinVal);
    setCertInfo(res.certificado ?? null);
    setErrorMsg(res.error ?? null);
  };

  const handleEjecutarFirma = () => {
    if (!p12Buffer) {
      setErrorMsg("Seleccione un archivo de certificado digital (.p12 / .pfx).");
      return;
    }
    if (!pin) {
      setErrorMsg("Ingrese el PIN de seguridad del certificado.");
      return;
    }

    setIsProcessing(true);
    setErrorMsg(null);
    setSuccessMsg(null);

    try {
      const res = firmar(p12Buffer, pin);
      if (res.tipo === "exito") {
        setSuccessMsg(res.texto);
        setTimeout(() => setOpen(false), 1500);
      } else {
        setErrorMsg(res.texto);
      }
    } catch (e: any) {
      setErrorMsg(e.message || "Error al procesar la firma en WebAssembly.");
    } finally {
      setIsProcessing(false);
    }
  };

  const handleReset = () => {
    setP12File(null);
    setP12Buffer(null);
    setCertInfo(null);
    setPin("");
    setErrorMsg(null);
    setSuccessMsg(null);
    if (fileInputRef.current) {
      fileInputRef.current.value = "";
    }
  };

  return (
    <>
      <Button
        size="sm"
        variant={esXmlFirmado ? "secondary" : "default"}
        onClick={() => setOpen(true)}
        className={`h-7 text-xs gap-1.5 font-medium shadow-sm ${
          esXmlFirmado
            ? "bg-slate-800 text-emerald-400 hover:bg-slate-700 border border-slate-600"
            : "bg-indigo-600 hover:bg-indigo-700 text-white"
        }`}
      >
        {esXmlFirmado ? (
          <>
            <ShieldCheck className="w-3.5 h-3.5 text-emerald-400" />
            Documento Firmado
          </>
        ) : (
          <>
            <KeyRound className="w-3.5 h-3.5" />
            Firmar XML
          </>
        )}
      </Button>

      <Dialog
        open={open}
        onOpenChange={setOpen}
        title="Firma Digital XAdES-EPES (Costa Rica v4.4)"
        description="Firme digitalmente su comprobante electrónico con su llave criptográfica (.p12 / .pfx). Todo el cifrado RSA-SHA256 se realiza en el cliente mediante WebAssembly (C++ Core + OpenSSL)."
        maxWidth="lg"
        footer={
          <>
            <Button variant="outline" size="sm" onClick={() => setOpen(false)} className="text-xs">
              Cerrar
            </Button>
            <Button
              size="sm"
              onClick={handleEjecutarFirma}
              disabled={!p12Buffer || pin.length === 0 || isProcessing}
              className="text-xs gap-1.5 bg-indigo-600 hover:bg-indigo-700 text-white"
            >
              <Lock className="w-3.5 h-3.5" />
              {isProcessing ? "Firmando en WebAssembly..." : "Aplicar Firma Digital"}
            </Button>
          </>
        }
      >
        <div className="space-y-4">
          {/* Alertas */}
          {errorMsg && (
            <Alert variant="destructive" className="py-2">
              <AlertTriangle className="w-4 h-4" />
              <AlertTitle className="text-xs font-semibold">Error de Firma</AlertTitle>
              <AlertDescription className="text-xs font-mono">{errorMsg}</AlertDescription>
            </Alert>
          )}

          {successMsg && (
            <Alert variant="success" className="py-2">
              <CheckCircle2 className="w-4 h-4" />
              <AlertTitle className="text-xs font-semibold">Firma Exitosa</AlertTitle>
              <AlertDescription className="text-xs">{successMsg}</AlertDescription>
            </Alert>
          )}

          {/* Carga de archivo .p12 */}
          <div className="space-y-2">
            <Label className="text-xs font-medium">Archivo Llave Criptográfica (.p12 / .pfx)</Label>
            <div className="flex gap-2">
              <div className="relative flex-1">
                <input
                  ref={fileInputRef}
                  type="file"
                  accept=".p12,.pfx"
                  onChange={handleFileChange}
                  className="hidden"
                  id="p12-upload"
                />
                <label
                  htmlFor="p12-upload"
                  className="flex items-center justify-center gap-2 px-3 py-2 border border-dashed rounded-md cursor-pointer hover:bg-slate-50 text-xs text-slate-700 bg-white w-full h-9 transition-colors"
                >
                  <Upload className="w-4 h-4 text-slate-500" />
                  <span className="truncate font-mono">
                    {p12File ? p12File.name : "Seleccionar archivo .p12 / .pfx"}
                  </span>
                </label>
              </div>
              {p12File && (
                <Button size="sm" variant="ghost" onClick={handleReset} className="h-9 text-xs px-2 text-slate-500">
                  Limpiar
                </Button>
              )}
            </div>
          </div>

          {/* PIN / Clave del certificado */}
          <div className="space-y-2">
            <Label className="text-xs font-medium">PIN del certificado</Label>
            <div className="relative">
              <Input
                type={showPin ? "text" : "password"}
                placeholder="Ej. 1234"
                value={pin}
                onChange={handlePinChange}
                maxLength={32}
                className="pr-10 h-9 font-mono text-sm tracking-widest"
              />
              <button
                type="button"
                onClick={() => setShowPin(!showPin)}
                className="absolute right-2.5 top-1/2 -translate-y-1/2 text-slate-400 hover:text-slate-600 focus:outline-none"
              >
                {showPin ? <EyeOff className="w-4 h-4" /> : <Eye className="w-4 h-4" />}
              </button>
            </div>
          </div>

          {/* Tarjeta de Información de Certificado Inspeccionado */}
          {certInfo && (
            <Card className="bg-slate-50 border-slate-200">
              <CardContent className="p-3 space-y-2">
                <div className="flex items-center justify-between">
                  <span className="text-xs font-semibold text-slate-800 flex items-center gap-1.5">
                    <FileCheck2 className="w-4 h-4 text-emerald-600" />
                    Certificado X.509 Verificado
                  </span>
                  <Badge variant={certInfo.vigente ? "success" : "destructive"} className="text-[10px] px-1.5 py-0">
                    {certInfo.vigente ? "Vigente" : "Fuera de vigencia"}
                  </Badge>
                </div>

                <div className="grid grid-cols-1 gap-1.5 text-xs">
                  <div>
                    <span className="text-slate-500 font-medium">Titular (Subject): </span>
                    <span className="font-mono text-slate-800 break-all">{certInfo.sujeto}</span>
                  </div>
                  <div>
                    <span className="text-slate-500 font-medium">Emisor (CA): </span>
                    <span className="font-mono text-slate-800 break-all">{certInfo.emisor}</span>
                  </div>
                  <div className="flex justify-between gap-2">
                    <div>
                      <span className="text-slate-500 font-medium">Serial: </span>
                      <span className="font-mono text-slate-800">{certInfo.numeroSerie}</span>
                    </div>
                    <div>
                      <span className="text-slate-500 font-medium">Vigencia: </span>
                      <span className="font-mono text-slate-700">{certInfo.validoHasta.slice(0, 10)}</span>
                    </div>
                  </div>
                  <div>
                    <span className="text-slate-500 font-medium">Huella SHA-256: </span>
                    <span className="font-mono text-[10px] text-slate-600 break-all">{certInfo.sha256Hex}</span>
                  </div>
                </div>
              </CardContent>
            </Card>
          )}

          {/* Aviso normativo de Hacienda */}
          <div className="text-[11px] text-slate-500 bg-slate-100/80 p-2.5 rounded border border-slate-200 leading-relaxed">
            Firma XAdES-EPES enveloped con RSA-SHA256 y canonicalización C14N 1.0 (Anexo 2). El certificado y el PIN no salen del navegador.
          </div>
        </div>
      </Dialog>
    </>
  );
}
