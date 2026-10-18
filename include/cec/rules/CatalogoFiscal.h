#pragma once
#include <string_view>
#include <array>
#include <algorithm>
#include <string>
#include <vector>
#include "cec/common/UnidadesMedida.h"
#include "cec/common/Monedas.h"
#include "cec/common/FormasFarmaceuticas.h"
#include "cec/common/Catalogos.h"

namespace cec::rules {

class CatalogoFiscal {
    /// @brief true si el valor numérico de un enum del dominio (leído del request) existe en el catálogo.
    static bool enCatalogo(const std::vector<common::ElementoCatalogo>& catalogo, int valor) {
        if (valor <= 0) return false;
        const std::string codigo = (valor < 10 ? "0" : "") + std::to_string(valor);
        return std::any_of(catalogo.begin(), catalogo.end(),
            [&codigo](const common::ElementoCatalogo& e) { return e.codigo == codigo; });
    }

public:
    // Códigos del request convertidos a los enums del dominio: un código fuera de catálogo queda con su valor
    // numérico (o 0 si no es numérico) y se rechaza aquí.
    template <typename E> static bool esCondicionVentaValida(E c) { return enCatalogo(common::Catalogos::GetCondicionesVenta(), static_cast<int>(c)); }
    template <typename E> static bool esMedioPagoValido(E m) { return enCatalogo(common::Catalogos::GetMediosPago(), static_cast<int>(m)); }
    template <typename E> static bool esTipoIdentificacionValido(E t) { return enCatalogo(common::Catalogos::GetTiposIdentificacion(), static_cast<int>(t)); }
    template <typename E> static bool esCodigoImpuestoValido(E c) { return enCatalogo(common::Catalogos::GetTiposImpuesto(), static_cast<int>(c)); }
    template <typename E> static bool esTarifaIVAValida(E t) { return enCatalogo(common::Catalogos::GetTarifasIVA(), static_cast<int>(t)); }

    static bool esUnidadMedidaValida(std::string_view u) noexcept {
        return common::UnidadesMedida::EsValida(u);
    }

    static bool esMonedaValida(std::string_view m) noexcept {
        return common::Monedas::EsValida(m);
    }

    static bool esTipoDocumentoExoneracionValido(std::string_view t) noexcept {
        constexpr std::array<std::string_view, 12> validas = {
            "01", "02", "03", "04", "05", "06", "07", "08", "09", "10", "11", "99"
        };
        return std::find(validas.begin(), validas.end(), t) != validas.end();
    }

    static bool esTipoDocumentoReferenciaValido(std::string_view t) noexcept {
        constexpr std::array<std::string_view, 21> validas = {
            "01", "02", "03", "04", "05", "06", "07", "08", "09", "10",
            "11", "12", "13", "14", "15", "16", "17", "18", "19", "20", "99"
        };
        return std::find(validas.begin(), validas.end(), t) != validas.end();
    }

    // En REP únicamente: 01, 04, 07, 08, 10 y 20.
    static bool esTipoDocumentoReferenciaRepValido(std::string_view t) noexcept {
        constexpr std::array<std::string_view, 6> validas = {
            "01", "04", "07", "08", "10", "20"
        };
        return std::find(validas.begin(), validas.end(), t) != validas.end();
    }

    // Nota 10: TipoDocIR cuyo Numero debe ser una clave numérica válida.
    static bool tipoDocReferenciaRequiereClave(std::string_view t) noexcept {
        constexpr std::array<std::string_view, 12> conClave = {
            "01", "02", "03", "04", "09", "11", "12", "15", "17", "18", "19", "20"
        };
        return std::find(conClave.begin(), conClave.end(), t) != conClave.end();
    }

    // Nota 9: incluye 13, 14, 15, 16 y 17.
    static bool esCodigoReferenciaValido(std::string_view c) noexcept {
        constexpr std::array<std::string_view, 17> validas = {
            "01", "02", "04", "05", "06", "07", "08", "09", "10",
            "11", "12", "13", "14", "15", "16", "17", "99"
        };
        return std::find(validas.begin(), validas.end(), c) != validas.end();
    }

    // Nota 12: tipo de código comercial.
    static bool esTipoCodigoComercialValido(std::string_view t) noexcept {
        return t == "01" || t == "02" || t == "03" || t == "04" || t == "99";
    }

    // TipoTransaccionType (Nota 22).
    static bool esTipoTransaccionValido(std::string_view t) noexcept {
        constexpr std::array<std::string_view, 13> validas = {
            "01", "02", "03", "04", "05", "06", "07", "08", "09", "10", "11", "12", "13"
        };
        return std::find(validas.begin(), validas.end(), t) != validas.end();
    }

    // CodigoDescuentoType (Nota 20).
    static bool esCodigoDescuentoValido(std::string_view c) noexcept {
        constexpr std::array<std::string_view, 10> validas = {
            "01", "02", "03", "04", "05", "06", "07", "08", "09", "99"
        };
        return std::find(validas.begin(), validas.end(), c) != validas.end();
    }

    // NombreInstitucion de la exoneración (Nota 23).
    static bool esNombreInstitucionValido(std::string_view c) noexcept {
        constexpr std::array<std::string_view, 13> validas = {
            "01", "02", "03", "04", "05", "06", "07", "08", "09", "10", "11", "12", "99"
        };
        return std::find(validas.begin(), validas.end(), c) != validas.end();
    }

    static bool esTipoOtrosCargosValido(std::string_view t) noexcept {
        constexpr std::array<std::string_view, 11> validas = {
            "01", "02", "03", "04", "05", "06", "07", "08", "09", "10", "99"
        };
        return std::find(validas.begin(), validas.end(), t) != validas.end();
    }

    // Nota 19: catálogo "Código de forma farmacéutica".
    static bool esFormaFarmaceuticaValida(std::string_view c) {
        return common::FormasFarmaceuticas::EsValida(std::string(c));
    }

    // VIN, número de serie o consecutivo de fabricación: máximo 17 caracteres alfanuméricos.
    static bool esVinValido(std::string_view vin) noexcept {
        if (vin.empty() || vin.length() > 17) return false;
        for (char ch : vin) {
            if (!::isalnum(static_cast<unsigned char>(ch))) return false;
        }
        return true;
    }
};

} // namespace cec::rules

