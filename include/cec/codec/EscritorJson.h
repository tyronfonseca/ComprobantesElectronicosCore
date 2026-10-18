#pragma once
#include <string>
#include <string_view>
#include <vector>
#include <json.hpp>
#include "cec/visitor/IEscritorArbol.h"
#include "cec/visitor/IFormatoSalida.h"
#include "cec/visitor/RecorridoComprobante.h"

namespace cec::codec {

/**
 * @brief Escribe los eventos del recorrido como JSON con las mismas etiquetas y estructura del XML v4.4.
 *
 * Las claves conservan el orden del Anexo 1. Los nodos son objetos, las listas son arreglos y los enteros
 * van como número. Los decimales van como texto ("1500.00000") para no perder precisión ni los decimales
 * que exige el Anexo 1.
 */
class EscritorJson final : public visitor::IEscritorArbol {
public:
    void abrirDocumento(std::string_view, std::string_view) override {
        raiz_ = nlohmann::ordered_json::object();
        pila_ = {&raiz_};
    }

    void cerrarDocumento() override { pila_.clear(); }

    void abrir(std::string_view nombre) override {
        pila_.push_back(&agregar(nombre, nlohmann::ordered_json::object()));
    }

    void cerrar() override { pila_.pop_back(); }

    void abrirLista(std::string_view nombre) override {
        pila_.push_back(&agregar(nombre, nlohmann::ordered_json::array()));
    }

    void cerrarLista() override { pila_.pop_back(); }

    void campo(std::string_view nombre, std::string_view valor, visitor::TipoValor tipo) override {
        if (tipo == visitor::TipoValor::Entero) {
            agregar(nombre, std::stoll(std::string(valor)));
        } else {
            agregar(nombre, std::string(valor));
        }
    }

    nlohmann::ordered_json resultado() && { return std::move(raiz_); }

private:
    nlohmann::ordered_json raiz_;
    std::vector<nlohmann::ordered_json*> pila_;

    /// @brief Dentro de una lista agrega un elemento; dentro de un objeto, una propiedad.
    nlohmann::ordered_json& agregar(std::string_view nombre, nlohmann::ordered_json valor) {
        nlohmann::ordered_json& padre = *pila_.back();
        if (padre.is_array()) {
            padre.push_back(std::move(valor));
            return padre.back();
        }
        auto& destino = padre[std::string(nombre)];
        destino = std::move(valor);
        return destino;
    }
};

/**
 * @brief Comprobante como JSON con la estructura del XML v4.4.
 */
class FormatoJson final : public visitor::IFormatoSalida {
public:
    static nlohmann::ordered_json comoJson(const domain::ComprobanteCanonico& inv) {
        EscritorJson escritor;
        visitor::RecorridoComprobante::recorrer(inv, escritor);
        return std::move(escritor).resultado();
    }

    std::string escribir(const domain::ComprobanteCanonico& inv) const override {
        return comoJson(inv).dump();
    }

    std::string_view tipoContenido() const noexcept override { return "application/json"; }
};

} // namespace cec::codec
