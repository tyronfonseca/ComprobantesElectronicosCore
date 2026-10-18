#include <gtest/gtest.h>
#include "../support/Fixtures.h"

using namespace cec;
using fixtures::procesar;
using fixtures::request;
using fixtures::valorXml;

/*
 * Cálculo (Anexo 1 v4.4): montos de línea, resumen y casos especiales verificados a mano.
 */

namespace {

std::string total(const service::ComprobanteResult& r, std::string_view etiqueta) {
    return valorXml(r.xml, etiqueta);
}

parsing::LineaDetalleInput lineaServicioSalud(const std::string& precio) {
    parsing::LineaDetalleInput l;
    l.codigoCABYS = "9310100000000"; // Servicios de salud humana (931)
    l.cantidad = "1";
    l.unidadMedida = "Sp";
    l.detalle = "Consulta medica general";
    l.precioUnitario = precio;
    l.impuestos = {{"01", "", "04", "", "", std::nullopt, ""}}; // IVA 4%
    return l;
}

} // namespace

TEST(Calculo, FacturaConDescuentosExoneracionYOtrosCargos) {
    const auto res = procesar(request("fe_completa"));
    ASSERT_TRUE(res.isSuccess) << fixtures::diagnosticos(res);
    const auto& inv = *res.comprobante;

    // Línea 1: 89270 - 1000 = 88270; IVA 13% = 11475.1, exonerado al 13% => impuesto neto 0.
    const auto& l1 = inv.lineas[0];
    EXPECT_EQ(l1.subTotal.toString(5), "88270.00000");
    EXPECT_EQ(l1.impuestos[0].monto.toString(5), "11475.10000");
    EXPECT_EQ(l1.exoneracion->montoExoneracion.toString(5), "11475.10000");
    EXPECT_EQ(l1.impuestoNeto.toString(5), "0.00000");

    // Línea 2: 3 x 450000.50 = 1350001.5 - 15000 = 1335001.5; IVA 13% = 173550.195.
    const auto& l2 = inv.lineas[1];
    EXPECT_EQ(l2.montoTotal.toString(5), "1350001.50000");
    EXPECT_EQ(l2.subTotal.toString(5), "1335001.50000");
    EXPECT_EQ(l2.impuestoNeto.toString(5), "173550.19500");

    // Línea 3: 12.5 x 1200 = 15000; IVA 1% = 150.
    EXPECT_EQ(inv.lineas[2].impuestoNeto.toString(5), "150.00000");

    EXPECT_EQ(total(res, "TotalDescuentos"), "16000.00000");
    EXPECT_EQ(total(res, "TotalVentaNeta"), "1438271.50000");
    EXPECT_EQ(total(res, "TotalImpuesto"), "173700.19500");
    EXPECT_EQ(total(res, "TotalOtrosCargos"), "33827.00000");
    EXPECT_EQ(total(res, "TotalComprobante"), "1645798.69500");
}

TEST(Calculo, SurtidoUnImpuestoPorCodigoYDesglosePorTarifa) {
    const auto res = procesar(request("fe_surtido"));
    ASSERT_TRUE(res.isSuccess) << fixtures::diagnosticos(res);
    const auto& l = res.comprobante->lineas[0];

    // Por unidad: 780 + 390 + 10 + 0 = 1180; por 2 surtidos = 2360, en un solo Impuesto 01.
    ASSERT_EQ(l.impuestos.size(), 1u);
    EXPECT_EQ(l.impuestos[0].monto.toString(5), "2360.00000");
    EXPECT_EQ(total(res, "TotalImpuesto"), "2360.00000");
    EXPECT_EQ(total(res, "TotalComprobante"), "28360.00000");

    // El desglose conserva la tarifa de cada componente: 13% (2340), 1% (20) y exento (0).
    ASSERT_EQ(res.comprobante->resumen.desgloseImpuestos.size(), 3u);
    EXPECT_EQ(res.comprobante->resumen.desgloseImpuestos[0].totalMonto.toString(5), "2340.00000");
}

TEST(Calculo, ReciboDePagoTotalIgualATotalVenta) {
    auto req = request("rep");
    req.detalleServicio[0].subTotal = "2000000"; // Pago parcial a registrar para el IVA
    const auto res = procesar(req);
    ASSERT_TRUE(res.isSuccess) << fixtures::diagnosticos(res);

    EXPECT_EQ(total(res, "MontoTotal"), "5000000.00000");
    EXPECT_EQ(total(res, "SubTotal"), "2000000.00000");
    EXPECT_EQ(total(res, "Monto"), "260000.00000");               // 13% del SubTotal
    EXPECT_EQ(total(res, "TotalImpuesto"), "260000.00000");
    EXPECT_EQ(total(res, "TotalComprobante"), total(res, "TotalVenta"));
}

