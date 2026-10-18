#include "cec/codec/JsonCodec.h"
#include "cec/codec/EscritorJson.h"
#include <json.hpp>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <optional>
#include <cstdint>
#include <cstdio>

using json = nlohmann::ordered_json; // Conserva el orden de inserción (estructura del Anexo 1)

namespace cec::codec {

namespace {

using namespace parsing;

/**
 * @brief Lee el request con validación de tipos. Un campo ausente o null queda vacío; un campo con un tipo
 * incorrecto lanza std::invalid_argument con su ruta (ej. "detalleServicio[0].impuestos[1].codigo").
 * Las reglas de negocio (obligatoriedad, catálogos, formatos) se validan después en el RuleEngine.
 */
class LectorRequest {
public:
    static ComprobanteRequestDTO leer(const json& raiz) {
        if (!raiz.is_object()) throw std::invalid_argument("El request debe ser un objeto JSON.");
        ComprobanteRequestDTO r;
        r.tipoDocumento = codigo(raiz, "tipoDocumento", "");
        r.clave = texto(raiz, "clave", "");
        r.proveedorSistemas = texto(raiz, "proveedorSistemas", "");
        r.codigoActividadEmisor = texto(raiz, "codigoActividadEmisor", "");
        r.codigoActividadReceptor = texto(raiz, "codigoActividadReceptor", "");
        r.numeroConsecutivo = texto(raiz, "numeroConsecutivo", "");
        r.fechaEmision = texto(raiz, "fechaEmision", "");

        if (const json* e = objeto(raiz, "emisor", "")) r.emisor = emisor(*e, "emisor");
        if (const json* rec = objeto(raiz, "receptor", "")) r.receptor = receptor(*rec, "receptor");

        r.condicionVenta = codigo(raiz, "condicionVenta", "");
        r.condicionVentaOtros = texto(raiz, "condicionVentaOtros", "");
        r.plazoCredito = entero(raiz, "plazoCredito", "");

        recorrerLista(raiz, "detalleServicio", "", [&](const json& l, const std::string& p) {
            r.detalleServicio.push_back(lineaDetalle(l, p));
        });
        recorrerLista(raiz, "otrosCargos", "", [&](const json& c, const std::string& p) {
            r.otrosCargos.push_back(otroCargo(c, p));
        });

        if (const json* res = objeto(raiz, "resumenFactura", "")) {
            const std::string rp = "resumenFactura";
            if (const json* m = objeto(*res, "codigoTipoMoneda", rp)) {
                r.codigoMoneda = texto(*m, "codigoMoneda", rp + ".codigoTipoMoneda");
                r.tipoCambio = decimal(*m, "tipoCambio", rp + ".codigoTipoMoneda");
            }
            recorrerLista(*res, "mediosPago", rp, [&](const json& m, const std::string& p) {
                r.mediosPago.push_back({ codigo(m, "tipoMedioPago", p), texto(m, "medioPagoOtros", p), decimal(m, "totalMedioPago", p) });
            });
        }

        recorrerLista(raiz, "informacionReferencia", "", [&](const json& i, const std::string& p) {
            InformacionReferenciaInput ref;
            ref.tipoDocIR = codigo(i, "tipoDocIR", p);
            ref.tipoDocRefOTRO = texto(i, "tipoDocRefOTRO", p);
            ref.numero = texto(i, "numero", p);
            ref.fechaEmisionIR = texto(i, "fechaEmisionIR", p);
            ref.codigo = codigo(i, "codigo", p);
            ref.codigoReferenciaOTRO = texto(i, "codigoReferenciaOTRO", p);
            ref.razon = texto(i, "razon", p);
            r.informacionReferencia.push_back(ref);
        });
        return r;
    }

private:
    static std::string ruta(const std::string& padre, const std::string& clave) {
        return padre.empty() ? clave : padre + "." + clave;
    }

    [[noreturn]] static void error(const std::string& r, const std::string& esperado) {
        throw std::invalid_argument(r + ": se esperaba " + esperado + ".");
    }

    static const json* valor(const json& o, const std::string& clave) {
        auto it = o.find(clave);
        return (it == o.end() || it->is_null()) ? nullptr : &*it;
    }

    static std::string texto(const json& o, const std::string& clave, const std::string& padre) {
        const json* v = valor(o, clave);
        if (!v) return "";
        if (v->is_string()) return v->get<std::string>();
        if (v->is_number_integer()) return std::to_string(v->get<int64_t>());
        error(ruta(padre, clave), "texto");
    }

