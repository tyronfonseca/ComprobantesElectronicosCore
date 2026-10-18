// ARCHIVO GENERADO por gen_territorios.py -- NO EDITAR A MANO.
#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace cec::common {


struct EntradaTerritorial {
    uint8_t          codigo;
    std::string_view nombre; 
};

class DivisionTerritorial {
public:
    // Codigo compuesto: Provincia(1) Canton(2) Distrito(2) Barrio(2) -> 7 digitos.
    // Ej.: (1,1,1,1) -> 1010101. No se almacena: se calcula.
    static constexpr uint32_t ComponerCodigoPostal(uint8_t p, uint8_t c, uint8_t d, uint8_t b) {
        return uint32_t(p) * 1000000u + uint32_t(c) * 10000u + uint32_t(d) * 100u + uint32_t(b);
    }
    // false si el valor no cabe en 7 digitos o la provincia es 0.
    static bool DescomponerCodigoPostal(uint32_t cod, uint8_t& p, uint8_t& c, uint8_t& d, uint8_t& b);

    // --- Validacion ---
    static bool EsProvinciaValida(uint8_t p);
    static bool EsCantonValido(uint8_t p, uint8_t c);
    static bool EsDistritoValido(uint8_t p, uint8_t c, uint8_t d);
    static bool EsBarrioValido(uint8_t p, uint8_t c, uint8_t d, uint8_t b);
    static bool EsBarrioValido(uint32_t codigoPostal);
    static bool EsUbicacionValida(uint8_t p, uint8_t c, uint8_t d);

    // --- Nombres (string_view vacio si el codigo no existe) ---
    static std::string_view GetNombreProvincia(uint8_t p);
    static std::string_view GetNombreCanton(uint8_t p, uint8_t c);
    static std::string_view GetNombreDistrito(uint8_t p, uint8_t c, uint8_t d);
    static std::string_view GetNombreBarrio(uint8_t p, uint8_t c, uint8_t d, uint8_t b);
    static std::string_view GetNombreBarrio(uint32_t codigoPostal);
    // "Provincia, Canton, Distrito" ("" si no existe). Aloca un std::string.
    static std::string GetNombreCompleto(uint8_t p, uint8_t c, uint8_t d);

    // --- Acceso por indice, sin heap (indice 0-based entre los elementos validos) ---
    // Util para capas C/FFI. "codigo" de la entrada es el codigo real (puede haber huecos).
    static uint32_t TotalProvincias();
    static uint32_t TotalCantones(uint8_t p);
    static uint32_t TotalDistritos(uint8_t p, uint8_t c);
    static uint32_t TotalBarrios(uint8_t p, uint8_t c, uint8_t d);
    static bool ProvinciaPorIndice(uint32_t i, EntradaTerritorial& out);
    static bool CantonPorIndice(uint8_t p, uint32_t i, EntradaTerritorial& out);
    static bool DistritoPorIndice(uint8_t p, uint8_t c, uint32_t i, EntradaTerritorial& out);
    static bool BarrioPorIndice(uint8_t p, uint8_t c, uint8_t d, uint32_t i, EntradaTerritorial& out);

    // --- Listados (alocan un vector solo cuando se llaman) ---
    static std::vector<EntradaTerritorial> ListarProvincias();
    static std::vector<EntradaTerritorial> ListarCantones(uint8_t p);
    static std::vector<EntradaTerritorial> ListarDistritos(uint8_t p, uint8_t c);
    static std::vector<EntradaTerritorial> ListarBarrios(uint8_t p, uint8_t c, uint8_t d);
};

}  // namespace cec::common
