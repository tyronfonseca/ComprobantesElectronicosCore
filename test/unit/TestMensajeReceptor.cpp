#include <gtest/gtest.h>
#include <string>
#include "cec/c_api/cec_c_api.h"
#include "cec/mensaje/MensajeReceptor.h"

/*
 * Mensaje Receptor v4.4: lectura del comprobante recibido, reglas, plazo del art. 10 y XML.
 */

namespace {

using cec::mensaje::MensajeReceptorRequest;

// Factura del proveedor 3101123456 al receptor 3101987654: 100000 + IVA 13000.
const char* kClave = "50601102600310112345600100001010000000001112345678";

const char* kFactura = R"(<?xml version="1.0" encoding="utf-8"?>
<FacturaElectronica xmlns="https://cdn.comprobanteselectronicos.go.cr/xml-schemas/v4.4/facturaElectronica">
  <Clave>50601102600310112345600100001010000000001112345678</Clave>
  <ProveedorSistemas>3101123456</ProveedorSistemas>
  <CodigoActividadEmisor>620100</CodigoActividadEmisor>
  <CodigoActividadReceptor>471100</CodigoActividadReceptor>
  <NumeroConsecutivo>00100001010000000001</NumeroConsecutivo>
  <FechaEmision>2026-10-01T10:00:00-06:00</FechaEmision>
  <Emisor>
    <Nombre>SOFTWARE EJEMPLO S.A.</Nombre>
    <Identificacion><Tipo>02</Tipo><Numero>3101123456</Numero></Identificacion>
    <CorreoElectronico>facturacion@ejemplo.cr</CorreoElectronico>
  </Emisor>
  <Receptor>
    <Nombre>CLIENTE EJEMPLO S.A.</Nombre>
    <Identificacion><Tipo>02</Tipo><Numero>3101987654</Numero></Identificacion>
  </Receptor>
  <ResumenFactura>
    <CodigoTipoMoneda><CodigoMoneda>CRC</CodigoMoneda><TipoCambio>1</TipoCambio></CodigoTipoMoneda>
    <TotalImpuesto>13000.00000</TotalImpuesto>
    <TotalComprobante>113000.00000</TotalComprobante>
  </ResumenFactura>
  <ds:Signature xmlns:ds="http://www.w3.org/2000/09/xmldsig#" Id="Signature-1"></ds:Signature>
</FacturaElectronica>)";

MensajeReceptorRequest aceptacion() {
    MensajeReceptorRequest r;
    r.clave = kClave;
    r.numeroCedulaEmisor = "3101123456";
    r.fechaEmisionDoc = "2026-10-05T09:00:00-06:00";
    r.mensaje = "1";
    r.montoTotalImpuesto = "13000";
    r.codigoActividad = "471100";
    r.condicionImpuesto = "01";
    r.montoTotalImpuestoAcreditar = "13000";
    r.totalFactura = "113000";
    r.numeroCedulaReceptor = "3101987654";
    r.numeroConsecutivoReceptor = "00100001050000000001";
    r.fechaEmisionComprobante = "2026-10-01T10:00:00-06:00";
    return r;
}

bool tiene(const cec::domain::DiagnosticList& d, const std::string& codigo) {
    for (const auto& x : d.items()) if (x.code == codigo) return true;
    return false;
}

} // namespace

TEST(MensajeReceptor, LeeLaFacturaDelProveedor) {
    auto [c, d] = cec::mensaje::leerComprobante(kFactura);
    EXPECT_FALSE(d.hasErrors());
    EXPECT_EQ(c.tipoDocumento, "01");
    EXPECT_EQ(c.version, "4.4");
    EXPECT_EQ(c.clave, kClave);
    EXPECT_EQ(c.emisorNumeroIdentificacion, "3101123456");
    EXPECT_EQ(c.emisorTipoIdentificacion, "02");
    EXPECT_EQ(c.receptorNumeroIdentificacion, "3101987654");
    EXPECT_EQ(c.codigoActividadReceptor, "471100");
    EXPECT_EQ(c.totalImpuesto, "13000.00000");
    EXPECT_EQ(c.totalComprobante, "113000.00000");
    EXPECT_TRUE(c.firmado);
    // Octubre 2026: el 1 de noviembre es domingo; 8 días hábiles desde el lunes 2 terminan el miércoles 11.
    EXPECT_EQ(c.plazoConfirmacion, "2026-11-11");
}

