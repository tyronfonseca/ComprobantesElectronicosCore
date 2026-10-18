#pragma once
#include <string>
#include <vector>
#include <optional>

namespace cec::common {

/**
 * Elemento genérico de un catálogo oficial de Hacienda v4.4
 */
struct ElementoCatalogo {
    std::string codigo;       // Código oficial del catálogo (ej. "01", "02", "99")
    std::string descripcion;  // Descripción oficial según notas técnicas y XSD
};

/**
 * Gestor unificado de catálogos oficiales de Facturación Electrónica Costa Rica (v4.4)
 */
class Catalogos {
public:
    // 1. Catálogos específicos
    static const std::vector<ElementoCatalogo>& GetCondicionesVenta();
    static const std::vector<ElementoCatalogo>& GetMediosPago();
    static const std::vector<ElementoCatalogo>& GetTiposIdentificacion();
    static const std::vector<ElementoCatalogo>& GetTarifasIVA();
    static const std::vector<ElementoCatalogo>& GetTiposImpuesto();
    static const std::vector<ElementoCatalogo>& GetTiposDescuento();
    static const std::vector<ElementoCatalogo>& GetTiposTransaccion();
    static const std::vector<ElementoCatalogo>& GetTiposExoneracion();
    static const std::vector<ElementoCatalogo>& GetInstitucionesExoneracion();
    static const std::vector<ElementoCatalogo>& GetTiposOtrosCargos();
    static const std::vector<ElementoCatalogo>& GetTiposDocumentoReferencia();
    static const std::vector<ElementoCatalogo>& GetCodigosReferencia();
    static const std::vector<ElementoCatalogo>& GetTiposCodigoComercial();
    static const std::vector<ElementoCatalogo>& GetIVACobradoFabrica();
    static const std::vector<ElementoCatalogo>& GetTiposDocumento();
    static const std::vector<ElementoCatalogo>& GetFormasFarmaceuticas();

    // 2. Consulta genérica por nombre de catálogo
    static std::vector<ElementoCatalogo> GetPorNombre(const std::string& nombreCatalogo);
    static std::vector<std::string> GetNombresCatalogos();
};

} // namespace cec::common
