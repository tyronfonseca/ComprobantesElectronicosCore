#ifndef CEC_C_API_H
#define CEC_C_API_H

/**
 * @file cec_c_api.h
 * @brief API de C de Comprobantes Electrónicos Core (CEC): comprobantes electrónicos v4.4 de Costa Rica.
 */

#include <stdint.h>
#include <stddef.h>

#if defined(__EMSCRIPTEN__)
  #include <emscripten.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__EMSCRIPTEN__)
  #define CEC_API EMSCRIPTEN_KEEPALIVE
#elif defined(_WIN32) && !defined(cec_STATIC)
  #if defined(cec_EXPORTS) || defined(cec_BUILDING_DLL)
    #define CEC_API __declspec(dllexport)
  #else
    #define CEC_API __declspec(dllimport)
  #endif
#else
  #if defined(__GNUC__) || defined(__clang__)
    #define CEC_API __attribute__((visibility("default")))
  #else
    #define CEC_API
  #endif
#endif

typedef enum cec_status {
    CEC_STATUS_OK = 0,
    CEC_STATUS_ERR_NULL_POINTER = 1,
    CEC_STATUS_ERR_SIZE_LIMIT = 2,
    CEC_STATUS_ERR_DECODE = 3,
    CEC_STATUS_ERR_INTERNAL = 4,
    CEC_STATUS_ERR_OUT_OF_RANGE = 5
} cec_status;

/* ========================================================================= */
/* ESTRUCTURAS DE ENTRADA                                                    */
/* ========================================================================= */
/*
 * Reflejan el request JSON (schemas/comprobante_request.schema.json) con los mismos nombres: cada campo
 * es una etiqueta del Anexo 1 v4.4 en camelCase y solo hay datos de entrada (los montos los calcula el motor).
 *
 * Convenciones:
 *   - Textos, códigos de catálogo ("01") y decimales ("1500.50") son const char* UTF-8; NULL = no informado.
 *     Los decimales van como texto para no perder precisión.
 *   - Los enteros opcionales usan CEC_SIN_VALOR cuando no se informan.
 *   - Las listas son puntero + cantidad; los nodos opcionales son punteros (NULL = no informado).
 *   - La memoria es del llamador y solo se lee durante cec_build.
 * Los structs pueden cambiar entre versiones mayores del ABI (ver cec_abi_version).
 */

#define CEC_SIN_VALOR (-1)

typedef struct cec_identificacion_c {
    const char* tipo;            /* Nota 4: "01" física, "02" jurídica, "03" DIMEX, "04" NITE, "05", "06" */
    const char* numero;
} cec_identificacion_c;

typedef struct cec_ubicacion_c {
    const char* provincia;
    const char* canton;
    const char* distrito;
    const char* barrio;
    const char* otrasSenas;
} cec_ubicacion_c;

typedef struct cec_telefono_c {
    const char* codigoPais;
    const char* numTelefono;
} cec_telefono_c;

typedef struct cec_emisor_c {
    const char* nombre;
    cec_identificacion_c identificacion;
    const char* registroFiscal8707;
    const char* nombreComercial;
    const cec_ubicacion_c* ubicacion;
    const char* otrasSenasExtranjero;
    const cec_telefono_c* telefono;
    const char* const* correoElectronico; /* {1,4} */
    size_t numCorreoElectronico;
} cec_emisor_c;

typedef struct cec_receptor_c {
    const char* nombre;
    const cec_identificacion_c* identificacion;
    const char* nombreComercial;
    const cec_ubicacion_c* ubicacion;
    const char* otrasSenasExtranjero;
    const cec_telefono_c* telefono;
    const char* correoElectronico;
} cec_receptor_c;

typedef struct cec_codigo_comercial_c {
    const char* tipo;            /* Nota 12 */
    const char* codigo;
} cec_codigo_comercial_c;

typedef struct cec_descuento_c {
    const char* montoDescuento;
    const char* codigoDescuento;     /* Nota 20 */
    const char* codigoDescuentoOTRO; /* Con código 99 */
    const char* naturalezaDescuento; /* Con código 99 */
} cec_descuento_c;

