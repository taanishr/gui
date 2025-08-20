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
#include "processContours.hpp"

constexpr std::string_view defaultFont {"/System/Library/Fonts/Supplemental/Arial.ttf"};


struct QuadPoint {
    simd_float2 position;
    simd_float2 offset;
    int metadataIndex;
};

struct Uniforms {
    simd_float3 color;
};

struct Text {
    Text(MTL::Device* device, FT_Library ft, float fontSize=64.0, simd_float3 color={0,0,0}, const std::string& font = defaultFont.data());
    ~Text();

    void setText(const std::string& text);
    void update();
    void resizeBuffer(MTL::Buffer** buffer, unsigned long numBytes);
    
    // device (for buffer allocation)
    MTL::Device* device;

    // text
    std::string text;
    
    // buffers
    // vertex
    MTL::Buffer* quadBuffer;
    
    // fragment
    std::vector<simd_float2> bezierPoints;
    MTL::Buffer* bezierPointsBuffer; // important thing; we do not want to duplicate points for same symbol; just duplicate starting index
    MTL::Buffer* glyphMetaBuffer;
    MTL::Buffer* uniformsBuffer;
    
    int numQuadPoints;
    int lastBezierPoint;
    
    // freetype
    FT_Face face;
    
    // uniforms
    std::string font;
    simd_float3 color;
    
    // size and positioning
    float fontSize;
    float x;
    float y;
    
    std::unordered_map<char,Glyph> glyphMap;
    std::unordered_map<char,int> glyphMetaMap;
    std::unordered_map<char,int> glyphBezierMap;
};

#endif
