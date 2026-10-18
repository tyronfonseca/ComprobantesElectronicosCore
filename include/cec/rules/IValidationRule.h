#pragma once
#include "cec/domain/Diagnostics.h"
#include "cec/domain/Comprobante.h"

namespace cec::rules {

/**
 * @brief Interfaz base para todas las reglas de validación fiscal.
 */
class IValidationRule {
public:
    virtual ~IValidationRule() = default;

    virtual void validate(const domain::ComprobanteCanonico& inv, domain::DiagnosticList& diags) const = 0;
};

} // namespace cec::rules