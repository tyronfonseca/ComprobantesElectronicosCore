/**
 * Tipos del request y del resultado de un comprobante electrónico v4.4.
 *
 * El request es el mismo JSON que define schemas/comprobante_request.schema.json: solo datos de entrada
 * (los montos calculados los produce el motor). Cada campo se llama como su etiqueta del Anexo 1 en camelCase
 * y las listas van en plural. La obligatoriedad según el tipo de comprobante la valida el motor de reglas.
 */

// ── Valores ──────────────────────────────────────────────────────────────────────────────────────

/** Monto o cantidad. Preferiblemente texto ("1500.50") para no perder precisión; también se acepta número. */
export type Decimal = string | number;

// ── Códigos de catálogo (Anexo 1, notas) ─────────────────────────────────────────────────────────

/** Nota 3: 01 FE, 02 ND, 03 NC, 04 TE, 08 FEC, 09 FEE, 10 REP. */
export type TipoDocumento = "01" | "02" | "03" | "04" | "08" | "09" | "10";

/** Nota 4: 01 física, 02 jurídica, 03 DIMEX, 04 NITE, 05 extranjero no domiciliado, 06 no contribuyente. */
export type TipoIdentificacion = "01" | "02" | "03" | "04" | "05" | "06";

/** Nota 5. */
export type CondicionVenta =
    | "01" | "02" | "03" | "04" | "05" | "06" | "07" | "08" | "09" | "10"
    | "11" | "12" | "13" | "14" | "15" | "99";

/** Nota 6. */
export type TipoMedioPago = "01" | "02" | "03" | "04" | "05" | "06" | "07" | "99";

/** Nota 8. */
export type CodigoImpuesto = "01" | "02" | "03" | "04" | "05" | "06" | "07" | "08" | "12" | "99";

/** Nota 8.1. */
export type CodigoTarifaIVA = "01" | "02" | "03" | "04" | "05" | "06" | "07" | "08" | "09" | "10" | "11";

/** Nota 9. */
export type CodigoReferencia =
    | "01" | "02" | "04" | "05" | "06" | "07" | "08" | "09" | "10"
    | "11" | "12" | "13" | "14" | "15" | "16" | "17" | "99";

/** Nota 10. */
export type TipoDocumentoReferencia =
    | "01" | "02" | "03" | "04" | "05" | "06" | "07" | "08" | "09" | "10"
    | "11" | "12" | "13" | "14" | "15" | "16" | "17" | "18" | "19" | "20" | "99";

/** Nota 10.1. */
export type TipoDocumentoExoneracion = "01" | "02" | "03" | "04" | "05" | "06" | "07" | "08" | "09" | "10" | "11" | "99";

/** Nota 12. */
export type TipoCodigoComercial = "01" | "02" | "03" | "04" | "99";

/** Nota 16. */
export type TipoOtrosCargos = "01" | "02" | "03" | "04" | "05" | "06" | "07" | "08" | "09" | "10" | "99";

/** Nota 20. */
export type CodigoDescuento = "01" | "02" | "03" | "04" | "05" | "06" | "07" | "08" | "09" | "99";

/** Nota 21. */
export type IvaCobradoFabrica = "01" | "02";

/** Nota 22. */
export type TipoTransaccion = "01" | "02" | "03" | "04" | "05" | "06" | "07" | "08" | "09" | "10" | "11" | "12" | "13";

/** Nota 23. */
export type InstitucionExoneracion = "01" | "02" | "03" | "04" | "05" | "06" | "07" | "08" | "09" | "10" | "11" | "12" | "99";

/** Situación del comprobante en la clave (Nota 3): 1 normal, 2 contingencia, 3 sin internet. */
export type Situacion = "1" | "2" | "3";

// ── Request ──────────────────────────────────────────────────────────────────────────────────────

export interface Identificacion {
    tipo: TipoIdentificacion;
    numero: string;
}

/** Códigos de la Nota 14: provincia de 1 dígito, cantón y distrito de 2 ("08"). El barrio es texto. */
export interface Ubicacion {
    provincia: string;
    canton: string;
    distrito: string;
    barrio?: string;
    otrasSenas: string;
}

export interface Telefono {
    codigoPais: string;
    numTelefono: string;
}

