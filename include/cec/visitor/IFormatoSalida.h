#pragma once
#include <string>
#include <string_view>
#include "cec/domain/Comprobante.h"

namespace cec::visitor {

/**
 * @brief Formato de salida de un comprobante ya validado y calculado.
 *
 * Los formatos en árbol (XML, JSON, proto) se implementan con RecorridoComprobante + un IEscritorArbol;
 * los de presentación (PDF) implementan esta interfaz directamente leyendo el ComprobanteCanonico.
 */
class IFormatoSalida {
public:
    virtual ~IFormatoSalida() = default;

    /// @brief Serializa el comprobante. El resultado puede ser texto o binario según el formato.
    virtual std::string escribir(const domain::ComprobanteCanonico& inv) const = 0;

    /// @brief Media type del resultado (ej. "application/xml").
    virtual std::string_view tipoContenido() const noexcept = 0;
};

} // namespace cec::visitor
