#include "textShaper.hpp"
#include "glyphs.hpp"
#include "utf8.hpp"
#include <hb-ft.h>
#include <hb.h>
#include <algorithm>
#include <memory>

char32_t ShapedCluster::codepoint() const {
    return utf8::at(text, 0).value;
}

TextShaper::TextShaper() {
    FT_Init_FreeType(&ft);
}

TextShaper::~TextShaper() {
    for (auto& [_, font] : fonts) {
        hb_font_destroy(font.harfBuzzFont);
        FT_Done_Face(font.face);
    }
    FT_Done_FreeType(ft);
}

TextShaper::Font& TextShaper::getFont(const std::string& fontName) {
    if (auto found = fonts.find(fontName); found != fonts.end()) {
        return found->second;
    }

    FT_Face face = nullptr;
    FT_New_Face(ft, fontName.c_str(), 0, &face);
    FT_Set_Pixel_Sizes(face, 0, BASE_PIXEL_HEIGHT);

    auto harfBuzzFont = hb_ft_font_create_referenced(face);
    auto [inserted, _] = fonts.emplace(fontName, Font{
        .face = face,
        .harfBuzzFont = harfBuzzFont
    });
    return inserted->second;
}

ShapedRun TextShaper::shape(std::string_view text, const std::string& fontName) {
    std::lock_guard lock(mutex);
    auto& font = getFont(fontName);

    using HarfBuzzBuffer = std::unique_ptr<hb_buffer_t, decltype(&hb_buffer_destroy)>;
    HarfBuzzBuffer buffer{hb_buffer_create(), hb_buffer_destroy};
    hb_buffer_add_utf8(
        buffer.get(),
        text.data(),
        static_cast<int>(text.size()),
        0,
        static_cast<int>(text.size())
    );
    hb_buffer_guess_segment_properties(buffer.get());
    hb_shape(font.harfBuzzFont, buffer.get(), nullptr, 0);

    unsigned int glyphCount = 0;
    const auto* infos = hb_buffer_get_glyph_infos(buffer.get(), &glyphCount);
    const auto* positions = hb_buffer_get_glyph_positions(buffer.get(), &glyphCount);

    ShapedRun result;
    result.glyphs.reserve(glyphCount);
    for (unsigned int i = 0; i < glyphCount; ++i) {
        result.glyphs.push_back({
            .glyphId = infos[i].codepoint,
            .byteOffset = infos[i].cluster,
            .xAdvance = static_cast<float>(positions[i].x_advance),
            .yAdvance = static_cast<float>(positions[i].y_advance),
            .xOffset = static_cast<float>(positions[i].x_offset),
            .yOffset = static_cast<float>(positions[i].y_offset)
        });
    }

    size_t glyphStart = 0;
    while (glyphStart < result.glyphs.size()) {
        const auto byteOffset = result.glyphs[glyphStart].byteOffset;
        size_t glyphEnd = glyphStart + 1;
        float advance = result.glyphs[glyphStart].xAdvance;
        while (glyphEnd < result.glyphs.size() &&
               result.glyphs[glyphEnd].byteOffset == byteOffset) {
            advance += result.glyphs[glyphEnd].xAdvance;
            ++glyphEnd;
        }

        size_t nextByteOffset = text.size();
        for (const auto& glyph : result.glyphs) {
            if (glyph.byteOffset > byteOffset) {
                nextByteOffset = std::min(nextByteOffset, size_t(glyph.byteOffset));
            }
        }
        const auto byteLength = nextByteOffset - byteOffset;
        result.clusters.push_back({
            .byteOffset = byteOffset,
            .byteLength = byteLength,
            .text = std::string{text.substr(byteOffset, byteLength)},
            .glyphStart = glyphStart,
            .glyphCount = glyphEnd - glyphStart,
            .advance = advance
        });
        glyphStart = glyphEnd;
    }

    return result;
}
