#include <gtest/gtest.h>
#include "../support/Fixtures.h"
#include "cec/codec/EscritorJson.h"
#include "cec/visitor/EscritorXml.h"

using namespace cec;
using fixtures::contieneXml;
using fixtures::procesar;
using fixtures::request;
using fixtures::valorXml;

/*
 * RecorridoComprobante: presencia, orden y formato de los nodos según la "Condición del Campo" del Anexo 1.
 */

namespace {

std::string xmlDe(const std::string& caso) {
    const auto res = procesar(request(caso));
    EXPECT_TRUE(res.isSuccess) << fixtures::diagnosticos(res);
    return res.xml;
}

/// @brief Posición de la etiqueta de apertura (npos si no está).
size_t pos(const std::string& xml, std::string_view etiqueta) {
    return xml.find("<" + std::string(etiqueta) + ">");
}

/// @brief Escritor que registra los eventos para verificar el recorrido sin depender de un formato.
class EscritorRegistro final : public visitor::IEscritorArbol {
public:
    std::vector<std::string> eventos;
    void abrirDocumento(std::string_view n, std::string_view) override { eventos.push_back("doc:" + std::string(n)); }
    void cerrarDocumento() override { eventos.push_back("/doc"); }
    void abrir(std::string_view n) override { eventos.push_back("<" + std::string(n)); }
    void cerrar() override { eventos.push_back(">"); }
    void abrirLista(std::string_view n) override { eventos.push_back("[" + std::string(n)); }
    void cerrarLista() override { eventos.push_back("]"); }
    void campo(std::string_view n, std::string_view v, visitor::TipoValor) override {
        eventos.push_back(std::string(n) + "=" + std::string(v));
    }
};

} // namespace

TEST(Recorrido, EncabezadoEnElOrdenDelAnexo) {
    const auto xml = xmlDe("fe_servicios");
    const char* orden[] = {"Clave", "ProveedorSistemas", "CodigoActividadEmisor", "CodigoActividadReceptor",
                           "NumeroConsecutivo", "FechaEmision", "Emisor", "Receptor", "CondicionVenta",
                           "DetalleServicio", "ResumenFactura"};
    for (size_t i = 1; i < std::size(orden); ++i) {
        EXPECT_LT(pos(xml, orden[i - 1]), pos(xml, orden[i])) << orden[i - 1] << " antes de " << orden[i];
    }
}

TEST(Recorrido, RaizYNamespaceDelTipoDeComprobante) {
    EXPECT_NE(xmlDe("nc").find("<NotaCreditoElectronica xmlns=\"https://cdn.comprobanteselectronicos.go.cr/xml-schemas/v4.4/notaCreditoElectronica\""),
              std::string::npos);
    EXPECT_NE(xmlDe("rep").find("<ReciboElectronicoPago xmlns="), std::string::npos);
}

TEST(Recorrido, CondicionalEnCeroSeOmite) {
    const auto xml = xmlDe("fe_servicios");
    EXPECT_TRUE(contieneXml(xml, "TotalServGravados"));
    EXPECT_FALSE(contieneXml(xml, "TotalServExentos"));
    EXPECT_FALSE(contieneXml(xml, "TotalDescuentos"));
}

TEST(Recorrido, ObligatorioEnCeroSeEmite) {
    // ImpuestoAsumidoEmisorFabrica es obligatorio (1) en FE aunque sea cero.
    EXPECT_EQ(valorXml(xmlDe("fe_servicios"), "ImpuestoAsumidoEmisorFabrica"), "0.00000");
}

TEST(Recorrido, FacturaDeExportacionSinNodosInexistentes) {
    const auto xml = xmlDe("fee");
    EXPECT_FALSE(contieneXml(xml, "BaseImponible"));
    EXPECT_FALSE(contieneXml(xml, "ImpuestoNeto"));
    EXPECT_FALSE(contieneXml(xml, "ImpuestoAsumidoEmisorFabrica"));
    EXPECT_FALSE(contieneXml(xml, "TotalServExonerado"));
    EXPECT_FALSE(contieneXml(xml, "CodigoActividadReceptor"));
}

TEST(Recorrido, ReciboDePagoSoloConSusNodos) {
    const auto xml = xmlDe("rep");
    for (const char* inexistente : {"CodigoActividadEmisor", "Cantidad", "UnidadMedida", "PrecioUnitario", "CodigoCABYS",
                                    "BaseImponible", "ImpuestoAsumidoEmisorFabrica", "TotalGravado", "TotalDescuentos"}) {
        EXPECT_FALSE(contieneXml(xml, inexistente)) << inexistente;
    }
    EXPECT_TRUE(contieneXml(xml, "MontoTotal"));
    EXPECT_TRUE(contieneXml(xml, "CodigoTarifaIVA"));
}

TEST(Recorrido, ExoneracionDentroDelImpuestoDeIVA) {
    const auto xml = xmlDe("fe_completa");
    const auto impuesto = pos(xml, "Impuesto");
    const auto exoneracion = pos(xml, "Exoneracion");
    ASSERT_NE(exoneracion, std::string::npos);
    EXPECT_LT(impuesto, exoneracion);
    EXPECT_LT(exoneracion, xml.find("</Impuesto>"));
    EXPECT_LT(pos(xml, "Monto"), exoneracion);
}

