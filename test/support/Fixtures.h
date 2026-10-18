#pragma once
#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include "cec/codec/JsonCodec.h"
#include "cec/service/ComprobanteService.h"

/**
 * Datos de prueba compartidos. Los requests base son los casos golden (test/golden/<caso>/request.json):
 * cada uno es un comprobante válido de su tipo. Los tests de reglas parten de uno y cambian un solo dato.
 */
namespace fixtures {

using cec::parsing::ComprobanteRequestDTO;
using cec::service::ComprobanteResult;

inline std::filesystem::path dirTests() {
    return std::filesystem::path(__FILE__).parent_path().parent_path();
}

inline std::string leerArchivo(const std::filesystem::path& ruta) {
    std::ifstream f(ruta, std::ios::binary);
    if (!f) throw std::runtime_error("No se pudo abrir " + ruta.string());
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

inline std::filesystem::path dirGolden(std::string_view caso) {
    return dirTests() / "golden" / caso;
}

inline std::string requestJson(std::string_view caso) {
    return leerArchivo(dirGolden(caso) / "request.json");
}

/// @brief Request base válido: fe_servicios, fe_completa, fe_surtido, fe_usd, fe_vehiculo_vin, te, nc, nd, fec, fee, rep.
inline ComprobanteRequestDTO request(std::string_view caso) {
    return cec::codec::JsonCodec::decodeRequest(requestJson(caso));
}

/// @brief Motor de reglas con un reloj fijo (2026-10-15 12:00 -06:00) para que la fecha de emisión no dependa del día.
inline const cec::rules::RuleEngine& motor() {
    static const cec::rules::RuleEngine engine(
        [] { return std::chrono::sys_days{std::chrono::year{2026} / 10 / 15} + std::chrono::hours{18}; },
        cec::rules::FechaEmisionRule::kToleranciaPorDefecto);
    return engine;
}

inline ComprobanteResult procesar(const ComprobanteRequestDTO& req) {
    return cec::service::ComprobanteService::process(req, motor());
}

inline bool tiene(const ComprobanteResult& r, std::string_view codigo) {
    for (const auto& d : r.diagnostics.items()) {
        if (d.code == codigo) return true;
    }
    return false;
}

/// @brief Ruta del primer diagnóstico con ese código ("" si no está).
inline std::string ruta(const ComprobanteResult& r, std::string_view codigo) {
    for (const auto& d : r.diagnostics.items()) {
        if (d.code == codigo) return d.path;
    }
    return "";
}

/// @brief Todos los diagnósticos, para los mensajes de fallo.
inline std::string diagnosticos(const ComprobanteResult& r) {
    std::string out;
    for (const auto& d : r.diagnostics.items()) out += d.code + " @ " + d.path + "\n";
    return out.empty() ? "(sin diagnósticos)" : out;
}

/// @brief Valor del primer <etiqueta>valor</etiqueta> del XML ("" si no está).
inline std::string valorXml(const std::string& xml, std::string_view etiqueta) {
    const std::string abre = "<" + std::string(etiqueta) + ">";
    const auto i = xml.find(abre);
    if (i == std::string::npos) return "";
    const auto j = xml.find("</" + std::string(etiqueta) + ">", i);
    return xml.substr(i + abre.size(), j - i - abre.size());
}

inline bool contieneXml(const std::string& xml, std::string_view etiqueta) {
    return xml.find("<" + std::string(etiqueta) + ">") != std::string::npos;
}

} // namespace fixtures

/// @brief Verifica que el request produzca el diagnóstico indicado.
#define EXPECT_DIAG(resultado, codigo) \
    EXPECT_TRUE(fixtures::tiene((resultado), (codigo))) << "Se esperaba " << (codigo) << "; hubo:\n" << fixtures::diagnosticos(resultado)

/// @brief Verifica que el request sea válido.
#define EXPECT_VALIDO(resultado) \
    EXPECT_TRUE((resultado).isSuccess) << fixtures::diagnosticos(resultado)
