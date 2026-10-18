#pragma once
#include <string>
#include <vector>
#include <optional>

namespace cec::common {

/**
 * Información de una Forma Farmacéutica oficial según Nota Técnica 9 y Esquema Hacienda v4.4
 */
struct FormaFarmaceuticaInfo {
    std::string codigo;       // Código oficial de Hacienda (ej. "01", "04", "101", "229")
    std::string descripcion;  // Descripción de la forma farmacéutica (ej. "Cápsula", "Jarabe", "Tableta")
};

/**
 * Catálogo completo de Formas Farmacéuticas Oficiales de Hacienda v4.4 (229 registros según Nota Técnica 9)
 */
class FormasFarmaceuticas {
public:
    // Lista completa de las 229 formas farmacéuticas oficiales
    static const std::vector<FormaFarmaceuticaInfo>& GetTodas();

    // Búsqueda por código oficial (ej. "01", "10", "229")
    static const FormaFarmaceuticaInfo* GetPorCodigo(const std::string& codigo);

    // Validación de código según catálogo oficial
    static bool EsValida(const std::string& codigo);

    // Obtener descripción de la forma farmacéutica
    static std::string GetDescripcion(const std::string& codigo);
};

} // namespace cec::common