TEST(MensajeReceptor, RechazaArchivosQueNoSonComprobantesConfirmables) {
    EXPECT_TRUE(tiene(cec::mensaje::leerComprobante("<MensajeHacienda/>").second, "MR_NO_ES_COMPROBANTE"));
    EXPECT_TRUE(tiene(cec::mensaje::leerComprobante("no es xml").second, "MR_XML_INVALIDO"));
    std::string tiquete = kFactura;
    tiquete.replace(tiquete.find("<FacturaElectronica "), 19, "<TiqueteElectronico");
    tiquete.replace(tiquete.rfind("</FacturaElectronica>"), 21, "</TiqueteElectronico>");
    EXPECT_TRUE(tiene(cec::mensaje::leerComprobante(tiquete).second, "MR_TIPO_NO_CONFIRMABLE"));
    std::string sinFirma = kFactura;
    sinFirma.erase(sinFirma.find("  <ds:Signature"), sinFirma.find("</FacturaElectronica>") - sinFirma.find("  <ds:Signature"));
    EXPECT_TRUE(tiene(cec::mensaje::leerComprobante(sinFirma).second, "MR_SIN_FIRMA"));
}

TEST(MensajeReceptor, PlazoDelArticulo10CuentaDiasHabiles) {
    // Septiembre 2026: el 1 de octubre es jueves → 1, 2, 5, 6, 7, 8, 9, 12.
    EXPECT_EQ(cec::mensaje::detalle::iso(*cec::mensaje::plazoConfirmacion("2026-09-30T23:00:00-06:00")), "2026-10-12");
    // Diciembre pasa al año siguiente: 1 de enero de 2027 es viernes → 1, 4, 5, 6, 7, 8, 11, 12.
    EXPECT_EQ(cec::mensaje::detalle::iso(*cec::mensaje::plazoConfirmacion("2026-12-15T08:00:00-06:00")), "2027-01-12");
    EXPECT_FALSE(cec::mensaje::plazoConfirmacion("2026-12-15").has_value());
}

TEST(MensajeReceptor, AceptacionTotalGeneraElXmlEnOrdenDelXsd) {
    auto res = cec::mensaje::procesar(aceptacion());
    ASSERT_TRUE(res.isSuccess) << (res.diagnostics.empty() ? "" : res.diagnostics.items()[0].message);
    EXPECT_EQ(res.plazoConfirmacion, "2026-11-11");
    const std::string& x = res.xml;
    EXPECT_NE(x.find("<MensajeReceptor xmlns=\"https://cdn.comprobanteselectronicos.go.cr/xml-schemas/v4.4/mensajeReceptor\""), std::string::npos);
    EXPECT_NE(x.find("<MontoTotalImpuesto>13000.00000</MontoTotalImpuesto>"), std::string::npos);
    EXPECT_EQ(x.find("MontoTotalDeGastoAplicable"), std::string::npos);
    const char* orden[] = {"<Clave>", "<NumeroCedulaEmisor>", "<FechaEmisionDoc>", "<Mensaje>", "<MontoTotalImpuesto>",
                           "<CodigoActividad>", "<CondicionImpuesto>", "<MontoTotalImpuestoAcreditar>", "<TotalFactura>",
                           "<NumeroCedulaReceptor>", "<NumeroConsecutivoReceptor>"};
    size_t pos = 0;
    for (const char* tag : orden) {
        const size_t p = x.find(tag);
        ASSERT_NE(p, std::string::npos) << tag;
        EXPECT_GT(p, pos) << tag;
        pos = p;
    }
}

