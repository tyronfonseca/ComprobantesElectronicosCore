#include "cec/c_api/cec_c_api.h"
#include "cec/codec/JsonCodec.h"
#include "cec/service/ComprobanteService.h"
#include "cec/common/DivisionTerritorial.h"
#include "cec/common/Catalogos.h"
#include "cec/common/Monedas.h"
#include "cec/common/UnidadesMedida.h"
#include "cec/common/FormasFarmaceuticas.h"
#include "cec/mensaje/MensajeReceptor.h"
#include <json.hpp>
#include <string>
#include <vector>
#include <cstring>
#include <cstdio>
#include <string_view>
#include <optional>
#include <iomanip>
#include <sstream>
#include <cstdint>
#include <exception>

struct cec_result {
    std::string jsonData;
    std::string xmlData;
    bool isSuccess = false;
    cec::domain::ResumenTotales resumen{};
    bool tieneComprobante = false;
    std::vector<cec::domain::Diagnostic> diagnosticos;
    cec::crypto::CertificadoInfo certificado{};
    std::string plazoConfirmacion;                 // Mensaje Receptor y lectura de comprobantes
    std::string signedInfo;                        // cec_preparar_firma
    cec::mensaje::ComprobanteRecibido recibido{};  // cec_leer_comprobante_xml
    bool tieneRecibido = false;
};

