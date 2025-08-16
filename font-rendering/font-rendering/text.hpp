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

struct Contour {
    const std::vector<simd_float2>& points;
    float penX;
    float penY;
};

struct ContourMeta {
    unsigned long start;
    unsigned long end;
    float minX;
    float maxX;
    float minY;
    float maxY;
};


struct TextUniforms {
    simd_float3 color;
    unsigned long numContours;
};

struct Text {
    Text(MTL::Device* device, FT_Library ft, float height=64.0, simd_float3 color={1,1,1}, const std::string& font = defaultFont.data());
    ~Text();
    
    void setText(const std::string& text);
    void update();
    void resizeBuffer(MTL::Buffer** buffer, unsigned long numBytes);
    
    MTL::Device* device;
    
    // buffers
    MTL::Buffer* uniformsBuffer;
    MTL::Buffer* quadBuffer;
    MTL::Buffer* contoursBuffer;
    MTL::Buffer* contoursMetaBuffer;
    
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
    
    // cache needs to account for positioning
    std::unordered_map<char, std::vector<std::vector<simd_float2>>> chContoursCache;
};