TEST(MensajeReceptor, ConsecutivoDebeSerDelTipoDelMensaje) {
    auto r = aceptacion();
    r.numeroConsecutivoReceptor = "00100001070000000001"; // 07 es rechazo
    EXPECT_TRUE(tiene(cec::mensaje::validar(r), "RULE_MR_CONSECUTIVO_TIPO"));
}

TEST(MensajeReceptor, IdentificacionDelEmisorDebeCoincidirConLaClave) {
    auto r = aceptacion();
    r.numeroCedulaEmisor = "3101999999";
    EXPECT_TRUE(tiene(cec::mensaje::validar(r), "RULE_MR_CLAVE_EMISOR"));
}

// MensajeReceptor_V4.4.xsd: el emisor puede tener hasta 20 caracteres (extranjero no domiciliado); el receptor, 9 a 12 dígitos.
TEST(MensajeReceptor, EmisorAdmiteHasta20CaracteresAlfanumericos) {
    auto r = aceptacion();
    r.numeroCedulaEmisor = "AB12345678901234567";  // 19 caracteres, no cabe en las 12 posiciones de la clave
    auto d = cec::mensaje::validar(r);
    EXPECT_FALSE(tiene(d, "RULE_MR_CEDULA_EMISOR"));
    EXPECT_FALSE(tiene(d, "RULE_MR_CLAVE_EMISOR"));

    r.numeroCedulaEmisor = "AB123456789012345678901"; // 23 caracteres
    EXPECT_TRUE(tiene(cec::mensaje::validar(r), "RULE_MR_CEDULA_EMISOR"));
    r.numeroCedulaEmisor = "3101-123456";            // con guion
    EXPECT_TRUE(tiene(cec::mensaje::validar(r), "RULE_MR_CEDULA_EMISOR"));
}

TEST(MensajeReceptor, ReceptorSiguePidiendo9A12Digitos) {
    auto r = aceptacion();
    r.numeroCedulaReceptor = "AB1234567";
    EXPECT_TRUE(tiene(cec::mensaje::validar(r), "RULE_MR_CEDULA_RECEPTOR"));
    r.numeroCedulaReceptor = "31019876541234";       // 14 dígitos
    EXPECT_TRUE(tiene(cec::mensaje::validar(r), "RULE_MR_CEDULA_RECEPTOR"));
}

TEST(MensajeReceptor, ConImpuestoExigeCondicionYActividad) {
    auto r = aceptacion();
    r.condicionImpuesto.clear();
    r.montoTotalImpuestoAcreditar.clear();
    EXPECT_TRUE(tiene(cec::mensaje::validar(r), "RULE_MR_CONDICION_REQUERIDA"));
    r = aceptacion();
    r.codigoActividad.clear();
    EXPECT_TRUE(tiene(cec::mensaje::validar(r), "RULE_MR_ACTIVIDAD_REQUERIDA"));
}

TEST(MensajeReceptor, MontosSegunLaCondicionDelImpuesto) {
    auto r = aceptacion();
    r.montoTotalImpuestoAcreditar = "12000";
    EXPECT_TRUE(tiene(cec::mensaje::validar(r), "RULE_MR_ACREDITAR_TOTAL"));

    r = aceptacion();
    r.condicionImpuesto = "04";
    r.montoTotalImpuestoAcreditar.clear();
    r.montoTotalDeGastoAplicable = "13000";
    EXPECT_FALSE(cec::mensaje::validar(r).hasErrors());
    r.montoTotalImpuestoAcreditar = "1";
    EXPECT_TRUE(tiene(cec::mensaje::validar(r), "RULE_MR_ACREDITAR_NO_APLICA"));

    r = aceptacion();
    r.condicionImpuesto = "02";
    r.montoTotalImpuestoAcreditar = "6500";
    r.montoTotalDeGastoAplicable = "6500";
    EXPECT_FALSE(cec::mensaje::validar(r).hasErrors());
    r.montoTotalDeGastoAplicable = "6000";
    EXPECT_TRUE(tiene(cec::mensaje::validar(r), "RULE_MR_MONTOS_IMPUESTO"));
}

