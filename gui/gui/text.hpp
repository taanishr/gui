////
////  text.hpp
////  font-rendering
////
////  Created by Taanish Reja on 8/15/25.
////
//
//#pragma once
//
//#include "metal_imports.hpp"
//#include "freetype.hpp"
//#include "process_contours.hpp"
//#include "frame_info.hpp"
//#include "buffer_helpers.hpp"
//#include "layout.hpp"
//#include "bounds.hpp"
//#include "glyphCache.hpp"
//#include <ranges>
//
//class Renderer;
//
//constexpr std::string_view defaultFont {"/System/Library/Fonts/Supplemental/Arial.ttf"};
//
//namespace TextRender {
//    struct QuadPoint {
//        simd_float2 position;
////        simd_float2 offset;
//        int glyphIndex;
//        int metadataIndex;
//    };
//
//    struct Uniforms {
//        simd_float4 color;
//    };
//
//    class Text {
//    public:
//        Text(Renderer& renderer, float x, float y, float fontSize=64.0, simd_float4 color={0,0,0,1}, const std::string& font = defaultFont.data());
//        ~Text();
//        
//        MTL::RenderPipelineState* getPipeline();
//        void setText(const std::string& text);
//        void addChar(char ch);
//        void removeChar();
//        void update(const TextLayout& layoutBox);
//        void encode(MTL::RenderCommandEncoder* encoder);
//        bool contains(simd_float2 point) const;
//        const TextLayout& layout() const;
////        FragmentTemplate& _imeasure(Constraints& constraints); // new measuring method; impl for now
//        const DrawableSize& measure();
//        
//        void buildPipeline(MTL::RenderPipelineState*& pipeline);
//        FrameInfo getFrameInfo();
//        
//    
//        // renderer (for device, view and frame info)
//        Renderer& renderer;
//        
//        // text
//        std::string text;
//        
//        // buffers
//        // vertex
//        MTL::Buffer* quadBuffer;
//        MTL::Buffer* quadOffsetBuffer;
//        
//        // fragment
//        std::vector<simd_float2> bezierPoints;
//        MTL::Buffer* bezierPointsBuffer; // important thing; we do not want to duplicate points for same symbol; just duplicate starting index
//        MTL::Buffer* glyphMetaBuffer;
//        MTL::Buffer* uniformsBuffer;
//        MTL::Buffer* frameInfoBuffer;
//        
//        int numQuadPoints;
//        int lastBezierPoint;
//        
//        // freetype
//        FT_Face face;
//        
//        // properties
//        std::string font;
//        simd_float4 color;
//        float fontSize;
//        
//        // layout
//        bool fragmented;
//        std::optional<simd_float2> measureFragment(int startingIndex, int endingIndex);
//        void fragment(float flowX, float flowY, float maxWidth, float maxHeight);
//        void defragment();
//        
//        float x;
//        float y;
//        DrawableSize intrinsicSize;
//        const TextLayout* textLayout;
//        
//        GlyphCache& glyphCache;
//        std::unordered_map<char,Glyph> glyphMap;
//        std::unordered_map<char,int> glyphBezierMap;
//    };
//}
