#pragma once

#include "hash_combine.hpp"
#include "glyphs.hpp"
#include <shared_mutex>

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
    GlyphCache();
    
    ~GlyphCache();
    
    const Glyph& retrieve(GlyphQuery glyphQuery);
    const Glyph& retrieve(const FontName& font, FontSize fontSize, char ch);
    
    std::shared_mutex cacheMutex;
    FT_Library ft;
    std::unordered_map<std::pair<FontName, FontSize>, FT_Face, GlyphFaceHash> fontFaces;
    std::unordered_map<GlyphQuery, Glyph, GlyphQueryHash> cache;
};