TEST(MensajeReceptor, RechazoNoLlevaCondicionNiMontos) {
    auto r = aceptacion();
    r.mensaje = "3";
    r.numeroConsecutivoReceptor = "00100001070000000001";
    auto d = cec::mensaje::validar(r);
    EXPECT_TRUE(tiene(d, "RULE_MR_RECHAZO_SIN_CONDICION"));
    EXPECT_TRUE(tiene(d, "RULE_MR_DETALLE_RECOMENDADO"));

    r.condicionImpuesto.clear();
    r.montoTotalImpuestoAcreditar.clear();
    r.detalleMensaje = "Los precios no corresponden a la orden de compra 123.";
    EXPECT_FALSE(cec::mensaje::validar(r).hasErrors());
}

TEST(MensajeReceptor, DetalleLimitadoA160Caracteres) {
    auto r = aceptacion();
    r.detalleMensaje = std::string(160, 'a');
    EXPECT_FALSE(tiene(cec::mensaje::validar(r), "RULE_MR_DETALLE_LARGO"));
    r.detalleMensaje = "ñ" + std::string(159, 'a'); // 161 bytes, 160 caracteres
    EXPECT_FALSE(tiene(cec::mensaje::validar(r), "RULE_MR_DETALLE_LARGO"));
    r.detalleMensaje += "a";
    EXPECT_TRUE(tiene(cec::mensaje::validar(r), "RULE_MR_DETALLE_LARGO"));
}

TEST(MensajeReceptor, AdvierteFueraDePlazo) {
    auto r = aceptacion();
    r.fechaEmisionDoc = "2026-11-12T08:00:00-06:00";
    auto res = cec::mensaje::procesar(r);
    EXPECT_TRUE(res.isSuccess);
    EXPECT_TRUE(tiene(res.diagnostics, "RULE_MR_FUERA_DE_PLAZO"));
    r.fechaEmisionDoc = "2026-11-11T16:00:00-06:00";
    EXPECT_FALSE(tiene(cec::mensaje::procesar(r).diagnostics, "RULE_MR_FUERA_DE_PLAZO"));
}

TEST(MensajeReceptor, MontoInvalido) {
    auto r = aceptacion();
    r.totalFactura = "113000.123456";
    EXPECT_TRUE(tiene(cec::mensaje::validar(r), "RULE_MR_MONTO_INVALIDO"));
    r.totalFactura = "-5";
    EXPECT_TRUE(tiene(cec::mensaje::validar(r), "RULE_MR_MONTO_INVALIDO"));
}

