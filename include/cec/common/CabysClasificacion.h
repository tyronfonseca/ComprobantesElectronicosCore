#pragma once
#include <string_view>

namespace cec::common {

/**
 * @brief Clasificación de códigos CABYS usada por las reglas fiscales y el motor de cálculo.
 */
class CabysClasificacion {
public:
    /// @brief Servicio si el CABYS inicia con 5-9; bien si inicia con 0-4.
    static constexpr bool EsServicio(std::string_view cabys) noexcept {
        return !cabys.empty() && cabys[0] >= '5' && cabys[0] <= '9';
    }

    /**
     * @brief Bebidas alcohólicas (Ley 8707, impuesto 04):
     *   241  Alcohol etílico sin desnaturalizar, destilados y otras bebidas espirituosas (2411, 2413)
     *   242  Vinos: vino de uvas, espumoso, vermut, sidra, perada (2421, 2422, 2423)
     *   2431 Cerveza de malta
     * No incluye 2432 (malta, incluso tostada): es el grano, no una bebida.
     */
    static constexpr bool EsBebidaAlcoholica(std::string_view cabys) noexcept {
        return cabys.starts_with("241") || cabys.starts_with("242") || cabys.starts_with("2431");
    }

    /**
     * @brief Bebidas envasadas sin contenido alcohólico (impuesto 05): clase 244
     *   2441 Agua: mineral o gaseada (244100100), natural (2441002xx), n.c.p. (244109900)
     *   2449 Otras bebidas no alcohólicas: gaseosas (244900100), sin gasificar (244900200),
     *        a base de agua mineral (244900300), dietéticas (244900400), a base de leche (244900500),
     *        a base de semillas o frutos secos (244900600), calóricas n.c.p. (244909900)
     */
    static constexpr bool EsBebidaSinAlcohol(std::string_view cabys) noexcept {
        return cabys.starts_with("244");
    }

    /**
     * @brief Jabón de tocador (impuesto 05): subclase 3532101
     *   353210101 Jabón en barras, panes, trozos o piezas
     *   353210102 Jabón líquido
     *   353210199 Jabón de otras formas, n.c.p.
     * No incluye 3532102 (preparaciones tensoactivas) ni 3532103 (papel, guata o fieltro impregnados).
     */
    static constexpr bool EsJabonTocador(std::string_view cabys) noexcept {
        return cabys.starts_with("3532101");
    }

    /**
     * @brief Surtidos, combos o paquetes habilitados por Hacienda (CABYS al 2026-06-01, descripción
     * "según los lineamientos establecidos en anexos y estructuras de comprobantes electrónicos"):
     *   2399999002200 Surtido de productos alimenticios
     *   3532901000000 Surtido de productos de limpieza
     *   3532902000000 Surtido de productos de cuidado personal
     */
    static constexpr bool EsSurtidoHabilitado(std::string_view cabys) noexcept {
        return cabys == "2399999002200" || cabys == "3532901000000" || cabys == "3532902000000";
    }

    /**
     * @brief Servicios de salud humana (IVA devuelto, Anexo 1 pág. 55): clase 931
     *   9310 Consultas a médicos, intervenciones en la salud y servicios relacionados
     *   9319 Servicios de salud humana, n.c.p.
     */
    static constexpr bool EsServicioSalud(std::string_view cabys) noexcept {
        return cabys.starts_with("931");
    }

    /**
     * @brief Medicamentos que requieren registro sanitario y forma farmacéutica (Anexo 1 pág. 28-29):
     *   3562 Medicamentos para el aparato digestivo y metabolismo, sangre y órganos
     *   3563 Medicamentos para otros sistemas n.c.p. y medicamentos varios
     *   3565 Medicamentos naturales, incluidos homeopáticos
     * No incluye 3561 (fármacos puros a granel), 3564 (veterinarios) ni 3569 (artículos médicos).
     */
    static constexpr bool EsMedicamento(std::string_view cabys) noexcept {
        return cabys.starts_with("3562") || cabys.starts_with("3563") || cabys.starts_with("3565");
    }

    /**
     * @brief Medios de transporte con número de VIN, serie o consecutivo de fabricación
     * (NumeroVINoSerie, Anexo 1 pág. 28, nota al pie 6: vehículos, aeronaves o embarcaciones):
     *   4911 Vehículos automotores          4991 Motocicletas y sidecars
     *   493  Buques y estructuras flotantes 494  Embarcaciones para deportes y recreo
     *   4961-4963 Aeronaves y naves espaciales
     * No incluye partes y accesorios (4912, 4964, 4994) ni bicicletas (4992, 4993).
     */
    static constexpr bool EsMedioTransporte(std::string_view cabys) noexcept {
        return cabys.starts_with("4911") || cabys.starts_with("4991") ||
               cabys.starts_with("493") || cabys.starts_with("494") ||
               cabys.starts_with("4961") || cabys.starts_with("4962") || cabys.starts_with("4963");
    }
};

} // namespace cec::common