typedef struct cec_datos_impuesto_especifico_c {
    const char* cantidadUnidadMedida;
    const char* porcentaje;
    const char* proporcion;
    const char* volumenUnidadConsumo;
    const char* impuestoUnidad;
} cec_datos_impuesto_especifico_c;

typedef struct cec_impuesto_c {
    const char* codigo;              /* Nota 8 */
    const char* codigoImpuestoOTRO;  /* Con código 99 */
    const char* codigoTarifaIVA;     /* Nota 8.1, con códigos 01 y 07 */
    const char* tarifa;              /* Tarifa propia de los impuestos 02, 12 y 99 */
    const char* factorCalculoIVA;    /* Con código 08 */
    const cec_datos_impuesto_especifico_c* datosImpuestoEspecifico; /* Con códigos 03-06 */
    const char* montoExportacion;    /* FEE, NC y ND */
} cec_impuesto_c;

typedef struct cec_exoneracion_c {
    const char* tipoDocumentoEX1;    /* Nota 10.1 */
    const char* tipoDocumentoOTRO;
    const char* numeroDocumento;
    int32_t articulo;                /* CEC_SIN_VALOR si no aplica */
    int32_t inciso;                  /* CEC_SIN_VALOR si no aplica */
    const char* nombreInstitucion;   /* Nota 23 */
    const char* nombreInstitucionOtros;
    const char* fechaEmisionEX;
    const char* tarifaExonerada;
} cec_exoneracion_c;

typedef struct cec_linea_surtido_c {
    const char* codigoCABYS;
    const cec_codigo_comercial_c* codigosComerciales;
    size_t numCodigosComerciales;
    const char* cantidad;
    const char* unidadMedida;
    const char* unidadMedidaComercial;
    const char* detalle;
    const char* precioUnitario;
    const cec_descuento_c* descuentos;
    size_t numDescuentos;
    const cec_impuesto_c* impuestos;
    size_t numImpuestos;
} cec_linea_surtido_c;

typedef struct cec_linea_c {
    int32_t numeroLinea;             /* CEC_SIN_VALOR: se numera según la posición */
    const char* partidaArancelaria;
    const char* codigoCABYS;
    const cec_codigo_comercial_c* codigosComerciales; /* {0,5} */
    size_t numCodigosComerciales;
    const char* cantidad;
    const char* unidadMedida;
    const char* tipoTransaccion;     /* Nota 22 */
    const char* unidadMedidaComercial;
    const char* detalle;
    const char* const* numerosVINoSerie;
    size_t numNumerosVINoSerie;
    const char* registroMedicamento;
    const char* formaFarmaceutica;
    const cec_linea_surtido_c* detalleSurtido;
    size_t numDetalleSurtido;
    const char* precioUnitario;
    const char* montoTotal;          /* Solo REP: monto a pagar, sin impuestos */
    const cec_descuento_c* descuentos;
    size_t numDescuentos;
    const char* subTotal;            /* Solo REP: monto del pago para el cálculo del IVA */
    const char* ivaCobradoFabrica;   /* Nota 21 */
    const char* baseImponible;       /* Solo si es editable */
    const cec_impuesto_c* impuestos;
    size_t numImpuestos;
    const char* impuestoAsumidoEmisorFabrica;
    const cec_exoneracion_c* exoneracion; /* Exonera el IVA de la línea */
} cec_linea_c;

typedef struct cec_otro_cargo_c {
    const char* tipoDocumentoOC;     /* Nota 16 */
    const char* tipoDocumentoOTROS;
    const cec_identificacion_c* identificacionTercero;
    const char* nombreTercero;
    const char* detalle;
    const char* porcentajeOC;
    const char* montoCargo;
} cec_otro_cargo_c;

typedef struct cec_medio_pago_c {
    const char* tipoMedioPago;       /* Nota 6 */
    const char* medioPagoOtros;
    const char* totalMedioPago;
} cec_medio_pago_c;

typedef struct cec_referencia_c {
    const char* tipoDocIR;           /* Nota 10 */
    const char* tipoDocRefOTRO;
    const char* numero;
    const char* fechaEmisionIR;
    const char* codigo;              /* Nota 9 */
    const char* codigoReferenciaOTRO;
    const char* razon;
} cec_referencia_c;