namespace {

inline const char* CStr(std::string_view s) { return s.empty() ? "" : s.data(); }

namespace dto = cec::parsing;

std::string texto(const char* s) { return s ? std::string(s) : std::string(); }

std::optional<int> entero(int32_t v) { return v == CEC_SIN_VALOR ? std::nullopt : std::optional<int>(v); }

template <typename C, typename F>
auto lista(const C* items, size_t n, F&& f) {
    std::vector<decltype(f(*items))> out;
    for (size_t i = 0; items && i < n; ++i) out.push_back(f(items[i]));
    return out;
}

std::vector<std::string> textos(const char* const* items, size_t n) {
    std::vector<std::string> out;
    for (size_t i = 0; items && i < n; ++i) if (items[i]) out.emplace_back(items[i]);
    return out;
}

dto::IdentificacionInput identificacion(const cec_identificacion_c& id) { return { texto(id.tipo), texto(id.numero) }; }

std::optional<dto::UbicacionInput> ubicacion(const cec_ubicacion_c* u) {
    if (!u) return std::nullopt;
    return dto::UbicacionInput{ texto(u->provincia), texto(u->canton), texto(u->distrito), texto(u->barrio), texto(u->otrasSenas) };
}

std::optional<dto::TelefonoInput> telefono(const cec_telefono_c* t) {
    if (!t) return std::nullopt;
    return dto::TelefonoInput{ texto(t->codigoPais), texto(t->numTelefono) };
}

dto::CodigoComercialInput codigoComercial(const cec_codigo_comercial_c& c) { return { texto(c.tipo), texto(c.codigo) }; }

dto::DescuentoInput descuento(const cec_descuento_c& d) {
    return { texto(d.montoDescuento), texto(d.codigoDescuento), texto(d.codigoDescuentoOTRO), texto(d.naturalezaDescuento) };
}

dto::ImpuestoInput impuesto(const cec_impuesto_c& i) {
    dto::ImpuestoInput out;
    out.codigo = texto(i.codigo);
    out.codigoImpuestoOTRO = texto(i.codigoImpuestoOTRO);
    out.codigoTarifaIVA = texto(i.codigoTarifaIVA);
    out.tarifa = texto(i.tarifa);
    out.factorCalculoIVA = texto(i.factorCalculoIVA);
    if (const auto* d = i.datosImpuestoEspecifico) {
        out.datosImpuestoEspecifico = dto::DatosImpuestoEspecificoInput{
            texto(d->cantidadUnidadMedida), texto(d->porcentaje), texto(d->proporcion),
            texto(d->volumenUnidadConsumo), texto(d->impuestoUnidad) };
    }
    out.montoExportacion = texto(i.montoExportacion);
    return out;
}

dto::LineaDetalleSurtidoInput lineaSurtido(const cec_linea_surtido_c& s) {
    dto::LineaDetalleSurtidoInput out;
    out.codigoCABYS = texto(s.codigoCABYS);
    out.codigosComerciales = lista(s.codigosComerciales, s.numCodigosComerciales, codigoComercial);
    out.cantidad = texto(s.cantidad);
    out.unidadMedida = texto(s.unidadMedida);
    out.unidadMedidaComercial = texto(s.unidadMedidaComercial);
    out.detalle = texto(s.detalle);
    out.precioUnitario = texto(s.precioUnitario);
    out.descuentos = lista(s.descuentos, s.numDescuentos, descuento);
    out.impuestos = lista(s.impuestos, s.numImpuestos, impuesto);
    return out;
}

dto::LineaDetalleInput linea(const cec_linea_c& l) {
    dto::LineaDetalleInput out;
    out.numeroLinea = entero(l.numeroLinea);
    out.partidaArancelaria = texto(l.partidaArancelaria);
    out.codigoCABYS = texto(l.codigoCABYS);
    out.codigosComerciales = lista(l.codigosComerciales, l.numCodigosComerciales, codigoComercial);
    out.cantidad = texto(l.cantidad);
    out.unidadMedida = texto(l.unidadMedida);
    out.tipoTransaccion = texto(l.tipoTransaccion);
    out.unidadMedidaComercial = texto(l.unidadMedidaComercial);
    out.detalle = texto(l.detalle);
    out.numerosVINoSerie = textos(l.numerosVINoSerie, l.numNumerosVINoSerie);
    out.registroMedicamento = texto(l.registroMedicamento);
    out.formaFarmaceutica = texto(l.formaFarmaceutica);
    out.detalleSurtido = lista(l.detalleSurtido, l.numDetalleSurtido, lineaSurtido);
    out.precioUnitario = texto(l.precioUnitario);
    out.montoTotal = texto(l.montoTotal);
    out.descuentos = lista(l.descuentos, l.numDescuentos, descuento);
    out.subTotal = texto(l.subTotal);
    out.ivaCobradoFabrica = texto(l.ivaCobradoFabrica);
    out.baseImponible = texto(l.baseImponible);
    out.impuestos = lista(l.impuestos, l.numImpuestos, impuesto);
    out.impuestoAsumidoEmisorFabrica = texto(l.impuestoAsumidoEmisorFabrica);
    if (const auto* e = l.exoneracion) {
        dto::ExoneracionInput ex;
        ex.tipoDocumentoEX1 = texto(e->tipoDocumentoEX1);
        ex.tipoDocumentoOTRO = texto(e->tipoDocumentoOTRO);
        ex.numeroDocumento = texto(e->numeroDocumento);
        ex.articulo = entero(e->articulo);
        ex.inciso = entero(e->inciso);
        ex.nombreInstitucion = texto(e->nombreInstitucion);
        ex.nombreInstitucionOtros = texto(e->nombreInstitucionOtros);
        ex.fechaEmisionEX = texto(e->fechaEmisionEX);
        ex.tarifaExonerada = texto(e->tarifaExonerada);
        out.exoneracion = ex;
    }
    return out;
}

dto::ComprobanteRequestDTO mapRequest(const cec_request_c& r) {
    dto::ComprobanteRequestDTO out;
    out.tipoDocumento = texto(r.tipoDocumento);
    out.clave = texto(r.clave);
    out.proveedorSistemas = texto(r.proveedorSistemas);
    out.codigoActividadEmisor = texto(r.codigoActividadEmisor);
    out.codigoActividadReceptor = texto(r.codigoActividadReceptor);
    out.numeroConsecutivo = texto(r.numeroConsecutivo);
    out.fechaEmision = texto(r.fechaEmision);

    const auto& e = r.emisor;
    out.emisor.nombre = texto(e.nombre);
    out.emisor.identificacion = identificacion(e.identificacion);
    out.emisor.registroFiscal8707 = texto(e.registroFiscal8707);
    out.emisor.nombreComercial = texto(e.nombreComercial);
    out.emisor.ubicacion = ubicacion(e.ubicacion);
    out.emisor.otrasSenasExtranjero = texto(e.otrasSenasExtranjero);
    out.emisor.telefono = telefono(e.telefono);
    out.emisor.correoElectronico = textos(e.correoElectronico, e.numCorreoElectronico);

    if (const auto* rec = r.receptor) {
        dto::ReceptorInput ro;
        ro.nombre = texto(rec->nombre);
        if (rec->identificacion) ro.identificacion = identificacion(*rec->identificacion);
        ro.nombreComercial = texto(rec->nombreComercial);
        ro.ubicacion = ubicacion(rec->ubicacion);
        ro.otrasSenasExtranjero = texto(rec->otrasSenasExtranjero);
        ro.telefono = telefono(rec->telefono);
        ro.correoElectronico = texto(rec->correoElectronico);
        out.receptor = ro;
    }

    out.condicionVenta = texto(r.condicionVenta);
    out.condicionVentaOtros = texto(r.condicionVentaOtros);
    out.plazoCredito = entero(r.plazoCredito);
    out.detalleServicio = lista(r.detalleServicio, r.numDetalleServicio, linea);
    out.otrosCargos = lista(r.otrosCargos, r.numOtrosCargos, [](const cec_otro_cargo_c& c) {
        dto::OtrosCargosInput oc;
        oc.tipoDocumentoOC = texto(c.tipoDocumentoOC);
        oc.tipoDocumentoOTROS = texto(c.tipoDocumentoOTROS);
        if (c.identificacionTercero) oc.identificacionTercero = identificacion(*c.identificacionTercero);
        oc.nombreTercero = texto(c.nombreTercero);
        oc.detalle = texto(c.detalle);
        oc.porcentajeOC = texto(c.porcentajeOC);
        oc.montoCargo = texto(c.montoCargo);
        return oc;
    });
    out.codigoMoneda = texto(r.codigoMoneda);
    out.tipoCambio = texto(r.tipoCambio);
    out.mediosPago = lista(r.mediosPago, r.numMediosPago, [](const cec_medio_pago_c& m) {
        return dto::MedioPagoInput{ texto(m.tipoMedioPago), texto(m.medioPagoOtros), texto(m.totalMedioPago) };
    });
    out.informacionReferencia = lista(r.informacionReferencia, r.numInformacionReferencia, [](const cec_referencia_c& i) {
        return dto::InformacionReferenciaInput{ texto(i.tipoDocIR), texto(i.tipoDocRefOTRO), texto(i.numero),
            texto(i.fechaEmisionIR), texto(i.codigo), texto(i.codigoReferenciaOTRO), texto(i.razon) };
    });
    return out;
}

using Operacion = cec::service::ComprobanteResult (*)(const dto::ComprobanteRequestDTO&);

cec::service::ComprobanteResult procesar(const dto::ComprobanteRequestDTO& r) {
    return cec::service::ComprobanteService::process(r);
}

cec::service::ComprobanteResult calcular(const dto::ComprobanteRequestDTO& r) {
    return cec::service::ComprobanteService::calcular(r);
}

cec_result* nuevoResultado(const cec::service::ComprobanteResult& res);

cec_status desdeStructs(const cec_request_c* request, cec_result** out_result, Operacion op) {
    if (!out_result) {
        return CEC_STATUS_ERR_NULL_POINTER;
    }
    *out_result = nullptr;
    if (!request) {
        return CEC_STATUS_ERR_NULL_POINTER;
    }
    try {
        *out_result = nuevoResultado(op(mapRequest(*request)));
        return CEC_STATUS_OK;
    } catch (...) {
        return CEC_STATUS_ERR_INTERNAL;
    }
}

cec_status desdeJson(const uint8_t* json_data, size_t json_len, cec_result** out_result, Operacion op) {
    if (!out_result) {
        return CEC_STATUS_ERR_NULL_POINTER;
    }
    *out_result = nullptr;
    if (!json_data) {
        return CEC_STATUS_ERR_NULL_POINTER;
    }
    if (json_len == 0 || json_len > 10 * 1024 * 1024) {
        return CEC_STATUS_ERR_SIZE_LIMIT;
    }
    try {
        std::string_view sv(reinterpret_cast<const char*>(json_data), json_len);
        dto::ComprobanteRequestDTO request;
        try {
            request = cec::codec::JsonCodec::decodeRequest(sv);
        } catch (const std::exception& e) {
            cec::service::ComprobanteResult res;
            res.diagnostics.addError("JSON_DECODE_ERROR", "$", e.what(), cec::domain::DiagnosticCategory::Structural);
            *out_result = nuevoResultado(res);
            return CEC_STATUS_OK;
        }
        *out_result = nuevoResultado(op(request));
        return CEC_STATUS_OK;
    } catch (...) {
        return CEC_STATUS_ERR_INTERNAL;
    }
}

cec_result* nuevoResultado(const cec::service::ComprobanteResult& res) {
    auto* r = new cec_result();
    r->isSuccess = res.isSuccess;
    r->jsonData = cec::codec::JsonCodec::encodeResult(res);
    r->xmlData = res.xml;
    if (res.comprobante.has_value()) {
        r->resumen = res.comprobante->resumen;
        r->tieneComprobante = true;
    }
    r->diagnosticos = res.diagnostics.items();
    return r;
}

} // namespace

