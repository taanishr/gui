//
//  text.hpp
//  font-rendering
//
//  Created by Taanish Reja on 8/15/25.
//

#include "metal_imports.hpp"
#include "bezier.hpp"
#include "freetype.hpp"
#include "renderFace.hpp"
#include "inputState.hpp"

constexpr std::string_view defaultFont {"/System/Library/Fonts/Supplemental/Arial.ttf"};

struct ContourBounds {
    unsigned long start;
    unsigned long end;
};


struct TextUniforms {
    simd_float3 color;
    unsigned long numContours;
};

struct Text {
    Text(MTL::Device* device, FT_Library ft, const std::string& font = defaultFont.data(), float height=64.0, simd_float3 color={1,1,1});
    ~Text();
    
    void setText(const std::string& text);
    void update();
    void resizeBuffer(MTL::Buffer** buffer, unsigned long numBytes);
    
    MTL::Device* device;
    
    // buffers
    MTL::Buffer* uniformsBuffer;
    MTL::Buffer* quadBuffer;
    MTL::Buffer* contoursBuffer;
    MTL::Buffer* contoursBoundsBuffer;
    
    // size and positioning
    float height;
    float x;
    float y;
    
    // font details
    TextUniforms textUniforms;
    std::string font;
    FT_Face face;
    
    // text
    std::string text;
};

