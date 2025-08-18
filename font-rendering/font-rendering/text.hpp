//
//  text.hpp
//  font-rendering
//
//  Created by Taanish Reja on 8/15/25.
//

#ifndef TEXT_HPP
#define TEXT_HPP

#include "metal_imports.hpp"
#include "bezier.hpp"
#include "freetype.hpp"
#include "renderFace.hpp"

constexpr std::string_view defaultFont {"/System/Library/Fonts/Supplemental/Arial.ttf"};

struct Quad {
    simd_float2 position;
    simd_float2 offset;
    unsigned int firstContour;
    unsigned int lastContour;
};

struct ContourMeta {
    unsigned long start;
    unsigned long end;
};


struct TextUniforms {
    simd_float3 color;
};

struct Text {
    Text(MTL::Device* device, FT_Library ft, float height=64.0, simd_float3 color={0,0,0}, const std::string& font = defaultFont.data());
    ~Text();
    
    void setText(const std::string& text);
    void update();
    void resizeBuffer(MTL::Buffer** buffer, unsigned long numBytes);
    
    MTL::Device* device;
    
    // buffers
    MTL::Buffer* uniformsBuffer;
    MTL::Buffer* glyphMetaBuffer;
    MTL::Buffer* quadBuffer;
    MTL::Buffer* contoursBuffer;
    MTL::Buffer* contoursMetaBuffer;
    
    // size and positioning
    float height;
    float x;
    float y;
    float quadHeight;
    float quadWidth;
    
    unsigned long numQuads;
    
    // font details
    TextUniforms textUniforms;
    std::string font;
    FT_Face face;
    
    // text
    std::string text;

    // cache needs to account for positioning
//    std::unordered_map<char, std::vector<std::vector<simd_float2>>> chContoursCache;
    
    // cache points and tags bc ft has hella latency on loading those
//    std::unordered_map<char, FT_Outline*> outlineCache;
};


#endif
