#pragma once
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <string_view>
#include <chrono>
#include "cec/rules/IValidationRule.h"
#include "cec/rules/CatalogoFiscal.h"
#include "cec/domain/Diagnostics.h"
#include "cec/domain/Comprobante.h"
#include "cec/parsing/ComprobanteRequestDTO.h"

#include "cec/rules/RuleValidationHelpers.h"
#include "cec/rules/ClaveNumericaRule.h"
#include "cec/rules/FechaEmisionRule.h"
#include "cec/rules/NumeroConsecutivoRule.h"
#include "cec/rules/ActividadEconomicaRule.h"
#include "cec/rules/EmisorRule.h"
#include "cec/rules/ReceptorRule.h"
#include "cec/rules/CondicionVentaRule.h"
#include "cec/rules/MediosPagoRule.h"
#include "cec/rules/MonedaRule.h"
#include "cec/rules/LineasDetalleRule.h"
#include "cec/rules/InformacionReferenciaRule.h"
#include "cec/rules/OtrosCargosRule.h"

namespace cec::rules {

/**
 * @brief Motor de Reglas que orquesta la ejecución desacoplada de reglas IValidationRule.
 */
class RuleEngine {
private:
    void registerDefaultRules(FechaEmisionRule::Reloj reloj, std::chrono::seconds toleranciaFecha) {
        _rules.push_back(std::make_unique<ClaveNumericaRule>());
        _rules.push_back(std::make_unique<FechaEmisionRule>(std::move(reloj), toleranciaFecha));
        _rules.push_back(std::make_unique<NumeroConsecutivoRule>());
        _rules.push_back(std::make_unique<ActividadEconomicaRule>());
        _rules.push_back(std::make_unique<EmisorRule>());
        _rules.push_back(std::make_unique<ReceptorRule>());
        _rules.push_back(std::make_unique<CondicionVentaRule>());
        _rules.push_back(std::make_unique<MediosPagoRule>());
        _rules.push_back(std::make_unique<MonedaRule>());
        _rules.push_back(std::make_unique<LineasDetalleRule>());
        _rules.push_back(std::make_unique<InformacionReferenciaRule>());
        _rules.push_back(std::make_unique<OtrosCargosRule>());
    }

    std::vector<std::unique_ptr<IValidationRule>> _rules;

public:
    RuleEngine() {
        registerDefaultRules([] { return std::chrono::system_clock::now(); }, FechaEmisionRule::kToleranciaPorDefecto);
    }

    /**
     * @brief Motor con las reglas por defecto y un reloj/tolerancia propios para la fecha de emisión
     * (útil en pruebas o cuando la hora de referencia viene de otra fuente).
     */
    RuleEngine(FechaEmisionRule::Reloj reloj, std::chrono::seconds toleranciaFecha) {
        registerDefaultRules(std::move(reloj), toleranciaFecha);
    }

    explicit RuleEngine(std::vector<std::unique_ptr<IValidationRule>> rules)
        : _rules(std::move(rules)) {}

    void addRule(std::unique_ptr<IValidationRule> rule) {
        if (rule) {
            _rules.push_back(std::move(rule));
        }
    }

    template<typename TRule, typename... Args>
    void emplaceRule(Args&&... args) {
        _rules.push_back(std::make_unique<TRule>(std::forward<Args>(args)...));
    }

    void clearRules() {
        _rules.clear();
    }

    size_t ruleCount() const noexcept {
        return _rules.size();
    }

    const std::vector<std::unique_ptr<IValidationRule>>& getRules() const noexcept {
        return _rules;
    }

    /**
     * @brief Ejecuta todas las reglas registradas sobre el comprobante.
     */
    domain::DiagnosticList validate(const domain::ComprobanteCanonico& inv) const {
        domain::DiagnosticList diags;
        for (const auto& rule : _rules) {
            if (rule) {
                rule->validate(inv, diags);
            }
        }
        return diags;
    }

    /**
     * @brief Validador estático compatible con la invocación directa por defecto.
     */
    static domain::DiagnosticList validateRules(const domain::ComprobanteCanonico& inv) {
        static const RuleEngine defaultEngine;
        return defaultEngine.validate(inv);
    }
};

} // namespace cec::rules
