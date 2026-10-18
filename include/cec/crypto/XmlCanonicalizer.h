#pragma once
#include <string>
#include "pugixml.hpp"
#include "cec/domain/Diagnostics.h"

namespace cec::crypto {

class XmlCanonicalizer {
public:
    /**
     * Canonicaliza un documento o nodo XML según la norma W3C Canonical XML 1.0 (C14N sin comentarios).
     * Si excludeEnvelopedSignature es true, excluye cualquier nodo <ds:Signature> o <Signature>.
     * Si el nodo es un elemento dentro de un documento, se canonicaliza en contexto: el elemento
     * renderiza también los namespaces declarados en sus ancestros (C14N inclusivo de un subárbol).
     */
    static std::pair<std::string, domain::DiagnosticList> canonicalize(const pugi::xml_node& node, bool excludeEnvelopedSignature = false);
    static std::pair<std::string, domain::DiagnosticList> canonicalizeXmlString(const std::string& xmlString, bool excludeEnvelopedSignature = false);
};

} // namespace cec::crypto
