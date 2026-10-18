#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <optional>

namespace cec::common {

/**
 * Información de una moneda oficial según ISO 4217 y Esquema Hacienda v4.4
 */
struct MonedaInfo {
    std::string codigo;       // Código ISO 4217 de 3 letras (ej. "CRC", "USD", "EUR")
    std::string simbolo;      // Símbolo monetario oficial (ej. "₡", "$", "€", "£")
    std::string descripcion;  // Nombre o descripción en español (ej. "Colón costarricense")
};

/**
 * Catálogo completo de Monedas Oficiales de Hacienda v4.4 (168 monedas ISO 4217; el XSD no admite XAG, XAU, XBA, XBB, XBC, XBD, XPD, XPT, XTS ni XXX)
 */
class Monedas {
public:
    static const std::vector<MonedaInfo>& GetTodas();
    static const MonedaInfo* GetPorCodigo(std::string_view codigo);
    static bool EsValida(std::string_view codigo);
    static std::string GetSimbolo(std::string_view codigo);
    static std::string GetDescripcion(std::string_view codigo);
};

} // namespace cec::common
