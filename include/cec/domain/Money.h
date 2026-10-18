#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <stdexcept>
#include <limits>
#include <cmath>
#include <cstdlib>
#include "cec/domain/Aritmetica128.h"

namespace cec::domain {

/**
 * @brief Representación monetaria de punto fijo puro para cálculos fiscales.
 * 
 * Reglas:
 * - Determinista y sin punto flotante IEEE (sin double).
 * - Escala interna fija de 5 decimales (1 unidad monetaria = 100,000 unidades internas).
 * - Detección explícita de overflow con productos intermedios de 128 bits (Aritmetica128.h, sin __int128: compila en MSVC).
 * - Sin std::locale, formato numérico determinista e inmutable.
 */
class Money {
public:
    static constexpr int64_t SCALE = 100000;      // 5 decimales
    static constexpr int DECIMAL_PLACES = 5;

    constexpr Money() noexcept : _micros(0) {}

    static constexpr Money fromScaled(int64_t scaledValue) noexcept {
        Money m;
        m._micros = scaledValue;
        return m;
    }

    static Money fromString(std::string_view str) {
        if (str.empty()) {
            throw std::invalid_argument("Cadena monetaria vacía");
        }

        size_t idx = 0;
        bool negative = false;
        if (str[idx] == '-') {
            negative = true;
            idx++;
        } else if (str[idx] == '+') {
            idx++;
        }

        if (idx >= str.size()) {
            throw std::invalid_argument("Cadena monetaria sin dígitos");
        }

        int64_t integerPart = 0;
        int64_t fractionPart = 0;
        bool hasDecimal = false;
        int fractionDigits = 0;

        for (; idx < str.size(); ++idx) {
            char c = str[idx];
            if (c >= '0' && c <= '9') {
                if (!hasDecimal) {
                    if (integerPart > (std::numeric_limits<int64_t>::max() - (c - '0')) / 10) {
                        throw std::overflow_error("Overflow en parte entera de Money");
                    }
                    integerPart = integerPart * 10 + (c - '0');
                } else {
                    if (fractionDigits < DECIMAL_PLACES) {
                        fractionPart = fractionPart * 10 + (c - '0');
                        fractionDigits++;
                    }
                    // Cualquier dígito más allá del 5to decimal se trunca o redondeará
                }
            } else if (c == '.') {
                if (hasDecimal) {
                    throw std::invalid_argument("Múltiples puntos decimales en cadena monetaria");
                }
                hasDecimal = true;
            } else {
                throw std::invalid_argument("Carácter inválido en cadena monetaria");
            }
        }

        // Rellenar decimales si vinieron menos de 5
        while (fractionDigits < DECIMAL_PLACES) {
            fractionPart *= 10;
            fractionDigits++;
        }

        // integerPart * SCALE + fractionPart debe caber en int64.
        if (integerPart > (std::numeric_limits<int64_t>::max() - fractionPart) / SCALE) {
            throw std::overflow_error("Overflow en conversión a Money");
        }

        int64_t val = integerPart * SCALE + fractionPart;
        if (negative) val = -val;

        return fromScaled(val);
    }

    constexpr int64_t scaledValue() const noexcept { return _micros; }

    constexpr bool isZero() const noexcept { return _micros == 0; }
    constexpr bool isPositive() const noexcept { return _micros > 0; }
    constexpr bool isNegative() const noexcept { return _micros < 0; }

    std::string toString(int decimals = DECIMAL_PLACES) const {
        if (decimals < 0 || decimals > DECIMAL_PLACES) decimals = DECIMAL_PLACES;
        
        int64_t val = _micros;
        bool neg = val < 0;
        if (neg) val = -val;

        int64_t intPart = val / SCALE;
        int64_t fracPart = val % SCALE;

        // Redondear a 'decimals' si se solicita menos de 5
        if (decimals < DECIMAL_PLACES) {
            int div = 1;
            for (int i = 0; i < (DECIMAL_PLACES - decimals); ++i) div *= 10;
            int64_t rem = fracPart % div;
            fracPart /= div;
            if (rem >= (div / 2)) {
                fracPart++;
                if (fracPart >= std::pow(10, decimals)) {
                    fracPart = 0;
                    intPart++;
                }
            }
        }

        std::string s;
        if (neg && (intPart > 0 || fracPart > 0)) s += "-";
        s += std::to_string(intPart);
        if (decimals > 0) {
            s += ".";
            std::string fracStr = std::to_string(fracPart);
            while (fracStr.length() < static_cast<size_t>(decimals)) {
                fracStr = "0" + fracStr;
            }
            s += fracStr;
        }
        return s;
    }

    constexpr Money operator+(const Money& other) const {
        int64_t res = 0;
        if (!aritmetica128::sumaSegura(_micros, other._micros, res)) {
            throw std::overflow_error("Overflow en suma de Money");
        }
        return fromScaled(res);
    }

    constexpr Money& operator+=(const Money& other) {
        *this = *this + other;
        return *this;
    }

    constexpr Money operator-(const Money& other) const {
        int64_t res = 0;
        if (!aritmetica128::restaSegura(_micros, other._micros, res)) {
            throw std::overflow_error("Overflow en resta de Money");
        }
        return fromScaled(res);
    }

    constexpr Money& operator-=(const Money& other) {
        *this = *this - other;
        return *this;
    }

    /**
     * @brief Multiplica por un valor escalado (ej: Cantidad o Tasa con SCALE = 100000)
     * y aplica redondeo Half-Up estándar de Hacienda.
     */
    constexpr Money multiplyScaled(int64_t otherScaled) const {
        // Redondeo Half-Up: se suma la mitad del divisor en la dirección del signo antes de dividir.
        int64_t res = 0;
        if (!aritmetica128::multiplicarDividirRedondeado(_micros, otherScaled, SCALE, res)) {
            throw std::overflow_error("Overflow en multiplicación monetaria");
        }
        return fromScaled(res);
    }

    /**
     * @brief Multiplica por un porcentaje escalado (ej: 13% = 1300000 con SCALE = 100000)
     * calculando (micros * percentScaled) / (SCALE * 100) con redondeo Half-Up.
     */
    constexpr Money multiplyPercent(int64_t percentScaled) const {
        int64_t res = 0;
        if (!aritmetica128::multiplicarDividirRedondeado(_micros, percentScaled, SCALE * 100, res)) {
            throw std::overflow_error("Overflow en multiplicación porcentual de Money");
        }
        return fromScaled(res);
    }

    /**
     * @brief Divide por un divisor escalado con redondeo Half-Up.
     */
    constexpr Money divideScaled(int64_t divisorScaled) const {
        if (divisorScaled == 0) {
            throw std::invalid_argument("División por cero en Money");
        }
        int64_t res = 0;
        if (!aritmetica128::multiplicarDividirRedondeado(_micros, SCALE, divisorScaled, res)) {
            throw std::overflow_error("Overflow en división monetaria");
        }
        return fromScaled(res);
    }

    constexpr bool operator==(const Money& o) const noexcept { return _micros == o._micros; }
    constexpr bool operator!=(const Money& o) const noexcept { return _micros != o._micros; }
    constexpr bool operator<(const Money& o) const noexcept { return _micros < o._micros; }
    constexpr bool operator<=(const Money& o) const noexcept { return _micros <= o._micros; }
    constexpr bool operator>(const Money& o) const noexcept { return _micros > o._micros; }
    constexpr bool operator>=(const Money& o) const noexcept { return _micros >= o._micros; }

private:
    int64_t _micros; // Valor escalado x 100,000
};

} // namespace cec::domain
