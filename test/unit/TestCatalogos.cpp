#include <gtest/gtest.h>
#include <string>
#include <cstring>
#include "cec/c_api/cec_c_api.h"

TEST(CatalogosCApiTest, DivisionTerritorialProvincias) {
    uint32_t total = cec_dta_total_provincias();
    EXPECT_EQ(total, 7u);

    uint8_t cod = 0;
    const char* nombre = nullptr;
    uint32_t cantones = 0;

    int32_t rc = cec_dta_provincia_por_indice(0, &cod, &nombre, &cantones);
    EXPECT_EQ(rc, 1);
    EXPECT_EQ(cod, 1);
    ASSERT_NE(nombre, nullptr);
    EXPECT_STREQ(nombre, "San José");
    EXPECT_EQ(cantones, 20u);

    // Fuera de rango
    rc = cec_dta_provincia_por_indice(7, &cod, &nombre, &cantones);
    EXPECT_EQ(rc, 0);

    // Validaciones
    EXPECT_EQ(cec_dta_validar_provincia(1), 1);
    EXPECT_EQ(cec_dta_validar_provincia(7), 1);
    EXPECT_EQ(cec_dta_validar_provincia(8), 0);
    EXPECT_EQ(cec_dta_validar_provincia(0), 0);

    // Nombre directo
    const char* p1 = cec_dta_nombre_provincia(1);
    ASSERT_NE(p1, nullptr);
    EXPECT_STREQ(p1, "San José");
    EXPECT_STREQ(cec_dta_nombre_provincia(99), "");
}

TEST(CatalogosCApiTest, DivisionTerritorialCantonesYDistritos) {
    // San José Central
    uint32_t cantonesSJ = cec_dta_total_cantones(1);
    EXPECT_EQ(cantonesSJ, 20u);

    uint8_t codCanton = 0;
    const char* nombreCanton = nullptr;
    uint32_t distritos = 0;
    int32_t rc = cec_dta_canton_por_indice(1, 0, &codCanton, &nombreCanton, &distritos);
    EXPECT_EQ(rc, 1);
    EXPECT_EQ(codCanton, 1);
    ASSERT_NE(nombreCanton, nullptr);
    EXPECT_STREQ(nombreCanton, "San José");
    EXPECT_EQ(distritos, 11u);

    EXPECT_EQ(cec_dta_validar_canton(1, 1), 1);
    EXPECT_EQ(cec_dta_validar_canton(1, 99), 0);

    // Distritos de San José Central
    uint32_t totalDist = cec_dta_total_distritos(1, 1);
    EXPECT_EQ(totalDist, 11u);

    uint8_t codDist = 0;
    const char* nombreDist = nullptr;
    uint32_t barrios = 0;
    rc = cec_dta_distrito_por_indice(1, 1, 0, &codDist, &nombreDist, &barrios);
    EXPECT_EQ(rc, 1);
    EXPECT_EQ(codDist, 1);
    ASSERT_NE(nombreDist, nullptr);
    EXPECT_STREQ(nombreDist, "Carmen");

    EXPECT_EQ(cec_dta_validar_distrito(1, 1, 1), 1);
    EXPECT_EQ(cec_dta_validar_distrito(1, 1, 99), 0);

    // Validación completa de ubicación
    EXPECT_EQ(cec_dta_validar_ubicacion(1, 1, 1), 1);
    EXPECT_EQ(cec_dta_validar_ubicacion(1, 1, 99), 0);
    EXPECT_EQ(cec_dta_validar_ubicacion(9, 1, 1), 0);
}

TEST(CatalogosCApiTest, CatalogoMonedas) {
    uint32_t total = cec_catalogo_total_monedas();
    EXPECT_GT(total, 0u);

    const char* cod = nullptr;
    const char* sim = nullptr;
    const char* desc = nullptr;

    int32_t rc = cec_catalogo_moneda_por_indice(0, &cod, &sim, &desc);
    EXPECT_EQ(rc, 1);
    ASSERT_NE(cod, nullptr);
    ASSERT_NE(desc, nullptr);

    // Buscar CRC o USD
    bool foundCRC = false;
    for (uint32_t i = 0; i < total; ++i) {
        cec_catalogo_moneda_por_indice(i, &cod, &sim, &desc);
        if (cod && std::strcmp(cod, "CRC") == 0) {
            foundCRC = true;
            EXPECT_STREQ(sim, "₡");
            break;
        }
    }
    EXPECT_TRUE(foundCRC);
}

