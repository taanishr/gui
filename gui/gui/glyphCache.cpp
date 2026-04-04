//
//  glyphCache.cpp
//  gui
//
//  Created by Taanish Reja on 9/12/25.
//

#include "glyphCache.hpp"
#include <print>

std::size_t GlyphFaceHash::operator()(const FontName& fontName) const
{
    std::size_t hv = 0;
    
    hash_combine(hv, fontName);
    
    return hv;
}

std::size_t GlyphCacheHash::operator()(const std::pair<FontName, uint32_t>& fontKey) const
{
    std::size_t hv = 0;
    
    hash_combine(hv, std::get<0>(fontKey));
    hash_combine(hv, std::get<1>(fontKey));
    
    return hv;
}

bool GlyphQuery::operator==(const GlyphQuery& other) const {
    return codepoint == other.codepoint
        && fontName == other.fontName;
}

std::size_t GlyphQueryHash::operator()(const GlyphQuery& queryKey) const
{
    std::size_t hv = 0;
    
    hash_combine(hv, queryKey.codepoint);
    hash_combine(hv, queryKey.fontName);
    
    return hv;
}


GlyphCache::GlyphCache()
{
    FT_Init_FreeType(&(this->ft));

    const std::string& fallbackFont = "/System/Library/Fonts/Supplemental/Arial Unicode.ttf";
    FT_Face fallbackFace = nullptr;
    FT_New_Face(this->ft, fallbackFont.c_str(), 0, &fallbackFace);
    FT_Set_Pixel_Sizes(fallbackFace, 0, BASE_PIXEL_HEIGHT);

    fontFaces["fallback"] = fallbackFace;
}


GlyphCache::~GlyphCache() {
    for (auto& pair : fontFaces)
        FT_Done_Face(pair.second);

    FT_Done_FreeType(this->ft);
}

const Glyph& GlyphCache::retrieve(GlyphQuery glyphQuery) {
    return GlyphCache::retrieve(glyphQuery.fontName, glyphQuery.codepoint);
}
                                  
const Glyph& GlyphCache::retrieve(const FontName& font, uint32_t codepoint)
{
    GlyphQuery query{codepoint, font};

    {
        std::shared_lock<std::shared_mutex> readLock(cacheMutex);
        auto it = cache.find(query);
        if (it != cache.end()) {
            return it->second;
        }
    }
    
    {
        std::unique_lock<std::shared_mutex> writeLock(cacheMutex);

        auto it = cache.find(query);
        if (it != cache.end()) {
            return it->second;
        }
        
        bool faceCached = fontFaces.find({font}) != fontFaces.end();
        
        if (!faceCached) {
            FT_Face newFace = nullptr;
            FT_New_Face(this->ft, font.c_str(), 0, &newFace);
            FT_Set_Pixel_Sizes(newFace, 0, BASE_PIXEL_HEIGHT);
            
            fontFaces[font] = newFace;
        }
        
        auto fontFace = fontFaces[font];

        FT_UInt glyphIndex = FT_Get_Char_Index(fontFace, codepoint);
        
        if (glyphIndex != 0) {
            FT_Load_Char(fontFace, codepoint, FT_LOAD_DEFAULT);
        }else {
            fontFace = fontFaces["fallback"];
            FT_Load_Char(fontFace, codepoint, FT_LOAD_DEFAULT);
        }
        
        if (codepoint != ' ') {
            auto glyph = processContours(fontFace);
            glyph.metrics = fontFace->glyph->metrics;
            cache[query] = glyph;
        }else {
            Glyph glyph;
            glyph.numContours = 0;
            glyph.contourSizes = {};
            glyph.metrics = fontFace->glyph->metrics;
            glyph.points = {};
            glyph.quad = {
                .topLeft = {0,0},
                .bottomRight = simd_float2{(float)fontFace->glyph->metrics.horiAdvance, (float)fontFace->glyph->metrics.height},
            };
            cache[query] = glyph;
        }  

        return cache[query];
    }
}


