#include <gtest/gtest.h>
#include <string>
#include "cec/c_api/cec_c_api.h"

/*
 * ABI de C v3: entrada por structs y por JSON, lectura de totales y diagnósticos sin JSON.
 */

namespace {

// Factura Electrónica de servicios con datos ficticios: 2 x 50000 + IVA 13% = 113000.
const char* kClave = "50601102600310112345600100001010000000001112345678";
const char* kConsecutivo = "00100001010000000001";
const char* kFecha = "2026-10-01T10:00:00-06:00";

struct RequestFE {
    cec_ubicacion_c ubicacion{"1", "01", "01", nullptr, "Avenida 2, calle 5"};
    cec_telefono_c telefono{"506", "22223333"};
    const char* correos[1] = {"facturacion@ejemplo.cr"};
    cec_identificacion_c idReceptor{"02", "3101987654"};
    cec_receptor_c receptor{"CLIENTE EJEMPLO S.A.", &idReceptor, nullptr, nullptr, nullptr, nullptr, "pagos@cliente.cr"};
    cec_impuesto_c iva{"01", nullptr, "08", nullptr, nullptr, nullptr, nullptr};
    cec_linea_c linea{};
    cec_medio_pago_c medio{"04", nullptr, nullptr};
    cec_request_c req{};

    RequestFE() {
        linea.numeroLinea = CEC_SIN_VALOR;
        linea.codigoCABYS = "8314100000200";
        linea.cantidad = "2";
        linea.unidadMedida = "Sp";
        linea.detalle = "Desarrollo de software a la medida";
        linea.precioUnitario = "50000";
        linea.impuestos = &iva;
        linea.numImpuestos = 1;

        req.tipoDocumento = "01";
        req.clave = kClave;
        req.proveedorSistemas = "3101123456";
        req.codigoActividadEmisor = "6201.0";
        req.numeroConsecutivo = kConsecutivo;
        req.fechaEmision = kFecha;
        req.emisor.nombre = "SOFTWARE EJEMPLO S.A.";
        req.emisor.identificacion = {"02", "3101123456"};
        req.emisor.ubicacion = &ubicacion;
        req.emisor.telefono = &telefono;
        req.emisor.correoElectronico = correos;
        req.emisor.numCorreoElectronico = 1;
        req.receptor = &receptor;
        req.condicionVenta = "01";
        req.plazoCredito = CEC_SIN_VALOR;
        req.detalleServicio = &linea;
        req.numDetalleServicio = 1;
        req.codigoMoneda = "CRC";
        req.mediosPago = &medio;
        req.numMediosPago = 1;
    }
};

const char* kRequestJson = R"({
  "tipoDocumento": "01", "clave": "50601102600310112345600100001010000000001112345678",
  "proveedorSistemas": "3101123456", "codigoActividadEmisor": "6201.0",
  "numeroConsecutivo": "00100001010000000001", "fechaEmision": "2026-10-01T10:00:00-06:00",
  "emisor": {"nombre": "SOFTWARE EJEMPLO S.A.", "identificacion": {"tipo": "02", "numero": "3101123456"},
             "ubicacion": {"provincia": "1", "canton": "01", "distrito": "01", "otrasSenas": "Avenida 2, calle 5"},
             "telefono": {"codigoPais": "506", "numTelefono": "22223333"}, "correoElectronico": ["facturacion@ejemplo.cr"]},
  "receptor": {"nombre": "CLIENTE EJEMPLO S.A.", "identificacion": {"tipo": "02", "numero": "3101987654"},
               "correoElectronico": "pagos@cliente.cr"},
  "condicionVenta": "01",
  "detalleServicio": [{"codigoCABYS": "8314100000200", "cantidad": "2", "unidadMedida": "Sp",
                       "detalle": "Desarrollo de software a la medida", "precioUnitario": "50000",
                       "impuestos": [{"codigo": "01", "codigoTarifaIVA": "08"}]}],
  "resumenFactura": {"codigoTipoMoneda": {"codigoMoneda": "CRC"}, "mediosPago": [{"tipoMedioPago": "04"}]}
})";

std::string xml(const cec_result* res) {
    const uint8_t* data = nullptr;
    size_t len = 0;
    EXPECT_EQ(cec_result_xml(res, &data, &len), CEC_STATUS_OK);
    return data ? std::string(reinterpret_cast<const char*>(data), len) : std::string();
}

bool exito(const cec_result* res) {
    int32_t ok = 0;
    EXPECT_EQ(cec_result_is_success(res, &ok), CEC_STATUS_OK);
    return ok == 1;
}

std::string diagnosticos(const cec_result* res) {
    size_t total = 0;
    cec_result_total_diagnosticos(res, &total);
    std::string out;
    for (size_t i = 0; i < total; ++i) {
        const char* codigo = nullptr;
        const char* ruta = nullptr;
        cec_result_diagnostico(res, i, nullptr, &codigo, &ruta, nullptr);
        out += std::string(codigo) + " @ " + ruta + "\n";
    }
    return out;
}

} // namespace

TEST(CApi, VersionDelAbi) {
    EXPECT_EQ(cec_abi_version(), 3u);
}

