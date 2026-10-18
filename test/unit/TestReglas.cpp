#include <gtest/gtest.h>
#include "../support/Fixtures.h"

using namespace cec;
using fixtures::procesar;
using fixtures::request;

/*
 * Reglas del Anexo 1 v4.4. Cada test parte de un comprobante válido (casos golden), cambia un dato
 * y verifica el diagnóstico. Las rutas usan los nombres del request.
 */

// ── Bases ────────────────────────────────────────────────────────────────────────────────────────

class BaseValidaTest : public ::testing::TestWithParam<std::string> {};

TEST_P(BaseValidaTest, ElComprobanteBaseEsValido) {
    const auto res = procesar(request(GetParam()));
    EXPECT_VALIDO(res);
    for (const auto& d : res.diagnostics.items()) {
        EXPECT_NE(d.severity, domain::DiagnosticSeverity::Error) << d.code;
    }
}

INSTANTIATE_TEST_SUITE_P(Reglas, BaseValidaTest,
    ::testing::Values("fe_servicios", "fe_completa", "fe_surtido", "fe_usd", "fe_vehiculo_vin",
                      "te", "nc", "nd", "fec", "fee", "rep"),
    [](const auto& info) { return info.param; });

// ── Encabezado ───────────────────────────────────────────────────────────────────────────────────

TEST(ReglasEncabezado, TipoDocumentoFueraDeNota3) {
    auto req = request("fe_servicios");
    req.tipoDocumento = "05";
    EXPECT_DIAG(procesar(req), "PARSE_TIPO_DOCUMENTO_INVALID");
}

TEST(ReglasEncabezado, ClaveDe50Posiciones) {
    auto req = request("fe_servicios");
    req.clave.pop_back();
    EXPECT_DIAG(procesar(req), "RULE_CLAVE_LENGTH");
}

TEST(ReglasEncabezado, ClaveConFechaDistintaALaEmision) {
    auto req = request("fe_servicios");
    req.fechaEmision = "2026-09-02T13:48:02-06:00";
    EXPECT_DIAG(procesar(req), "RULE_CLAVE_FECHA_MISMATCH");
}

TEST(ReglasEncabezado, ConsecutivoConTipoDeComprobanteDistinto) {
    auto req = request("fe_servicios");
    req.numeroConsecutivo.replace(8, 2, "04");
    EXPECT_DIAG(procesar(req), "RULE_CONSECUTIVO_TIPO_DOC");
}

TEST(ReglasEncabezado, FechaEmisionFutura) {
    auto req = request("fe_servicios");
    req.fechaEmision = "2026-10-16T10:00:00-06:00";
    EXPECT_DIAG(procesar(req), "RULE_FECHA_EMISION_FUTURA");
}

TEST(ReglasEncabezado, ProveedorSistemasObligatorio) {
    auto req = request("fe_servicios");
    req.proveedorSistemas.clear();
    EXPECT_DIAG(procesar(req), "RULE_PROVEEDOR_SISTEMAS_REQUIRED");
}

TEST(ReglasEncabezado, ActividadReceptorObligatoriaEnFEC) {
    auto req = request("fec");
    req.codigoActividadReceptor.clear();
    EXPECT_DIAG(procesar(req), "RULE_ACTIVIDAD_RECEPTOR_REQUIRED");
}

TEST(ReglasEncabezado, ActividadReceptorInexistenteEnFEE) {
    auto req = request("fee");
    req.codigoActividadReceptor = "620101";
    EXPECT_DIAG(procesar(req), "RULE_ACTIVIDAD_RECEPTOR_PROHIBIDA");
}

// ── Emisor y receptor ────────────────────────────────────────────────────────────────────────────

TEST(ReglasPartes, CedulaFisicaDe9Digitos) {
    auto req = request("fe_servicios");
    req.receptor->identificacion = {"01", "1023405678"};
    const auto res = procesar(req);
    EXPECT_DIAG(res, "RULE_CEDULA_FISICA_LENGTH");
    EXPECT_EQ(fixtures::ruta(res, "RULE_CEDULA_FISICA_LENGTH"), "receptor.identificacion.numero");
}

TEST(ReglasPartes, CedulaJuridicaDe10Caracteres) {
    auto req = request("fe_servicios");
    req.receptor->identificacion = {"02", "310112493"};
    EXPECT_DIAG(procesar(req), "RULE_CEDULA_JURIDICA_LENGTH");
}

