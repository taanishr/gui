#pragma once

#include "hash_combine.hpp"
#include "process_contours.hpp"

using FontSize = int;
using FontName = std::string;

struct GlyphFaceHash {
    std::size_t operator()(const std::pair<FontName, FontSize>& cacheKey) const;
};

struct GlyphCacheHash {
    std::size_t operator()(const std::tuple<FontName, FontSize, char>& fontKey) const;
};


struct GlyphCache {
    GlyphCache(FT_Library ft);
    
    ~GlyphCache();
    
    const Glyph& retrieve(const FontName& font, FontSize fontSize, char ch);
    
    FT_Library ft;
    std::unordered_map<std::pair<FontName, FontSize>, FT_Face, GlyphFaceHash> fontFaces;
    std::unordered_map<std::tuple<FontName, FontSize, char>, Glyph, GlyphCacheHash> cache;
};