extern "C" {

uint32_t cec_abi_version(void) {
    return 3;
}

cec_status cec_build(
    const cec_request_c* request,
    cec_result** out_result
) {
    return desdeStructs(request, out_result, procesar);
}

cec_status cec_build_json(
    const uint8_t* json_data,
    size_t json_len,
    cec_result** out_result
) {
    return desdeJson(json_data, json_len, out_result, procesar);
}

cec_status cec_calcular(
    const cec_request_c* request,
    cec_result** out_result
) {
    return desdeStructs(request, out_result, calcular);
}

cec_status cec_calcular_json(
    const uint8_t* json_data,
    size_t json_len,
    cec_result** out_result
) {
    return desdeJson(json_data, json_len, out_result, calcular);
}

cec_status cec_sign_xml(
    const uint8_t* xml_data,
    size_t xml_len,
    const uint8_t* p12_data,
    size_t p12_len,
    const char* pin,
    cec_result** out_result
) {
    if (!out_result) {
        return CEC_STATUS_ERR_NULL_POINTER;
    }
    *out_result = nullptr;

    if (!xml_data || !p12_data || !pin) {
        return CEC_STATUS_ERR_NULL_POINTER;
    }

    if (xml_len == 0 || xml_len > 10 * 1024 * 1024 || p12_len == 0 || p12_len > 10 * 1024 * 1024) {
        return CEC_STATUS_ERR_SIZE_LIMIT;
    }

    try {
        std::string xmlStr(reinterpret_cast<const char*>(xml_data), xml_len);
        std::vector<uint8_t> p12Bytes(p12_data, p12_data + p12_len);
        std::string pinStr(pin);

        auto signResult = cec::service::ComprobanteService::signXml(xmlStr, p12Bytes, pinStr);

        auto* r = new cec_result();
        r->isSuccess = signResult.isSuccess;
        r->jsonData = cec::codec::JsonCodec::encodeSignResult(signResult);
        r->xmlData = signResult.xml;
        r->diagnosticos = signResult.diagnostics.items();
        r->certificado = signResult.certInfo;
        *out_result = r;

        return CEC_STATUS_OK;
    } catch (...) {
        return CEC_STATUS_ERR_INTERNAL;
    }
}

namespace {

/// Resultado de una operación de firma (cec_sign_xml, cec_preparar_firma, cec_completar_firma).
cec_result* resultadoFirma(const cec::service::FirmaResult& f) {
    auto* r = new cec_result();
    r->isSuccess = f.isSuccess;
    r->jsonData = cec::codec::JsonCodec::encodeSignResult(f);
    r->xmlData = f.xml;
    r->diagnosticos = f.diagnostics.items();
    r->certificado = f.certInfo;
    r->signedInfo = f.signedInfo;
    return r;
}

} // namespace

cec_status cec_preparar_firma(
    const uint8_t* xml_data,
    size_t xml_len,
    const uint8_t* cert_der,
    size_t cert_len,
    cec_result** out_result
) {
    if (!out_result) return CEC_STATUS_ERR_NULL_POINTER;
    *out_result = nullptr;
    if (!xml_data || !cert_der) return CEC_STATUS_ERR_NULL_POINTER;
    if (xml_len == 0 || xml_len > 10 * 1024 * 1024 || cert_len == 0 || cert_len > 1024 * 1024) {
        return CEC_STATUS_ERR_SIZE_LIMIT;
    }
    try {
        std::string xmlStr(reinterpret_cast<const char*>(xml_data), xml_len);
        std::vector<uint8_t> der(cert_der, cert_der + cert_len);
        *out_result = resultadoFirma(cec::service::ComprobanteService::prepararFirma(xmlStr, der));
        return CEC_STATUS_OK;
    } catch (...) {
        return CEC_STATUS_ERR_INTERNAL;
    }
}

cec_status cec_completar_firma(
    const uint8_t* xml_data,
    size_t xml_len,
    const char* signature_value_base64,
    cec_result** out_result
) {
    if (!out_result) return CEC_STATUS_ERR_NULL_POINTER;
    *out_result = nullptr;
    if (!xml_data || !signature_value_base64) return CEC_STATUS_ERR_NULL_POINTER;
    if (xml_len == 0 || xml_len > 10 * 1024 * 1024) return CEC_STATUS_ERR_SIZE_LIMIT;
    try {
        std::string xmlStr(reinterpret_cast<const char*>(xml_data), xml_len);
        *out_result = resultadoFirma(cec::service::ComprobanteService::completarFirma(xmlStr, signature_value_base64));
        return CEC_STATUS_OK;
    } catch (...) {
        return CEC_STATUS_ERR_INTERNAL;
    }
}

} // extern "C"

