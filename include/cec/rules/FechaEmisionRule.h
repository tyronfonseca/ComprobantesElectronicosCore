#pragma once
#include <chrono>
#include <functional>
#include <optional>
#include <string>
#include "cec/rules/IValidationRule.h"
#include "cec/rules/RuleValidationHelpers.h"

namespace cec::rules {

/**
 * @brief Regla: Fecha de emisión del comprobante.
 *
 * - Formato RFC3339 sección 5.6 ("date-time").
 * - No puede ser posterior a la fecha de generación del comprobante. Se compara contra un reloj
 *   inyectable (por defecto el reloj del sistema) con una tolerancia para desfases de reloj.
 */
class FechaEmisionRule : public IValidationRule {
public:
    using Reloj = std::function<std::chrono::system_clock::time_point()>;

    static constexpr std::chrono::seconds kToleranciaPorDefecto{5 * 60};

    explicit FechaEmisionRule(Reloj reloj = [] { return std::chrono::system_clock::now(); },
                              std::chrono::seconds tolerancia = kToleranciaPorDefecto)
        : _reloj(std::move(reloj)), _tolerancia(tolerancia) {}

    void validate(const domain::ComprobanteCanonico& inv, domain::DiagnosticList& diags) const override {
        if (!RuleValidationHelpers::esFechaRFC3339(inv.fechaEmision)) {
            diags.addError("RULE_FECHA_EMISION_FORMAT", "fechaEmision",
                "La fecha de emisión debe tener formato RFC3339 (YYYY-MM-DDThh:mm:ss±hh:mm).",
                domain::DiagnosticCategory::BusinessRule);
            return;
        }

        const auto fecha = aInstante(inv.fechaEmision);
        if (fecha.has_value() && _reloj && *fecha > _reloj() + _tolerancia) {
            diags.addError("RULE_FECHA_EMISION_FUTURA", "fechaEmision",
                "La fecha de emisión no puede ser posterior a la fecha de generación del comprobante.",
                domain::DiagnosticCategory::BusinessRule);
        }
    }

    /**
     * @brief Convierte una fecha RFC3339 (ya validada) a un instante UTC.
     */
    static std::optional<std::chrono::system_clock::time_point> aInstante(const std::string& s) {
        using namespace std::chrono;
        if (!RuleValidationHelpers::esFechaRFC3339(s)) return std::nullopt;

        const int y = std::stoi(s.substr(0, 4));
        const unsigned mo = static_cast<unsigned>(std::stoi(s.substr(5, 2)));
        const unsigned d = static_cast<unsigned>(std::stoi(s.substr(8, 2)));
        const int hh = std::stoi(s.substr(11, 2));
        const int mi = std::stoi(s.substr(14, 2));
        const int ss = std::stoi(s.substr(17, 2));

        // Zona horaria: 'Z' o ±hh:mm al final (después de una posible fracción de segundo).
        int offsetMin = 0;
        const char ultimo = s.back();
        if (ultimo != 'Z') {
            const std::string zona = s.substr(s.size() - 6);
            const int signo = zona[0] == '-' ? -1 : 1;
            offsetMin = signo * (std::stoi(zona.substr(1, 2)) * 60 + std::stoi(zona.substr(4, 2)));
        }

        const sys_days dia{year{y} / month{mo} / day{d}};
        return time_point_cast<system_clock::duration>(
            dia + hours{hh} + minutes{mi} + seconds{ss} - minutes{offsetMin});
    }

private:
    Reloj _reloj;
    std::chrono::seconds _tolerancia;
};

} // namespace cec::rules