    /// @brief Código de catálogo: texto ("01") o entero (1 -> "01").
    static std::string codigo(const json& o, const std::string& clave, const std::string& padre) {
        const json* v = valor(o, clave);
        if (!v) return "";
        if (v->is_string()) return v->get<std::string>();
        if (v->is_number_integer()) {
            char buf[24];
            std::snprintf(buf, sizeof(buf), "%02lld", static_cast<long long>(v->get<int64_t>()));
            return buf;
        }
        error(ruta(padre, clave), "un código de catálogo (texto o entero)");
    }

    /// @brief Monto o cantidad: preferiblemente texto ("1500.50"); se acepta número.
    static std::string decimal(const json& o, const std::string& clave, const std::string& padre) {
        const json* v = valor(o, clave);
        if (!v) return "";
        if (v->is_string()) return v->get<std::string>();
        if (v->is_number_integer()) return std::to_string(v->get<int64_t>());
        if (v->is_number_float()) {
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(5) << v->get<double>();
            std::string s = ss.str();
            s.erase(s.find_last_not_of('0') + 1);
            if (!s.empty() && s.back() == '.') s.pop_back();
            return s;
        }
        error(ruta(padre, clave), "un decimal (texto o número)");
    }

    static std::optional<int> entero(const json& o, const std::string& clave, const std::string& padre) {
        const json* v = valor(o, clave);
        if (!v) return std::nullopt;
        if (v->is_number_integer()) return v->get<int>();
        if (v->is_string()) {
            const std::string s = v->get<std::string>();
            if (!s.empty() && std::all_of(s.begin(), s.end(), ::isdigit) && s.size() <= 9) return std::stoi(s);
        }
        error(ruta(padre, clave), "un entero");
    }

    static const json* objeto(const json& o, const std::string& clave, const std::string& padre) {
        const json* v = valor(o, clave);
        if (v && !v->is_object()) error(ruta(padre, clave), "un objeto");
        return v;
    }

    template <typename F>
    static void recorrerLista(const json& o, const std::string& clave, const std::string& padre, F&& f) {
        const json* v = valor(o, clave);
        if (!v) return;
        const std::string r = ruta(padre, clave);
        if (!v->is_array()) error(r, "una lista");
        for (size_t i = 0; i < v->size(); ++i) {
            const std::string p = r + "[" + std::to_string(i) + "]";
            if (!(*v)[i].is_object()) error(p, "un objeto");
            f((*v)[i], p);
        }
    }

    static std::vector<std::string> textos(const json& o, const std::string& clave, const std::string& padre) {
        std::vector<std::string> out;
        const json* v = valor(o, clave);
        if (!v) return out;
        const std::string r = ruta(padre, clave);
        if (!v->is_array()) error(r, "una lista de textos");
        for (size_t i = 0; i < v->size(); ++i) {
            if (!(*v)[i].is_string()) error(r + "[" + std::to_string(i) + "]", "texto");
            out.push_back((*v)[i].get<std::string>());
        }
        return out;
    }

    static IdentificacionInput identificacion(const json& o, const std::string& p) {
        return { codigo(o, "tipo", p), texto(o, "numero", p) };
    }

    static UbicacionInput ubicacion(const json& o, const std::string& p) {
        return { texto(o, "provincia", p), codigo(o, "canton", p), codigo(o, "distrito", p),
                 texto(o, "barrio", p), texto(o, "otrasSenas", p) };
    }

    static TelefonoInput telefono(const json& o, const std::string& p) {
        return { texto(o, "codigoPais", p), texto(o, "numTelefono", p) };
    }

    static EmisorInput emisor(const json& o, const std::string& p) {
        EmisorInput e;
        e.nombre = texto(o, "nombre", p);
        if (const json* id = objeto(o, "identificacion", p)) e.identificacion = identificacion(*id, p + ".identificacion");
        e.registroFiscal8707 = texto(o, "registroFiscal8707", p);
        e.nombreComercial = texto(o, "nombreComercial", p);
        if (const json* u = objeto(o, "ubicacion", p)) e.ubicacion = ubicacion(*u, p + ".ubicacion");
        e.otrasSenasExtranjero = texto(o, "otrasSenasExtranjero", p);
        if (const json* t = objeto(o, "telefono", p)) e.telefono = telefono(*t, p + ".telefono");
        e.correoElectronico = textos(o, "correoElectronico", p);
        return e;
    }