TEST(CatalogosCApiTest, CatalogoUnidadesMedida) {
    uint32_t total = cec_catalogo_total_unidades_medida();
    EXPECT_GT(total, 0u);

    const char* cod = nullptr;
    const char* sim = nullptr;
    const char* desc = nullptr;

    bool foundUnidad = false;
    for (uint32_t i = 0; i < total; ++i) {
        cec_catalogo_unidad_medida_por_indice(i, &cod, &sim, &desc);
        if (cod && (std::strcmp(cod, "Unid") == 0 || std::strcmp(cod, "Sp") == 0)) {
            foundUnidad = true;
            break;
        }
    }
    EXPECT_TRUE(foundUnidad);
}

namespace {
bool existeUnidad(const char* codigo) {
    const char *cod = nullptr, *sim = nullptr, *desc = nullptr;
    for (uint32_t i = 0; i < cec_catalogo_total_unidades_medida(); ++i) {
        cec_catalogo_unidad_medida_por_indice(i, &cod, &sim, &desc);
        if (cod && std::strcmp(cod, codigo) == 0) return true;
    }
    return false;
}

bool existeMoneda(const char* codigo) {
    const char *cod = nullptr, *sim = nullptr, *desc = nullptr;
    for (uint32_t i = 0; i < cec_catalogo_total_monedas(); ++i) {
        cec_catalogo_moneda_por_indice(i, &cod, &sim, &desc);
        if (cod && std::strcmp(cod, codigo) == 0) return true;
    }
    return false;
}
} // namespace

// El XSD v4.4 distingue mayúsculas de minúsculas en la unidad de medida (Kg, M, Min, kWh...).
TEST(CatalogosCApiTest, UnidadesDeMedidaSegunElXsd) {
    EXPECT_EQ(cec_catalogo_total_unidades_medida(), 101u);
    for (const char* c : {"Kg", "G", "M", "Mm", "Min", "D", "U", "Ua", "Al", "kWh", "Unid", "h", "Sp"}) EXPECT_TRUE(existeUnidad(c)) << c;
    for (const char* c : {"kg", "g", "m", "mm", "min", "d", "u", "ua"}) EXPECT_FALSE(existeUnidad(c)) << c;
}

// El XSD v4.4 ya no admite estos códigos ISO 4217 de metales y de uso especial.
TEST(CatalogosCApiTest, MonedasSinLosCodigosRetiradosDelXsd) {
    EXPECT_EQ(cec_catalogo_total_monedas(), 168u);
    for (const char* c : {"CRC", "USD", "EUR"}) EXPECT_TRUE(existeMoneda(c)) << c;
    for (const char* c : {"XAG", "XAU", "XBA", "XBB", "XBC", "XBD", "XPD", "XPT", "XTS", "XXX"}) EXPECT_FALSE(existeMoneda(c)) << c;
}

TEST(CatalogosCApiTest, CatalogoFormasFarmaceuticas) {
    uint32_t total = cec_catalogo_total_formas_farmaceuticas();
    EXPECT_EQ(total, 229u);

    const char* cod = nullptr;
    const char* desc = nullptr;

    int32_t rc = cec_catalogo_forma_farmaceutica_por_indice(0, &cod, &desc);
    EXPECT_EQ(rc, 1);
    EXPECT_STREQ(cod, "01");
    ASSERT_NE(desc, nullptr);
    EXPECT_STRNE(desc, "");
}

TEST(CatalogosCApiTest, CatalogosGenericos) {
    uint32_t totalNombres = cec_catalogo_total_nombres();
    EXPECT_GT(totalNombres, 0u);

    bool foundTipoDoc = false;
    for (uint32_t i = 0; i < totalNombres; ++i) {
        const char* name = cec_catalogo_nombre_por_indice(i);
        ASSERT_NE(name, nullptr);
        if (std::strcmp(name, "TiposDocumento") == 0) {
            foundTipoDoc = true;
        }
    }
    EXPECT_TRUE(foundTipoDoc);

    uint32_t totalElementos = cec_catalogo_total_elementos("TiposDocumento");
    EXPECT_GT(totalElementos, 0u);

    const char* cod = nullptr;
    const char* desc = nullptr;
    int32_t rc = cec_catalogo_elemento_por_indice("TiposDocumento", 0, &cod, &desc);
    EXPECT_EQ(rc, 1);
    ASSERT_NE(cod, nullptr);
    ASSERT_NE(desc, nullptr);

    // Consulta de catálogo inexistente
    EXPECT_EQ(cec_catalogo_total_elementos("no_existe"), 0u);
    EXPECT_EQ(cec_catalogo_elemento_por_indice("no_existe", 0, &cod, &desc), 0);
}
