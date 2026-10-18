#include <gtest/gtest.h>
#include "cec/domain/Money.h"

using namespace cec::domain;

TEST(MoneyTest, ParsingAndStringConversion) {
    Money m1 = Money::fromString("100.50");
    EXPECT_EQ(m1.scaledValue(), 10050000);
    EXPECT_EQ(m1.toString(2), "100.50");
    EXPECT_EQ(m1.toString(5), "100.50000");

    Money m2 = Money::fromString("0.00001");
    EXPECT_EQ(m2.scaledValue(), 1);
    EXPECT_EQ(m2.toString(5), "0.00001");

    Money m3 = Money::fromString("1234567.89123");
    EXPECT_EQ(m3.scaledValue(), 123456789123LL);
    EXPECT_EQ(m3.toString(5), "1234567.89123");
}

TEST(MoneyTest, ArithmeticOperations) {
    Money a = Money::fromString("10.50000");
    Money b = Money::fromString("2.25000");

    Money sum = a + b;
    EXPECT_EQ(sum.toString(2), "12.75");

    Money diff = a - b;
    EXPECT_EQ(diff.toString(2), "8.25");

    // Multiplicación: 10.50 * 2.0 = 21.00
    Money prod = a.multiplyScaled(200000); // 2 unidades
    EXPECT_EQ(prod.toString(2), "21.00");

    // Redondeo Half-Up: 10.00 * 0.13 = 1.30
    Money iva = Money::fromString("10.00").multiplyScaled(13000); // 13% en base SCALE 100,000
    EXPECT_EQ(iva.toString(2), "1.30");
}

TEST(MoneyTest, OverflowDetection) {
    EXPECT_THROW(Money::fromString("999999999999999999999999999999"), std::exception);
}

// ── Aritmética de 128 bits sin __int128 (para que compile en MSVC) ───────────────────────────────

#include <cstdint>
#include <limits>
#include <random>
#include <vector>
#include "cec/domain/Aritmetica128.h"

namespace {
using namespace cec::domain::aritmetica128;
constexpr int64_t kMax = std::numeric_limits<int64_t>::max();
constexpr int64_t kMin = std::numeric_limits<int64_t>::min();

// Resultado de multiplicarDividirRedondeado como (ok, valor).
std::pair<bool, int64_t> redondeado(int64_t a, int64_t b, int64_t d) {
    int64_t r = 0;
    const bool ok = multiplicarDividirRedondeado(a, b, d, r);
    return {ok, ok ? r : 0};
}
std::pair<bool, int64_t> truncado(int64_t a, int64_t b, int64_t d) {
    int64_t r = 0;
    const bool ok = multiplicarDividirTruncado(a, b, d, r);
    return {ok, ok ? r : 0};
}
} // namespace

TEST(Aritmetica128, RedondeoHalfUpAlejadoDeCero) {
    EXPECT_EQ(redondeado(1, 1, 2), (std::pair<bool, int64_t>{true, 1}));    //  0,5 ->  1
    EXPECT_EQ(redondeado(-1, 1, 2), (std::pair<bool, int64_t>{true, -1}));  // -0,5 -> -1
    EXPECT_EQ(redondeado(1, 1, -2), (std::pair<bool, int64_t>{true, -1}));
    EXPECT_EQ(redondeado(-1, -1, 2), (std::pair<bool, int64_t>{true, 1}));
    EXPECT_EQ(redondeado(7, 1, 3), (std::pair<bool, int64_t>{true, 2}));    //  2,33 -> 2
    EXPECT_EQ(redondeado(8, 1, 3), (std::pair<bool, int64_t>{true, 3}));    //  2,67 -> 3
    EXPECT_EQ(redondeado(0, 123, 7), (std::pair<bool, int64_t>{true, 0}));
    EXPECT_EQ(truncado(8, 1, 3), (std::pair<bool, int64_t>{true, 2}));
    EXPECT_EQ(truncado(-8, 1, 3), (std::pair<bool, int64_t>{true, -2}));    // hacia cero
}

TEST(Aritmetica128, LimitesDeInt64) {
    EXPECT_EQ(redondeado(kMax, kMax, kMax), (std::pair<bool, int64_t>{true, kMax}));
    EXPECT_EQ(redondeado(kMin, 1, 1), (std::pair<bool, int64_t>{true, kMin}));
    EXPECT_EQ(redondeado(kMin, -1, 1).first, false);     // +2^63 no cabe
    EXPECT_EQ(redondeado(kMax, 2, 1).first, false);
    EXPECT_EQ(redondeado(kMin, kMin, kMin), (std::pair<bool, int64_t>{true, kMin}));   // (-2^63 * -2^63) / -2^63
    EXPECT_EQ(redondeado(1, 1, 0).first, false);        // división por cero
    EXPECT_EQ(truncado(1, 1, 0).first, false);
    int64_t r = 0;
    EXPECT_TRUE(sumaSegura(kMax - 1, 1, r));
    EXPECT_EQ(r, kMax);
    EXPECT_FALSE(sumaSegura(kMax, 1, r));
    EXPECT_FALSE(sumaSegura(kMin, -1, r));
    EXPECT_TRUE(restaSegura(kMin + 1, 1, r));
    EXPECT_EQ(r, kMin);
    EXPECT_FALSE(restaSegura(kMin, 1, r));
    EXPECT_FALSE(restaSegura(kMax, -1, r));
}