// Helpers con tipos de C++: fuera del bloque extern "C" para no tener enlace de C.
namespace {

nlohmann::json diagnosticosJson(const cec::domain::DiagnosticList& diags) {
    nlohmann::json out = nlohmann::json::array();
    for (const auto& d : diags.items()) {
        out.push_back({
            {"severity", d.severity == cec::domain::DiagnosticSeverity::Error ? "error" : "warning"},
            {"code", d.code},
            {"path", d.path},
            {"message", d.message}
        });
    }
    return out;
}

cec_result* resultadoMensaje(bool exito, const cec::domain::DiagnosticList& diags, nlohmann::json datos, std::string xml) {
    auto* r = new cec_result();
    r->isSuccess = exito;
    datos["status"] = exito ? "ok" : "invalid";
    datos["diagnostics"] = diagnosticosJson(diags);
    if (!xml.empty()) datos["xml"] = xml;
    r->jsonData = datos.dump();
    r->xmlData = std::move(xml);
    r->diagnosticos = diags.items();
    return r;
}

/// Campo de texto del JSON: los montos se aceptan también como número.
std::string campoMensaje(const nlohmann::json& j, const char* nombre) {
    if (!j.contains(nombre) || j[nombre].is_null()) return "";
    const auto& v = j[nombre];
    if (v.is_string()) return v.get<std::string>();
    if (v.is_number_integer()) return std::to_string(v.get<long long>());
    if (v.is_number()) {
        std::ostringstream s;
        s << std::fixed << std::setprecision(5) << v.get<double>();
        return s.str();
    }
    throw std::invalid_argument(std::string("El campo ") + nombre + " debe ser texto.");
}

/// Valida el mensaje y arma el resultado (compartido por cec_mensaje_receptor y cec_mensaje_receptor_json).
cec_result* resultadoDeMensaje(const cec::mensaje::MensajeReceptorRequest& req) {
    auto res = cec::mensaje::procesar(req);
    nlohmann::json datos = nlohmann::json::object();
    if (!res.plazoConfirmacion.empty()) datos["plazoConfirmacion"] = res.plazoConfirmacion;
    cec_result* r = resultadoMensaje(res.isSuccess, res.diagnostics, std::move(datos), std::move(res.xml));
    r->plazoConfirmacion = res.plazoConfirmacion;
    return r;
}

/// Resultado de leer un comprobante recibido (compartido por la lectura y sus accesores).
cec_result* resultadoDeLectura(const cec::mensaje::ComprobanteRecibido& c, const cec::domain::DiagnosticList& diags) {
    nlohmann::json comp = {
        {"tipoDocumento", c.tipoDocumento},
        {"nombreDocumento", c.nombreDocumento},
        {"version", c.version},
        {"clave", c.clave},
        {"numeroConsecutivo", c.numeroConsecutivo},
        {"fechaEmision", c.fechaEmision},
        {"emisor", {{"nombre", c.emisorNombre}, {"tipoIdentificacion", c.emisorTipoIdentificacion},
                    {"numeroIdentificacion", c.emisorNumeroIdentificacion}, {"correo", c.emisorCorreo}}},
        {"receptor", {{"nombre", c.receptorNombre}, {"tipoIdentificacion", c.receptorTipoIdentificacion},
                      {"numeroIdentificacion", c.receptorNumeroIdentificacion}}},
        {"codigoActividadReceptor", c.codigoActividadReceptor},
        {"codigoMoneda", c.codigoMoneda},
        {"tipoCambio", c.tipoCambio},
        {"totalImpuesto", c.totalImpuesto},
        {"totalComprobante", c.totalComprobante},
        {"firmado", c.firmado},
        {"plazoConfirmacion", c.plazoConfirmacion},
    };
    cec_result* r = resultadoMensaje(!diags.hasErrors(), diags, {{"comprobante", comp}}, "");
    r->recibido = c;
    r->tieneRecibido = !c.nombreDocumento.empty();
    r->plazoConfirmacion = c.plazoConfirmacion;
    return r;
}

} // namespace