typedef struct cec_request_c {
    const char* tipoDocumento;       /* Nota 3: "01" FE, "02" ND, "03" NC, "04" TE, "08" FEC, "09" FEE, "10" REP */
    const char* clave;
    const char* proveedorSistemas;
    const char* codigoActividadEmisor;
    const char* codigoActividadReceptor;
    const char* numeroConsecutivo;
    const char* fechaEmision;
    cec_emisor_c emisor;
    const cec_receptor_c* receptor;
    const char* condicionVenta;      /* Nota 5 */
    const char* condicionVentaOtros;
    int32_t plazoCredito;            /* Días; CEC_SIN_VALOR si no aplica */
    const cec_linea_c* detalleServicio;
    size_t numDetalleServicio;
    const cec_otro_cargo_c* otrosCargos;
    size_t numOtrosCargos;
    const char* codigoMoneda;
    const char* tipoCambio;
    const cec_medio_pago_c* mediosPago;
    size_t numMediosPago;
    const cec_referencia_c* informacionReferencia;
    size_t numInformacionReferencia;
} cec_request_c;

/* ========================================================================= */
/* ESTRUCTURAS DE SALIDA DE TOTALES                                         */
/* ========================================================================= */

/**
 * Totales del ResumenFactura (Anexo 1 v4.4) como enteros escalados x 100000 (1500.5 -> 150050000).
 * El comprobante completo está en cec_result_xml y en el JSON de cec_result_data (campo "comprobante").
 */
typedef struct cec_resumen_totales_c {
    int64_t totalServGravados;
    int64_t totalServExentos;
    int64_t totalServExonerado;
    int64_t totalServNoSujeto;
    int64_t totalMercanciasGravadas;
    int64_t totalMercanciasExentas;
    int64_t totalMercExonerada;
    int64_t totalMercNoSujeta;
    int64_t totalGravado;
    int64_t totalExento;
    int64_t totalExonerado;
    int64_t totalNoSujeto;
    int64_t totalVenta;
    int64_t totalDescuentos;
    int64_t totalVentaNeta;
    int64_t totalImpuesto;
    int64_t totalImpAsumEmisorFabrica;
    int64_t totalIVADevuelto;
    int64_t totalOtrosCargos;
    int64_t totalComprobante;
} cec_resumen_totales_c;

typedef struct cec_result cec_result;

/* ========================================================================= */
/* FUNCIONES DE LA C-API                                                     */
/* ========================================================================= */

/**
 * @brief Versión del ABI de C. 3: structs de entrada con la estructura del Anexo 1 v4.4, totales completos
 * y diagnósticos accesibles sin JSON.
 */
CEC_API uint32_t cec_abi_version(void);

/**
 * @brief Punto de entrada: valida las reglas fiscales, calcula y serializa el XML v4.4 sin firmar.
 *
 * Devuelve CEC_STATUS_OK aunque el comprobante sea inválido: cec_result_is_success indica el resultado y
 * los diagnósticos se leen con cec_result_total_diagnosticos / cec_result_diagnostico.
 */
CEC_API cec_status cec_build(
    const cec_request_c* request,
    cec_result** out_result
);

/**
 * @brief Igual que cec_build pero con el request en JSON UTF-8 (schemas/comprobante_request.schema.json).
 * Un JSON mal formado o con tipos incorrectos produce el diagnóstico JSON_DECODE_ERROR con la ruta del campo.
 */
CEC_API cec_status cec_build_json(
    const uint8_t* json_data,
    size_t json_len,
    cec_result** out_result
);

/**
 * @brief Vista previa del cálculo para edición en vivo: calcula montos y totales aunque las reglas reporten
 * errores (solo los errores de formato lo impiden) y no genera XML. El comprobante calculado se lee en el
 * campo "comprobante" de cec_result_data y los totales con cec_result_totales; los diagnósticos, igual que en
 * cec_build. Para emitir siempre se usa cec_build.
 */
CEC_API cec_status cec_calcular(
    const cec_request_c* request,
    cec_result** out_result
);

/**
 * @brief Igual que cec_calcular pero con el request en JSON UTF-8.
 */
