#include "cec/common/UnidadesMedida.h"
#include <sstream>
#include <algorithm>

namespace cec::common {

/**
 * Nota 15 
 */
static const std::vector<UnidadMedidaInfo> kUnidadesMedida = {
    // Principales servicios y comercio general
    {"Sp", "Sp", "Servicios Profesionales"},
    {"Spe", "Spe", "Servicios Personales"},
    {"St", "St", "Servicios Técnicos"},
    {"Os", "Os", "Otro tipo de servicios"},
    {"Unid", "Unid", "Unidad"},
    {"Acv", "Acv", "Activo Virtual"},
    {"Al", "Al", "Alquiler de uso habitacional"},
    {"Alc", "Alc", "Alquiler de uso comercial"},
    {"Cm", "Cm", "Comisiones"},
    {"Cc", "Cc", "Cajuela de café"},
    {"Cu", "Cu", "Cuartillos de café"},
    {"Fa", "Fa", "Fanega de café"},
    {"I", "I", "Intereses"},
    // Tiempo
    {"D", "D", "Día"},
    {"h", "h", "Hora"},
    {"Min", "Min", "Minuto"},
    {"s", "s", "Segundo"},
    // Longitud, Masa, Volumen y Área
    {"M", "M", "Metro"},
    {"cm", "cm", "Centímetro"},
    {"Mm", "Mm", "Milímetro"},
    {"Km", "Km", "Kilómetro"},
    {"Ln", "Ln", "Pulgada"},
    {"m²", "m²", "Metro cuadrado"},
    {"m³", "m³", "Metro cúbico"},
    {"Kg", "Kg", "Kilogramo"},
    {"G", "G", "Gramo"},
    {"t", "t", "Tonelada"},
    {"Qq", "Qq", "Quintal"},
    {"Oz", "Oz", "Onzas"},
    {"L", "L", "Litro"},
    {"mL", "mL", "Mililitro"},
    {"Gal", "Gal", "Galón"},
    // Sistema Internacional y Derivadas (RTC 443:2010)
    {"A", "A", "Ampere"},
    {"K", "K", "Kelvin"},
    {"Mol", "Mol", "Mol"},
    {"Cd", "Cd", "Candela"},
    {"m/s", "m/s", "Metro por segundo"},
    {"m/s²", "m/s²", "Metro por segundo cuadrado"},
    {"1/m", "1/m", "1 por metro"},
    {"kg/m³", "kg/m³", "Kilogramo por metro cúbico"},
    {"A/m²", "A/m²", "Ampere por metro cuadrado"},
    {"A/m", "A/m", "Ampere por metro"},
    {"mol/m³", "mol/m³", "Mol por metro cúbico"},
    {"cd/m²", "cd/m²", "Candela por metro cuadrado"},
    {"1", "1", "Uno (índice de refracción)"},
    {"Rad", "Rad", "Radián"},
    {"Sr", "Sr", "Estereorradián"},
    {"Hz", "Hz", "Hertz"},
    {"N", "N", "Newton"},
    {"Pa", "Pa", "Pascal"},
    {"J", "J", "Joule"},
    {"W", "W", "Watt"},
    {"Kw", "Kw", "Kilovatios"},
    {"kWh", "kWh", "Kilovatios por hora"},
    {"C", "C", "Coulomb"},
    {"V", "V", "Volt"},
    {"F", "F", "Farad"},
    {"Ω", "Ω", "Ohm"},
    {"S", "S", "Siemens"},
    {"Wb", "Wb", "Weber"},
    {"T", "T", "Tesla"},
    {"H", "H", "Henry"},
    {"°C", "°C", "Grado Celsius"},
    {"Lm", "Lm", "Lumen"},
    {"Lx", "Lx", "Lux"},
    {"Bq", "Bq", "Becquerel"},
    {"Gy", "Gy", "Gray"},
    {"Sv", "Sv", "Sievert"},
    {"Kat", "Kat", "Katal"},
    {"Pa·s", "Pa·s", "Pascal segundo"},
    {"N·m", "N·m", "Newton metro"},
    {"N/m", "N/m", "Newton por metro"},
    {"rad/s", "rad/s", "Radián por segundo"},
    {"rad/s²", "rad/s²", "Radián por segundo cuadrado"},
    {"W/m²", "W/m²", "Watt por metro cuadrado"},
    {"J/K", "J/K", "Joule por kelvin"},
    {"J/(kg·K)", "J/(kg·K)", "Joule por kilogramo kelvin"},
    {"J/kg", "J/kg", "Joule por kilogramo"},
    {"W/(m·K)", "W/(m·K)", "Watt por metro kelvin"},
    {"J/m³", "J/m³", "Joule por metro cúbico"},
    {"V/m", "V/m", "Volt por metro"},
    {"C/m³", "C/m³", "Coulomb por metro cúbico"},
    {"C/m²", "C/m²", "Coulomb por metro cuadrado"},
    {"F/m", "F/m", "Farad por metro"},
    {"H/m", "H/m", "Henry por metro"},
    {"J/mol", "J/mol", "Joule por mol"},
    {"J/(mol·K)", "J/(mol·K)", "Joule por mol kelvin"},
    {"C/kg", "C/kg", "Coulomb por kilogramo"},
    {"Gy/s", "Gy/s", "Gray por segundo"},
    {"W/sr", "W/sr", "Watt por estereorradián"},
    {"W/(m²·sr)", "W/(m²·sr)", "Watt por metro cuadrado estereorradián"},
    {"kat/m³", "kat/m³", "Katal por metro cúbico"},
    {"º", "º", "Grado sexagesimal"},
    {"´", "´", "Minuto sexagesimal"},
    {"´´", "´´", "Segundo sexagesimal"},
    {"Np", "Np", "Neper"},
    {"B", "B", "Bel"},
    {"eV", "eV", "Electronvolt"},
    {"U", "U", "Unidad de masa atómica unificada"},
    {"Ua", "Ua", "Unidad astronómica"},
    {"Otros", "Otros", "Otros"}
};

const std::vector<UnidadMedidaInfo>& UnidadesMedida::GetTodas() {
    return kUnidadesMedida;
}

const UnidadMedidaInfo* UnidadesMedida::GetPorCodigo(std::string_view codigo) {
    for (const auto& u : kUnidadesMedida) {
        if (u.codigo == codigo) {
            return &u;
        }
    }
    return nullptr;
}

bool UnidadesMedida::EsValida(std::string_view codigo) {
    return GetPorCodigo(codigo) != nullptr;
}

std::string UnidadesMedida::GetSimbolo(std::string_view codigo) {
    const auto* u = GetPorCodigo(codigo);
    return u ? u->simbolo : "";
}

std::string UnidadesMedida::GetDescripcion(std::string_view codigo) {
    const auto* u = GetPorCodigo(codigo);
    return u ? u->descripcion : "";
}

} // namespace cec::common