TEST(Calculo, MonedaExtranjeraConservaLosMontosEnEsaMoneda) {
    const auto res = procesar(request("fe_usd"));
    ASSERT_TRUE(res.isSuccess) << fixtures::diagnosticos(res);
    EXPECT_EQ(total(res, "CodigoMoneda"), "USD");
    EXPECT_EQ(total(res, "TipoCambio"), "512.37000");
    EXPECT_EQ(total(res, "TotalComprobante"), "198.03250"); // 175.25 + 13%
}

TEST(Calculo, ColonesSinTipoDeCambioUsa1) {
    auto req = request("fe_servicios");
    req.tipoCambio.clear();
    const auto res = procesar(req);
    ASSERT_TRUE(res.isSuccess) << fixtures::diagnosticos(res);
    EXPECT_EQ(total(res, "TipoCambio"), "1.00000");
}

TEST(Calculo, UnSoloMedioDePagoRecibeElTotal) {
    const auto res = procesar(request("fe_servicios"));
    ASSERT_TRUE(res.isSuccess) << fixtures::diagnosticos(res);
    EXPECT_EQ(total(res, "TotalMedioPago"), total(res, "TotalComprobante"));
}

TEST(Calculo, IvaDevueltoServiciosDeSaludPagadosConTarjeta) {
    auto req = request("fe_servicios");
    req.detalleServicio = {lineaServicioSalud("20000")};
    req.mediosPago = {{"02", "", ""}};
    const auto res = procesar(req);
    ASSERT_TRUE(res.isSuccess) << fixtures::diagnosticos(res);
    // IVA 4% = 800, devuelto completo al pagarse todo con tarjeta.
    EXPECT_EQ(total(res, "TotalIVADevuelto"), "800.00000");
    EXPECT_EQ(total(res, "TotalComprobante"), "20000.00000");
}

TEST(Calculo, IvaDevueltoProporcionalEnVentaMixta) {
    auto req = request("fe_servicios");
    auto producto = req.detalleServicio[0];
    producto.precioUnitario = "10000";
    producto.numeroLinea = 2;
    req.detalleServicio = {lineaServicioSalud("20000"), producto};
    req.detalleServicio[0].numeroLinea = 1;
    // Venta neta 30000; con tarjeta 15000 => devuelto = 15000 / 30000 x 800 = 400.
    req.mediosPago = {{"02", "", "15000"}, {"01", "", "16700"}};
    const auto res = procesar(req);
    ASSERT_TRUE(res.isSuccess) << fixtures::diagnosticos(res);
    EXPECT_EQ(total(res, "TotalIVADevuelto"), "400.00000");
    EXPECT_EQ(total(res, "TotalComprobante"), "31700.00000"); // 30000 + 2100 - 400
}

TEST(Calculo, ServicioYMercanciaSeClasificanPorCabys) {
    const auto res = procesar(request("fe_completa"));
    ASSERT_TRUE(res.isSuccess) << fixtures::diagnosticos(res);
    // Los totales del resumen suman el MontoTotal de las líneas (Anexo 1, ResumenFactura):
    // servicio exonerado al 100% (línea 1) y mercancías gravadas (líneas 2 y 3).
    EXPECT_EQ(total(res, "TotalServExonerado"), "89270.00000");
    EXPECT_EQ(total(res, "TotalMercanciasGravadas"), "1365001.50000");
    EXPECT_EQ(total(res, "TotalVenta"), "1454271.50000");
}

TEST(Calculo, VistaPreviaConErroresDeReglas) {
    auto req = request("fe_servicios");
    req.emisor.nombre.clear();
    const auto res = service::ComprobanteService::calcular(req, fixtures::motor());
    EXPECT_FALSE(res.isSuccess);
    EXPECT_DIAG(res, "RULE_EMISOR_NOMBRE_LENGTH");
    ASSERT_TRUE(res.comprobante.has_value());
    EXPECT_EQ(res.comprobante->resumen.totalComprobante.toString(5), "100875.10000");
    EXPECT_TRUE(res.xml.empty());
}

TEST(Calculo, VistaPreviaSeDetieneConErroresDeFormato) {
    auto req = request("fe_servicios");
    req.detalleServicio[0].precioUnitario = "abc";
    const auto res = service::ComprobanteService::calcular(req, fixtures::motor());
    EXPECT_DIAG(res, "PARSE_DECIMAL_INVALID");
    EXPECT_FALSE(res.comprobante.has_value());
}
