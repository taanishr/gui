#pragma once

#include "freetype.hpp"
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

struct ShapedGlyph {
    uint32_t glyphId{};
    uint32_t byteOffset{};
    float xAdvance{};
    float yAdvance{};
    float xOffset{};
    float yOffset{};
};

struct ShapedCluster {
    size_t byteOffset{};
    size_t byteLength{};
    size_t glyphStart{};
    size_t glyphCount{};
    float advance{};
};

struct ShapedRun {
    std::vector<ShapedGlyph> glyphs;
    std::vector<ShapedCluster> clusters;
};

struct hb_font_t;

class TextShaper {
public:
    TextShaper();
    ~TextShaper();

    TextShaper(const TextShaper&) = delete;
    TextShaper& operator=(const TextShaper&) = delete;

    ShapedRun shape(std::string_view text, const std::string& font);

private:
    struct Font {
        FT_Face face{};
        hb_font_t* harfBuzzFont{};
    };

    Font& getFont(const std::string& font);

    FT_Library ft{};
    std::unordered_map<std::string, Font> fonts;
    std::mutex mutex;
};