TEST(ReglasPartes, EmisorExtranjeroNoDomiciliadoSoloEnFEC) {
    auto req = request("fe_servicios");
    req.emisor.identificacion = {"05", "US123456789"};
    EXPECT_DIAG(procesar(req), "RULE_EMISOR_EXTRANJERO_SOLO_FEC");
}

TEST(ReglasPartes, ReceptorExtranjeroEnFEExigeCondicion12) {
    auto req = request("fe_servicios");
    req.receptor->identificacion = {"05", "US998877665"};
    req.receptor->ubicacion.reset();
    req.receptor->otrasSenasExtranjero = "100 Tech Blvd, Silicon Valley, CA";
    EXPECT_DIAG(procesar(req), "RULE_FE_RECEPTOR_EXTRANJERO_CONDICION_12");
}

TEST(ReglasPartes, ReceptorExtranjeroProhibidoEnFEC) {
    auto req = request("fec");
    req.receptor->identificacion = {"05", "US998877665"};
    EXPECT_DIAG(procesar(req), "RULE_RECEPTOR_EXTRANJERO_PROHIBIDO");
}

TEST(ReglasPartes, ProvinciaFueraDeNota14) {
    auto req = request("fe_servicios");
    req.emisor.ubicacion->provincia = "9";
    EXPECT_DIAG(procesar(req), "RULE_UBICACION_PROVINCIA_FORMAT");
}

TEST(ReglasPartes, CantonInexistenteEnLaProvincia) {
    auto req = request("fe_servicios");
    req.emisor.ubicacion->canton = "99";
    EXPECT_DIAG(procesar(req), "RULE_UBICACION_CANTON_INVALID");
}

TEST(ReglasPartes, RegistroFiscal8707DeMaximo12) {
    auto req = request("fe_servicios");
    req.emisor.registroFiscal8707 = "1234567890123";
    EXPECT_DIAG(procesar(req), "RULE_EMISOR_REGISTRO_FISCAL_LENGTH");
}

TEST(ReglasPartes, RegistroFiscal8707InexistenteEnREP) {
    auto req = request("rep");
    req.emisor.registroFiscal8707 = "123456";
    EXPECT_DIAG(procesar(req), "RULE_REP_REGISTRO_FISCAL_PROHIBIDO");
}

TEST(ReglasPartes, BebidaAlcoholicaSinRegistro8707EsAdvertencia) {
    auto req = request("fe_servicios");
    auto& l = req.detalleServicio[0];
    l.codigoCABYS = "2421000000000"; // Vino (242)
    l.unidadMedida = "Unid";
    const auto res = procesar(req);
    EXPECT_DIAG(res, "RULE_EMISOR_REGISTRO_FISCAL_8707_REQUERIDO");
}

// ── Condición de venta ───────────────────────────────────────────────────────────────────────────

TEST(ReglasCondicionVenta, PlazoObligatorioEnCredito) {
    auto req = request("fe_servicios");
    req.condicionVenta = "02";
    req.plazoCredito.reset();
    EXPECT_DIAG(procesar(req), "RULE_PLAZO_CREDITO_REQUIRED");
}

TEST(ReglasCondicionVenta, OtrosExigeDetalle) {
    auto req = request("fe_servicios");
    req.condicionVenta = "99";
    EXPECT_DIAG(procesar(req), "RULE_CONDICION_VENTA_OTROS_REQUIRED");
}

TEST(ReglasCondicionVenta, Codigo12SoloEnFE) {
    auto req = request("te");
    req.condicionVenta = "12";
    EXPECT_DIAG(procesar(req), "RULE_CONDICION_VENTA_12_SOLO_FE");
}

TEST(ReglasCondicionVenta, Codigo13SoloEnFEC) {
    auto req = request("fe_servicios");
    req.condicionVenta = "13";
    EXPECT_DIAG(procesar(req), "RULE_CONDICION_VENTA_13_SOLO_FEC");
}

// ── Líneas de detalle ────────────────────────────────────────────────────────────────────────────

TEST(ReglasDetalle, NumeroLineaCorrelativo) {
    auto req = request("fe_completa");
    req.detalleServicio[1].numeroLinea = 3;
    const auto res = procesar(req);
    EXPECT_DIAG(res, "RULE_NUMERO_LINEA_CORRELATIVO");
    EXPECT_EQ(fixtures::ruta(res, "RULE_NUMERO_LINEA_CORRELATIVO"), "detalleServicio[1].numeroLinea");
}