#ifdef __SIZEOF_INT128__
// Compara con __int128 (solo en GCC y Clang) usando exactamente las fórmulas que tenía Money antes.
TEST(Aritmetica128, CoincideConInt128ConMilesDeEntradas) {
    std::mt19937_64 rng(20261018);
    const std::vector<int64_t> especiales = {0, 1, -1, 2, -2, 99999, 100000, 100001, 10000000, 1300000, kMax, kMax - 1, kMin, kMin + 1,
                                              int64_t{1} << 32, -(int64_t{1} << 32), int64_t{1} << 62, 4294967295LL, 4294967296LL};
    auto aleatorio = [&]() -> int64_t {
        switch (rng() % 4) {
            case 0: return especiales[rng() % especiales.size()];
            case 1: return static_cast<int64_t>(rng());                                   // todo el rango
            case 2: return static_cast<int64_t>(rng() % 2000000) - 1000000;               // pequeño
            default: return static_cast<int64_t>(rng() >> (rng() % 64));                  // de cualquier magnitud
        }
    };
    for (int i = 0; i < 300000; ++i) {
        const int64_t a = aleatorio(), b = aleatorio();
        int64_t d = aleatorio();
        if (d == 0) d = 1;

        // Redondeado: (prod >= 0) ? (prod + half) / d : (prod - half) / d, half = |d| / 2
        const __int128_t prod = static_cast<__int128_t>(a) * b;
        const __int128_t absD = d < 0 ? -static_cast<__int128_t>(d) : static_cast<__int128_t>(d);
        const __int128_t half = absD / 2;
        const __int128_t esperado = (prod >= 0) ? (prod + half) / d : (prod - half) / d;
        const bool cabe = esperado <= kMax && esperado >= kMin;
        const auto obtenido = redondeado(a, b, d);
        ASSERT_EQ(obtenido.first, cabe) << a << " * " << b << " / " << d;
        if (cabe) ASSERT_EQ(obtenido.second, static_cast<int64_t>(esperado)) << a << " * " << b << " / " << d;

        // Truncado: prod / d
        const __int128_t t = prod / d;
        const bool cabeT = t <= kMax && t >= kMin;
        const auto obtenidoT = truncado(a, b, d);
        ASSERT_EQ(obtenidoT.first, cabeT) << a << " * " << b << " / " << d;
        if (cabeT) ASSERT_EQ(obtenidoT.second, static_cast<int64_t>(t)) << a << " * " << b << " / " << d;

        // Suma y resta
        int64_t r = 0;
        const __int128_t s = static_cast<__int128_t>(a) + b, m = static_cast<__int128_t>(a) - b;
        ASSERT_EQ(sumaSegura(a, b, r), s <= kMax && s >= kMin);
        ASSERT_EQ(restaSegura(a, b, r), m <= kMax && m >= kMin);
    }
}
#endif

TEST(MoneyTest, MultiplicacionesYDivisionesConRedondeo) {
    // 10,00 * 13 % = 1,30 ; 0,00005 * 1 = 0,00001 (la mitad se redondea hacia arriba)
    EXPECT_EQ(Money::fromString("10").multiplyPercent(13 * Money::SCALE).toString(2), "1.30");
    EXPECT_EQ(Money::fromScaled(5).multiplyScaled(Money::SCALE / 10).scaledValue(), 1);
    EXPECT_EQ(Money::fromScaled(-5).multiplyScaled(Money::SCALE / 10).scaledValue(), -1);
    // 1,00 / 3,00 = 0,33333
    EXPECT_EQ(Money::fromString("1").divideScaled(3 * Money::SCALE).toString(5), "0.33333");
    // Divisor negativo
    EXPECT_EQ(Money::fromString("1").divideScaled(-2 * Money::SCALE).toString(5), "-0.50000");
    EXPECT_THROW(Money::fromScaled(1).divideScaled(0), std::invalid_argument);
    EXPECT_THROW(Money::fromScaled(std::numeric_limits<int64_t>::max()).multiplyScaled(2 * Money::SCALE), std::overflow_error);
    EXPECT_THROW(Money::fromScaled(std::numeric_limits<int64_t>::max()) + Money::fromScaled(1), std::overflow_error);
    EXPECT_THROW(Money::fromScaled(std::numeric_limits<int64_t>::min()) - Money::fromScaled(1), std::overflow_error);
}

TEST(MoneyTest, LimiteDeFromString) {
    // 92233720368547,75807 es el mayor valor representable (INT64_MAX / 100000)
    EXPECT_EQ(Money::fromString("92233720368547.75807").scaledValue(), std::numeric_limits<int64_t>::max());
    EXPECT_THROW(Money::fromString("92233720368547.75808"), std::overflow_error);
    EXPECT_THROW(Money::fromString("92233720368548"), std::overflow_error);
    EXPECT_EQ(Money::fromString("-92233720368547.75807").scaledValue(), -std::numeric_limits<int64_t>::max());
}
