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

struct FrameInfo {
    float width;
    float height;
};

struct QuadPoint {
    simd_float2 position;
    simd_float2 offset;
    int metadataIndex;
};

struct Uniforms {
    simd_float3 color;
};


struct Text {
    Text(MTL::Device* device, MTK::View* view, FT_Library ft,float x, float y, float fontSize=64.0, simd_float3 color={0,0,0}, const std::string& font = defaultFont.data());
    ~Text();
    
    static void buildTextPipeline(MTL::RenderPipelineState*& pipeline, MTL::Device* device, MTL::PixelFormat colorFmt, MTL::PixelFormat depthFmt);
    static MTL::RenderPipelineState* getTextPipeline(MTL::Device* device, MTL::PixelFormat colorFmt, MTL::PixelFormat depthFmt);
    void setText(const std::string& text);
    void update();
    void resizeBuffer(MTL::Buffer*& buffer, unsigned long numBytes);
    void encode(MTL::RenderCommandEncoder* encoder);
    FrameInfo getFrameInfo();
    
    // device (for buffer allocation)
    MTL::Device* device;
    // view (for screen size/color and depth fmts)
    MTK::View* view;

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
    MTL::Buffer* frameInfoBuffer;
    
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
    std::unordered_map<char,int> glyphBezierMap;
};

#endif
