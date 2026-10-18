#pragma once

/**
 * @file CEC.h
 * @brief Cabecera maestra de Comprobantes Electrónicos Core (CEC).
 *
 * Incluye la API de C++ para validar, calcular, serializar y firmar comprobantes electrónicos
 * v4.4 de Costa Rica. Desde C u otros lenguajes se usa include/cec/c_api/cec_c_api.h.
 */

// Dominio: comprobante canónico, montos y diagnósticos
#include "domain/Comprobante.h"
#include "domain/Diagnostics.h"
#include "domain/Money.h"
#include "domain/Types.h"

// Catálogos del Anexo 1 y división territorial
#include "common/CabysClasificacion.h"
#include "common/Catalogos.h"
#include "common/Codigos.h"
#include "common/DivisionTerritorial.h"
#include "common/FormasFarmaceuticas.h"
#include "common/Monedas.h"
#include "common/UnidadesMedida.h"

// Request, reglas y cálculo
#include "parsing/ComprobanteRequestDTO.h"
#include "rules/RuleEngine.h"
#include "calculation/CalculationEngine.h"

// Formatos de salida (visitor) y JSON
#include "visitor/IFormatoSalida.h"
#include "visitor/RecorridoComprobante.h"
#include "visitor/EscritorXml.h"
#include "codec/JsonCodec.h"
#include "codec/EscritorJson.h"

// Firma XAdES-EPES
#include "crypto/Pkcs12Signer.h"
#include "crypto/XadesSigner.h"

// Mensaje Receptor: confirmación o rechazo de un comprobante recibido
#include "mensaje/MensajeReceptor.h"

// Punto de entrada: procesar, calcular y firmar
#include "service/ComprobanteService.h"
