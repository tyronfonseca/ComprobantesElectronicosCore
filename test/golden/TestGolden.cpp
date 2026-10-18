#include <gtest/gtest.h>
#include <cstdlib>
#include <fstream>
#include <json.hpp>
#include "../support/Fixtures.h"
#include "cec/codec/EscritorJson.h"

/**
 * Golden: cada carpeta test/golden/<caso>/ trae request.json y la salida esperada
 * (expected.xml y expected_comprobante.json). Se compara exacto.
 *
 * Para regenerar después de un cambio intencional (revisar el diff antes de commitear):
 *   GOLDEN_UPDATE=1 ./build/CECTests --gtest_filter='Golden*'
 */
namespace {

class GoldenTest : public ::testing::TestWithParam<std::string> {};

void escribir(const std::filesystem::path& ruta, const std::string& contenido) {
    std::ofstream(ruta, std::ios::binary) << contenido;
}

TEST_P(GoldenTest, SalidaCoincideConLaEsperada) {
    const std::string caso = GetParam();
    const auto dir = fixtures::dirGolden(caso);
    const auto res = fixtures::procesar(fixtures::request(caso));
    ASSERT_TRUE(res.isSuccess) << fixtures::diagnosticos(res);

    const std::string xml = res.xml;
    const std::string comprobante = cec::codec::FormatoJson::comoJson(*res.comprobante).dump(2) + "\n";

    if (std::getenv("GOLDEN_UPDATE")) {
        escribir(dir / "expected.xml", xml);
        escribir(dir / "expected_comprobante.json", comprobante);
        GTEST_SKIP() << "Golden regenerado: " << caso;
    }

    EXPECT_EQ(xml, fixtures::leerArchivo(dir / "expected.xml"));
    EXPECT_EQ(comprobante, fixtures::leerArchivo(dir / "expected_comprobante.json"));
}

INSTANTIATE_TEST_SUITE_P(Golden, GoldenTest,
    ::testing::Values("fe_servicios", "fe_completa", "fe_surtido", "fe_usd", "fe_vehiculo_vin",
                      "te", "nc", "nd", "fec", "fee", "rep"),
    [](const auto& info) { return info.param; });

} // namespace
