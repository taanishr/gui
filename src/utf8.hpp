#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

namespace utf8 {
    struct CodePoint {
        char32_t value{};
        std::size_t byteOffset{};
        std::size_t byteLength{};
    };

    inline CodePoint at(std::string_view text, std::size_t byteOffset) {
        const auto lead = static_cast<std::uint8_t>(text[byteOffset]);
        std::size_t length = 1;
        char32_t value = lead;

        if ((lead & 0xE0) == 0xC0) {
            length = 2;
            value = lead & 0x1F;
        } else if ((lead & 0xF0) == 0xE0) {
            length = 3;
            value = lead & 0x0F;
        } else if ((lead & 0xF8) == 0xF0) {
            length = 4;
            value = lead & 0x07;
        }

        for (std::size_t i = 1; i < length; ++i) {
            value = (value << 6) |
                (static_cast<std::uint8_t>(text[byteOffset + i]) & 0x3F);
        }
        return {value, byteOffset, length};
    }

    inline std::vector<CodePoint> codePoints(std::string_view text) {
        std::vector<CodePoint> result;
        result.reserve(text.size());
        for (std::size_t offset = 0; offset < text.size();) {
            auto codepoint = at(text, offset);
            result.push_back(codepoint);
            offset += codepoint.byteLength;
        }
        return result;
    }
}
