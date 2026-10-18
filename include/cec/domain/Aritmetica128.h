#pragma once
/**
 * @file Aritmetica128.h
 * @brief Multiplicación y división con producto intermedio de 128 bits, en C++ estándar.
 *
 * Money necesita multiplicar dos enteros de 64 bits y dividir el resultado sin desbordar. GCC y Clang tienen
 * __int128, pero MSVC no, así que esto se hace con dos palabras de 64 bits y funciona igual en todos los
 * compiladores. Es deliberadamente simple (división bit a bit): los cálculos fiscales no están en una ruta crítica
 * y la claridad vale más que la velocidad. test/unit/TestMoney.cpp lo compara con __int128 donde existe.
 */
#include <cstdint>
#include <limits>

namespace cec::domain::aritmetica128 {

/// Entero sin signo de 128 bits.
struct U128 {
    uint64_t hi = 0;
    uint64_t lo = 0;
};

/// Valor absoluto como entero sin signo (correcto también para INT64_MIN).
constexpr uint64_t magnitud(int64_t v) noexcept {
    return v < 0 ? uint64_t{0} - static_cast<uint64_t>(v) : static_cast<uint64_t>(v);
}

/// a * b completo, con productos parciales de 32 bits.
constexpr U128 multiplicar(uint64_t a, uint64_t b) noexcept {
    constexpr uint64_t kMascara = 0xFFFFFFFFu;
    const uint64_t a0 = a & kMascara, a1 = a >> 32;
    const uint64_t b0 = b & kMascara, b1 = b >> 32;
    const uint64_t p00 = a0 * b0, p01 = a0 * b1, p10 = a1 * b0, p11 = a1 * b1;
    const uint64_t medio = (p00 >> 32) + (p01 & kMascara) + (p10 & kMascara);
    U128 r;
    r.lo = (p00 & kMascara) | (medio << 32);
    r.hi = p11 + (p01 >> 32) + (p10 >> 32) + (medio >> 32);
    return r;
}

/// n + x, con acarreo hacia la palabra alta.
constexpr U128 sumar(U128 n, uint64_t x) noexcept {
    U128 r{n.hi, n.lo + x};
    if (r.lo < n.lo) ++r.hi;
    return r;
}

/// Cociente entero de n / d (d distinto de cero), división larga bit a bit.
constexpr U128 dividir(U128 n, uint64_t d) noexcept {
    U128 q;
    uint64_t resto = 0;
    for (int i = 127; i >= 0; --i) {
        const uint64_t bit = (i >= 64 ? n.hi >> (i - 64) : n.lo >> i) & 1u;
        const bool acarreo = (resto >> 63) != 0;   // resto * 2 puede pasar de 64 bits; entonces es mayor que d
        resto = (resto << 1) | bit;
        if (acarreo || resto >= d) {
            resto -= d;
            if (i >= 64) q.hi |= uint64_t{1} << (i - 64);
            else q.lo |= uint64_t{1} << i;
        }
    }
    return q;
}

namespace detalle {

/// El producto a*b dividido entre d es negativo cuando los signos difieren (un producto cero nunca es negativo).
constexpr bool resultadoNegativo(int64_t a, int64_t b, int64_t d) noexcept {
    const bool productoNegativo = a != 0 && b != 0 && ((a < 0) != (b < 0));
    return productoNegativo != (d < 0);
}

/// Pasa la magnitud a int64 con el signo indicado; false si no cabe (INT64_MIN sí cabe).
constexpr bool aInt64(U128 magnitudCociente, bool negativo, int64_t& salida) noexcept {
    constexpr uint64_t kMax = static_cast<uint64_t>(std::numeric_limits<int64_t>::max());
    if (magnitudCociente.hi != 0) return false;
    if (!negativo) {
        if (magnitudCociente.lo > kMax) return false;
        salida = static_cast<int64_t>(magnitudCociente.lo);
        return true;
    }
    if (magnitudCociente.lo > kMax + 1) return false;
    salida = magnitudCociente.lo == kMax + 1 ? std::numeric_limits<int64_t>::min()
                                              : -static_cast<int64_t>(magnitudCociente.lo);
    return true;
}

} // namespace detalle

/**
 * @brief a * b / d con redondeo half-up (la mitad se aleja de cero), igual que la división de enteros más la mitad
 * del divisor en la dirección del signo. Devuelve false si d es cero o el resultado no cabe en int64.
 */
constexpr bool multiplicarDividirRedondeado(int64_t a, int64_t b, int64_t d, int64_t& salida) noexcept {
    if (d == 0) return false;
    const uint64_t divisor = magnitud(d);
    const U128 cociente = dividir(sumar(multiplicar(magnitud(a), magnitud(b)), divisor / 2), divisor);
    return detalle::aInt64(cociente, detalle::resultadoNegativo(a, b, d), salida);
}

/// a * b / d truncando hacia cero, como la división de enteros. Devuelve false si d es cero o no cabe en int64.
constexpr bool multiplicarDividirTruncado(int64_t a, int64_t b, int64_t d, int64_t& salida) noexcept {
    if (d == 0) return false;
    const U128 cociente = dividir(multiplicar(magnitud(a), magnitud(b)), magnitud(d));
    return detalle::aInt64(cociente, detalle::resultadoNegativo(a, b, d), salida);
}

/// a + b; false si desborda int64.
constexpr bool sumaSegura(int64_t a, int64_t b, int64_t& salida) noexcept {
    if ((b > 0 && a > std::numeric_limits<int64_t>::max() - b) ||
        (b < 0 && a < std::numeric_limits<int64_t>::min() - b)) return false;
    salida = a + b;
    return true;
}

/// a - b; false si desborda int64.
constexpr bool restaSegura(int64_t a, int64_t b, int64_t& salida) noexcept {
    if ((b < 0 && a > std::numeric_limits<int64_t>::max() + b) ||
        (b > 0 && a < std::numeric_limits<int64_t>::min() + b)) return false;
    salida = a - b;
    return true;
}

} // namespace cec::domain::aritmetica128
