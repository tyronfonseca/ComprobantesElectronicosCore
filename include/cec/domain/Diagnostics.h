#pragma once
#include <string>
#include <vector>
#include <string_view>

namespace cec::domain {

enum class DiagnosticSeverity {
    Warning,
    Error
};

enum class DiagnosticCategory {
    Structural,
    Schema,
    Catalog,
    BusinessRule,
    Calculation,
    PostCondition
};

/**
 * @brief Diagnóstico inmutable y tipado del resultado.
 * Los consumidores deciden por code y path, nunca comparando mensajes.
 */
struct Diagnostic {
    DiagnosticSeverity severity;
    DiagnosticCategory category;
    std::string code;      // Código estable (ej. "RULE_CABYS_INVALID", "MISSING_FIELD")
    std::string path;      // Object path (ej. "lineas[0].cabys", "receptor.identificacion")
    std::string message;   // Orientativo en español
};

class DiagnosticList {
public:
    void addError(std::string_view code, std::string_view path, std::string_view message, DiagnosticCategory cat = DiagnosticCategory::BusinessRule) {
        _items.push_back({ DiagnosticSeverity::Error, cat, std::string(code), std::string(path), std::string(message) });
        _hasErrors = true;
    }

    void addWarning(std::string_view code, std::string_view path, std::string_view message, DiagnosticCategory cat = DiagnosticCategory::BusinessRule) {
        _items.push_back({ DiagnosticSeverity::Warning, cat, std::string(code), std::string(path), std::string(message) });
    }

    void merge(const DiagnosticList& other) {
        for (const auto& item : other._items) {
            _items.push_back(item);
            if (item.severity == DiagnosticSeverity::Error) {
                _hasErrors = true;
            }
        }
    }

    bool hasErrors() const noexcept { return _hasErrors; }
    bool empty() const noexcept { return _items.empty(); }
    const std::vector<Diagnostic>& items() const noexcept { return _items; }
    size_t size() const noexcept { return _items.size(); }

private:
    std::vector<Diagnostic> _items;
    bool _hasErrors = false;
};

} // namespace cec::domain