// El XSD v4.4 publicado no enumera los códigos de referencia 13 a 17 ni los tipos 19 y 20 que sí están en el Anexo:
// la biblioteca los acepta, pero avisa porque el XML podría fallar la validación de esquema.
TEST(ReglasReferencia, AvisaCuandoElAnexoYElXsdNoCoinciden) {
    auto req = request("rep");
    ASSERT_FALSE(req.informacionReferencia.empty());
    auto r = procesar(req);
    EXPECT_VALIDO(r);
    EXPECT_FALSE(fixtures::tiene(r, "RULE_REF_CODIGO_FUERA_DEL_XSD"));

    req.informacionReferencia[0].codigo = "17";
    r = procesar(req);
    EXPECT_VALIDO(r);
    EXPECT_DIAG(r, "RULE_REF_CODIGO_FUERA_DEL_XSD");
    EXPECT_EQ(fixtures::ruta(r, "RULE_REF_CODIGO_FUERA_DEL_XSD"), "informacionReferencia[0].codigo");

    req = request("rep");
    req.informacionReferencia[0].tipoDocIR = "20";
    r = procesar(req);
    EXPECT_DIAG(r, "RULE_REF_TIPO_DOC_FUERA_DEL_XSD");
}

TEST(ReglasDetalle, UnidadDeMedidaDistingueMayusculas) {
    auto req = request("fe_completa");
    for (auto& l : req.detalleServicio) l.unidadMedida = "Kg";
    EXPECT_VALIDO(procesar(req));
    for (auto& l : req.detalleServicio) l.unidadMedida = "kg"; // v4.4: la minúscula ya no existe
    EXPECT_DIAG(procesar(req), "RULE_UNIDAD_MEDIDA_INVALID");
}

TEST(ReglasPartes, TelefonoEsUnEnteroDeAlMenos100) {
    auto req = request("fe_servicios");
    req.emisor.telefono = parsing::TelefonoInput{"506", "99"};
    auto r = procesar(req);
    EXPECT_DIAG(r, "RULE_TELEFONO_NUMERO_FORMAT");
    EXPECT_EQ(fixtures::ruta(r, "RULE_TELEFONO_NUMERO_FORMAT"), "emisor.telefono.numTelefono");

    req.emisor.telefono = parsing::TelefonoInput{"506", "00000042"}; // 42, aunque tenga 8 posiciones
    EXPECT_DIAG(procesar(req), "RULE_TELEFONO_NUMERO_FORMAT");

    req.emisor.telefono = parsing::TelefonoInput{"506", "911"};       // número especial: advertencia, no error
    r = procesar(req);
    EXPECT_FALSE(fixtures::tiene(r, "RULE_TELEFONO_NUMERO_FORMAT"));
    EXPECT_DIAG(r, "RULE_TELEFONO_NUMERO_ESPECIAL");

    req.emisor.telefono = parsing::TelefonoInput{"506", "99999999999999999999"}; // 20 dígitos: no debe desbordar ni lanzar
    EXPECT_FALSE(fixtures::tiene(procesar(req), "RULE_TELEFONO_NUMERO_FORMAT"));
}

TEST(ReglasDetalle, CabysDe13Digitos) {
    auto req = request("fe_servicios");
    req.detalleServicio[0].codigoCABYS = "83141";
    const auto res = procesar(req);
    EXPECT_DIAG(res, "RULE_CABYS_FORMAT");
    EXPECT_EQ(fixtures::ruta(res, "RULE_CABYS_FORMAT"), "detalleServicio[0].codigoCABYS");
}

TEST(ReglasDetalle, FacturaSinLineasExigeOtrosCargosQueLoPermitan) {
    // En FE el detalle solo puede omitirse con otros cargos 04, 08, 09 o 10 (Anexo 1, DetalleServicio).
    auto req = request("fe_servicios");
    req.detalleServicio.clear();
    EXPECT_DIAG(procesar(req), "RULE_NO_LINES_OTROS_CARGOS_REQUIRED");
}

TEST(ReglasDetalle, ExportacionSinLineas) {
    // DetalleServicio es obligatorio (1) en FEE, FEC y REP.
    auto req = request("fee");
    req.detalleServicio.clear();
    EXPECT_DIAG(procesar(req), "RULE_NO_LINES");
}

TEST(ReglasDetalle, MaximoCincoCodigosComerciales) {
    auto req = request("fe_servicios");
    req.detalleServicio[0].codigosComerciales.assign(6, {"01", "COD"});
    EXPECT_DIAG(procesar(req), "RULE_CODIGO_COMERCIAL_MAX_EXCEEDED");
}

