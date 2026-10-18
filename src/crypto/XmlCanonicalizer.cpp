#include "cec/crypto/XmlCanonicalizer.h"
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>

namespace cec::crypto {

namespace {

static std::string escapeAttributeValue(const std::string& str) {
    std::string out;
    out.reserve(str.size() + 10);
    for (char c : str) {
        switch (c) {
            case '&': out.append("&amp;"); break;
            case '<': out.append("&lt;"); break;
            case '"': out.append("&quot;"); break;
            case '\t': out.append("&#x9;"); break;
            case '\n': out.append("&#xA;"); break;
            case '\r': out.append("&#xD;"); break;
            default: out.push_back(c); break;
        }
    }
    return out;
}

static std::string escapeText(const std::string& str) {
    std::string out;
    out.reserve(str.size() + 10);
    for (char c : str) {
        switch (c) {
            case '&': out.append("&amp;"); break;
            case '<': out.append("&lt;"); break;
            case '>': out.append("&gt;"); break;
            case '\r': out.append("&#xD;"); break;
            default: out.push_back(c); break;
        }
    }
    return out;
}

struct CanonicalAttribute {
    std::string prefix;
    std::string localName;
    std::string fullName;
    std::string value;
    bool isNamespace = false;
};

// Comparador para ordenar atributos y namespaces según C14N:
// 1. Declaración por defecto xmlns
// 2. Declaraciones xmlns:prefix por prefijo
// 3. Atributos sin prefijo por nombre
// 4. Atributos con prefijo por nombre
static bool compareCanonicalAttributes(const CanonicalAttribute& a, const CanonicalAttribute& b) {
    if (a.isNamespace != b.isNamespace) {
        return a.isNamespace > b.isNamespace; // Namespaces primero
    }
    if (a.isNamespace && b.isNamespace) {
        // xmlns va primero; comparar xmlns consigo mismo debe dar false (orden estricto).
        if (a.fullName == "xmlns") return b.fullName != "xmlns";
        if (b.fullName == "xmlns") return false;
        return a.fullName < b.fullName;
    }
    return a.fullName < b.fullName;
}

static bool isNamespaceAttribute(const std::string& attrName) {
    return attrName == "xmlns" || attrName.rfind("xmlns:", 0) == 0;
}

static std::string namespacePrefix(const std::string& attrName) {
    return (attrName == "xmlns") ? "" : attrName.substr(6);
}

// Namespaces en el ámbito de los ancestros del nodo (el más cercano gana).
static std::map<std::string, std::string> inheritedNamespaces(const pugi::xml_node& node) {
    std::map<std::string, std::string> inherited;
    for (pugi::xml_node p = node.parent(); p && p.type() == pugi::node_element; p = p.parent()) {
        for (const auto& attr : p.attributes()) {
            std::string attrName = attr.name();
            if (isNamespaceAttribute(attrName)) {
                inherited.emplace(namespacePrefix(attrName), attr.value());
            }
        }
    }
    return inherited;
}

static void canonicalizeElement(
    const pugi::xml_node& node,
    std::ostringstream& ss,
    std::map<std::string, std::string>& parentNamespaces,
    bool excludeEnvelopedSignature,
    const std::map<std::string, std::string>* inheritedForApex = nullptr
) {
    std::string nodeName = node.name();
    if (nodeName.empty()) return;

    if (excludeEnvelopedSignature) {
        if (nodeName == "ds:Signature" || nodeName == "Signature") {
            return;
        }
    }

    ss << "<" << nodeName;

    std::vector<CanonicalAttribute> attrs;
    std::map<std::string, std::string> currentNamespaces = parentNamespaces;

    // C14N inclusivo: el elemento raíz de un subárbol (apex) renderiza todos los namespaces
    // en ámbito, también los declarados en sus ancestros. Sin esto, el digest de SignedInfo,
    // SignedProperties o KeyInfo no coincide con el que calcula el verificador.
    if (inheritedForApex) {
        for (const auto& [prefix, uri] : *inheritedForApex) {
            if (prefix.empty() && uri.empty()) continue; // xmlns="" no se renderiza
            std::string declName = prefix.empty() ? "xmlns" : "xmlns:" + prefix;
            if (node.attribute(declName.c_str())) continue; // el propio nodo la redefine
            CanonicalAttribute ca;
            ca.fullName = declName;
            ca.value = uri;
            ca.isNamespace = true;
            currentNamespaces[prefix] = uri;
            attrs.push_back(ca);
        }
    }

    for (const auto& attr : node.attributes()) {
        std::string attrName = attr.name();
        std::string attrVal = attr.value();

        CanonicalAttribute ca;
        ca.fullName = attrName;
        ca.value = attrVal;

        if (attrName == "xmlns" || attrName.rfind("xmlns:", 0) == 0) {
            ca.isNamespace = true;
            std::string prefix = (attrName == "xmlns") ? "" : attrName.substr(6);
            auto it = parentNamespaces.find(prefix);
            if (it == parentNamespaces.end() || it->second != attrVal) {
                currentNamespaces[prefix] = attrVal;
                attrs.push_back(ca);
            }
        } else {
            ca.isNamespace = false;
            attrs.push_back(ca);
        }
    }

    std::sort(attrs.begin(), attrs.end(), compareCanonicalAttributes);

    for (const auto& a : attrs) {
        ss << " " << a.fullName << "=\"" << escapeAttributeValue(a.value) << "\"";
    }

    ss << ">";

    for (const auto& child : node.children()) {
        if (child.type() == pugi::node_element) {
            canonicalizeElement(child, ss, currentNamespaces, excludeEnvelopedSignature);
        } else if (child.type() == pugi::node_pcdata || child.type() == pugi::node_cdata) {
            ss << escapeText(child.value());
        }
    }

    ss << "</" << nodeName << ">";
}

} // namespace

std::pair<std::string, domain::DiagnosticList> XmlCanonicalizer::canonicalize(const pugi::xml_node& node, bool excludeEnvelopedSignature) {
    domain::DiagnosticList diags;
    std::ostringstream ss;
    std::map<std::string, std::string> rootNamespaces;

    if (node.type() == pugi::node_document) {
        for (const auto& child : node.children()) {
            if (child.type() == pugi::node_element) {
                canonicalizeElement(child, ss, rootNamespaces, excludeEnvelopedSignature);
            }
        }
    } else if (node.type() == pugi::node_element) {
        auto inherited = inheritedNamespaces(node);
        canonicalizeElement(node, ss, rootNamespaces, excludeEnvelopedSignature, &inherited);
    } else {
        diags.addError("C14N_NODE_INVALID", "xml", "El nodo proporcionado no es un documento o elemento XML válido.", domain::DiagnosticCategory::Structural);
        return {"", diags};
    }

    return {ss.str(), diags};
}

std::pair<std::string, domain::DiagnosticList> XmlCanonicalizer::canonicalizeXmlString(const std::string& xmlString, bool excludeEnvelopedSignature) {
    domain::DiagnosticList diags;
    pugi::xml_document doc;
    pugi::xml_parse_result res = doc.load_string(xmlString.c_str(), pugi::parse_default | pugi::parse_ws_pcdata);
    if (!res) {
        diags.addError("C14N_PARSE_ERROR", "xml", std::string("Error al parsear XML para canonicalización: ") + res.description(), domain::DiagnosticCategory::Structural);
        return {"", diags};
    }
    return canonicalize(doc, excludeEnvelopedSignature);
}

} // namespace cec::crypto

