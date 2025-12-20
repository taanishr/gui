#pragma once

#include "hash_combine.hpp"
#include "glyphs.hpp"

using FontSize = float;
using FontName = std::string;

struct GlyphFaceHash {
    std::size_t operator()(const std::pair<FontName, FontSize>& cacheKey) const;
};

struct GlyphCacheHash {
    std::size_t operator()(const std::tuple<FontName, FontSize, char>& fontKey) const;
};

struct GlyphQuery {
    char ch;
    FontName fontName;
    FontSize fontSize;
    
    bool operator==(const GlyphQuery& other) const;
};

struct GlyphQueryHash {
    std::size_t operator()(const GlyphQuery& queryKey) const;
};


struct GlyphCache {
    GlyphCache(FT_Library ft);
    
    ~GlyphCache();
    
    const Glyph& retrieve(GlyphQuery glyphQuery);
    const Glyph& retrieve(const FontName& font, FontSize fontSize, char ch);
    
    FT_Library ft;
    std::unordered_map<std::pair<FontName, FontSize>, FT_Face, GlyphFaceHash> fontFaces;
    std::unordered_map<std::tuple<FontName, FontSize, char>, Glyph, GlyphCacheHash> cache;
};
