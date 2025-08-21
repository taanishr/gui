//
//  text.hpp
//  font-rendering
//
//  Created by Taanish Reja on 8/15/25.
//

#ifndef TEXT_HPP
#define TEXT_HPP

#include "metal_imports.hpp"
#include "freetype.hpp"
#include "process_contours.hpp"
#include "renderable.hpp"

constexpr std::string_view defaultFont {"/System/Library/Fonts/Supplemental/Arial.ttf"};

struct QuadPoint {
    simd_float2 position;
    simd_float2 offset;
    int metadataIndex;
};

struct Uniforms {
    simd_float3 color;
};


class Text : public Renderable {
public:
    Text(MTL::Device* device, MTK::View* view, FT_Library ft,float x, float y, float fontSize=64.0, simd_float3 color={0,0,0}, const std::string& font = defaultFont.data());
    ~Text();
    
    MTL::RenderPipelineState* getPipeline();
    void setText(const std::string& text);
    void update();
    void encode(MTL::RenderCommandEncoder* encoder);
private:
    void buildPipeline(MTL::RenderPipelineState*& pipeline);
    void resizeBuffer(MTL::Buffer*& buffer, unsigned long numBytes);
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
