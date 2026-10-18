#pragma once
#include <string_view>

namespace cec::visitor {

/**
 * @brief Tipo del valor de un campo hoja, para que cada formato lo represente como corresponda
 * (en XML todo es texto; en JSON o proto los enteros pueden ir como número).
 */
enum class TipoValor {
    Texto,
    Entero,
    Decimal  // Ya formateado con los decimales que exige el Anexo 1 (ej. "1500.00000")
};

/**
 * @brief Receptor de los eventos que emite RecorridoComprobante.
 *
 * El recorrido decide qué nodos existen, en qué orden y con qué formato (Anexo 1 v4.4);
 * cada implementación solo traduce esos eventos a su formato (XML, JSON, proto...).
 *
 * Los elementos repetibles se envuelven en abrirLista()/cerrarLista(): dentro de una lista,
 * cada elemento llega como abrir(nombre) o campo(nombre, ...) con el mismo nombre de la lista.
 */
class IEscritorArbol {
public:
    virtual ~IEscritorArbol() = default;

    /// @brief Inicia el documento. `nombre` y `espacioNombres` son la etiqueta raíz y el namespace del XML Schema (Nota 2).
    virtual void abrirDocumento(std::string_view nombre, std::string_view espacioNombres) = 0;
    virtual void cerrarDocumento() = 0;

    /// @brief Nodo compuesto (ComplexType).
    virtual void abrir(std::string_view nombre) = 0;
    virtual void cerrar() = 0;

    /// @brief Grupo de elementos repetibles con el mismo nombre (ej. LineaDetalle {1,1000}).
    virtual void abrirLista(std::string_view nombre) = 0;
    virtual void cerrarLista() = 0;

    /// @brief Campo hoja.
    virtual void campo(std::string_view nombre, std::string_view valor, TipoValor tipo) = 0;
};

} // namespace cec::visitor
