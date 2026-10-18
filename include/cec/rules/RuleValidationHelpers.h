#pragma once
#include <string>
#include <string_view>
#include <regex>
#include <algorithm>
#include <cstdint>
#include <cctype>
#include "cec/domain/Diagnostics.h"
#include "cec/domain/Comprobante.h"
#include "cec/common/DivisionTerritorial.h"
#include "cec/rules/CatalogoFiscal.h"

namespace cec::rules {

/**
 * @brief Funciones auxiliares reutilizables por las reglas de validación.
 */
class RuleValidationHelpers {
public:
    /**
     * @brief Cantidad de caracteres (code points) de un texto UTF-8.
     * Hacienda define los largos en caracteres; std::string::length() cuenta bytes
     * y sobrecuenta letras con tilde o ñ.
     */
    static std::size_t utf8Length(std::string_view s) noexcept {
        return static_cast<std::size_t>(std::count_if(s.begin(), s.end(),
            [](unsigned char ch) { return (ch & 0xC0) != 0x80; }));
    }

    /// @brief true si el texto tiene entre min y max caracteres UTF-8 (inclusive).
    static bool textoEnRango(std::string_view s, std::size_t min, std::size_t max) noexcept {
        const std::size_t n = utf8Length(s);
        return n >= min && n <= max;
    }

    /**
     * @brief Estructura de la clave numérica (notas 3 y 4.1): 50 posiciones,
     * "506" + fecha DDMMYY + identificación (12, alfanumérica para personas jurídicas)
     * + consecutivo (20 dígitos) + situación (1, 2 o 3) + código de seguridad (8 dígitos).
     */
    static bool esClaveEstructuraValida(std::string_view clave) noexcept {
        if (clave.length() != 50 || clave.substr(0, 3) != "506") return false;
        for (std::size_t i = 0; i < clave.length(); ++i) {
            const auto ch = static_cast<unsigned char>(clave[i]);
            const bool enIdentificacion = i >= 9 && i <= 20;
            if (enIdentificacion ? !std::isalnum(ch) : !std::isdigit(ch)) return false;
        }
        return clave[41] >= '1' && clave[41] <= '3';
    }