    static ReceptorInput receptor(const json& o, const std::string& p) {
        ReceptorInput r;
        r.nombre = texto(o, "nombre", p);
        if (const json* id = objeto(o, "identificacion", p)) r.identificacion = identificacion(*id, p + ".identificacion");
        r.nombreComercial = texto(o, "nombreComercial", p);
        if (const json* u = objeto(o, "ubicacion", p)) r.ubicacion = ubicacion(*u, p + ".ubicacion");
        r.otrasSenasExtranjero = texto(o, "otrasSenasExtranjero", p);
        if (const json* t = objeto(o, "telefono", p)) r.telefono = telefono(*t, p + ".telefono");
        r.correoElectronico = texto(o, "correoElectronico", p);
        return r;
    }

    static std::vector<CodigoComercialInput> codigosComerciales(const json& o, const std::string& p) {
        std::vector<CodigoComercialInput> out;
        recorrerLista(o, "codigosComerciales", p, [&](const json& c, const std::string& cp) {
            out.push_back({ codigo(c, "tipo", cp), texto(c, "codigo", cp) });
        });
        return out;
    }

    static std::vector<DescuentoInput> descuentos(const json& o, const std::string& p) {
        std::vector<DescuentoInput> out;
        recorrerLista(o, "descuentos", p, [&](const json& d, const std::string& dp) {
            out.push_back({ decimal(d, "montoDescuento", dp), codigo(d, "codigoDescuento", dp),
                            texto(d, "codigoDescuentoOTRO", dp), texto(d, "naturalezaDescuento", dp) });
        });
        return out;
    }

    static std::vector<ImpuestoInput> impuestos(const json& o, const std::string& p) {
        std::vector<ImpuestoInput> out;
        recorrerLista(o, "impuestos", p, [&](const json& i, const std::string& ip) {
            ImpuestoInput imp;
            imp.codigo = codigo(i, "codigo", ip);
            imp.codigoImpuestoOTRO = texto(i, "codigoImpuestoOTRO", ip);
            imp.codigoTarifaIVA = codigo(i, "codigoTarifaIVA", ip);
            imp.tarifa = decimal(i, "tarifa", ip);
            imp.factorCalculoIVA = decimal(i, "factorCalculoIVA", ip);
            if (const json* d = objeto(i, "datosImpuestoEspecifico", ip)) {
                const std::string dp = ip + ".datosImpuestoEspecifico";
                imp.datosImpuestoEspecifico = DatosImpuestoEspecificoInput{
                    decimal(*d, "cantidadUnidadMedida", dp), decimal(*d, "porcentaje", dp), decimal(*d, "proporcion", dp),
                    decimal(*d, "volumenUnidadConsumo", dp), decimal(*d, "impuestoUnidad", dp) };
            }
            imp.montoExportacion = decimal(i, "montoExportacion", ip);
            out.push_back(imp);
        });
        return out;
    }

    static ExoneracionInput exoneracion(const json& o, const std::string& p) {
        ExoneracionInput e;
        e.tipoDocumentoEX1 = codigo(o, "tipoDocumentoEX1", p);
        e.tipoDocumentoOTRO = texto(o, "tipoDocumentoOTRO", p);
        e.numeroDocumento = texto(o, "numeroDocumento", p);
        e.articulo = entero(o, "articulo", p);
        e.inciso = entero(o, "inciso", p);
        e.nombreInstitucion = codigo(o, "nombreInstitucion", p);
        e.nombreInstitucionOtros = texto(o, "nombreInstitucionOtros", p);
        e.fechaEmisionEX = texto(o, "fechaEmisionEX", p);
        e.tarifaExonerada = decimal(o, "tarifaExonerada", p);
        return e;
    }

    static LineaDetalleInput lineaDetalle(const json& o, const std::string& p) {
        LineaDetalleInput l;
        l.numeroLinea = entero(o, "numeroLinea", p);
        l.partidaArancelaria = texto(o, "partidaArancelaria", p);
        l.codigoCABYS = texto(o, "codigoCABYS", p);
        l.codigosComerciales = codigosComerciales(o, p);
        l.cantidad = decimal(o, "cantidad", p);
        l.unidadMedida = texto(o, "unidadMedida", p);
        l.tipoTransaccion = codigo(o, "tipoTransaccion", p);
        l.unidadMedidaComercial = texto(o, "unidadMedidaComercial", p);
        l.detalle = texto(o, "detalle", p);
        l.numerosVINoSerie = textos(o, "numerosVINoSerie", p);
        l.registroMedicamento = texto(o, "registroMedicamento", p);
        l.formaFarmaceutica = texto(o, "formaFarmaceutica", p);
        recorrerLista(o, "detalleSurtido", p, [&](const json& s, const std::string& sp) {
            LineaDetalleSurtidoInput c;
            c.codigoCABYS = texto(s, "codigoCABYS", sp);
            c.codigosComerciales = codigosComerciales(s, sp);
            c.cantidad = decimal(s, "cantidad", sp);
            c.unidadMedida = texto(s, "unidadMedida", sp);
            c.unidadMedidaComercial = texto(s, "unidadMedidaComercial", sp);
            c.detalle = texto(s, "detalle", sp);
            c.precioUnitario = decimal(s, "precioUnitario", sp);
            c.descuentos = descuentos(s, sp);
            c.impuestos = impuestos(s, sp);
            l.detalleSurtido.push_back(c);
        });
        l.precioUnitario = decimal(o, "precioUnitario", p);
        l.montoTotal = decimal(o, "montoTotal", p);
        l.descuentos = descuentos(o, p);
        l.subTotal = decimal(o, "subTotal", p);
        l.ivaCobradoFabrica = codigo(o, "ivaCobradoFabrica", p);
        l.baseImponible = decimal(o, "baseImponible", p);
        l.impuestos = impuestos(o, p);
        l.impuestoAsumidoEmisorFabrica = decimal(o, "impuestoAsumidoEmisorFabrica", p);
        if (const json* e = objeto(o, "exoneracion", p)) l.exoneracion = exoneracion(*e, p + ".exoneracion");
        return l;
    }