TEST(ReglasDetalle, TipoCodigoComercialDeNota12) {
    auto req = request("fe_servicios");
    req.detalleServicio[0].codigosComerciales = {{"07", "COD"}};
    const auto res = procesar(req);
    EXPECT_DIAG(res, "RULE_CODIGO_COMERCIAL_TIPO_INVALID");
    EXPECT_EQ(fixtures::ruta(res, "RULE_CODIGO_COMERCIAL_TIPO_INVALID"), "detalleServicio[0].codigosComerciales[0].tipo");
}

TEST(ReglasDetalle, SubTotalSoloEditableEnREP) {
    auto req = request("fe_servicios");
    req.detalleServicio[0].subTotal = "500";
    EXPECT_DIAG(procesar(req), "RULE_SUBTOTAL_NO_EDITABLE");
}

TEST(ReglasDetalle, MontoTotalSoloSeInformaEnREP) {
    auto req = request("fe_servicios");
    req.detalleServicio[0].montoTotal = "500";
    EXPECT_DIAG(procesar(req), "PARSE_MONTO_TOTAL_NO_EDITABLE");
}

TEST(ReglasDetalle, DecimalInvalidoIndicaLaRuta) {
    auto req = request("fe_servicios");
    req.detalleServicio[0].precioUnitario = "1.000,50";
    const auto res = procesar(req);
    EXPECT_DIAG(res, "PARSE_DECIMAL_INVALID");
    EXPECT_EQ(fixtures::ruta(res, "PARSE_DECIMAL_INVALID"), "detalleServicio[0].precioUnitario");
}

// ── Descuentos ───────────────────────────────────────────────────────────────────────────────────

TEST(ReglasDescuentos, MaximoCincoPorLinea) {
    auto req = request("fe_completa");
    req.detalleServicio[1].descuentos.assign(6, {"10", "07", "", ""});
    EXPECT_DIAG(procesar(req), "RULE_DESCUENTO_MAX_EXCEEDED");
}

TEST(ReglasDescuentos, CodigoDeNota20) {
    auto req = request("fe_completa");
    req.detalleServicio[1].descuentos[0].codigoDescuento = "55";
    const auto res = procesar(req);
    EXPECT_DIAG(res, "RULE_DESCUENTO_CODIGO_INVALID");
    EXPECT_EQ(fixtures::ruta(res, "RULE_DESCUENTO_CODIGO_INVALID"), "detalleServicio[1].descuentos[0].codigoDescuento");
}

TEST(ReglasDescuentos, Codigo99ExigeDescripcionYNaturaleza) {
    auto req = request("fe_completa");
    auto& d = req.detalleServicio[0].descuentos[0];
    d.codigoDescuentoOTRO.clear();
    d.naturalezaDescuento.clear();
    const auto res = procesar(req);
    EXPECT_DIAG(res, "RULE_DESCUENTO_OTRO_LENGTH");
    EXPECT_DIAG(res, "RULE_DESCUENTO_NATURALEZA_LENGTH");
}

// ── Impuestos ────────────────────────────────────────────────────────────────────────────────────

TEST(ReglasImpuestos, TarifaTransitoriaSoloEnNotas) {
    auto req = request("fe_servicios");
    req.detalleServicio[0].impuestos[0].codigoTarifaIVA = "05";
    EXPECT_DIAG(procesar(req), "RULE_TARIFA_TRANSITORIA_SOLO_NOTAS");
}

TEST(ReglasImpuestos, CodigoDeImpuestoDeNota8) {
    auto req = request("fe_servicios");
    req.detalleServicio[0].impuestos[0].codigo = "09";
    EXPECT_DIAG(procesar(req), "RULE_IMPUESTO_CODIGO_INVALID");
}

TEST(ReglasImpuestos, IVAExigeCodigoDeTarifa) {
    auto req = request("fe_servicios");
    req.detalleServicio[0].impuestos[0].codigoTarifaIVA.clear();
    const auto res = procesar(req);
    EXPECT_DIAG(res, "RULE_TARIFA_IVA_REQUIRED");
    EXPECT_EQ(fixtures::ruta(res, "RULE_TARIFA_IVA_REQUIRED"), "detalleServicio[0].impuestos[0].codigoTarifaIVA");
}

TEST(ReglasImpuestos, CodigoDeTarifaDeNota81) {
    auto req = request("fe_servicios");
    req.detalleServicio[0].impuestos[0].codigoTarifaIVA = "12";
    EXPECT_DIAG(procesar(req), "RULE_TARIFA_IVA_INVALID");
}

TEST(ReglasImpuestos, TarifaDelIVACoincideConSuCodigo) {
    auto req = request("fe_servicios");
    auto& imp = req.detalleServicio[0].impuestos[0];
    imp.codigoTarifaIVA = "08";
    imp.tarifa = "4";
    EXPECT_DIAG(procesar(req), "RULE_TARIFA_IVA_NO_COINCIDE");
}