CEC_API cec_status cec_calcular_json(
    const uint8_t* json_data,
    size_t json_len,
    cec_result** out_result
);

/**
 * @brief Firma un XML oficial con un certificado digital PKCS#12 (.p12/.pfx) usando XAdES-EPES Enveloped (RSA-SHA256).
 */
CEC_API cec_status cec_sign_xml(
    const uint8_t* xml_data,
    size_t xml_len,
    const uint8_t* p12_data,
    size_t p12_len,
    const char* pin,
    cec_result** out_result
);

/**
 * @brief Firma en dos pasos, para cuando la llave privada está fuera del proceso (keystore, HSM).
 * Paso 1: prepara la firma XAdES-EPES con el certificado público (X.509 DER). El resultado trae el XML con
 * SignatureValue vacío (cec_result_xml) y, en el campo "signedInfo" de cec_result_data, el SignedInfo
 * canonicalizado que se firma con RSA-SHA256.
 */
CEC_API cec_status cec_preparar_firma(
    const uint8_t* xml_data,
    size_t xml_len,
    const uint8_t* cert_der,
    size_t cert_len,
    cec_result** out_result
);

/**
 * @brief Paso 2: inserta el SignatureValue (Base64, terminado en cero) en el XML preparado y verifica la
 * firma con el certificado de KeyInfo. Diagnóstico SIGN_VERIFY_FAILED si no corresponde.
 */
CEC_API cec_status cec_completar_firma(
    const uint8_t* xml_data,
    size_t xml_len,
    const char* signature_value_base64,
    cec_result** out_result
);

/**
 * @brief Lee el certificado de un PKCS#12 (.p12/.pfx) sin firmar: titular, emisor, vigencia y huella.
 * Los datos se obtienen con cec_result_certificado (o en el campo "certificado" de cec_result_data).
 */
CEC_API cec_status cec_leer_certificado(
    const uint8_t* p12_data,
    size_t p12_len,
    const char* pin,
    cec_result** out_result
);

/**
 * @brief Mensaje Receptor v4.4 (confirmación o rechazo de un comprobante recibido) desde JSON UTF-8 con los nodos
 * del XSD en camelCase: clave, numeroCedulaEmisor, fechaEmisionDoc, mensaje, detalleMensaje, montoTotalImpuesto,
 * codigoActividad, condicionImpuesto, montoTotalImpuestoAcreditar, montoTotalDeGastoAplicable, totalFactura,
 * numeroCedulaReceptor, numeroConsecutivoReceptor y, opcional, fechaEmisionComprobante (para el plazo del art. 10).
 * Valida y genera el XML sin firmar (se firma con cec_sign_xml o en dos pasos). El campo "plazoConfirmacion"
 * (AAAA-MM-DD) de cec_result_data trae el último día del plazo.
 */
CEC_API cec_status cec_mensaje_receptor_json(
    const uint8_t* json_data,
    size_t json_len,
    cec_result** out_result
);

/**
 * @brief Lee el XML de un comprobante recibido (el que envía el proveedor) para confirmarlo con un Mensaje Receptor.
 * El campo "comprobante" de cec_result_data trae tipo, clave, emisor, receptor, fecha, moneda, totales, si está
 * firmado y el plazo de confirmación. Los diagnósticos de error indican por qué no se puede confirmar.
 */
CEC_API cec_status cec_leer_comprobante_xml(
    const uint8_t* xml_data,
    size_t xml_len,
    cec_result** out_result
);

/**
 * @brief Datos del Mensaje Receptor con los nodos de MensajeReceptor_V4.4.xsd (mismos nombres que el JSON de
 * cec_mensaje_receptor_json). Los textos son UTF-8 terminados en cero; NULL = no informado. Los montos son
 * decimales en texto ("13000.50").
 */
