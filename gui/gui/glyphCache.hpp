#pragma once

#include "hash_combine.hpp"
#include "glyphs.hpp"
#include <shared_mutex>

using FontName = std::string;

struct GlyphFaceHash {
    std::size_t operator()(const FontName& fontName) const;
};

struct GlyphCacheHash {
    std::size_t operator()(const std::pair<FontName, char>& fontKey) const;
};

struct GlyphQuery {
    char ch;
    FontName fontName;
    
    bool operator==(const GlyphQuery& other) const;
};

struct GlyphQueryHash {
    std::size_t operator()(const GlyphQuery& queryKey) const;
};


struct GlyphCache {
    GlyphCache();
    
    ~GlyphCache();
    
    const Glyph& retrieve(GlyphQuery glyphQuery);
    const Glyph& retrieve(const FontName& font, char ch);
    
    std::shared_mutex cacheMutex;
    FT_Library ft;
    std::unordered_map<FontName, FT_Face, GlyphFaceHash> fontFaces;
    std::unordered_map<GlyphQuery, Glyph, GlyphQueryHash> cache;
};
