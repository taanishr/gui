//
//  glyphCache.cpp
//  gui
//
//  Created by Taanish Reja on 9/12/25.
//

#include "glyphCache.hpp"

std::size_t GlyphFaceHash::operator()(const std::pair<FontName, FontSize>& cacheKey) const
{
    std::size_t hv = 0;
    
    hash_combine(hv, cacheKey.first);
    hash_combine(hv, cacheKey.second);
    
    return hv;
}

std::size_t GlyphCacheHash::operator()(const std::tuple<FontName, FontSize, char>& fontKey) const
{
    std::size_t hv = 0;
    
    hash_combine(hv, std::get<0>(fontKey));
    hash_combine(hv, std::get<1>(fontKey));
    hash_combine(hv, std::get<2>(fontKey));
    
    return hv;
}

bool GlyphQuery::operator==(const GlyphQuery& other) const {
    return ch == other.ch
        && fontName == other.fontName
        && fontSize == other.fontSize;
}

std::size_t GlyphQueryHash::operator()(const GlyphQuery& queryKey) const
{
    std::size_t hv = 0;
    
    hash_combine(hv, queryKey.ch);
    hash_combine(hv, queryKey.fontName);
    hash_combine(hv, queryKey.fontSize);
    
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
    return GlyphCache::retrieve(glyphQuery.fontName, glyphQuery.fontSize, glyphQuery.ch);
}
                                  
const Glyph& GlyphCache::retrieve(const FontName& font, FontSize fontSize, char ch)
{
    GlyphQuery query{ch, font, fontSize};

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
        
        bool faceCached = fontFaces.find({font, fontSize}) != fontFaces.end();
        
        if (!faceCached) {
            FT_Face newFace = nullptr;
            FT_New_Face(this->ft, font.c_str(), 0, &newFace);
            FT_Set_Pixel_Sizes(newFace, 0, fontSize);
            
            fontFaces[{font, fontSize}] = newFace;
        }
        
        auto fontFace = fontFaces[{font, fontSize}];

        FT_Load_Char(fontFace, ch, FT_LOAD_NO_BITMAP | FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT);
        
        if (ch != ' ') {
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