typedef struct cec_mensaje_receptor_c {
    const char* clave;                        /* 50 dígitos del comprobante que se confirma o rechaza */
    const char* numeroCedulaEmisor;           /* quien emitió el comprobante (el proveedor) */
    const char* fechaEmisionDoc;              /* fecha del mensaje, RFC 3339 */
    const char* mensaje;                      /* "1" aceptado, "2" aceptado parcialmente, "3" rechazado */
    const char* detalleMensaje;
    const char* montoTotalImpuesto;
    const char* codigoActividad;              /* actividad económica del receptor */
    const char* condicionImpuesto;            /* Nota 18: "01" a "05" */
    const char* montoTotalImpuestoAcreditar;
    const char* montoTotalDeGastoAplicable;
    const char* totalFactura;
    const char* numeroCedulaReceptor;         /* quien confirma (el receptor del comprobante) */
    const char* numeroConsecutivoReceptor;    /* 20 dígitos; tipo 05, 06 o 07 según el mensaje */
    const char* fechaEmisionComprobante;      /* opcional: FechaEmision del comprobante, para el plazo del art. 10 */
} cec_mensaje_receptor_c;

/**
 * @brief Igual que cec_mensaje_receptor_json pero con el mensaje en un struct, sin JSON. El último día del plazo
 * del art. 10 se lee con cec_result_plazo_confirmacion.
 */
CEC_API cec_status cec_mensaje_receptor(
    const cec_mensaje_receptor_c* request,
    cec_result** out_result
);

/**
 * @brief Último día del plazo del art. 10 (AAAA-MM-DD) de un resultado de cec_mensaje_receptor, o de
 * cec_leer_comprobante_xml. CEC_STATUS_ERR_OUT_OF_RANGE si no se pudo calcular (no se indicó la fecha del
 * comprobante). El texto vive hasta cec_result_free.
 */
CEC_API cec_status cec_result_plazo_confirmacion(
    const cec_result* res,
    const char** out_plazo
);

/**
 * @brief SignedInfo canonicalizado (C14N) que se firma con RSA-SHA256, en el resultado de cec_preparar_firma.
 * CEC_STATUS_ERR_OUT_OF_RANGE en cualquier otro resultado. Los bytes viven hasta cec_result_free.
 */
CEC_API cec_status cec_result_signed_info(
    const cec_result* res,
    const uint8_t** data,
    size_t* len
);

/**
 * @brief Datos del comprobante recibido leído con cec_leer_comprobante_xml. Los textos son UTF-8 terminados en
 * cero (vacíos si el XML no los trae) y viven hasta cec_result_free.
 */
typedef struct cec_comprobante_recibido_c {
    const char* tipoDocumento;               /* "01", "02", "03"... según el elemento raíz */
    const char* nombreDocumento;             /* FacturaElectronica, NotaCreditoElectronica... */
    const char* version;                     /* "4.4", "4.3"... según el namespace */
    const char* clave;
    const char* numeroConsecutivo;
    const char* fechaEmision;
    const char* emisorNombre;
    const char* emisorTipoIdentificacion;
    const char* emisorNumeroIdentificacion;
    const char* emisorCorreo;
    const char* receptorNombre;
    const char* receptorTipoIdentificacion;
    const char* receptorNumeroIdentificacion;
    const char* codigoActividadReceptor;
    const char* codigoMoneda;
    const char* tipoCambio;
    const char* totalImpuesto;
    const char* totalComprobante;
    int32_t firmado;                         /* 1 si el XML trae un nodo de firma */
    const char* plazoConfirmacion;           /* AAAA-MM-DD; vacío si la fecha de emisión no es válida */
} cec_comprobante_recibido_c;

/**
 * @brief Datos del comprobante de un resultado de cec_leer_comprobante_xml, sin JSON.
 * Si el XML se pudo leer pero no se puede confirmar (no es un comprobante, es de un tipo que no se confirma, falta la
 * firma...), cec_result_is_success es 0, los diagnósticos dicen por qué y los datos son los que se alcanzaron a leer.
 * CEC_STATUS_ERR_OUT_OF_RANGE si el XML ni siquiera se pudo leer (mal formado).
 */
CEC_API cec_status cec_result_comprobante_recibido(
    const cec_result* res,
    cec_comprobante_recibido_c* out_comprobante
);

/**
 * @brief Datos del certificado usado en cec_sign_xml o leído con cec_leer_certificado.
 * Los textos son UTF-8 terminados en cero y viven hasta cec_result_free.
 */
