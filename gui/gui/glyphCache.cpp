//
//  glyphCache.cpp
//  gui
//
//  Created by Taanish Reja on 9/12/25.
//

#include "glyphCache.hpp"

std::size_t GlyphFaceHash::operator()(const FontName& fontName) const
{
    std::size_t hv = 0;
    
    hash_combine(hv, fontName);
    
    return hv;
}

std::size_t GlyphCacheHash::operator()(const std::pair<FontName, char>& fontKey) const
{
    std::size_t hv = 0;
    
    hash_combine(hv, std::get<0>(fontKey));
    hash_combine(hv, std::get<1>(fontKey));
    
    return hv;
}

bool GlyphQuery::operator==(const GlyphQuery& other) const {
    return ch == other.ch
        && fontName == other.fontName;
}

std::size_t GlyphQueryHash::operator()(const GlyphQuery& queryKey) const
{
    std::size_t hv = 0;
    
    hash_combine(hv, queryKey.ch);
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
    return GlyphCache::retrieve(glyphQuery.fontName, glyphQuery.ch);
}
                                  
const Glyph& GlyphCache::retrieve(const FontName& font, char ch)
{
    GlyphQuery query{ch, font};

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
            
            fontFaces[{font}] = newFace;
        }
        
        auto fontFace = fontFaces[{font}];

        FT_Load_Char(fontFace, ch, FT_LOAD_DEFAULT);
        
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