/// Hacienda rechaza la tarifa 01 en la FEE (error -101) y los totales gravados/exentos dejan de cuadrar.
class FEETarifaNoSujetoTest : public ::testing::TestWithParam<std::string> {};

TEST_P(FEETarifaNoSujetoTest, ExportacionNoAdmiteTarifasDeNoSujeto) {
    auto req = request("fee");
    req.detalleServicio[0].impuestos[0].codigoTarifaIVA = GetParam();
    const auto res = procesar(req);
    EXPECT_DIAG(res, "RULE_FEE_TARIFA_NO_SUJETO");
    EXPECT_FALSE(res.isSuccess);
}

INSTANTIATE_TEST_SUITE_P(Reglas, FEETarifaNoSujetoTest, ::testing::Values("01", "11"),
    [](const auto& info) { return "Tarifa" + info.param; });

TEST(ReglasImpuestos, ExportacionExentaConTarifa10) {
    auto req = request("fee");
    req.detalleServicio[0].impuestos[0].codigoTarifaIVA = "10";
    EXPECT_VALIDO(procesar(req));
}

TEST(ReglasImpuestos, ExportacionSinIVADeCalculoEspecial) {
    auto req = request("fee");
    req.detalleServicio[0].impuestos[0].codigo = "07";
    EXPECT_DIAG(procesar(req), "RULE_FEE_IMPUESTO_07_NO_APLICA");
}

TEST(ReglasImpuestos, SelectivoDeConsumoExigeTarifa) {
    auto req = request("fe_servicios");
    req.detalleServicio[0].impuestos.push_back({"02", "", "", "", "", std::nullopt, ""});
    EXPECT_DIAG(procesar(req), "RULE_IMPUESTO_TARIFA_REQUIRED");
}

TEST(ReglasImpuestos, CementoAlCincoPorCiento) {
    auto req = request("fe_servicios");
    req.detalleServicio[0].impuestos.push_back({"12", "", "", "4", "", std::nullopt, ""});
    EXPECT_DIAG(procesar(req), "RULE_IMPUESTO_CEMENTO_TARIFA");
}

TEST(ReglasImpuestos, BienesUsadosExigeFactor) {
    auto req = request("fe_servicios");
    req.detalleServicio[0].impuestos = {{"08", "", "", "", "", std::nullopt, ""}};
    EXPECT_DIAG(procesar(req), "RULE_IMPUESTO_FACTOR_IVA_REQUIRED");
}

TEST(ReglasImpuestos, EspecificoExigeDatos) {
    auto req = request("fe_servicios");
    auto& l = req.detalleServicio[0];
    l.codigoCABYS = "2449001000000"; // Gaseosa (244)
    l.unidadMedida = "Unid";
    l.impuestos.push_back({"05", "", "", "", "", std::nullopt, ""});
    EXPECT_DIAG(procesar(req), "RULE_IMPUESTO_DATOS_ESPECIFICOS_REQUIRED");
}

// ── Exoneración ──────────────────────────────────────────────────────────────────────────────────

TEST(ReglasExoneracion, NoSuperaLaTarifaDelIVA) {
    auto req = request("fe_completa");
    req.detalleServicio[0].exoneracion->tarifaExonerada = "15";
    const auto res = procesar(req);
    EXPECT_DIAG(res, "RULE_EXONERACION_EXCEDE_TARIFA");
    EXPECT_EQ(fixtures::ruta(res, "RULE_EXONERACION_EXCEDE_TARIFA"), "detalleServicio[0].exoneracion.tarifaExonerada");
}

TEST(ReglasExoneracion, TiposInhabilitadosFueraDeNotas) {
    auto req = request("fe_completa");
    req.detalleServicio[0].exoneracion->tipoDocumentoEX1 = "01";
    EXPECT_DIAG(procesar(req), "RULE_EXONERACION_TIPO_DOC_INHABILITADO");
}

TEST(ReglasExoneracion, ArticuloObligatorioSegunElTipo) {
    auto req = request("fe_completa");
    req.detalleServicio[0].exoneracion->tipoDocumentoEX1 = "02";
    EXPECT_DIAG(procesar(req), "RULE_EXONERACION_ARTICULO_REQUIRED");
}

TEST(ReglasExoneracion, InexistenteEnFEE) {
    auto req = request("fee");
    req.detalleServicio[0].exoneracion = request("fe_completa").detalleServicio[0].exoneracion;
    EXPECT_DIAG(procesar(req), "RULE_FEE_EXONERACION_PROHIBIDA");
}