typedef struct cec_certificado_c {
    const char* sujeto;
    const char* emisor;
    const char* numeroSerie;
    const char* validoDesde;        /* ISO 8601 UTC */
    const char* validoHasta;        /* ISO 8601 UTC */
    const char* sha256Hex;
    const char* sha256Base64;
    const char* certificadoBase64;  /* X.509 DER en Base64 */
    int32_t vigente;                /* 1 si la fecha actual está dentro de la vigencia */
} cec_certificado_c;

/**
 * @brief Certificado del resultado. CEC_STATUS_ERR_OUT_OF_RANGE si el resultado no trae certificado.
 */
CEC_API cec_status cec_result_certificado(
    const cec_result* res,
    cec_certificado_c* out_certificado
);

/**
 * @brief Indica si la operación concluyó exitosamente sin errores de validación.
 */
CEC_API cec_status cec_result_is_success(
    const cec_result* res,
    int32_t* out_is_success
);

/**
 * @brief Obtiene el XML oficial generado o firmado.
 */
CEC_API cec_status cec_result_xml(
    const cec_result* res,
    const uint8_t** data,
    size_t* len
);

/**
 * @brief Totales del ResumenFactura calculados. CEC_STATUS_ERR_OUT_OF_RANGE si el resultado no trae comprobante
 * calculado (comprobante inválido en cec_build, error de formato en cec_calcular, o resultado de firma).
 */
CEC_API cec_status cec_result_totales(
    const cec_result* res,
    cec_resumen_totales_c* out_totales
);

/**
 * @brief JSON del resultado: status, metadata, diagnostics, xml y comprobante (estructura del XML v4.4).
 */
CEC_API cec_status cec_result_data(
    const cec_result* res,
    const uint8_t** data,
    size_t* len
);

/**
 * @brief Severidad de un diagnóstico.
 */
typedef enum cec_severidad {
    CEC_SEVERIDAD_ERROR = 0,
    CEC_SEVERIDAD_ADVERTENCIA = 1
} cec_severidad;

/**
 * @brief Cantidad de diagnósticos (errores y advertencias) del resultado.
 */
CEC_API cec_status cec_result_total_diagnosticos(
    const cec_result* res,
    size_t* out_total
);

/**
 * @brief Diagnóstico por índice. Los textos son UTF-8 terminados en cero y viven hasta cec_result_free.
 * `out_ruta` usa los nombres del request (ej. "detalleServicio[0].impuestos[1].codigo"). Cualquier salida puede ser NULL.
 */
CEC_API cec_status cec_result_diagnostico(
    const cec_result* res,
    size_t indice,
    cec_severidad* out_severidad,
    const char** out_codigo,
    const char** out_ruta,
    const char** out_mensaje
);

/**
 * @brief Libera la memoria y recursos asociados a un handle de resultado.
 */
CEC_API void cec_result_free(cec_result* res);

/* ========================================================================= */
/* C-API: DIVISIÓN TERRITORIAL DE COSTA RICA (DTA)                          */
/* ========================================================================= */

/**
 * @brief Obtiene el total de provincias de Costa Rica (7).
 */
CEC_API uint32_t cec_dta_total_provincias(void);

/**
 * @brief Obtiene una provincia por índice (0 a 6).
 */
CEC_API int32_t cec_dta_provincia_por_indice(
    uint32_t indice,
    uint8_t* out_codigo,
    const char** out_nombre,
    uint32_t* out_total_cantones
);

/**
 * @brief Obtiene el total de cantones de una provincia.
 */
CEC_API uint32_t cec_dta_total_cantones(uint8_t codigo_provincia);

/**
 * @brief Obtiene un cantón por índice dentro de una provincia.
 */
CEC_API int32_t cec_dta_canton_por_indice(
    uint8_t codigo_provincia,
    uint32_t indice,
    uint8_t* out_codigo,
    const char** out_nombre,
    uint32_t* out_total_distritos
);

/**
 * @brief Obtiene el total de distritos de un cantón.
 */
CEC_API uint32_t cec_dta_total_distritos(uint8_t codigo_provincia, uint8_t codigo_canton);

/**
 * @brief Obtiene un distrito por índice dentro de un cantón.
 */