    static OtrosCargosInput otroCargo(const json& o, const std::string& p) {
        OtrosCargosInput c;
        c.tipoDocumentoOC = codigo(o, "tipoDocumentoOC", p);
        c.tipoDocumentoOTROS = texto(o, "tipoDocumentoOTROS", p);
        if (const json* id = objeto(o, "identificacionTercero", p)) c.identificacionTercero = identificacion(*id, p + ".identificacionTercero");
        c.nombreTercero = texto(o, "nombreTercero", p);
        c.detalle = texto(o, "detalle", p);
        c.porcentajeOC = decimal(o, "porcentajeOC", p);
        c.montoCargo = decimal(o, "montoCargo", p);
        return c;
    }
};

} // namespace

parsing::ComprobanteRequestDTO JsonCodec::decodeRequest(std::string_view jsonStr) {
    json raiz;
    try {
        raiz = json::parse(jsonStr);
    } catch (const std::exception& e) {
        throw std::invalid_argument(std::string("JSON inválido: ") + e.what());
    }
    return LectorRequest::leer(raiz);
}

std::string JsonCodec::encodeResult(const service::ComprobanteResult& result) {
    json root;
    root["status"] = result.isSuccess ? "ok" : "invalid";

    // Metadata
    root["metadata"] = {
        {"coreVersion", result.metadata.coreVersion},
        {"standardVersion", result.metadata.standardVersion},
        {"schemaVersion", result.metadata.schemaVersion},
        {"rulesetVersion", result.metadata.rulesetVersion}
    };

    // Diagnostics
    json diags = json::array();
    for (const auto& d : result.diagnostics.items()) {
        diags.push_back({
            {"severity", d.severity == domain::DiagnosticSeverity::Error ? "error" : "warning"},
            {"code", d.code},
            {"path", d.path},
            {"message", d.message}
        });
    }
    root["diagnostics"] = diags;

    // XML
    if (!result.xml.empty()) {
        root["xml"] = result.xml;
    }

    if (result.comprobante.has_value()) {
        // Comprobante completo con la estructura del XML v4.4 (mismo recorrido que el XML).
        root["comprobante"] = FormatoJson::comoJson(*result.comprobante);
    }

    return root.dump();
}

std::string JsonCodec::encodeSignResult(const service::FirmaResult& result) {
    json root;
    root["status"] = result.isSuccess ? "ok" : "invalid";

    json diags = json::array();
    for (const auto& d : result.diagnostics.items()) {
        diags.push_back({
            {"severity", d.severity == domain::DiagnosticSeverity::Error ? "error" : "warning"},
            {"code", d.code},
            {"path", d.path},
            {"message", d.message}
        });
    }
    root["diagnostics"] = diags;

    if (!result.xml.empty()) {
        root["xml"] = result.xml;
    }
    if (!result.signedInfo.empty()) {
        root["signedInfo"] = result.signedInfo;
    }

    const auto& c = result.certInfo;
    if (!c.certificadoBase64.empty()) {
        root["certificado"] = {
            {"sujeto", c.sujeto},
            {"emisor", c.emisor},
            {"numeroSerie", c.serialNumber},
            {"validoDesde", c.validoDesde},
            {"validoHasta", c.validoHasta},
            {"sha256Hex", c.digestSha256Hex},
            {"sha256Base64", c.digestSha256Base64},
            {"certificadoBase64", c.certificadoBase64},
            {"alias", c.alias},
            {"vigente", c.esValido}
        };
    }

    return root.dump();
}

} // namespace cec::codec