export interface Emisor {
    nombre: string;
    identificacion: Identificacion;
    registroFiscal8707?: string;
    nombreComercial?: string;
    ubicacion?: Ubicacion;
    otrasSenasExtranjero?: string;
    telefono?: Telefono;
    /** De 1 a 4 correos. */
    correoElectronico?: string[];
}

export interface Receptor {
    nombre: string;
    identificacion?: Identificacion;
    nombreComercial?: string;
    ubicacion?: Ubicacion;
    otrasSenasExtranjero?: string;
    telefono?: Telefono;
    correoElectronico?: string;
}

export interface CodigoComercial {
    tipo: TipoCodigoComercial;
    codigo: string;
}

export interface Descuento {
    montoDescuento: Decimal;
    codigoDescuento: CodigoDescuento;
    /** Con código 99. */
    codigoDescuentoOTRO?: string;
    /** Con código 99. */
    naturalezaDescuento?: string;
}

/** Impuestos específicos no tarifarios (códigos 03, 04, 05 y 06). */
export interface DatosImpuestoEspecifico {
    cantidadUnidadMedida?: Decimal;
    porcentaje?: Decimal;
    proporcion?: Decimal;
    volumenUnidadConsumo?: Decimal;
    impuestoUnidad?: Decimal;
}

export interface Impuesto {
    codigo: CodigoImpuesto;
    /** Con código 99. */
    codigoImpuestoOTRO?: string;
    /** Con los códigos 01 y 07: la tarifa del IVA sale de este código. */
    codigoTarifaIVA?: CodigoTarifaIVA;
    /** Tarifa propia de los impuestos 02, 12 y 99 (13 = 13%). */
    tarifa?: Decimal;
    /** Con el código 08 (IVA bienes usados). */
    factorCalculoIVA?: Decimal;
    datosImpuestoEspecifico?: DatosImpuestoEspecifico;
    /** FEE, NC y ND: exportación de mercancías con impuestos especiales. */
    montoExportacion?: Decimal;
}

/** Exonera el IVA de la línea; en el XML va dentro del Impuesto de IVA. */
export interface Exoneracion {
    tipoDocumentoEX1: TipoDocumentoExoneracion;
    tipoDocumentoOTRO?: string;
    numeroDocumento: string;
    /** Obligatorio con los tipos 02, 03, 06, 07 y 08. */
    articulo?: number;
    inciso?: number;
    nombreInstitucion: InstitucionExoneracion;
    nombreInstitucionOtros?: string;
    fechaEmisionEX: string;
    tarifaExonerada: Decimal;
}

export interface LineaDetalleSurtido {
    codigoCABYS: string;
    codigosComerciales?: CodigoComercial[];
    cantidad: Decimal;
    unidadMedida: string;
    unidadMedidaComercial?: string;
    detalle: string;
    precioUnitario: Decimal;
    descuentos?: Descuento[];
    impuestos: Impuesto[];
}

export interface LineaDetalle {
    /** Correlativo 1..N; si se omite se numera según la posición. */
    numeroLinea?: number;
    /** FEE: obligatoria para bienes (12 dígitos). */
    partidaArancelaria?: string;
    codigoCABYS?: string;
    /** Hasta 5. */
    codigosComerciales?: CodigoComercial[];
    cantidad?: Decimal;
    unidadMedida?: string;
    tipoTransaccion?: TipoTransaccion;
    unidadMedidaComercial?: string;
    detalle: string;
    numerosVINoSerie?: string[];
    registroMedicamento?: string;
    formaFarmaceutica?: string;
    /** Componentes de un surtido, combo o paquete (hasta 20). */
    detalleSurtido?: LineaDetalleSurtido[];
    precioUnitario?: Decimal;
    /** Solo REP: monto a pagar de lo facturado, sin impuestos. */
    montoTotal?: Decimal;
    /** Hasta 5. */
    descuentos?: Descuento[];
    /** Solo REP: monto del pago a registrar para el cálculo del IVA. */
    subTotal?: Decimal;
    ivaCobradoFabrica?: IvaCobradoFabrica;
    /** Solo cuando es editable (IVACobradoFabrica 01 o impuesto 07). */
    baseImponible?: Decimal;
    impuestos?: Impuesto[];
    impuestoAsumidoEmisorFabrica?: Decimal;
    exoneracion?: Exoneracion;
}