TEST(CApi, BuildConStructs) {
    RequestFE f;
    cec_result* res = nullptr;
    ASSERT_EQ(cec_build(&f.req, &res), CEC_STATUS_OK);
    ASSERT_NE(res, nullptr);
    EXPECT_TRUE(exito(res)) << diagnosticos(res);

    cec_resumen_totales_c t{};
    ASSERT_EQ(cec_result_totales(res, &t), CEC_STATUS_OK);
    EXPECT_EQ(t.totalServGravados, 10000000000LL);  // 100000.00000
    EXPECT_EQ(t.totalImpuesto, 1300000000LL);       //  13000.00000
    EXPECT_EQ(t.totalComprobante, 11300000000LL);   // 113000.00000

    const std::string x = xml(res);
    EXPECT_NE(x.find("<NumeroLinea>1</NumeroLinea>"), std::string::npos); // CEC_SIN_VALOR: según la posición
    EXPECT_NE(x.find("<TotalComprobante>113000.00000</TotalComprobante>"), std::string::npos);
    cec_result_free(res);
}

TEST(CApi, StructsYJsonProducenElMismoXml) {
    RequestFE f;
    cec_result* deStructs = nullptr;
    cec_result* deJson = nullptr;
    ASSERT_EQ(cec_build(&f.req, &deStructs), CEC_STATUS_OK);
    const std::string json = kRequestJson;
    ASSERT_EQ(cec_build_json(reinterpret_cast<const uint8_t*>(json.data()), json.size(), &deJson), CEC_STATUS_OK);
    EXPECT_TRUE(exito(deJson)) << diagnosticos(deJson);
    EXPECT_EQ(xml(deStructs), xml(deJson));
    cec_result_free(deStructs);
    cec_result_free(deJson);
}

TEST(CApi, DiagnosticosSinJson) {
    RequestFE f;
    f.linea.codigoCABYS = "123"; // CABYS de 13 dígitos
    cec_result* res = nullptr;
    ASSERT_EQ(cec_build(&f.req, &res), CEC_STATUS_OK);
    EXPECT_FALSE(exito(res));

    size_t total = 0;
    ASSERT_EQ(cec_result_total_diagnosticos(res, &total), CEC_STATUS_OK);
    bool encontrado = false;
    for (size_t i = 0; i < total; ++i) {
        cec_severidad sev{};
        const char* codigo = nullptr;
        const char* ruta = nullptr;
        const char* mensaje = nullptr;
        ASSERT_EQ(cec_result_diagnostico(res, i, &sev, &codigo, &ruta, &mensaje), CEC_STATUS_OK);
        if (std::string(codigo) == "RULE_CABYS_FORMAT") {
            encontrado = true;
            EXPECT_EQ(sev, CEC_SEVERIDAD_ERROR);
            EXPECT_STREQ(ruta, "detalleServicio[0].codigoCABYS");
            EXPECT_GT(std::string(mensaje).size(), 0u);
        }
    }
    EXPECT_TRUE(encontrado) << diagnosticos(res);
    EXPECT_EQ(cec_result_diagnostico(res, total, nullptr, nullptr, nullptr, nullptr), CEC_STATUS_ERR_OUT_OF_RANGE);
    EXPECT_TRUE(xml(res).empty());
    cec_result_free(res);
}

TEST(CApi, JsonConTipoIncorrectoDevuelveDiagnostico) {
    const std::string json = R"({"tipoDocumento": "01", "detalleServicio": [{"impuestos": "01"}]})";
    cec_result* res = nullptr;
    ASSERT_EQ(cec_build_json(reinterpret_cast<const uint8_t*>(json.data()), json.size(), &res), CEC_STATUS_OK);
    EXPECT_FALSE(exito(res));
    const char* codigo = nullptr;
    const char* mensaje = nullptr;
    ASSERT_EQ(cec_result_diagnostico(res, 0, nullptr, &codigo, nullptr, &mensaje), CEC_STATUS_OK);
    EXPECT_STREQ(codigo, "JSON_DECODE_ERROR");
    EXPECT_STREQ(mensaje, "detalleServicio[0].impuestos: se esperaba una lista.");
    cec_result_free(res);
}

TEST(CApi, PunterosNulosYLimites) {
    cec_result* res = nullptr;
    EXPECT_EQ(cec_build(nullptr, &res), CEC_STATUS_ERR_NULL_POINTER);
    RequestFE f;
    EXPECT_EQ(cec_build(&f.req, nullptr), CEC_STATUS_ERR_NULL_POINTER);
    const uint8_t byte = '{';
    EXPECT_EQ(cec_build_json(&byte, 0, &res), CEC_STATUS_ERR_SIZE_LIMIT);
    EXPECT_EQ(cec_result_total_diagnosticos(nullptr, nullptr), CEC_STATUS_ERR_NULL_POINTER);
    cec_result_free(nullptr);
}

TEST(CApi, CalcularDaMontosAunqueFaltenDatos) {
    RequestFE f;
    f.req.emisor.nombre = nullptr; // Comprobante incompleto: la regla del nombre falla
    cec_result* res = nullptr;
    ASSERT_EQ(cec_calcular(&f.req, &res), CEC_STATUS_OK);
    EXPECT_FALSE(exito(res));
    EXPECT_NE(diagnosticos(res).find("RULE_EMISOR_NOMBRE_LENGTH"), std::string::npos) << diagnosticos(res);
    EXPECT_TRUE(xml(res).empty());

    cec_resumen_totales_c t{};
    ASSERT_EQ(cec_result_totales(res, &t), CEC_STATUS_OK);
    EXPECT_EQ(t.totalComprobante, 11300000000LL);
    cec_result_free(res);
}

TEST(CApi, TotalesSoloConComprobanteCalculado) {
    RequestFE f;
    f.linea.codigoCABYS = "123"; // Error de regla: cec_build no calcula
    cec_result* res = nullptr;
    ASSERT_EQ(cec_build(&f.req, &res), CEC_STATUS_OK);
    cec_resumen_totales_c t{};
    EXPECT_EQ(cec_result_totales(res, &t), CEC_STATUS_ERR_OUT_OF_RANGE);
    cec_result_free(res);
}
