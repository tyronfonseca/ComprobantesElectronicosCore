#pragma once
#include <string>
#include <string_view>
#include <vector>

namespace cec::common {

struct UnidadMedidaInfo {
    std::string codigo;
    std::string simbolo;
    std::string descripcion;
};

class UnidadesMedida {
public:
    // Lista completa de unidades de medida oficiales según estándar RTC 443:2010 y Hacienda v4.4
    static const std::vector<UnidadMedidaInfo>& GetTodas();

    // Consulta de unidad por código (case-sensitive según catálogo de Hacienda)
    static const UnidadMedidaInfo* GetPorCodigo(std::string_view codigo);

    // Validación
    static bool EsValida(std::string_view codigo);

    // Obtener símbolo y descripción
    static std::string GetSimbolo(std::string_view codigo);
    static std::string GetDescripcion(std::string_view codigo);
};

} // namespace cec::common
