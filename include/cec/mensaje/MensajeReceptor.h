#pragma once
/**
 * @file MensajeReceptor.h
 * @brief Mensaje Receptor v4.4: confirmación (total o parcial) o rechazo de un comprobante recibido
 * (art. 10 de la resolución MH-DGT-RES-0027-2024 y MensajeReceptor_V4.4.xsd).
 *
 * Tres piezas:
 *   - leerComprobante: lee el XML que envió el proveedor y extrae lo que el mensaje necesita.
 *   - validar / generar: reglas del mensaje y XML sin firmar (se firma con XadesSigner, igual que un comprobante).
 *   - plazoConfirmacion: último día para enviar el mensaje (8 días hábiles desde el primer día del mes siguiente).
 */
#include <algorithm>
#include <cctype>
#include <chrono>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <cstdio>
#include "pugixml.hpp"
#include "cec/domain/Diagnostics.h"
#include "cec/domain/Money.h"
#include "cec/rules/RuleValidationHelpers.h"

namespace cec::mensaje {

using domain::DiagnosticCategory;
using domain::DiagnosticList;
using domain::Money;

inline constexpr std::string_view kNamespaceMensajeReceptor =
    "https://cdn.comprobanteselectronicos.go.cr/xml-schemas/v4.4/mensajeReceptor";

/** Datos del mensaje con los nombres de los nodos del XSD (en camelCase). Los montos van como texto decimal. */
struct MensajeReceptorRequest {
    std::string clave;                       ///< Clave del comprobante que se confirma o rechaza
    std::string numeroCedulaEmisor;          ///< Identificación de quien emitió el comprobante (el proveedor)
    std::string fechaEmisionDoc;             ///< Fecha del mensaje, RFC 3339
    std::string mensaje;                     ///< 1 aceptado, 2 aceptado parcialmente, 3 rechazado
    std::string detalleMensaje;
    std::string montoTotalImpuesto;
    std::string codigoActividad;             ///< Actividad económica del receptor
    std::string condicionImpuesto;           ///< Nota 18: 01 a 05
    std::string montoTotalImpuestoAcreditar;
    std::string montoTotalDeGastoAplicable;
    std::string totalFactura;
    std::string numeroCedulaReceptor;        ///< Identificación de quien confirma (el receptor del comprobante)
    std::string numeroConsecutivoReceptor;   ///< 20 dígitos; tipo 05, 06 o 07 según el mensaje
    /// Opcional: FechaEmision del comprobante, para revisar el plazo del art. 10 y el orden de las fechas.
    std::string fechaEmisionComprobante;
};

/** Datos del comprobante recibido (XML del proveedor). */
struct ComprobanteRecibido {
    std::string tipoDocumento;               ///< 01, 02, 03… según el elemento raíz
    std::string nombreDocumento;             ///< FacturaElectronica, NotaCreditoElectronica…
    std::string version;                     ///< 4.4, 4.3… según el namespace
    std::string clave;
    std::string numeroConsecutivo;
    std::string fechaEmision;
    std::string emisorNombre;
    std::string emisorTipoIdentificacion;
    std::string emisorNumeroIdentificacion;
    std::string emisorCorreo;
    std::string receptorNombre;
    std::string receptorTipoIdentificacion;
    std::string receptorNumeroIdentificacion;
    std::string codigoActividadReceptor;
    std::string codigoMoneda;
    std::string tipoCambio;
    std::string totalImpuesto;
    std::string totalComprobante;
    bool firmado = false;
    std::string plazoConfirmacion;           ///< AAAA-MM-DD; vacío si la fecha de emisión no es válida
};

namespace detalle {

inline bool soloDigitos(std::string_view s) {
    return !s.empty() && std::all_of(s.begin(), s.end(), [](unsigned char c) { return c >= '0' && c <= '9'; });
}

inline bool cedulaValida(std::string_view s) { return s.size() >= 9 && s.size() <= 12 && soloDigitos(s); }

/// Identificación de quien emitió el comprobante: hasta 20 caracteres, números y letras (puede ser un extranjero no
/// domiciliado). MensajeReceptor_V4.4.xsd solo limita su longitud.
inline bool identificacionEmisorValida(std::string_view s) {
    return !s.empty() && s.size() <= 20 && std::all_of(s.begin(), s.end(), [](unsigned char c) { return std::isalnum(c) != 0; });
}

/** Caracteres (puntos de código UTF-8), no bytes: el XSD limita DetalleMensaje a 160 caracteres. */
inline size_t caracteres(std::string_view s) {
    return static_cast<size_t>(std::count_if(s.begin(), s.end(), [](unsigned char c) { return (c & 0xC0) != 0x80; }));
}

/** Nombre local de un nodo (sin prefijo de namespace). */
inline std::string_view local(const pugi::xml_node& n) {
    std::string_view nombre = n.name();
    const auto dos = nombre.find(':');
    return dos == std::string_view::npos ? nombre : nombre.substr(dos + 1);
}

inline pugi::xml_node hijo(const pugi::xml_node& padre, std::string_view nombre) {
    for (pugi::xml_node n = padre.first_child(); n; n = n.next_sibling()) {
        if (n.type() == pugi::node_element && local(n) == nombre) return n;
    }
    return {};
}

inline std::string texto(pugi::xml_node n, std::initializer_list<std::string_view> ruta) {
    for (auto nombre : ruta) {
        n = hijo(n, nombre);
        if (!n) return "";
    }
    std::string t = n.text().get();
    const auto ini = t.find_first_not_of(" \t\r\n");
    if (ini == std::string::npos) return "";
    return t.substr(ini, t.find_last_not_of(" \t\r\n") - ini + 1);
}

/** Monto opcional: nullopt si está vacío; diagnóstico si no es un decimal válido de hasta 5 decimales. */
inline std::optional<Money> monto(const std::string& valor, std::string_view ruta, std::string_view etiqueta, DiagnosticList& d) {
    if (valor.empty()) return std::nullopt;
    const auto punto = valor.find('.');
    const bool formato = valor.find_first_not_of("0123456789.") == std::string::npos && valor.find('.', punto + 1) == std::string::npos &&
                         (punto == std::string::npos || (valor.size() - punto - 1 >= 1 && valor.size() - punto - 1 <= 5)) &&
                         (punto == std::string::npos ? valor.size() : punto) >= 1 && (punto == std::string::npos ? valor.size() : punto) <= 13;
    if (!formato) {
        d.addError("RULE_MR_MONTO_INVALIDO", ruta,
                   std::string(etiqueta) + " debe ser un monto positivo con hasta 13 enteros y 5 decimales.", DiagnosticCategory::Schema);
        return std::nullopt;
    }
    return Money::fromString(valor);
}

inline std::chrono::year_month_day fecha(const std::string& rfc3339) {
    using namespace std::chrono;
    return year_month_day{year{std::stoi(rfc3339.substr(0, 4))}, month{static_cast<unsigned>(std::stoi(rfc3339.substr(5, 2)))},
                          day{static_cast<unsigned>(std::stoi(rfc3339.substr(8, 2)))}};
}

inline std::string iso(const std::chrono::year_month_day& f) {
    char buf[11];
    std::snprintf(buf, sizeof(buf), "%04d-%02u-%02u", static_cast<int>(f.year()), static_cast<unsigned>(f.month()),
                  static_cast<unsigned>(f.day()));
    return buf;
}

inline std::string dmy(const std::chrono::year_month_day& f) {
    char buf[11];
    std::snprintf(buf, sizeof(buf), "%02u/%02u/%04d", static_cast<unsigned>(f.day()), static_cast<unsigned>(f.month()),
                  static_cast<int>(f.year()));
    return buf;
}

/** Tipo de comprobante por el elemento raíz del XML. */
inline std::string tipoPorRaiz(std::string_view raiz) {
    if (raiz == "FacturaElectronica") return "01";
    if (raiz == "NotaDebitoElectronica") return "02";
    if (raiz == "NotaCreditoElectronica") return "03";
    if (raiz == "TiqueteElectronico") return "04";
    if (raiz == "FacturaElectronicaCompra") return "08";
    if (raiz == "FacturaElectronicaExportacion") return "09";
    if (raiz == "ReciboElectronicoPago") return "10";
    return "";
}

/** Tipo de documento del NumeroConsecutivoReceptor según el mensaje (Nota 4.1). */
inline std::string tipoConsecutivo(std::string_view mensaje) {
    if (mensaje == "1") return "05";
    if (mensaje == "2") return "06";
    if (mensaje == "3") return "07";
    return "";
}

} // namespace detalle

/**
 * @brief Último día del plazo del art. 10: 8 días hábiles contados desde el primer día del mes siguiente al de la
 * transacción. Cuenta de lunes a viernes; los feriados no se descuentan (el plazo real puede ser uno o dos días mayor).
 */
inline std::optional<std::chrono::year_month_day> plazoConfirmacion(const std::string& fechaEmisionComprobante) {
    using namespace std::chrono;
    if (!rules::RuleValidationHelpers::esFechaRFC3339(fechaEmisionComprobante)) return std::nullopt;
    const auto f = detalle::fecha(fechaEmisionComprobante);
    sys_days dia{(f.year() / f.month() / 1d) + months{1}};
    int habiles = 0;
    for (;; dia += days{1}) {
        const weekday w{dia};
        if (w != Saturday && w != Sunday && ++habiles == 8) return year_month_day{dia};
    }
}

/** Lee el XML de un comprobante recibido. Diagnósticos de error si no se puede confirmar con un Mensaje Receptor. */
inline std::pair<ComprobanteRecibido, DiagnosticList> leerComprobante(const std::string& xml) {
    ComprobanteRecibido c;
    DiagnosticList d;
    pugi::xml_document doc;
    if (!doc.load_string(xml.c_str())) {
        d.addError("MR_XML_INVALIDO", "xml", "El archivo no es un XML válido.", DiagnosticCategory::Structural);
        return {c, d};
    }
    const pugi::xml_node raiz = doc.document_element();
    c.nombreDocumento = std::string(detalle::local(raiz));
    c.tipoDocumento = detalle::tipoPorRaiz(c.nombreDocumento);
    if (c.nombreDocumento == "MensajeHacienda" || c.nombreDocumento == "MensajeReceptor") {
        d.addError("MR_NO_ES_COMPROBANTE", "xml",
                   "El archivo es un " + c.nombreDocumento + ", no un comprobante. Cargue el XML de la factura que envió el proveedor.",
                   DiagnosticCategory::Structural);
        return {c, d};
    }
    if (c.tipoDocumento.empty()) {
        d.addError("MR_NO_ES_COMPROBANTE", "xml", "El XML no es un comprobante electrónico de Hacienda (" + c.nombreDocumento + ").",
                   DiagnosticCategory::Structural);
        return {c, d};
    }
    const std::string ns = raiz.attribute("xmlns").value();
    const auto barra = ns.find("/xml-schemas/v");
    if (barra != std::string::npos) c.version = ns.substr(barra + 14, ns.find('/', barra + 14) - barra - 14);

    c.clave = detalle::texto(raiz, {"Clave"});
    c.numeroConsecutivo = detalle::texto(raiz, {"NumeroConsecutivo"});
    c.fechaEmision = detalle::texto(raiz, {"FechaEmision"});
    c.emisorNombre = detalle::texto(raiz, {"Emisor", "Nombre"});
    c.emisorTipoIdentificacion = detalle::texto(raiz, {"Emisor", "Identificacion", "Tipo"});
    c.emisorNumeroIdentificacion = detalle::texto(raiz, {"Emisor", "Identificacion", "Numero"});
    c.emisorCorreo = detalle::texto(raiz, {"Emisor", "CorreoElectronico"});
    c.receptorNombre = detalle::texto(raiz, {"Receptor", "Nombre"});
    c.receptorTipoIdentificacion = detalle::texto(raiz, {"Receptor", "Identificacion", "Tipo"});
    c.receptorNumeroIdentificacion = detalle::texto(raiz, {"Receptor", "Identificacion", "Numero"});
    c.codigoActividadReceptor = detalle::texto(raiz, {"CodigoActividadReceptor"});
    c.codigoMoneda = detalle::texto(raiz, {"ResumenFactura", "CodigoTipoMoneda", "CodigoMoneda"});
    c.tipoCambio = detalle::texto(raiz, {"ResumenFactura", "CodigoTipoMoneda", "TipoCambio"});
    c.totalImpuesto = detalle::texto(raiz, {"ResumenFactura", "TotalImpuesto"});
    c.totalComprobante = detalle::texto(raiz, {"ResumenFactura", "TotalComprobante"});
    c.firmado = static_cast<bool>(detalle::hijo(raiz, "Signature"));
    if (const auto p = plazoConfirmacion(c.fechaEmision)) c.plazoConfirmacion = detalle::iso(*p);

    if (c.clave.size() != 50 || !detalle::soloDigitos(c.clave)) {
        d.addError("MR_CLAVE_INVALIDA", "clave", "El comprobante no tiene una clave de 50 dígitos.", DiagnosticCategory::Structural);
    }
    if (c.tipoDocumento != "01" && c.tipoDocumento != "02" && c.tipoDocumento != "03") {
        d.addError("MR_TIPO_NO_CONFIRMABLE", "tipoDocumento",
                   "Solo se confirman o rechazan facturas electrónicas y notas de débito o crédito; este XML es un " + c.nombreDocumento + ".");
    }
    if (c.receptorNumeroIdentificacion.empty()) {
        d.addError("MR_SIN_RECEPTOR", "receptor",
                   "El comprobante no identifica al receptor: no se puede confirmar (es una venta a consumidor final).");
    }
    if (!c.firmado) {
        d.addError("MR_SIN_FIRMA", "xml", "El XML no está firmado por el emisor (falta ds:Signature).", DiagnosticCategory::Structural);
    }
    if (c.totalComprobante.empty()) {
        d.addError("MR_SIN_TOTAL", "resumenFactura.totalComprobante", "El comprobante no tiene TotalComprobante.", DiagnosticCategory::Structural);
    }
    if (c.version != "4.4" && !c.version.empty()) {
        d.addWarning("MR_VERSION", "xml", "El comprobante es de la versión " + c.version + " del Anexo; el mensaje se envía en la 4.4.");
    }
    return {c, d};
}

/** Reglas del Mensaje Receptor (XSD v4.4 y Nota 18). */
inline DiagnosticList validar(const MensajeReceptorRequest& r) {
    using detalle::cedulaValida;
    using detalle::identificacionEmisorValida;
    DiagnosticList d;

    if (r.clave.size() != 50 || !detalle::soloDigitos(r.clave)) {
        d.addError("RULE_MR_CLAVE", "clave", "La clave del comprobante debe tener 50 dígitos.", DiagnosticCategory::Schema);
    } else {
        // Posiciones 30 y 31 de la clave: tipo de comprobante dentro del consecutivo.
        const std::string tipo = r.clave.substr(29, 2);
        if (tipo != "01" && tipo != "02" && tipo != "03") {
            d.addError("RULE_MR_TIPO_NO_CONFIRMABLE", "clave",
                       "Solo se confirman facturas electrónicas y notas de débito o crédito (tipo " + tipo + " en la clave).");
        }
        // La clave guarda la identificación en 12 posiciones: una más larga (extranjero) no se puede comparar.
        if (identificacionEmisorValida(r.numeroCedulaEmisor) && r.numeroCedulaEmisor.size() <= 12 &&
            r.clave.substr(9, 12) != std::string(12 - r.numeroCedulaEmisor.size(), '0') + r.numeroCedulaEmisor) {
            d.addError("RULE_MR_CLAVE_EMISOR", "numeroCedulaEmisor", "La identificación del emisor no coincide con la de la clave del comprobante.");
        }
    }
    if (!identificacionEmisorValida(r.numeroCedulaEmisor)) {
        d.addError("RULE_MR_CEDULA_EMISOR", "numeroCedulaEmisor", "La identificación del emisor debe tener hasta 20 caracteres (números y letras), sin guiones.",
                   DiagnosticCategory::Schema);
    }
    if (!cedulaValida(r.numeroCedulaReceptor)) {
        d.addError("RULE_MR_CEDULA_RECEPTOR", "numeroCedulaReceptor",
                   "La identificación del receptor debe tener de 9 a 12 dígitos, sin guiones.", DiagnosticCategory::Schema);
    }
    const bool fechaValida = rules::RuleValidationHelpers::esFechaRFC3339(r.fechaEmisionDoc);
    if (!fechaValida) {
        d.addError("RULE_MR_FECHA", "fechaEmisionDoc", "La fecha del mensaje debe tener formato RFC 3339 (AAAA-MM-DDThh:mm:ss-06:00).",
                   DiagnosticCategory::Schema);
    }

    if (fechaValida && rules::RuleValidationHelpers::esFechaRFC3339(r.fechaEmisionComprobante) &&
        std::chrono::sys_days{detalle::fecha(r.fechaEmisionDoc)} < std::chrono::sys_days{detalle::fecha(r.fechaEmisionComprobante)}) {
        d.addError("RULE_MR_FECHA_ANTERIOR", "fechaEmisionDoc", "La fecha del mensaje es anterior a la del comprobante.");
    }

    const std::string tipoMensaje = detalle::tipoConsecutivo(r.mensaje);
    if (tipoMensaje.empty()) {
        d.addError("RULE_MR_MENSAJE", "mensaje", "El mensaje debe ser 1 (aceptado), 2 (aceptado parcialmente) o 3 (rechazado).",
                   DiagnosticCategory::Schema);
    }
    if (r.numeroConsecutivoReceptor.size() != 20 || !detalle::soloDigitos(r.numeroConsecutivoReceptor)) {
        d.addError("RULE_MR_CONSECUTIVO", "numeroConsecutivoReceptor", "El consecutivo del mensaje debe tener 20 dígitos.",
                   DiagnosticCategory::Schema);
    } else if (!tipoMensaje.empty() && r.numeroConsecutivoReceptor.substr(8, 2) != tipoMensaje) {
        d.addError("RULE_MR_CONSECUTIVO_TIPO", "numeroConsecutivoReceptor",
                   "El tipo de documento del consecutivo (posiciones 9 y 10) debe ser " + tipoMensaje + " para este mensaje.");
    }

    const size_t largoDetalle = detalle::caracteres(r.detalleMensaje);
    if (largoDetalle > 160) {
        d.addError("RULE_MR_DETALLE_LARGO", "detalleMensaje",
                   "El detalle del mensaje admite hasta 160 caracteres (tiene " + std::to_string(largoDetalle) + ").", DiagnosticCategory::Schema);
    }
    if ((r.mensaje == "2" || r.mensaje == "3") && r.detalleMensaje.empty()) {
        d.addWarning("RULE_MR_DETALLE_RECOMENDADO", "detalleMensaje",
                     "Indique el motivo de la aceptación parcial o del rechazo: el emisor lo necesita para hacer la nota de crédito o débito.");
    }

    if (r.codigoActividad.size() > 6) {
        d.addError("RULE_MR_ACTIVIDAD", "codigoActividad", "El código de actividad económica tiene hasta 6 caracteres.",
                   DiagnosticCategory::Schema);
    }

    const auto total = detalle::monto(r.totalFactura, "totalFactura", "El total de la factura", d);
    if (r.totalFactura.empty()) d.addError("RULE_MR_TOTAL_REQUERIDO", "totalFactura", "Falta el total de la factura.", DiagnosticCategory::Schema);
    const auto impuesto = detalle::monto(r.montoTotalImpuesto, "montoTotalImpuesto", "El monto total del impuesto", d);
    const auto acreditar = detalle::monto(r.montoTotalImpuestoAcreditar, "montoTotalImpuestoAcreditar", "El impuesto a acreditar", d);
    const auto gasto = detalle::monto(r.montoTotalDeGastoAplicable, "montoTotalDeGastoAplicable", "El gasto aplicable", d);
    if (total && impuesto && *impuesto > *total) {
        d.addError("RULE_MR_IMPUESTO_MAYOR_TOTAL", "montoTotalImpuesto", "El impuesto no puede ser mayor que el total de la factura.");
    }

    const bool conImpuesto = impuesto && !impuesto->isZero();
    const std::string& ci = r.condicionImpuesto;
    if (r.mensaje == "3") {
        if (!ci.empty() || !r.montoTotalImpuestoAcreditar.empty() || !r.montoTotalDeGastoAplicable.empty()) {
            d.addError("RULE_MR_RECHAZO_SIN_CONDICION", ci.empty() ? "montoTotalImpuestoAcreditar" : "condicionImpuesto",
                       "En un rechazo no se indican la condición del impuesto ni los montos a acreditar o de gasto.");
        }
        return d;
    }
    if (!ci.empty() && (ci.size() != 2 || ci < "01" || ci > "05")) {
        d.addError("RULE_MR_CONDICION_INVALIDA", "condicionImpuesto", "La condición del impuesto debe ser de 01 a 05 (Nota 18).",
                   DiagnosticCategory::Catalog);
        return d;
    }
    if (!conImpuesto) {
        if (!ci.empty() || (acreditar && !acreditar->isZero()) || (gasto && !gasto->isZero())) {
            d.addError("RULE_MR_CONDICION_SIN_IMPUESTO", ci.empty() ? "montoTotalImpuestoAcreditar" : "condicionImpuesto",
                       "La factura no tiene impuesto: no se indican la condición ni montos a acreditar o de gasto.");
        }
        return d;
    }
    if (r.montoTotalImpuesto.empty() && !ci.empty()) {
        d.addError("RULE_MR_IMPUESTO_REQUERIDO", "montoTotalImpuesto", "Indique el monto total del impuesto de la factura.");
    }
    if (ci.empty()) {
        d.addError("RULE_MR_CONDICION_REQUERIDA", "condicionImpuesto",
                   "La factura tiene impuesto: indique si genera crédito de IVA, es bien de capital, gasto o proporcionalidad.");
        return d;
    }
    if (r.codigoActividad.empty()) {
        d.addError("RULE_MR_ACTIVIDAD_REQUERIDA", "codigoActividad",
                   "Indique la actividad económica a la que se aplica el crédito o el gasto del impuesto.");
    }

    const Money cero = Money::fromScaled(0);
    const Money a = acreditar.value_or(cero);
    const Money g = gasto.value_or(cero);
    if (ci == "01" || ci == "03") {
        if (!acreditar) {
            d.addError("RULE_MR_ACREDITAR_REQUERIDO", "montoTotalImpuestoAcreditar", "Indique el monto del impuesto a acreditar.");
        } else if (a != *impuesto) {
            d.addError("RULE_MR_ACREDITAR_TOTAL", "montoTotalImpuestoAcreditar",
                       "Con crédito total del IVA (o bienes de capital) se acredita todo el impuesto de la factura (" + impuesto->toString(5) + ").");
        }
        if (!g.isZero()) {
            d.addError("RULE_MR_GASTO_NO_APLICA", "montoTotalDeGastoAplicable", "Con crédito total del IVA no hay impuesto como gasto.");
        }
    } else if (ci == "04") {
        if (!a.isZero()) {
            d.addError("RULE_MR_ACREDITAR_NO_APLICA", "montoTotalImpuestoAcreditar", "Un gasto corriente no genera crédito de IVA.");
        }
        if (!gasto) {
            d.addError("RULE_MR_GASTO_REQUERIDO", "montoTotalDeGastoAplicable", "Indique el monto del impuesto que se aplica como gasto.");
        } else if (g != *impuesto) {
            d.addError("RULE_MR_GASTO_TOTAL", "montoTotalDeGastoAplicable",
                       "Como gasto corriente, todo el impuesto de la factura (" + impuesto->toString(5) + ") se aplica como gasto.");
        }
    } else {
        // 02 crédito parcial y 05 proporcionalidad: una parte se acredita y el resto va al gasto.
        if (!acreditar) {
            d.addError("RULE_MR_ACREDITAR_REQUERIDO", "montoTotalImpuestoAcreditar", "Indique el monto del impuesto a acreditar.");
        } else if (a + g != *impuesto) {
            d.addError("RULE_MR_MONTOS_IMPUESTO", "montoTotalDeGastoAplicable",
                       "El impuesto a acreditar más el gasto aplicable debe ser igual al impuesto de la factura (" + impuesto->toString(5) + ").");
        }
    }

    return d;
}

/**
 * Plazo del art. 10. Es advertencia y no error: pasado el plazo la factura se presume aceptada, pero el receptor
 * puede necesitar el mensaje igualmente (por ejemplo, un rechazo); Hacienda decide si lo admite.
 */
inline void revisarPlazo(const MensajeReceptorRequest& r, DiagnosticList& d) {
    if (!rules::RuleValidationHelpers::esFechaRFC3339(r.fechaEmisionDoc)) return;
    const auto plazo = plazoConfirmacion(r.fechaEmisionComprobante);
    if (!plazo) return;
    if (std::chrono::sys_days{detalle::fecha(r.fechaEmisionDoc)} > std::chrono::sys_days{*plazo}) {
        d.addWarning("RULE_MR_FUERA_DE_PLAZO", "fechaEmisionDoc",
                     "Venció el plazo del art. 10 (" + detalle::dmy(*plazo) +
                         "): sin mensaje, la factura se presume aceptada en su totalidad y Hacienda puede rechazar este mensaje.");
    }
}

/** XML sin firmar, en el orden del XSD. Los montos se escriben con 5 decimales, como en los comprobantes. */
inline std::string generar(const MensajeReceptorRequest& r) {
    pugi::xml_document doc;
    pugi::xml_node raiz = doc.append_child("MensajeReceptor");
    raiz.append_attribute("xmlns") = std::string(kNamespaceMensajeReceptor).c_str();
    raiz.append_attribute("xmlns:xsi") = "http://www.w3.org/2001/XMLSchema-instance";
    auto campo = [&](const char* nombre, const std::string& valor) {
        if (!valor.empty()) raiz.append_child(nombre).text().set(valor.c_str());
    };
    auto dinero = [&](const char* nombre, const std::string& valor) {
        if (!valor.empty()) campo(nombre, Money::fromString(valor).toString(5));
    };
    campo("Clave", r.clave);
    campo("NumeroCedulaEmisor", r.numeroCedulaEmisor);
    campo("FechaEmisionDoc", r.fechaEmisionDoc);
    campo("Mensaje", r.mensaje);
    campo("DetalleMensaje", r.detalleMensaje);
    dinero("MontoTotalImpuesto", r.montoTotalImpuesto);
    campo("CodigoActividad", r.codigoActividad);
    campo("CondicionImpuesto", r.condicionImpuesto);
    dinero("MontoTotalImpuestoAcreditar", r.montoTotalImpuestoAcreditar);
    dinero("MontoTotalDeGastoAplicable", r.montoTotalDeGastoAplicable);
    dinero("TotalFactura", r.totalFactura);
    campo("NumeroCedulaReceptor", r.numeroCedulaReceptor);
    campo("NumeroConsecutivoReceptor", r.numeroConsecutivoReceptor);

    std::ostringstream out;
    out << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
    raiz.print(out, "", pugi::format_raw | pugi::format_no_declaration);
    return out.str();
}

struct MensajeReceptorResult {
    bool isSuccess = false;
    std::string xml;
    DiagnosticList diagnostics;
    std::string plazoConfirmacion;
};

/** Valida y, si no hay errores, genera el XML sin firmar. */
inline MensajeReceptorResult procesar(const MensajeReceptorRequest& r) {
    MensajeReceptorResult res;
    res.diagnostics = validar(r);
    revisarPlazo(r, res.diagnostics);
    if (const auto p = plazoConfirmacion(r.fechaEmisionComprobante)) res.plazoConfirmacion = detalle::iso(*p);
    if (res.diagnostics.hasErrors()) return res;
    res.xml = generar(r);
    res.isSuccess = true;
    return res;
}

} // namespace cec::mensaje