CEC_API int32_t cec_dta_distrito_por_indice(
    uint8_t codigo_provincia,
    uint8_t codigo_canton,
    uint32_t indice,
    uint8_t* out_codigo,
    const char** out_nombre,
    uint32_t* out_total_barrios
);

/**
 * @brief Obtiene el total de barrios de un distrito.
 */
CEC_API uint32_t cec_dta_total_barrios(uint8_t codigo_provincia, uint8_t codigo_canton, uint8_t codigo_distrito);

/**
 * @brief Obtiene un barrio por índice dentro de un distrito.
 */
CEC_API int32_t cec_dta_barrio_por_indice(
    uint8_t codigo_provincia,
    uint8_t codigo_canton,
    uint8_t codigo_distrito,
    uint32_t indice,
    uint8_t* out_codigo,
    const char** out_nombre
);

/**
 * @brief Validaciones de división territorial (retorna 1 si es válido, 0 si no).
 */
CEC_API int32_t cec_dta_validar_provincia(uint8_t codigo_provincia);
CEC_API int32_t cec_dta_validar_canton(uint8_t codigo_provincia, uint8_t codigo_canton);
CEC_API int32_t cec_dta_validar_distrito(uint8_t codigo_provincia, uint8_t codigo_canton, uint8_t codigo_distrito);
CEC_API int32_t cec_dta_validar_barrio(uint8_t codigo_provincia, uint8_t codigo_canton, uint8_t codigo_distrito, uint8_t codigo_barrio);
CEC_API int32_t cec_dta_validar_ubicacion(uint8_t codigo_provincia, uint8_t codigo_canton, uint8_t codigo_distrito);

/**
 * @brief Consultas de nombres (devuelve puntero a cadena estática de solo lectura).
 */
CEC_API const char* cec_dta_nombre_provincia(uint8_t codigo_provincia);
CEC_API const char* cec_dta_nombre_canton(uint8_t codigo_provincia, uint8_t codigo_canton);
CEC_API const char* cec_dta_nombre_distrito(uint8_t codigo_provincia, uint8_t codigo_canton, uint8_t codigo_distrito);
CEC_API const char* cec_dta_nombre_barrio(uint8_t codigo_provincia, uint8_t codigo_canton, uint8_t codigo_distrito, uint8_t codigo_barrio);

/* ========================================================================= */
/* C-API: CATÁLOGOS OFICIALES DE FACTURACIÓN ELECTRÓNICA V4.4               */
/* ========================================================================= */

/**
 * @brief Monedas oficiales (CRC, USD, EUR, etc.).
 */
CEC_API uint32_t cec_catalogo_total_monedas(void);
CEC_API int32_t cec_catalogo_moneda_por_indice(
    uint32_t indice,
    const char** out_codigo,
    const char** out_simbolo,
    const char** out_descripcion
);

/**
 * @brief Unidades de medida oficiales (RTC 443:2010 / Hacienda v4.4).
 */
CEC_API uint32_t cec_catalogo_total_unidades_medida(void);
CEC_API int32_t cec_catalogo_unidad_medida_por_indice(
    uint32_t indice,
    const char** out_codigo,
    const char** out_simbolo,
    const char** out_descripcion
);

/**
 * @brief Formas farmacéuticas Nota Técnica 9 (01 a 229).
 */
CEC_API uint32_t cec_catalogo_total_formas_farmaceuticas(void);
CEC_API int32_t cec_catalogo_forma_farmaceutica_por_indice(
    uint32_t indice,
    const char** out_codigo,
    const char** out_descripcion
);

/**
 * @brief Nombres de todos los catálogos disponibles.
 */
CEC_API uint32_t cec_catalogo_total_nombres(void);
CEC_API const char* cec_catalogo_nombre_por_indice(uint32_t indice);

/**
 * @brief Consulta genérica de elementos de cualquier catálogo por nombre.
 */
CEC_API uint32_t cec_catalogo_total_elementos(const char* nombre_catalogo);
CEC_API int32_t cec_catalogo_elemento_por_indice(
    const char* nombre_catalogo,
    uint32_t indice,
    const char** out_codigo,
    const char** out_descripcion
);

#ifdef __cplusplus
}
#endif

#endif /* CEC_C_API_H */
