//
//  text.hpp
//  font-rendering
//
//  Created by Taanish Reja on 8/15/25.
//

#pragma once

#include "metal_imports.hpp"
#include "freetype.hpp"
#include "process_contours.hpp"
#include "renderable.hpp"
#include "frame_info.hpp"
#include "buffer_helpers.hpp"

class Renderer;

constexpr std::string_view defaultFont {"/System/Library/Fonts/Supplemental/Arial.ttf"};

namespace TextRender {
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
        Text(Renderer& renderer, float x, float y, float fontSize=64.0, simd_float3 color={0,0,0}, const std::string& font = defaultFont.data());
        ~Text();
        
        MTL::RenderPipelineState* getPipeline();
        void setText(const std::string& text);
        void addChar(char ch);
        void removeChar();
        void update();
        void encode(MTL::RenderCommandEncoder* encoder);
        bool inBounds(simd_float2 point) const;
        const Bounds& bounds() const;
    private:
        void buildPipeline(MTL::RenderPipelineState*& pipeline);
        FrameInfo getFrameInfo();
        
    
        // renderer (for device, view and frame info)
        Renderer& renderer;
        
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
        
        Bounds elementBounds;
        
        std::unordered_map<char,Glyph> glyphMap;
        std::unordered_map<char,int> glyphBezierMap;
    };
}