TEST(MensajeReceptor, CApiJson) {
    const std::string json = R"({"clave":"50601102600310112345600100001010000000001112345678","numeroCedulaEmisor":"3101123456",
        "fechaEmisionDoc":"2026-10-05T09:00:00-06:00","mensaje":"1","montoTotalImpuesto":13000,"codigoActividad":"471100",
        "condicionImpuesto":"01","montoTotalImpuestoAcreditar":"13000","totalFactura":"113000","numeroCedulaReceptor":"3101987654",
        "numeroConsecutivoReceptor":"00100001050000000001","fechaEmisionComprobante":"2026-10-01T10:00:00-06:00"})";
    cec_result* res = nullptr;
    ASSERT_EQ(cec_mensaje_receptor_json(reinterpret_cast<const uint8_t*>(json.data()), json.size(), &res), CEC_STATUS_OK);
    int32_t ok = 0;
    cec_result_is_success(res, &ok);
    EXPECT_EQ(ok, 1);
    const uint8_t* data = nullptr;
    size_t len = 0;
    cec_result_data(res, &data, &len);
    const std::string salida(reinterpret_cast<const char*>(data), len);
    EXPECT_NE(salida.find("\"plazoConfirmacion\":\"2026-11-11\""), std::string::npos);
    EXPECT_NE(salida.find("<MensajeReceptor"), std::string::npos);
    cec_result_free(res);

    const std::string factura = kFactura;
    ASSERT_EQ(cec_leer_comprobante_xml(reinterpret_cast<const uint8_t*>(factura.data()), factura.size(), &res), CEC_STATUS_OK);
    cec_result_is_success(res, &ok);
    EXPECT_EQ(ok, 1);
    cec_result_data(res, &data, &len);
    EXPECT_NE(std::string(reinterpret_cast<const char*>(data), len).find("\"numeroIdentificacion\":\"3101987654\""), std::string::npos);
    cec_result_free(res);

    const std::string malo = "{\"clave\": [1]}";
    ASSERT_EQ(cec_mensaje_receptor_json(reinterpret_cast<const uint8_t*>(malo.data()), malo.size(), &res), CEC_STATUS_OK);
    cec_result_is_success(res, &ok);
    EXPECT_EQ(ok, 0);
    cec_result_free(res);
}

// ── API de C con structs (sin JSON) ──────────────────────────────────────────────────────────────

TEST(MensajeReceptor, CApiConStructs) {
    cec_mensaje_receptor_c m = {};
    m.clave = kClave;
    m.numeroCedulaEmisor = "3101123456";
    m.fechaEmisionDoc = "2026-10-05T09:00:00-06:00";
    m.mensaje = "1";
    m.montoTotalImpuesto = "13000";
    m.codigoActividad = "471100";
    m.condicionImpuesto = "01";
    m.montoTotalImpuestoAcreditar = "13000";
    m.totalFactura = "113000";
    m.numeroCedulaReceptor = "3101987654";
    m.numeroConsecutivoReceptor = "00100001050000000001";
    m.fechaEmisionComprobante = "2026-10-01T10:00:00-06:00";

    cec_result* res = nullptr;
    ASSERT_EQ(cec_mensaje_receptor(&m, &res), CEC_STATUS_OK);
    int32_t ok = 0;
    cec_result_is_success(res, &ok);
    EXPECT_EQ(ok, 1);
    const char* plazo = nullptr;
    ASSERT_EQ(cec_result_plazo_confirmacion(res, &plazo), CEC_STATUS_OK);
    EXPECT_STREQ(plazo, "2026-11-11");
    const uint8_t* xml = nullptr;
    size_t len = 0;
    cec_result_xml(res, &xml, &len);
    const std::string conStruct(reinterpret_cast<const char*>(xml), len);
    EXPECT_NE(conStruct.find("<MensajeReceptor"), std::string::npos);
    cec_result_free(res);

    // El mismo mensaje por JSON produce el mismo XML
    const std::string json = R"({"clave":"50601102600310112345600100001010000000001112345678","numeroCedulaEmisor":"3101123456",
        "fechaEmisionDoc":"2026-10-05T09:00:00-06:00","mensaje":"1","montoTotalImpuesto":"13000","codigoActividad":"471100",
        "condicionImpuesto":"01","montoTotalImpuestoAcreditar":"13000","totalFactura":"113000","numeroCedulaReceptor":"3101987654",
        "numeroConsecutivoReceptor":"00100001050000000001","fechaEmisionComprobante":"2026-10-01T10:00:00-06:00"})";
    ASSERT_EQ(cec_mensaje_receptor_json(reinterpret_cast<const uint8_t*>(json.data()), json.size(), &res), CEC_STATUS_OK);
    cec_result_xml(res, &xml, &len);
    EXPECT_EQ(std::string(reinterpret_cast<const char*>(xml), len), conStruct);
    cec_result_free(res);

    // Los campos no informados (NULL) producen diagnósticos con su ruta, no un fallo
    cec_mensaje_receptor_c vacio = {};
    ASSERT_EQ(cec_mensaje_receptor(&vacio, &res), CEC_STATUS_OK);
    cec_result_is_success(res, &ok);
    EXPECT_EQ(ok, 0);
    EXPECT_EQ(cec_result_plazo_confirmacion(res, &plazo), CEC_STATUS_ERR_OUT_OF_RANGE);
    size_t total = 0;
    cec_result_total_diagnosticos(res, &total);
    EXPECT_GT(total, 0u);
    cec_result_free(res);

    EXPECT_EQ(cec_mensaje_receptor(nullptr, &res), CEC_STATUS_ERR_NULL_POINTER);
}

