#include <gtest/gtest.h>
#include <json.hpp>
#include "../support/Fixtures.h"

using cec::codec::JsonCodec;

/*
 * Lector del request JSON: tipos, conversión de códigos y errores con ruta.
 */

namespace {

std::string mensajeDeError(std::string_view json) {
    try {
        JsonCodec::decodeRequest(json);
    } catch (const std::invalid_argument& e) {
        return e.what();
    }
    return "";
}

} // namespace

TEST(LectorJson, LeeTodosLosCamposDelRequest) {
    const auto req = fixtures::request("fe_completa");
    EXPECT_EQ(req.tipoDocumento, "01");
    EXPECT_EQ(req.emisor.identificacion.tipo, "03");
    ASSERT_EQ(req.detalleServicio.size(), 3u);
    EXPECT_EQ(req.detalleServicio[0].descuentos[0].codigoDescuentoOTRO, "Promocion especial");
    EXPECT_EQ(req.detalleServicio[0].exoneracion->tarifaExonerada, "13");
    EXPECT_EQ(req.detalleServicio[1].codigosComerciales[0].tipo, "04");
    EXPECT_EQ(req.otrosCargos[1].identificacionTercero->numero, "3101000202");
    EXPECT_EQ(req.codigoMoneda, "CRC");
    EXPECT_EQ(req.mediosPago[0].tipoMedioPago, "04");
}

TEST(LectorJson, CodigosEnterosSeConviertenADosDigitos) {
    const auto req = JsonCodec::decodeRequest(R"({"tipoDocumento": 1, "condicionVenta": 2, "emisor": {"identificacion": {"tipo": 2, "numero": "3101123456"}}})");
    EXPECT_EQ(req.tipoDocumento, "01");
    EXPECT_EQ(req.condicionVenta, "02");
    EXPECT_EQ(req.emisor.identificacion.tipo, "02");
}

TEST(LectorJson, DecimalesNumericosSeConviertenATexto) {
    const auto req = JsonCodec::decodeRequest(R"({"detalleServicio": [{"cantidad": 2, "precioUnitario": 1500.25}]})");
    EXPECT_EQ(req.detalleServicio[0].cantidad, "2");
    EXPECT_EQ(req.detalleServicio[0].precioUnitario, "1500.25");
}

TEST(LectorJson, AusenteONullQuedaVacio) {
    const auto req = JsonCodec::decodeRequest(R"({"tipoDocumento": "01", "clave": null})");
    EXPECT_TRUE(req.clave.empty());
    EXPECT_FALSE(req.receptor.has_value());
    EXPECT_FALSE(req.plazoCredito.has_value());
}

TEST(LectorJson, TipoIncorrectoIndicaLaRuta) {
    EXPECT_EQ(mensajeDeError(R"({"detalleServicio": [{}, {"impuestos": [{"codigo": {"x": 1}}]}]})"),
              "detalleServicio[1].impuestos[0].codigo: se esperaba un código de catálogo (texto o entero).");
    EXPECT_EQ(mensajeDeError(R"({"emisor": []})"), "emisor: se esperaba un objeto.");
    EXPECT_EQ(mensajeDeError(R"({"detalleServicio": {}})"), "detalleServicio: se esperaba una lista.");
    EXPECT_EQ(mensajeDeError(R"({"plazoCredito": "treinta"})"), "plazoCredito: se esperaba un entero.");
    EXPECT_EQ(mensajeDeError(R"({"emisor": {"correoElectronico": [1]}})"), "emisor.correoElectronico[0]: se esperaba texto.");
}

TEST(LectorJson, JsonInvalido) {
    EXPECT_NE(mensajeDeError("{no es json"), "");
    EXPECT_EQ(mensajeDeError("[]"), "El request debe ser un objeto JSON.");
}

TEST(ResultadoJson, IncluyeElComprobanteConLaEstructuraDelXml) {
    const auto res = fixtures::procesar(fixtures::request("fe_servicios"));
    ASSERT_TRUE(res.isSuccess);
    const auto j = nlohmann::json::parse(JsonCodec::encodeResult(res));
    EXPECT_EQ(j["status"], "ok");
    EXPECT_EQ(j["comprobante"]["Clave"], res.comprobante->clave);
    EXPECT_EQ(j["comprobante"]["ResumenFactura"]["TotalComprobante"], "100875.10000");
}
