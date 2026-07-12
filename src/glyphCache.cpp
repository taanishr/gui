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

bool GlyphQuery::operator==(const GlyphQuery& other) const {
    return glyphId == other.glyphId
        && fontName == other.fontName;
}



std::size_t GlyphQueryHash::operator()(const GlyphQuery& queryKey) const
{
    std::size_t hv = 0;
    
    hash_combine(hv, queryKey.glyphId);
    hash_combine(hv, queryKey.fontName);
    
    return hv;
}


GlyphCache::GlyphCache()
{
    FT_Init_FreeType(&(this->ft));

}


GlyphCache::~GlyphCache() {
    for (auto& pair : fontFaces)
        FT_Done_Face(pair.second);

    FT_Done_FreeType(this->ft);
}

const Glyph& GlyphCache::retrieve(GlyphQuery glyphQuery) {
    return GlyphCache::retrieve(glyphQuery.fontName, glyphQuery.glyphId);
}
                                  
FT_Face GlyphCache::getFace(const FontName& font)
{
    if (auto found = fontFaces.find(font); found != fontFaces.end()) {
        return found->second;
    }

    FT_Face face = nullptr;
    FT_New_Face(ft, font.c_str(), 0, &face);
    FT_Set_Pixel_Sizes(face, 0, BASE_PIXEL_HEIGHT);
    fontFaces[font] = face;
    return face;
}

float GlyphCache::lineHeight(const FontName& font) {
    std::unique_lock<std::shared_mutex> lock(cacheMutex);
    return getFace(font)->size->metrics.height;
}

const Glyph& GlyphCache::retrieve(const FontName& font, uint32_t glyphId)
{
    GlyphQuery query{glyphId, font};

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
        
        auto fontFace = getFace(font);
        FT_Load_Glyph(fontFace, glyphId, FT_LOAD_DEFAULT);
        
        if (fontFace->glyph->format == FT_GLYPH_FORMAT_OUTLINE &&
            fontFace->glyph->outline.n_contours > 0) {
            auto glyph = processContours(fontFace);
            glyph.metrics = fontFace->glyph->metrics;
            glyph.lineHeight = fontFace->size->metrics.height;
            cache[query] = glyph;
        } else {
            Glyph glyph;
            glyph.numContours = 0;
            glyph.contourSizes = {};
            glyph.metrics = fontFace->glyph->metrics;
            glyph.lineHeight = fontFace->size->metrics.height;
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