export interface OtrosCargos {
    tipoDocumentoOC: TipoOtrosCargos;
    tipoDocumentoOTROS?: string;
    identificacionTercero?: Identificacion;
    nombreTercero?: string;
    detalle: string;
    porcentajeOC?: Decimal;
    montoCargo: Decimal;
}

export interface MedioPago {
    tipoMedioPago: TipoMedioPago;
    medioPagoOtros?: string;
    /** Obligatorio con dos o más medios de pago (deben sumar el total del comprobante). */
    totalMedioPago?: Decimal;
}

export interface InformacionReferencia {
    tipoDocIR: TipoDocumentoReferencia;
    tipoDocRefOTRO?: string;
    numero?: string;
    fechaEmisionIR: string;
    codigo?: CodigoReferencia;
    codigoReferenciaOTRO?: string;
    razon?: string;
}

export interface ComprobanteRequest {
    tipoDocumento: TipoDocumento;
    clave: string;
    proveedorSistemas: string;
    codigoActividadEmisor?: string;
    codigoActividadReceptor?: string;
    numeroConsecutivo: string;
    /** RFC 3339, ej. "2026-10-01T10:00:00-06:00". */
    fechaEmision: string;
    emisor: Emisor;
    receptor?: Receptor;
    condicionVenta: CondicionVenta;
    condicionVentaOtros?: string;
    /** Días; obligatorio con condición de venta 02 (crédito). */
    plazoCredito?: number;
    detalleServicio?: LineaDetalle[];
    otrosCargos?: OtrosCargos[];
    resumenFactura: {
        codigoTipoMoneda: { codigoMoneda: string; tipoCambio?: Decimal };
        mediosPago?: MedioPago[];
    };
    informacionReferencia?: InformacionReferencia[];
}

// ── Resultado ────────────────────────────────────────────────────────────────────────────────────

export interface Diagnostico {
    severidad: "error" | "advertencia";
    /** Código estable, ej. "RULE_CABYS_FORMAT". */
    codigo: string;
    /** Ruta del campo en el request, ej. "detalleServicio[0].impuestos[1].codigo". */
    ruta: string;
    mensaje: string;
}

/** Totales del ResumenFactura como decimales en texto (5 decimales); los condicionales en cero no vienen. */
export interface ResumenFacturaSalida {
    CodigoTipoMoneda: { CodigoMoneda: string; TipoCambio: string };
    TotalServGravados?: string;
    TotalServExentos?: string;
    TotalServExonerado?: string;
    TotalServNoSujeto?: string;
    TotalMercanciasGravadas?: string;
    TotalMercanciasExentas?: string;
    TotalMercExonerada?: string;
    TotalMercNoSujeta?: string;
    TotalGravado?: string;
    TotalExento?: string;
    TotalExonerado?: string;
    TotalNoSujeto?: string;
    TotalVenta: string;
    TotalDescuentos?: string;
    TotalVentaNeta: string;
    TotalDesgloseImpuesto?: { Codigo: string; CodigoTarifaIVA?: string; TotalMontoImpuesto: string }[];
    TotalImpuesto?: string;
    TotalImpAsumEmisorFabrica?: string;
    TotalIVADevuelto?: string;
    TotalOtrosCargos?: string;
    MedioPago?: { TipoMedioPago: string; MedioPagoOtros?: string; TotalMedioPago?: string }[];
    TotalComprobante: string;
}

/**
 * Comprobante calculado con la misma estructura y etiquetas del XML v4.4. Los nodos que no son el resumen
 * van sin tipar en detalle: su forma la define el Anexo 1.
 */
export interface ComprobanteSalida {
    Clave: string;
    NumeroConsecutivo: string;
    FechaEmision: string;
    ResumenFactura: ResumenFacturaSalida;
    [etiqueta: string]: unknown;
}

export interface ResultadoComprobante {
    /** true si no hubo errores (puede haber advertencias). */
    exito: boolean;
    diagnosticos: Diagnostico[];
    /** XML sin firmar; solo si exito. */
    xml?: string;
    /** Comprobante calculado; solo si exito. */
    comprobante?: ComprobanteSalida;
}

/**
 * Vista previa del cálculo: el comprobante viene aunque haya errores de reglas (no si hay errores de formato),
 * para mostrar montos mientras se edita. Nunca trae XML: para emitir se usa procesar.
 */