extern "C" {

cec_status cec_mensaje_receptor_json(
    const uint8_t* json_data,
    size_t json_len,
    cec_result** out_result
) {
    if (!out_result) return CEC_STATUS_ERR_NULL_POINTER;
    *out_result = nullptr;
    if (!json_data) return CEC_STATUS_ERR_NULL_POINTER;
    if (json_len == 0 || json_len > 1024 * 1024) return CEC_STATUS_ERR_SIZE_LIMIT;
    try {
        cec::mensaje::MensajeReceptorRequest req;
        try {
            const auto j = nlohmann::json::parse(std::string_view(reinterpret_cast<const char*>(json_data), json_len));
            if (!j.is_object()) throw std::invalid_argument("El mensaje debe ser un objeto JSON.");
            req.clave = campoMensaje(j, "clave");
            req.numeroCedulaEmisor = campoMensaje(j, "numeroCedulaEmisor");
            req.fechaEmisionDoc = campoMensaje(j, "fechaEmisionDoc");
            req.mensaje = campoMensaje(j, "mensaje");
            req.detalleMensaje = campoMensaje(j, "detalleMensaje");
            req.montoTotalImpuesto = campoMensaje(j, "montoTotalImpuesto");
            req.codigoActividad = campoMensaje(j, "codigoActividad");
            req.condicionImpuesto = campoMensaje(j, "condicionImpuesto");
            req.montoTotalImpuestoAcreditar = campoMensaje(j, "montoTotalImpuestoAcreditar");
            req.montoTotalDeGastoAplicable = campoMensaje(j, "montoTotalDeGastoAplicable");
            req.totalFactura = campoMensaje(j, "totalFactura");
            req.numeroCedulaReceptor = campoMensaje(j, "numeroCedulaReceptor");
            req.numeroConsecutivoReceptor = campoMensaje(j, "numeroConsecutivoReceptor");
            req.fechaEmisionComprobante = campoMensaje(j, "fechaEmisionComprobante");
        } catch (const std::exception& e) {
            cec::domain::DiagnosticList diags;
            diags.addError("JSON_DECODE_ERROR", "$", e.what(), cec::domain::DiagnosticCategory::Structural);
            *out_result = resultadoMensaje(false, diags, nlohmann::json::object(), "");
            return CEC_STATUS_OK;
        }
        *out_result = resultadoDeMensaje(req);
        return CEC_STATUS_OK;
    } catch (...) {
        return CEC_STATUS_ERR_INTERNAL;
    }
}

cec_status cec_leer_comprobante_xml(
    const uint8_t* xml_data,
    size_t xml_len,
    cec_result** out_result
) {
    if (!out_result) return CEC_STATUS_ERR_NULL_POINTER;
    *out_result = nullptr;
    if (!xml_data) return CEC_STATUS_ERR_NULL_POINTER;
    if (xml_len == 0 || xml_len > 10 * 1024 * 1024) return CEC_STATUS_ERR_SIZE_LIMIT;
    try {
        auto [c, diags] = cec::mensaje::leerComprobante(std::string(reinterpret_cast<const char*>(xml_data), xml_len));
        *out_result = resultadoDeLectura(c, diags);
        return CEC_STATUS_OK;
    } catch (...) {
        return CEC_STATUS_ERR_INTERNAL;
    }
}

cec_status cec_mensaje_receptor(
    const cec_mensaje_receptor_c* request,
    cec_result** out_result
) {
    if (!out_result) return CEC_STATUS_ERR_NULL_POINTER;
    *out_result = nullptr;
    if (!request) return CEC_STATUS_ERR_NULL_POINTER;
    try {
        cec::mensaje::MensajeReceptorRequest req;
        req.clave = texto(request->clave);
        req.numeroCedulaEmisor = texto(request->numeroCedulaEmisor);
        req.fechaEmisionDoc = texto(request->fechaEmisionDoc);
        req.mensaje = texto(request->mensaje);
        req.detalleMensaje = texto(request->detalleMensaje);
        req.montoTotalImpuesto = texto(request->montoTotalImpuesto);
        req.codigoActividad = texto(request->codigoActividad);
        req.condicionImpuesto = texto(request->condicionImpuesto);
        req.montoTotalImpuestoAcreditar = texto(request->montoTotalImpuestoAcreditar);
        req.montoTotalDeGastoAplicable = texto(request->montoTotalDeGastoAplicable);
        req.totalFactura = texto(request->totalFactura);
        req.numeroCedulaReceptor = texto(request->numeroCedulaReceptor);
        req.numeroConsecutivoReceptor = texto(request->numeroConsecutivoReceptor);
        req.fechaEmisionComprobante = texto(request->fechaEmisionComprobante);
        *out_result = resultadoDeMensaje(req);
        return CEC_STATUS_OK;
    } catch (...) {
        return CEC_STATUS_ERR_INTERNAL;
    }
}

cec_status cec_result_plazo_confirmacion(const cec_result* res, const char** out_plazo) {
    if (!res || !out_plazo) return CEC_STATUS_ERR_NULL_POINTER;
    if (res->plazoConfirmacion.empty()) return CEC_STATUS_ERR_OUT_OF_RANGE;
    *out_plazo = res->plazoConfirmacion.c_str();
    return CEC_STATUS_OK;
}

cec_status cec_result_signed_info(const cec_result* res, const uint8_t** data, size_t* len) {
    if (!res || !data || !len) return CEC_STATUS_ERR_NULL_POINTER;
    if (res->signedInfo.empty()) return CEC_STATUS_ERR_OUT_OF_RANGE;
    *data = reinterpret_cast<const uint8_t*>(res->signedInfo.data());
    *len = res->signedInfo.size();
    return CEC_STATUS_OK;
}

cec_status cec_result_comprobante_recibido(const cec_result* res, cec_comprobante_recibido_c* out) {
    if (!res || !out) return CEC_STATUS_ERR_NULL_POINTER;
    if (!res->tieneRecibido) return CEC_STATUS_ERR_OUT_OF_RANGE;
    const auto& c = res->recibido;
    out->tipoDocumento = c.tipoDocumento.c_str();
    out->nombreDocumento = c.nombreDocumento.c_str();
    out->version = c.version.c_str();
    out->clave = c.clave.c_str();
    out->numeroConsecutivo = c.numeroConsecutivo.c_str();
    out->fechaEmision = c.fechaEmision.c_str();
    out->emisorNombre = c.emisorNombre.c_str();
    out->emisorTipoIdentificacion = c.emisorTipoIdentificacion.c_str();
    out->emisorNumeroIdentificacion = c.emisorNumeroIdentificacion.c_str();
    out->emisorCorreo = c.emisorCorreo.c_str();
    out->receptorNombre = c.receptorNombre.c_str();
    out->receptorTipoIdentificacion = c.receptorTipoIdentificacion.c_str();
    out->receptorNumeroIdentificacion = c.receptorNumeroIdentificacion.c_str();
    out->codigoActividadReceptor = c.codigoActividadReceptor.c_str();
    out->codigoMoneda = c.codigoMoneda.c_str();
    out->tipoCambio = c.tipoCambio.c_str();
    out->totalImpuesto = c.totalImpuesto.c_str();
    out->totalComprobante = c.totalComprobante.c_str();
    out->firmado = c.firmado ? 1 : 0;
    out->plazoConfirmacion = c.plazoConfirmacion.c_str();
    return CEC_STATUS_OK;
}

cec_status cec_leer_certificado(
    const uint8_t* p12_data,
    size_t p12_len,
    const char* pin,
    cec_result** out_result
) {
    if (!out_result) {
        return CEC_STATUS_ERR_NULL_POINTER;
    }
    *out_result = nullptr;
    if (!p12_data || !pin) {
        return CEC_STATUS_ERR_NULL_POINTER;
    }
    if (p12_len == 0 || p12_len > 10 * 1024 * 1024) {
        return CEC_STATUS_ERR_SIZE_LIMIT;
    }
    try {
        std::vector<uint8_t> p12Bytes(p12_data, p12_data + p12_len);
        auto lectura = cec::service::ComprobanteService::leerCertificado(p12Bytes, pin);
        auto* r = new cec_result();
        r->isSuccess = lectura.isSuccess;
        r->jsonData = cec::codec::JsonCodec::encodeSignResult(lectura);
        r->diagnosticos = lectura.diagnostics.items();
        r->certificado = lectura.certInfo;
        *out_result = r;
        return CEC_STATUS_OK;
    } catch (...) {
        return CEC_STATUS_ERR_INTERNAL;
    }
}

cec_status cec_result_certificado(
    const cec_result* res,
    cec_certificado_c* out_certificado
) {
    if (!res || !out_certificado) {
        return CEC_STATUS_ERR_NULL_POINTER;
    }
    const auto& c = res->certificado;
    if (c.certificadoBase64.empty()) {
        return CEC_STATUS_ERR_OUT_OF_RANGE;
    }
    out_certificado->sujeto = c.sujeto.c_str();
    out_certificado->emisor = c.emisor.c_str();
    out_certificado->numeroSerie = c.serialNumber.c_str();
    out_certificado->validoDesde = c.validoDesde.c_str();
    out_certificado->validoHasta = c.validoHasta.c_str();
    out_certificado->sha256Hex = c.digestSha256Hex.c_str();
    out_certificado->sha256Base64 = c.digestSha256Base64.c_str();
    out_certificado->certificadoBase64 = c.certificadoBase64.c_str();
    out_certificado->vigente = c.esValido ? 1 : 0;
    return CEC_STATUS_OK;
}

cec_status cec_result_is_success(
    const cec_result* res,
    int32_t* out_is_success
) {
    if (!res || !out_is_success) {
        return CEC_STATUS_ERR_NULL_POINTER;
    }
    *out_is_success = res->isSuccess ? 1 : 0;
    return CEC_STATUS_OK;
}

cec_status cec_result_xml(
    const cec_result* res,
    const uint8_t** data,
    size_t* len
) {
    if (!res || !data || !len) {
        return CEC_STATUS_ERR_NULL_POINTER;
    }
    if (res->xmlData.empty()) {
        *data = nullptr;
        *len = 0;
    } else {
        *data = reinterpret_cast<const uint8_t*>(res->xmlData.data());
        *len = res->xmlData.size();
    }
    return CEC_STATUS_OK;
}

cec_status cec_result_totales(
    const cec_result* res,
    cec_resumen_totales_c* out_totales
) {
    if (!res || !out_totales) {
        return CEC_STATUS_ERR_NULL_POINTER;
    }
    if (!res->tieneComprobante) {
        return CEC_STATUS_ERR_OUT_OF_RANGE;
    }
    const auto& t = res->resumen;
    out_totales->totalServGravados = t.totalServiciosGravados.scaledValue();
    out_totales->totalServExentos = t.totalServiciosExentos.scaledValue();
    out_totales->totalServExonerado = t.totalServiciosExonerados.scaledValue();
    out_totales->totalServNoSujeto = t.totalServiciosNoSujetos.scaledValue();
    out_totales->totalMercanciasGravadas = t.totalMercanciasGravadas.scaledValue();
    out_totales->totalMercanciasExentas = t.totalMercanciasExentas.scaledValue();
    out_totales->totalMercExonerada = t.totalMercanciasExoneradas.scaledValue();
    out_totales->totalMercNoSujeta = t.totalMercanciasNoSujetas.scaledValue();
    out_totales->totalGravado = t.totalGravado.scaledValue();
    out_totales->totalExento = t.totalExento.scaledValue();
    out_totales->totalExonerado = t.totalExonerado.scaledValue();
    out_totales->totalNoSujeto = t.totalNoSujeto.scaledValue();
    out_totales->totalVenta = t.totalVenta.scaledValue();
    out_totales->totalDescuentos = t.totalDescuentos.scaledValue();
    out_totales->totalVentaNeta = t.totalVentaNeta.scaledValue();
    out_totales->totalImpuesto = t.totalImpuesto.scaledValue();
    out_totales->totalImpAsumEmisorFabrica = t.totalImpAsumEmisorFabrica.scaledValue();
    out_totales->totalIVADevuelto = t.totalIvaDevuelto.scaledValue();
    out_totales->totalOtrosCargos = t.totalOtrosCargos.scaledValue();
    out_totales->totalComprobante = t.totalComprobante.scaledValue();

    return CEC_STATUS_OK;
}

cec_status cec_result_data(
    const cec_result* res,
    const uint8_t** data,
    size_t* len
) {
    if (!res || !data || !len) {
        return CEC_STATUS_ERR_NULL_POINTER;
    }
    *data = reinterpret_cast<const uint8_t*>(res->jsonData.data());
    *len = res->jsonData.size();
    return CEC_STATUS_OK;
}

cec_status cec_result_total_diagnosticos(
    const cec_result* res,
    size_t* out_total
) {
    if (!res || !out_total) {
        return CEC_STATUS_ERR_NULL_POINTER;
    }
    *out_total = res->diagnosticos.size();
    return CEC_STATUS_OK;
}

cec_status cec_result_diagnostico(
    const cec_result* res,
    size_t indice,
    cec_severidad* out_severidad,
    const char** out_codigo,
    const char** out_ruta,
    const char** out_mensaje
) {
    if (!res) {
        return CEC_STATUS_ERR_NULL_POINTER;
    }
    if (indice >= res->diagnosticos.size()) {
        return CEC_STATUS_ERR_OUT_OF_RANGE;
    }
    const auto& d = res->diagnosticos[indice];
    if (out_severidad) {
        *out_severidad = d.severity == cec::domain::DiagnosticSeverity::Error ? CEC_SEVERIDAD_ERROR : CEC_SEVERIDAD_ADVERTENCIA;
    }
    if (out_codigo) *out_codigo = d.code.c_str();
    if (out_ruta) *out_ruta = d.path.c_str();
    if (out_mensaje) *out_mensaje = d.message.c_str();
    return CEC_STATUS_OK;
}

void cec_result_free(cec_result* res) {
    delete res;
}

/* ========================================================================= */
/* C-API: DIVISIÓN TERRITORIAL DE COSTA RICA (DTA)                          */
/* ========================================================================= */
uint32_t cec_dta_total_provincias(void) {
    return cec::common::DivisionTerritorial::TotalProvincias();
}
 
int32_t cec_dta_provincia_por_indice(
    uint32_t indice,
    uint8_t* out_codigo,
    const char** out_nombre,
    uint32_t* out_total_cantones
) {
    cec::common::EntradaTerritorial e;
    if (!cec::common::DivisionTerritorial::ProvinciaPorIndice(indice, e)) return 0;
    if (out_codigo) *out_codigo = e.codigo;
    if (out_nombre) *out_nombre = CStr(e.nombre);
    if (out_total_cantones) *out_total_cantones = cec::common::DivisionTerritorial::TotalCantones(e.codigo);
    return 1;
}
 
uint32_t cec_dta_total_cantones(uint8_t codigo_provincia) {
    return cec::common::DivisionTerritorial::TotalCantones(codigo_provincia);
}
 
int32_t cec_dta_canton_por_indice(
    uint8_t codigo_provincia,
    uint32_t indice,
    uint8_t* out_codigo,
    const char** out_nombre,
    uint32_t* out_total_distritos
) {
    cec::common::EntradaTerritorial e;
    if (!cec::common::DivisionTerritorial::CantonPorIndice(codigo_provincia, indice, e)) return 0;
    if (out_codigo) *out_codigo = e.codigo;
    if (out_nombre) *out_nombre = CStr(e.nombre);
    if (out_total_distritos) *out_total_distritos = cec::common::DivisionTerritorial::TotalDistritos(codigo_provincia, e.codigo);
    return 1;
}
 
uint32_t cec_dta_total_distritos(uint8_t codigo_provincia, uint8_t codigo_canton) {
    return cec::common::DivisionTerritorial::TotalDistritos(codigo_provincia, codigo_canton);
}
 
int32_t cec_dta_distrito_por_indice(
    uint8_t codigo_provincia,
    uint8_t codigo_canton,
    uint32_t indice,
    uint8_t* out_codigo,
    const char** out_nombre,
    uint32_t* out_total_barrios
) {
    cec::common::EntradaTerritorial e;
    if (!cec::common::DivisionTerritorial::DistritoPorIndice(codigo_provincia, codigo_canton, indice, e)) return 0;
    if (out_codigo) *out_codigo = e.codigo;
    if (out_nombre) *out_nombre = CStr(e.nombre);
    if (out_total_barrios) *out_total_barrios = cec::common::DivisionTerritorial::TotalBarrios(codigo_provincia, codigo_canton, e.codigo);
    return 1;
}
 
uint32_t cec_dta_total_barrios(uint8_t codigo_provincia, uint8_t codigo_canton, uint8_t codigo_distrito) {
    return cec::common::DivisionTerritorial::TotalBarrios(codigo_provincia, codigo_canton, codigo_distrito);
}
 
int32_t cec_dta_barrio_por_indice(
    uint8_t codigo_provincia,
    uint8_t codigo_canton,
    uint8_t codigo_distrito,
    uint32_t indice,
    uint8_t* out_codigo,
    const char** out_nombre
) {
    cec::common::EntradaTerritorial e;
    if (!cec::common::DivisionTerritorial::BarrioPorIndice(codigo_provincia, codigo_canton, codigo_distrito, indice, e)) return 0;
    if (out_codigo) *out_codigo = e.codigo;
    if (out_nombre) *out_nombre = CStr(e.nombre);
    return 1;
}
 
int32_t cec_dta_validar_provincia(uint8_t codigo_provincia) {
    return cec::common::DivisionTerritorial::EsProvinciaValida(codigo_provincia) ? 1 : 0;
}
 
int32_t cec_dta_validar_canton(uint8_t codigo_provincia, uint8_t codigo_canton) {
    return cec::common::DivisionTerritorial::EsCantonValido(codigo_provincia, codigo_canton) ? 1 : 0;
}
 
int32_t cec_dta_validar_distrito(uint8_t codigo_provincia, uint8_t codigo_canton, uint8_t codigo_distrito) {
    return cec::common::DivisionTerritorial::EsDistritoValido(codigo_provincia, codigo_canton, codigo_distrito) ? 1 : 0;
}
 
int32_t cec_dta_validar_barrio(uint8_t codigo_provincia, uint8_t codigo_canton, uint8_t codigo_distrito, uint8_t codigo_barrio) {
    return cec::common::DivisionTerritorial::EsBarrioValido(codigo_provincia, codigo_canton, codigo_distrito, codigo_barrio) ? 1 : 0;
}
 
int32_t cec_dta_validar_ubicacion(uint8_t codigo_provincia, uint8_t codigo_canton, uint8_t codigo_distrito) {
    return cec::common::DivisionTerritorial::EsUbicacionValida(codigo_provincia, codigo_canton, codigo_distrito) ? 1 : 0;
}
 
const char* cec_dta_nombre_provincia(uint8_t codigo_provincia) {
    return CStr(cec::common::DivisionTerritorial::GetNombreProvincia(codigo_provincia));
}
 
const char* cec_dta_nombre_canton(uint8_t codigo_provincia, uint8_t codigo_canton) {
    return CStr(cec::common::DivisionTerritorial::GetNombreCanton(codigo_provincia, codigo_canton));
}
 
const char* cec_dta_nombre_distrito(uint8_t codigo_provincia, uint8_t codigo_canton, uint8_t codigo_distrito) {
    return CStr(cec::common::DivisionTerritorial::GetNombreDistrito(codigo_provincia, codigo_canton, codigo_distrito));
}
 
const char* cec_dta_nombre_barrio(uint8_t codigo_provincia, uint8_t codigo_canton, uint8_t codigo_distrito, uint8_t codigo_barrio) {
    return CStr(cec::common::DivisionTerritorial::GetNombreBarrio(codigo_provincia, codigo_canton, codigo_distrito, codigo_barrio));
}

/* ========================================================================= */
/* C-API: CATÁLOGOS OFICIALES DE FACTURACIÓN ELECTRÓNICA V4.4               */
/* ========================================================================= */

uint32_t cec_catalogo_total_monedas(void) {
    return static_cast<uint32_t>(cec::common::Monedas::GetTodas().size());
}

int32_t cec_catalogo_moneda_por_indice(
    uint32_t indice,
    const char** out_codigo,
    const char** out_simbolo,
    const char** out_descripcion
) {
    const auto& todas = cec::common::Monedas::GetTodas();
    if (indice >= todas.size()) return 0;
    const auto& m = todas[indice];
    if (out_codigo) *out_codigo = m.codigo.c_str();
    if (out_simbolo) *out_simbolo = m.simbolo.c_str();
    if (out_descripcion) *out_descripcion = m.descripcion.c_str();
    return 1;
}

uint32_t cec_catalogo_total_unidades_medida(void) {
    return static_cast<uint32_t>(cec::common::UnidadesMedida::GetTodas().size());
}

int32_t cec_catalogo_unidad_medida_por_indice(
    uint32_t indice,
    const char** out_codigo,
    const char** out_simbolo,
    const char** out_descripcion
) {
    const auto& todas = cec::common::UnidadesMedida::GetTodas();
    if (indice >= todas.size()) return 0;
    const auto& u = todas[indice];
    if (out_codigo) *out_codigo = u.codigo.c_str();
    if (out_simbolo) *out_simbolo = u.simbolo.c_str();
    if (out_descripcion) *out_descripcion = u.descripcion.c_str();
    return 1;
}

uint32_t cec_catalogo_total_formas_farmaceuticas(void) {
    return static_cast<uint32_t>(cec::common::FormasFarmaceuticas::GetTodas().size());
}

int32_t cec_catalogo_forma_farmaceutica_por_indice(
    uint32_t indice,
    const char** out_codigo,
    const char** out_descripcion
) {
    const auto& todas = cec::common::FormasFarmaceuticas::GetTodas();
    if (indice >= todas.size()) return 0;
    const auto& ff = todas[indice];
    if (out_codigo) *out_codigo = ff.codigo.c_str();
    if (out_descripcion) *out_descripcion = ff.descripcion.c_str();
    return 1;
}

uint32_t cec_catalogo_total_nombres(void) {
    static const auto s_nombres = cec::common::Catalogos::GetNombresCatalogos();
    return static_cast<uint32_t>(s_nombres.size());
}

const char* cec_catalogo_nombre_por_indice(uint32_t indice) {
    static const auto s_nombres = cec::common::Catalogos::GetNombresCatalogos();
    if (indice >= s_nombres.size()) return "";
    return s_nombres[indice].c_str();
}

uint32_t cec_catalogo_total_elementos(const char* nombre_catalogo) {
    if (!nombre_catalogo) return 0;
    return static_cast<uint32_t>(cec::common::Catalogos::GetPorNombre(nombre_catalogo).size());
}

int32_t cec_catalogo_elemento_por_indice(
    const char* nombre_catalogo,
    uint32_t indice,
    const char** out_codigo,
    const char** out_descripcion
) {
    if (!nombre_catalogo) return 0;
    static thread_local std::vector<cec::common::ElementoCatalogo> s_elementosCache;
    static thread_local std::string s_ultimoCatalogo;

    if (s_ultimoCatalogo != nombre_catalogo) {
        s_elementosCache = cec::common::Catalogos::GetPorNombre(nombre_catalogo);
        s_ultimoCatalogo = nombre_catalogo;
    }

    if (indice >= s_elementosCache.size()) return 0;
    const auto& el = s_elementosCache[indice];
    if (out_codigo) *out_codigo = el.codigo.c_str();
    if (out_descripcion) *out_descripcion = el.descripcion.c_str();
    return 1;
}

} // extern "C"