// ── Bienes especiales ────────────────────────────────────────────────────────────────────────────

TEST(ReglasBienesEspeciales, PartidaArancelariaSoloEnExportacion) {
    auto req = request("fe_servicios");
    req.detalleServicio[0].partidaArancelaria = "847130000000";
    EXPECT_DIAG(procesar(req), "RULE_PARTIDA_ARANCELARIA_NOT_ALLOWED");
}

TEST(ReglasBienesEspeciales, PartidaArancelariaObligatoriaParaBienesEnFEE) {
    auto req = request("fee");
    auto& l = req.detalleServicio[0];
    l.codigoCABYS = "4523000000000"; // Computadora portátil (bien)
    l.unidadMedida = "Unid";
    EXPECT_DIAG(procesar(req), "RULE_PARTIDA_ARANCELARIA_REQUIRED");
}

TEST(ReglasBienesEspeciales, PartidaArancelariaDe12Digitos) {
    auto req = request("fee");
    auto& l = req.detalleServicio[0];
    l.codigoCABYS = "4523000000000";
    l.unidadMedida = "Unid";
    l.partidaArancelaria = "8471300000";
    EXPECT_DIAG(procesar(req), "RULE_PARTIDA_ARANCELARIA_FORMAT");
}

TEST(ReglasBienesEspeciales, VehiculoSinVINEsAdvertencia) {
    auto req = request("fe_vehiculo_vin");
    req.detalleServicio[0].numerosVINoSerie.clear();
    const auto res = procesar(req);
    EXPECT_DIAG(res, "RULE_VIN_REQUERIDO");
    EXPECT_EQ(fixtures::ruta(res, "RULE_VIN_REQUERIDO"), "detalleServicio[0].numerosVINoSerie");
}

TEST(ReglasBienesEspeciales, VINVacio) {
    auto req = request("fe_vehiculo_vin");
    req.detalleServicio[0].numerosVINoSerie = {""};
    EXPECT_DIAG(procesar(req), "RULE_VIN_LENGTH");
}

TEST(ReglasBienesEspeciales, FormaFarmaceuticaDeNota9) {
    auto req = request("fe_servicios");
    req.detalleServicio[0].formaFarmaceutica = "ZZ";
    EXPECT_DIAG(procesar(req), "RULE_FORMA_FARMACEUTICA_INVALID");
}

TEST(ReglasBienesEspeciales, SurtidoInexistenteEnFEC) {
    auto req = request("fec");
    req.detalleServicio[0].detalleSurtido = request("fe_surtido").detalleServicio[0].detalleSurtido;
    EXPECT_DIAG(procesar(req), "RULE_FEC_SURTIDO_PROHIBIDO");
}

TEST(ReglasBienesEspeciales, SurtidoHabilitadoExigeCodigoDelFabricante) {
    auto req = request("fe_surtido"); // CABYS 2399999002200: surtido de productos alimenticios
    req.detalleServicio[0].codigosComerciales = {{"01", "CANASTA-01"}};
    EXPECT_DIAG(procesar(req), "RULE_SURTIDO_CODIGO_FABRICANTE_REQUIRED");
}

TEST(ReglasBienesEspeciales, SurtidoDeMaximo20Componentes) {
    auto req = request("fe_surtido");
    auto& s = req.detalleServicio[0].detalleSurtido;
    while (s.size() <= 20) s.push_back(s[0]);
    EXPECT_DIAG(procesar(req), "RULE_SURTIDO_MAX_COMPONENTES");
}

// ── Otros cargos ─────────────────────────────────────────────────────────────────────────────────

TEST(ReglasOtrosCargos, TipoDeNota16) {
    auto req = request("fe_completa");
    req.otrosCargos[0].tipoDocumentoOC = "55";
    const auto res = procesar(req);
    EXPECT_DIAG(res, "RULE_CARGO_TIPO_INVALID");
    EXPECT_EQ(fixtures::ruta(res, "RULE_CARGO_TIPO_INVALID"), "otrosCargos[0].tipoDocumentoOC");
}

TEST(ReglasOtrosCargos, CobroDeTerceroExigeIdentificacion) {
    auto req = request("fe_completa");
    req.otrosCargos[1].identificacionTercero.reset();
    EXPECT_DIAG(procesar(req), "RULE_CARGO_TERCERO_REQUIRED");
}

