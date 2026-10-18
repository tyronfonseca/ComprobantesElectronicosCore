#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace cec::crypto {

class Base64 {
public:
    static std::string encode(const uint8_t* data, size_t len) {
        static const char tbl[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string out;
        out.reserve(((len + 2) / 3) * 4);
        for (size_t i = 0; i < len; i += 3) {
            uint32_t v = (static_cast<uint32_t>(data[i]) << 16);
            if (i + 1 < len) v |= (static_cast<uint32_t>(data[i + 1]) << 8);
            if (i + 2 < len) v |= static_cast<uint32_t>(data[i + 2]);

            out.push_back(tbl[(v >> 18) & 0x3F]);
            out.push_back(tbl[(v >> 12) & 0x3F]);
            out.push_back((i + 1 < len) ? tbl[(v >> 6) & 0x3F] : '=');
            out.push_back((i + 2 < len) ? tbl[v & 0x3F] : '=');
        }
        return out;
    }

    static std::string encode(const std::vector<uint8_t>& data) {
        return encode(data.data(), data.size());
    }

    static std::string encode(const std::string& str) {
        return encode(reinterpret_cast<const uint8_t*>(str.data()), str.size());
    }

    static std::vector<uint8_t> decode(const std::string& in) {
        std::vector<uint8_t> out;
        std::vector<int> T(256, -1);
        for (int i = 0; i < 64; i++) {
            T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i;
        }

        int val = 0, valb = -8;
        for (unsigned char c : in) {
            if (c == '=') break;
            if (c == ' ' || c == '\n' || c == '\r' || c == '\t') continue;
            if (T[c] == -1) continue;
            val = (val << 6) + T[c];
            valb += 6;
            if (valb >= 0) {
                out.push_back(static_cast<uint8_t>((val >> valb) & 0xFF));
                valb -= 8;
            }
        }
        return out;
    }
};

} // namespace cec::crypto

