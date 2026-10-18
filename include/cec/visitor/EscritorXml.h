#pragma once
#include <string>
#include <string_view>
#include <vector>
#include "cec/visitor/IEscritorArbol.h"
#include "cec/visitor/IFormatoSalida.h"
#include "cec/visitor/RecorridoComprobante.h"

namespace cec::visitor {

/**
 * @brief Escribe los eventos del recorrido como el XML del comprobante (sin firmar).
 * Las listas no generan etiqueta: cada elemento repetido lleva su propio nombre.
 */
class EscritorXml final : public IEscritorArbol {
public:
    void abrirDocumento(std::string_view nombre, std::string_view espacioNombres) override {
        out_ += "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
        out_ += '<';
        out_ += nombre;
        out_ += " xmlns=\"";
        out_ += espacioNombres;
        out_ += "\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\n";
        abiertos_.emplace_back(nombre);
    }

    void cerrarDocumento() override { cerrar(); }

    void abrir(std::string_view nombre) override {
        sangria();
        out_ += '<';
        out_ += nombre;
        out_ += ">\n";
        abiertos_.emplace_back(nombre);
    }

    void cerrar() override {
        const std::string nombre = std::move(abiertos_.back());
        abiertos_.pop_back();
        sangria();
        out_ += "</";
        out_ += nombre;
        out_ += ">\n";
    }

    void abrirLista(std::string_view) override {}
    void cerrarLista() override {}

    void campo(std::string_view nombre, std::string_view valor, TipoValor) override {
        sangria();
        out_ += '<';
        out_ += nombre;
        out_ += '>';
        escapar(valor);
        out_ += "</";
        out_ += nombre;
        out_ += ">\n";
    }

    std::string resultado() && { return std::move(out_); }

private:
    std::string out_;
    std::vector<std::string> abiertos_;

    void sangria() { out_.append(abiertos_.size() * 2, ' '); }

    void escapar(std::string_view v) {
        for (char c : v) {
            switch (c) {
                case '&':  out_ += "&amp;";  break;
                case '"':  out_ += "&quot;"; break;
                case '\'': out_ += "&apos;"; break;
                case '<':  out_ += "&lt;";   break;
                case '>':  out_ += "&gt;";   break;
                default:   out_ += c;        break;
            }
        }
    }
};

/**
 * @brief XML del comprobante según el XML Schema v4.4 de su tipo (Nota 2), listo para firmar.
 */
class FormatoXml final : public IFormatoSalida {
public:
    std::string escribir(const domain::ComprobanteCanonico& inv) const override {
        EscritorXml escritor;
        RecorridoComprobante::recorrer(inv, escritor);
        return std::move(escritor).resultado();
    }

    std::string_view tipoContenido() const noexcept override { return "application/xml"; }
};

} // namespace cec::visitor