TEST(ReglasOtrosCargos, TerceroExigeTipoDeIdentificacion) {
    auto req = request("fe_completa");
    req.otrosCargos[1].identificacionTercero->tipo.clear();
    const auto res = procesar(req);
    EXPECT_DIAG(res, "RULE_CARGO_TERCERO_TIPO_REQUIRED");
    EXPECT_EQ(fixtures::ruta(res, "RULE_CARGO_TERCERO_TIPO_REQUIRED"), "otrosCargos[1].identificacionTercero.tipo");
}

TEST(ReglasOtrosCargos, SinLineasPermitidoConCargoDeTercero) {
    auto req = request("fe_completa");
    req.detalleServicio.clear();
    req.otrosCargos.erase(req.otrosCargos.begin()); // Queda solo el cobro de tercero (04)
    const auto res = procesar(req);
    EXPECT_FALSE(fixtures::tiene(res, "RULE_NO_LINES")) << fixtures::diagnosticos(res);
    EXPECT_FALSE(fixtures::tiene(res, "RULE_NO_LINES_OTROS_CARGOS_REQUIRED")) << fixtures::diagnosticos(res);
}

TEST(ReglasOtrosCargos, SinLineasCargoNoPermitidoSenalaSuTipo) {
    auto req = request("fe_completa");
    req.detalleServicio.clear();
    req.otrosCargos[0].tipoDocumentoOC = "06"; // Servicio 10%: no se admite sin líneas
    const auto res = procesar(req);
    EXPECT_DIAG(res, "RULE_NO_LINES_OTROS_CARGOS_INVALIDO");
    // La ruta es la del campo del request, para que el formulario muestre el error debajo del tipo de cargo.
    EXPECT_EQ(fixtures::ruta(res, "RULE_NO_LINES_OTROS_CARGOS_INVALIDO"), "otrosCargos[0].tipoDocumentoOC");
}

// ── Moneda y medios de pago ──────────────────────────────────────────────────────────────────────

TEST(ReglasResumen, MonedaDelCatalogo) {
    auto req = request("fe_servicios");
    req.codigoMoneda = "ZZZ";
    EXPECT_DIAG(procesar(req), "RULE_MONEDA_INVALID");
}

TEST(ReglasResumen, TipoDeCambioObligatorioEnMonedaExtranjera) {
    auto req = request("fe_usd");
    req.tipoCambio.clear();
    const auto res = procesar(req);
    EXPECT_DIAG(res, "RULE_TIPO_CAMBIO_REQUIRED");
    EXPECT_EQ(fixtures::ruta(res, "RULE_TIPO_CAMBIO_REQUIRED"), "resumenFactura.codigoTipoMoneda.tipoCambio");
}

TEST(ReglasResumen, TipoDeCambio1EnColones) {
    auto req = request("fe_servicios");
    req.tipoCambio = "2";
    EXPECT_DIAG(procesar(req), "RULE_TIPO_CAMBIO_CRC");
}

TEST(ReglasResumen, MedioDePagoDeNota6) {
    auto req = request("fe_servicios");
    ASSERT_FALSE(req.mediosPago.empty());
    req.mediosPago[0].tipoMedioPago = "08";
    EXPECT_DIAG(procesar(req), "RULE_MEDIO_PAGO_TIPO_INVALID");
}

TEST(ReglasEncabezado, CondicionDeVentaDeNota5) {
    auto req = request("fe_servicios");
    req.condicionVenta = "16";
    EXPECT_DIAG(procesar(req), "RULE_CONDICION_VENTA_INVALID");
}

TEST(ReglasReceptor, TipoDeIdentificacionDeNota4) {
    auto req = request("fe_servicios");
    ASSERT_TRUE(req.receptor && req.receptor->identificacion);
    req.receptor->identificacion->tipo = "07";
    const auto res = procesar(req);
    EXPECT_DIAG(res, "RULE_ID_TIPO_INVALID");
    EXPECT_EQ(fixtures::ruta(res, "RULE_ID_TIPO_INVALID"), "receptor.identificacion.tipo");
}

TEST(ReglasResumen, MaximoCuatroMediosDePago) {
    auto req = request("fe_servicios");
    req.mediosPago.assign(5, {"01", "", "1"});
    EXPECT_DIAG(procesar(req), "RULE_MEDIO_PAGO_MAX_EXCEEDED");
}

TEST(ReglasResumen, MedioPagoOtrosExigeDetalle) {
    auto req = request("fe_servicios");
    req.mediosPago = {{"99", "", ""}};
    const auto res = procesar(req);
    EXPECT_DIAG(res, "RULE_MEDIO_PAGO_OTROS_LENGTH");
    EXPECT_EQ(fixtures::ruta(res, "RULE_MEDIO_PAGO_OTROS_LENGTH"), "resumenFactura.mediosPago[0].medioPagoOtros");
}