TEST(Recorrido, SurtidoSinTarifaEnLaLineaYConTarifaEnLosComponentes) {
    const auto xml = xmlDe("fe_surtido");
    // El componente tiene un campo DetalleSurtido dentro del nodo DetalleSurtido (Anexo 1): el cierre del nodo es el último.
    const auto finSurtido = xml.rfind("</DetalleSurtido>");
    const std::string lineaSinSurtido = xml.substr(finSurtido, xml.find("</LineaDetalle>") - finSurtido);
    EXPECT_FALSE(contieneXml(lineaSinSurtido, "Tarifa"));
    EXPECT_FALSE(contieneXml(lineaSinSurtido, "CodigoTarifaIVA"));
    EXPECT_TRUE(contieneXml(xml.substr(0, finSurtido), "TarifaSurtido"));
}

TEST(Recorrido, CodigoTarifaIVASoloConIVA) {
    auto req = request("fe_servicios");
    req.detalleServicio[0].impuestos.push_back({"02", "", "", "10", "", std::nullopt, ""}); // Selectivo de consumo 10%
    const auto res = procesar(req);
    ASSERT_TRUE(res.isSuccess) << fixtures::diagnosticos(res);
    const auto segundo = res.xml.find("<Impuesto>", res.xml.find("</Impuesto>"));
    const std::string impuesto02 = res.xml.substr(segundo, res.xml.find("</Impuesto>", segundo) - segundo);
    EXPECT_EQ(valorXml(impuesto02, "Codigo"), "02");
    EXPECT_EQ(valorXml(impuesto02, "Tarifa"), "10.00");
    EXPECT_FALSE(contieneXml(impuesto02, "CodigoTarifaIVA"));
}

TEST(Recorrido, OtrosCargosConTerceroAntesDelResumen) {
    const auto xml = xmlDe("fe_completa");
    EXPECT_LT(pos(xml, "OtrosCargos"), pos(xml, "ResumenFactura"));
    EXPECT_EQ(valorXml(xml.substr(pos(xml, "IdentificacionTercero")), "Tipo"), "02");
}

TEST(Recorrido, DecimalesSegunElCampo) {
    const auto xml = xmlDe("fe_completa");
    EXPECT_EQ(valorXml(xml, "Cantidad"), "1.000");          // 16,3
    EXPECT_EQ(valorXml(xml, "Tarifa"), "13.00");            // 4,2
    EXPECT_EQ(valorXml(xml, "PrecioUnitario"), "89270.00000"); // 18,5
}

TEST(Recorrido, EventosConListasParaRepetibles) {
    const auto res = procesar(request("fe_servicios"));
    ASSERT_TRUE(res.isSuccess);
    EscritorRegistro w;
    visitor::RecorridoComprobante::recorrer(*res.comprobante, w);
    EXPECT_EQ(w.eventos.front(), "doc:FacturaElectronica");
    EXPECT_EQ(w.eventos.back(), "/doc");
    EXPECT_NE(std::find(w.eventos.begin(), w.eventos.end(), "[LineaDetalle"), w.eventos.end());
    EXPECT_NE(std::find(w.eventos.begin(), w.eventos.end(), "[CorreoElectronico"), w.eventos.end());
    // Cada abrir/abrirLista tiene su cierre.
    int nivel = 0;
    for (const auto& e : w.eventos) {
        if (e[0] == '<' || e[0] == '[') ++nivel;
        if (e == ">" || e == "]") --nivel;
        ASSERT_GE(nivel, 0);
    }
    EXPECT_EQ(nivel, 0);
}

TEST(Recorrido, XmlEscapaCaracteresEspeciales) {
    auto req = request("fe_servicios");
    req.detalleServicio[0].detalle = "Diseño & desarrollo <web>";
    const auto res = procesar(req);
    ASSERT_TRUE(res.isSuccess) << fixtures::diagnosticos(res);
    EXPECT_EQ(valorXml(res.xml, "Detalle"), "Diseño &amp; desarrollo &lt;web&gt;");
}

TEST(EscritorJson, MismaEstructuraQueElXml) {
    const auto res = procesar(request("fe_completa"));
    ASSERT_TRUE(res.isSuccess);
    const auto j = cec::codec::FormatoJson::comoJson(*res.comprobante);
    EXPECT_EQ(j.begin().key(), "Clave");
    EXPECT_TRUE(j["DetalleServicio"]["LineaDetalle"].is_array());
    EXPECT_EQ(j["DetalleServicio"]["LineaDetalle"].size(), 3u);
    EXPECT_EQ(j["DetalleServicio"]["LineaDetalle"][0]["NumeroLinea"], 1);                  // Entero como número
    EXPECT_EQ(j["ResumenFactura"]["TotalComprobante"], "1645798.69500");                   // Decimal como texto
    EXPECT_EQ(j["DetalleServicio"]["LineaDetalle"][0]["Impuesto"][0]["Exoneracion"]["TarifaExonerada"], "13.00");
    EXPECT_TRUE(j["Emisor"]["CorreoElectronico"].is_array());
}