    /**
     * @brief Valida fecha-hora RFC3339 sección 5.6 ("date-time"): YYYY-MM-DDThh:mm:ss[.frac](Z|±hh:mm).
     */
    static bool esFechaRFC3339(const std::string& s) {
        static const std::regex re(R"(^(\d{4})-(\d{2})-(\d{2})T(\d{2}):(\d{2}):(\d{2})(\.\d+)?(Z|[+-](\d{2}):(\d{2}))$)");
        std::smatch m;

        if (!std::regex_match(s, m, re)) return false;

        const int anio = std::stoi(m[1]), mes = std::stoi(m[2]), dia = std::stoi(m[3]);
        const int hh = std::stoi(m[4]), mi = std::stoi(m[5]), ss = std::stoi(m[6]);

        if (mes < 1 || mes > 12 || hh > 23 || mi > 59 || ss > 60) return false;

        static constexpr int diasMes[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        const bool bisiesto = (anio % 4 == 0 && anio % 100 != 0) || anio % 400 == 0;
        const int maxDia = diasMes[mes - 1] + ((mes == 2 && bisiesto) ? 1 : 0);

        if (dia < 1 || dia > maxDia) return false;

        if (m[9].matched && (std::stoi(m[9]) > 23 || std::stoi(m[10]) > 59)) return false;

        return true;
    }

    static void validarUbicacion(const std::string& path, const domain::Ubicacion& u, domain::DiagnosticList& diags, size_t maxOtrasSenas = 250) {
        const bool provinciaOk = u.provincia.length() == 1 && u.provincia[0] >= '1' && u.provincia[0] <= '7';
        const bool cantonOk = u.canton.length() == 2 && std::all_of(u.canton.begin(), u.canton.end(), ::isdigit);
        const bool distritoOk = u.distrito.length() == 2 && std::all_of(u.distrito.begin(), u.distrito.end(), ::isdigit);

        if (!provinciaOk) {
            diags.addError("RULE_UBICACION_PROVINCIA_FORMAT", path + ".provincia", "La provincia debe ser un dígito entre 1 y 7.", domain::DiagnosticCategory::BusinessRule);
        }
        if (!cantonOk) {
            diags.addError("RULE_UBICACION_CANTON_FORMAT", path + ".canton", "El cantón debe tener exactamente 2 dígitos numéricos.", domain::DiagnosticCategory::BusinessRule);
        }
        if (!distritoOk) {
            diags.addError("RULE_UBICACION_DISTRITO_FORMAT", path + ".distrito", "El distrito debe tener exactamente 2 dígitos numéricos.", domain::DiagnosticCategory::BusinessRule);
        }

        // Nota 14: la combinación provincia/cantón/distrito debe existir en la división territorial.
        if (provinciaOk && cantonOk && distritoOk) {
            const auto p = static_cast<uint8_t>(u.provincia[0] - '0');
            const auto c = static_cast<uint8_t>(std::stoi(u.canton));
            const auto d = static_cast<uint8_t>(std::stoi(u.distrito));
            if (!common::DivisionTerritorial::EsCantonValido(p, c)) {
                diags.addError("RULE_UBICACION_CANTON_INVALID", path + ".canton", "El cantón no existe en la provincia indicada (Nota 14).", domain::DiagnosticCategory::Catalog);
            } else if (!common::DivisionTerritorial::EsDistritoValido(p, c, d)) {
                diags.addError("RULE_UBICACION_DISTRITO_INVALID", path + ".distrito", "El distrito no existe en el cantón indicado (Nota 14).", domain::DiagnosticCategory::Catalog);
            }
        }
        // Barrio es opcional
        if (u.barrio.has_value() && !u.barrio.value().empty() && !textoEnRango(*u.barrio, 5, 50)) {
            diags.addError("RULE_UBICACION_BARRIO_LENGTH", path + ".barrio", "El barrio debe tener entre 5 y 50 caracteres.", domain::DiagnosticCategory::BusinessRule);
        }
        if (!textoEnRango(u.otrasSenas, 5, maxOtrasSenas)) {
            diags.addError("RULE_UBICACION_OTRAS_SENAS_LENGTH", path + ".otrasSenas", "Las otras señas deben tener entre 5 y " + std::to_string(maxOtrasSenas) + " caracteres.", domain::DiagnosticCategory::BusinessRule);
        }
    }

    static void validarEmail(const std::string& path, const std::string& email, domain::DiagnosticList& diags) {
        if (email.empty()) {
            diags.addError("RULE_EMAIL_EMPTY", path,"El correo electrónico no puede estar vacío.");
        }

        if (utf8Length(email) > 160) {
            diags.addError("RULE_EMAIL_LENGTH", path,"El correo electrónico no debe superar los 160 caracteres.");
        }

        static const std::regex emailRegex(R"(^\s*(([^<>()\[\]\.,;:\s@\"]+(\.[^<>()\[\]\.,;:\s@\"]+)*)|(\".+\"))@(([^<>()\[\]\.,;:\s@\"]+\.)+[^<>()\[\]\.,;:\s@\"]{0,})\s*$)");
        if (!std::regex_match(email, emailRegex)) {
            diags.addError("RULE_EMAIL_FORMAT", path,"El formato del correo electrónico es inválido.");
        }
    }

    static void validarTelefono(const std::string& path, const domain::Telefono& t, domain::DiagnosticList& diags) {
        if (t.codigoPais.empty() || t.codigoPais.length() > 3 || !std::all_of(t.codigoPais.begin(), t.codigoPais.end(), ::isdigit)) {
            diags.addError("RULE_TELEFONO_CODIGO_PAIS_FORMAT", path + ".codigoPais", "El código de país debe tener entre 1 y 3 dígitos numéricos.", domain::DiagnosticCategory::BusinessRule);
        }
        if (t.numero.empty() || t.numero.length() > 20 || !std::all_of(t.numero.begin(), t.numero.end(), ::isdigit)) {
            diags.addError("RULE_TELEFONO_NUMERO_FORMAT", path + ".numTelefono", "El número de teléfono debe tener entre 8 y 20 dígitos numéricos.", domain::DiagnosticCategory::BusinessRule);
        } else if (t.numero.size() - std::min(t.numero.find_first_not_of('0'), t.numero.size()) < 3) {
            // El XSD define NumTelefono como entero con mínimo 100 (tres cifras significativas); cubre el 911.
            diags.addError("RULE_TELEFONO_NUMERO_FORMAT", path + ".numTelefono", "El número de teléfono debe ser un entero mayor o igual a 100.", domain::DiagnosticCategory::BusinessRule);
        } else if (t.numero.length() < 8) {
            // Se permiten números telefónicos especiales, por ejemplo el 911.
            diags.addWarning("RULE_TELEFONO_NUMERO_ESPECIAL", path + ".numTelefono", "El número de teléfono tiene menos de 8 dígitos; solo se permite si es un número especial (por ejemplo, 911).");
        }
    }

    static void validarIdentificacion(const std::string& path, const domain::Identificacion& id, domain::DiagnosticList& diags) {
        const std::string& num = id.numero;

        if (!CatalogoFiscal::esTipoIdentificacionValido(id.tipo)) {
            diags.addError("RULE_ID_TIPO_INVALID", path + ".tipo", "El tipo de identificación no existe en el catálogo de Hacienda (Nota 4).", domain::DiagnosticCategory::Catalog);
            return;
        }

        switch (id.tipo) {
            case domain::TipoIdentificacion::CedulaFisica:
                if (!std::all_of(num.begin(), num.end(), ::isdigit)) {
                    diags.addError("RULE_ID_NON_NUMERIC", path + ".numero", "El número de identificación debe ser estrictamente numérico.", domain::DiagnosticCategory::BusinessRule);
                    return;
                }
                if (num.length() != 9) {
                    diags.addError("RULE_CEDULA_FISICA_LENGTH", path + ".numero", "La cédula física debe tener exactamente 9 dígitos numéricos.", domain::DiagnosticCategory::BusinessRule);
                }
                if (num[0] == '0') {
                    diags.addError("RULE_CEDULA_FISICA_ZERO", path + ".numero", "La cédula física no puede empezar con cero.", domain::DiagnosticCategory::BusinessRule);
                }
                break;
            case domain::TipoIdentificacion::CedulaJuridica:
                // Admite caracteres alfanuméricos según el Registro Nacional
                if (!std::all_of(num.begin(), num.end(), ::isalnum)) {
                    diags.addError("RULE_ID_NON_ALPHANUMERIC", path + ".numero", "La cédula jurídica solo admite letras y números, sin guiones.", domain::DiagnosticCategory::BusinessRule);
                    return;
                }
                if (num.length() != 10) {
                    diags.addError("RULE_CEDULA_JURIDICA_LENGTH", path + ".numero", "La cédula jurídica debe tener exactamente 10 caracteres.", domain::DiagnosticCategory::BusinessRule);
                }
                break;
            case domain::TipoIdentificacion::DIMEX:
                if (!std::all_of(num.begin(), num.end(), ::isdigit)) {
                    diags.addError("RULE_ID_NON_NUMERIC", path + ".numero", "El número de identificación debe ser estrictamente numérico.", domain::DiagnosticCategory::BusinessRule);
                    return;
                }
                if (num.length() != 11 && num.length() != 12) {
                    diags.addError("RULE_DIMEX_LENGTH", path + ".numero", "El DIMEX debe tener 11 o 12 dígitos numéricos.", domain::DiagnosticCategory::BusinessRule);
                }
                if (num[0] == '0') {
                    diags.addError("RULE_DIMEX_ZERO", path + ".numero", "El DIMEX no puede empezar con cero.", domain::DiagnosticCategory::BusinessRule);
                }
                break;
            case domain::TipoIdentificacion::NITE:
                if (!std::all_of(num.begin(), num.end(), ::isdigit)) {
                    diags.addError("RULE_ID_NON_NUMERIC", path + ".numero", "El número de identificación debe ser estrictamente numérico.", domain::DiagnosticCategory::BusinessRule);
                    return;
                }
                if (num.length() != 10) {
                    diags.addError("RULE_NITE_LENGTH", path + ".numero", "El NITE debe tener exactamente 10 dígitos numéricos.", domain::DiagnosticCategory::BusinessRule);
                }
                break;
            case domain::TipoIdentificacion::ExtranjeroNoDomiciliado:
            case domain::TipoIdentificacion::ExtranjeroIdentificacion:
                if (num.empty() || num.length() > 20) {
                    diags.addError("RULE_ID_EXTRANJERO_LENGTH", path + ".numero", "La identificación de extranjero debe tener entre 1 y 20 caracteres.", domain::DiagnosticCategory::BusinessRule);
                } else if (!std::all_of(num.begin(), num.end(), ::isalnum)) {
                    diags.addError("RULE_ID_NON_ALPHANUMERIC", path + ".numero", "La identificación de Extranjero No Domiciliado o No Contribuyente solo admite letras y números.", domain::DiagnosticCategory::BusinessRule);
                }
                break;
        }
    }
};

} // namespace cec::rules