TEST(ReglasResumen, VariosMediosDePagoDebenSumarElTotal) {
    auto req = request("fe_servicios");
    req.mediosPago = {{"01", "", "50000"}, {"02", "", "1000"}};
    EXPECT_DIAG(procesar(req), "RULE_MEDIO_PAGO_SUMA_MISMATCH");
}

// ── Información de referencia ────────────────────────────────────────────────────────────────────

TEST(ReglasReferencia, ObligatoriaEnNotaDeCredito) {
    auto req = request("nc");
    req.informacionReferencia.clear();
    EXPECT_DIAG(procesar(req), "RULE_REFERENCIA_REQUIRED");
}

TEST(ReglasReferencia, ObligatoriaEnFEC) {
    auto req = request("fec");
    req.informacionReferencia.clear();
    EXPECT_DIAG(procesar(req), "RULE_FEC_REFERENCIA_REQUIRED");
}

TEST(ReglasReferencia, MaximoDiez) {
    auto req = request("nc");
    req.informacionReferencia.assign(11, req.informacionReferencia[0]);
    EXPECT_DIAG(procesar(req), "RULE_REFERENCIA_MAX_EXCEEDED");
}

TEST(ReglasReferencia, TipoDoc16SoloEnFEC) {
    auto req = request("nc");
    req.informacionReferencia[0].tipoDocIR = "16";
    const auto res = procesar(req);
    EXPECT_DIAG(res, "RULE_REF_TIPO_16_SOLO_FEC");
    EXPECT_EQ(fixtures::ruta(res, "RULE_REF_TIPO_16_SOLO_FEC"), "informacionReferencia[0].tipoDocIR");
}

TEST(ReglasReferencia, Codigo12SoloEnNotaDeCredito) {
    auto req = request("nd");
    req.informacionReferencia[0].codigo = "12";
    EXPECT_DIAG(procesar(req), "RULE_REF_CODIGO_12_ONLY_NC");
}

TEST(ReglasReferencia, Codigo17SoloEnREP) {
    auto req = request("nc");
    req.informacionReferencia[0].codigo = "17";
    EXPECT_DIAG(procesar(req), "RULE_REF_CODIGO_17_ONLY_REP");
}

TEST(ReglasReferencia, NCQueModificaFEEAdmitePartidaArancelaria) {
    auto req = request("nc");
    req.informacionReferencia[0].tipoDocIR = "19"; // Factura Electrónica de Exportación
    auto& l = req.detalleServicio[0];
    l.codigoCABYS = "4523000000000";
    l.unidadMedida = "Unid";
    l.partidaArancelaria = "847130000000";
    const auto res = procesar(req);
    EXPECT_FALSE(fixtures::tiene(res, "RULE_PARTIDA_ARANCELARIA_NOT_ALLOWED")) << fixtures::diagnosticos(res);
}

// ── Recibo Electrónico de Pago ───────────────────────────────────────────────────────────────────

TEST(ReglasREP, CondicionDeVentaDelREP) {
    auto req = request("rep");
    req.condicionVenta = "01";
    EXPECT_DIAG(procesar(req), "RULE_REP_CONDICION_VENTA_INVALID");
}

TEST(ReglasREP, MontoTotalPositivo) {
    auto req = request("rep");
    req.detalleServicio[0].montoTotal = "0";
    const auto res = procesar(req);
    EXPECT_DIAG(res, "RULE_REP_MONTO_PAGO_NON_POSITIVE");
    EXPECT_EQ(fixtures::ruta(res, "RULE_REP_MONTO_PAGO_NON_POSITIVE"), "detalleServicio[0].montoTotal");
}

TEST(ReglasREP, SinDescuentos) {
    auto req = request("rep");
    req.detalleServicio[0].descuentos = {{"100", "07", "", ""}};
    EXPECT_DIAG(procesar(req), "RULE_REP_DESCUENTO_PROHIBIDO");
}

TEST(ReglasREP, SinOtrosCargos) {
    auto req = request("rep");
    req.otrosCargos = request("fe_completa").otrosCargos;
    EXPECT_DIAG(procesar(req), "RULE_REP_OTROS_CARGOS_PROHIBIDO");
}

TEST(ReglasREP, ReferenciaObligatoria) {
    auto req = request("rep");
    req.informacionReferencia.clear();
    EXPECT_DIAG(procesar(req), "RULE_REP_REFERENCIA_REQUIRED");
}
