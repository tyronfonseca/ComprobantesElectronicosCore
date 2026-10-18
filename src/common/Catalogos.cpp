#include "cec/common/Catalogos.h"
#include "cec/common/Codigos.h"
#include "cec/common/FormasFarmaceuticas.h"
#include <sstream>
#include <algorithm>

namespace cec::common {

template <typename EnumType, typename CodeFunc, typename DescFunc>
static std::vector<ElementoCatalogo> BuildCatalogFromEnum(const std::vector<EnumType>& values, CodeFunc codeFn, DescFunc descFn) {
    std::vector<ElementoCatalogo> result;
    result.reserve(values.size());
    for (auto v : values) {
        result.push_back({ codeFn(v), descFn(v) });
    }
    return result;
}

const std::vector<ElementoCatalogo>& Catalogos::GetCondicionesVenta() {
    static const auto cat = BuildCatalogFromEnum(get_all_condiciones_venta(), get_condicion_venta_code, get_condicion_venta_description);
    return cat;
}

const std::vector<ElementoCatalogo>& Catalogos::GetMediosPago() {
    static const auto cat = BuildCatalogFromEnum(get_all_medios_pago(), get_medio_pago_code, get_medio_pago_description);
    return cat;
}

const std::vector<ElementoCatalogo>& Catalogos::GetTiposIdentificacion() {
    static const auto cat = BuildCatalogFromEnum(get_all_tipos_identificacion(), get_tipo_identificacion_code, get_tipo_identificacion_description);
    return cat;
}

const std::vector<ElementoCatalogo>& Catalogos::GetTarifasIVA() {
    static const auto cat = BuildCatalogFromEnum(get_all_tarifas_iva(), get_tarifa_iva_code, get_tarifa_iva_description);
    return cat;
}

const std::vector<ElementoCatalogo>& Catalogos::GetTiposImpuesto() {
    static const auto cat = BuildCatalogFromEnum(get_all_codigos_impuesto(), get_codigo_impuesto_code, get_codigo_impuesto_description);
    return cat;
}

const std::vector<ElementoCatalogo>& Catalogos::GetTiposDescuento() {
    static const auto cat = BuildCatalogFromEnum(get_all_codigos_descuento(), get_codigo_descuento_code, get_codigo_descuento_description);
    return cat;
}

const std::vector<ElementoCatalogo>& Catalogos::GetTiposTransaccion() {
    static const auto cat = BuildCatalogFromEnum(get_all_tipos_transaccion(), get_tipo_transaccion_code, get_tipo_transaccion_description);
    return cat;
}

const std::vector<ElementoCatalogo>& Catalogos::GetTiposExoneracion() {
    static const auto cat = BuildCatalogFromEnum(get_all_tipos_exoneracion(), get_tipo_exoneracion_code, get_tipo_exoneracion_description);
    return cat;
}

const std::vector<ElementoCatalogo>& Catalogos::GetInstitucionesExoneracion() {
    static const auto cat = BuildCatalogFromEnum(get_all_instituciones_exoneracion(), get_institucion_exoneracion_code, get_institucion_exoneracion_description);
    return cat;
}

const std::vector<ElementoCatalogo>& Catalogos::GetTiposOtrosCargos() {
    static const auto cat = BuildCatalogFromEnum(get_all_tipos_otros_cargos(), get_tipo_doc_otros_cargos_code, get_tipo_doc_otros_cargos_description);
    return cat;
}

const std::vector<ElementoCatalogo>& Catalogos::GetTiposDocumentoReferencia() {
    static const auto cat = BuildCatalogFromEnum(get_all_tipos_doc_referencia(), get_tipo_doc_referencia_code, get_tipo_doc_referencia_description);
    return cat;
}

const std::vector<ElementoCatalogo>& Catalogos::GetCodigosReferencia() {
    static const auto cat = BuildCatalogFromEnum(get_all_codigos_referencia(), get_codigo_referencia_code, get_codigo_referencia_description);
    return cat;
}

const std::vector<ElementoCatalogo>& Catalogos::GetTiposCodigoComercial() {
    static const auto cat = BuildCatalogFromEnum(get_all_tipos_codigo_comercial(), get_tipo_codigo_comercial_code, get_tipo_codigo_comercial_description);
    return cat;
}

const std::vector<ElementoCatalogo>& Catalogos::GetIVACobradoFabrica() {
    static const auto cat = BuildCatalogFromEnum(get_all_iva_cobrado_fabrica(), get_iva_cobrado_fabrica_code, get_iva_cobrado_fabrica_description);
    return cat;
}

const std::vector<ElementoCatalogo>& Catalogos::GetTiposDocumento() {
    static const auto cat = BuildCatalogFromEnum(get_all_tipos_documento(), get_tipo_documento_code, get_tipo_documento_description);
    return cat;
}

const std::vector<ElementoCatalogo>& Catalogos::GetFormasFarmaceuticas() {
    static const auto* formasPtr = []() {
        auto* v = new std::vector<ElementoCatalogo>();
        const auto& todas = FormasFarmaceuticas::GetTodas();
        v->reserve(todas.size());
        for (const auto& item : todas) {
            v->push_back({ item.codigo, item.descripcion });
        }
        return v;
    }();
    return *formasPtr;
}

std::vector<ElementoCatalogo> Catalogos::GetPorNombre(const std::string& nombreCatalogo) {
    if (nombreCatalogo == "CondicionesVenta" || nombreCatalogo == "condicionesventa") return GetCondicionesVenta();
    if (nombreCatalogo == "MediosPago" || nombreCatalogo == "mediospago") return GetMediosPago();
    if (nombreCatalogo == "TiposIdentificacion" || nombreCatalogo == "tiposidentificacion") return GetTiposIdentificacion();
    if (nombreCatalogo == "TarifasIVA" || nombreCatalogo == "tarifasiva") return GetTarifasIVA();
    if (nombreCatalogo == "TiposImpuesto" || nombreCatalogo == "tiposimpuesto") return GetTiposImpuesto();
    if (nombreCatalogo == "TiposDescuento" || nombreCatalogo == "tiposdescuento") return GetTiposDescuento();
    if (nombreCatalogo == "TiposTransaccion" || nombreCatalogo == "tipostransaccion") return GetTiposTransaccion();
    if (nombreCatalogo == "TiposExoneracion" || nombreCatalogo == "tiposexoneracion") return GetTiposExoneracion();
    if (nombreCatalogo == "InstitucionesExoneracion" || nombreCatalogo == "institucionesexoneracion") return GetInstitucionesExoneracion();
    if (nombreCatalogo == "TiposOtrosCargos" || nombreCatalogo == "tiposotroscargos") return GetTiposOtrosCargos();
    if (nombreCatalogo == "TiposDocumentoReferencia" || nombreCatalogo == "tiposdocumentoreferencia") return GetTiposDocumentoReferencia();
    if (nombreCatalogo == "CodigosReferencia" || nombreCatalogo == "codigosreferencia") return GetCodigosReferencia();
    if (nombreCatalogo == "TiposCodigoComercial" || nombreCatalogo == "tiposcodigocomercial") return GetTiposCodigoComercial();
    if (nombreCatalogo == "IVACobradoFabrica" || nombreCatalogo == "ivacobradofabrica") return GetIVACobradoFabrica();
    if (nombreCatalogo == "TiposDocumento" || nombreCatalogo == "tiposdocumento") return GetTiposDocumento();
    if (nombreCatalogo == "FormasFarmaceuticas" || nombreCatalogo == "formasfarmaceuticas") return GetFormasFarmaceuticas();
    return {};
}

std::vector<std::string> Catalogos::GetNombresCatalogos() {
    return {
        "CondicionesVenta",
        "MediosPago",
        "TiposIdentificacion",
        "TarifasIVA",
        "TiposImpuesto",
        "TiposDescuento",
        "TiposTransaccion",
        "TiposExoneracion",
        "InstitucionesExoneracion",
        "TiposOtrosCargos",
        "TiposDocumentoReferencia",
        "CodigosReferencia",
        "TiposCodigoComercial",
        "IVACobradoFabrica",
        "TiposDocumento",
        "FormasFarmaceuticas",
    };
}

} // namespace cec::common