export interface ResultadoCalculo {
    /** true si no hubo errores. */
    exito: boolean;
    diagnosticos: Diagnostico[];
    comprobante?: ComprobanteSalida;
}

export interface Certificado {
    sujeto: string;
    emisor: string;
    numeroSerie: string;
    /** ISO 8601 UTC. */
    validoDesde: string;
    /** ISO 8601 UTC. */
    validoHasta: string;
    sha256Hex: string;
    sha256Base64: string;
    /** X.509 DER en Base64. */
    certificadoBase64: string;
    /** friendlyName del PKCS#12: alias de la llave al cargarlo en un keystore ("" si no tiene). */
    alias: string;
    /** La fecha actual está dentro de la vigencia. */
    vigente: boolean;
}

export interface ResultadoFirma {
    exito: boolean;
    diagnosticos: Diagnostico[];
    /** XML firmado (XAdES-EPES); solo si exito. */
    xml?: string;
    certificado?: Certificado;
}

/** Paso 1 de la firma en dos pasos (prepararFirma). */
export interface ResultadoFirmaPreparada {
    exito: boolean;
    diagnosticos: Diagnostico[];
    /** XML con la firma XAdES-EPES armada y SignatureValue vacío; solo si exito. */
    xml?: string;
    /** SignedInfo canonicalizado (C14N): se firma con RSA-SHA256 y el resultado va a completarFirma. */
    signedInfo?: string;
    certificado?: Certificado;
}

export interface ResultadoCertificado {
    exito: boolean;
    diagnosticos: Diagnostico[];
    certificado?: Certificado;
}

// ── Mensaje Receptor ─────────────────────────────────────────────────────────────────────────────

/** 1 aceptado, 2 aceptado parcialmente, 3 rechazado. */
export type CodigoMensajeReceptor = "1" | "2" | "3";

/**
 * Datos del Mensaje Receptor (MensajeReceptor_V4.4.xsd) en camelCase. Los montos son texto decimal.
 * `fechaEmisionComprobante` (opcional) es la FechaEmision del comprobante recibido: con ella se revisa el plazo del art. 10.
 */
export interface MensajeReceptorRequest {
    clave: string;
    numeroCedulaEmisor: string;
    fechaEmisionDoc: string;
    mensaje: CodigoMensajeReceptor;
    detalleMensaje?: string;
    montoTotalImpuesto?: string;
    codigoActividad?: string;
    /** Nota 18: 01 crédito IVA, 02 crédito parcial, 03 bienes de capital, 04 gasto corriente, 05 proporcionalidad. */
    condicionImpuesto?: string;
    montoTotalImpuestoAcreditar?: string;
    montoTotalDeGastoAplicable?: string;
    totalFactura: string;
    numeroCedulaReceptor: string;
    /** 20 dígitos; el tipo (posiciones 9 y 10) es 05, 06 o 07 según el mensaje. */
    numeroConsecutivoReceptor: string;
    fechaEmisionComprobante?: string;
}

export interface ResultadoMensajeReceptor {
    exito: boolean;
    diagnosticos: Diagnostico[];
    /** XML sin firmar; solo si exito. */
    xml?: string;
    /** Último día del plazo del art. 10 (AAAA-MM-DD), si se indicó fechaEmisionComprobante. */
    plazoConfirmacion?: string;
}

/** Datos del XML de un comprobante recibido. */
export interface ComprobanteRecibido {
    tipoDocumento: string;
    nombreDocumento: string;
    version: string;
    clave: string;
    numeroConsecutivo: string;
    fechaEmision: string;
    emisor: { nombre: string; tipoIdentificacion: string; numeroIdentificacion: string; correo: string };
    receptor: { nombre: string; tipoIdentificacion: string; numeroIdentificacion: string };
    codigoActividadReceptor: string;
    codigoMoneda: string;
    tipoCambio: string;
    totalImpuesto: string;
    totalComprobante: string;
    firmado: boolean;
    /** Último día para confirmarlo (AAAA-MM-DD); vacío si la fecha de emisión no es válida. */
    plazoConfirmacion: string;
}

export interface ResultadoLecturaComprobante {
    /** false si no se puede confirmar con un Mensaje Receptor (ver diagnosticos). */
    exito: boolean;
    diagnosticos: Diagnostico[];
    comprobante?: ComprobanteRecibido;
}