TEST(MensajeReceptor, CApiLeeElComprobanteSinJson) {
    const std::string factura = kFactura;
    cec_result* res = nullptr;
    ASSERT_EQ(cec_leer_comprobante_xml(reinterpret_cast<const uint8_t*>(factura.data()), factura.size(), &res), CEC_STATUS_OK);
    cec_comprobante_recibido_c c = {};
    ASSERT_EQ(cec_result_comprobante_recibido(res, &c), CEC_STATUS_OK);
    EXPECT_STREQ(c.tipoDocumento, "01");
    EXPECT_STREQ(c.nombreDocumento, "FacturaElectronica");
    EXPECT_STREQ(c.version, "4.4");
    EXPECT_STREQ(c.clave, kClave);
    EXPECT_STREQ(c.emisorNombre, "SOFTWARE EJEMPLO S.A.");
    EXPECT_STREQ(c.emisorNumeroIdentificacion, "3101123456");
    EXPECT_STREQ(c.receptorNumeroIdentificacion, "3101987654");
    EXPECT_STREQ(c.codigoMoneda, "CRC");
    EXPECT_STREQ(c.totalComprobante, "113000.00000");
    EXPECT_EQ(c.firmado, 1);
    EXPECT_STREQ(c.plazoConfirmacion, "2026-11-11");
    const char* plazo = nullptr;
    ASSERT_EQ(cec_result_plazo_confirmacion(res, &plazo), CEC_STATUS_OK);
    EXPECT_STREQ(plazo, "2026-11-11");
    cec_result_free(res);

    // Un XML que no es un comprobante: no se puede confirmar, con datos parciales y el motivo en los diagnósticos
    const std::string otro = "<Otro/>";
    ASSERT_EQ(cec_leer_comprobante_xml(reinterpret_cast<const uint8_t*>(otro.data()), otro.size(), &res), CEC_STATUS_OK);
    int32_t ok = 1;
    cec_result_is_success(res, &ok);
    EXPECT_EQ(ok, 0);
    size_t diagnosticos = 0;
    cec_result_total_diagnosticos(res, &diagnosticos);
    EXPECT_GT(diagnosticos, 0u);
    ASSERT_EQ(cec_result_comprobante_recibido(res, &c), CEC_STATUS_OK);
    EXPECT_STREQ(c.nombreDocumento, "Otro");
    cec_result_free(res);

    // Texto que ni siquiera es XML: sin datos
    const std::string basura = "esto no es xml";
    ASSERT_EQ(cec_leer_comprobante_xml(reinterpret_cast<const uint8_t*>(basura.data()), basura.size(), &res), CEC_STATUS_OK);
    cec_result_is_success(res, &ok);
    EXPECT_EQ(ok, 0);
    EXPECT_EQ(cec_result_comprobante_recibido(res, &c), CEC_STATUS_ERR_OUT_OF_RANGE);
    cec_result_free(res);

    EXPECT_EQ(cec_result_comprobante_recibido(nullptr, &c), CEC_STATUS_ERR_NULL_POINTER);
}
