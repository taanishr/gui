//
//  shell.hpp
//  gui
//
//  Created by Taanish Reja on 8/20/25.
//

#pragma once
#include "metal_imports.hpp"
#include "printers.hpp"
#include "drawable.hpp"
#include "sdf_helpers.hpp"
#include <print>
#include "frame_info.hpp"
#include "color.hpp"
#include "layout_box.hpp"

class Renderer;

namespace ShellRender {
    struct QuadPoint {
        simd_float2 position;
    };

    struct Uniforms {
        simd_float4 color;
        simd_float2 rectCenter;
        simd_float2 halfExtent;
        float cornerRadius;
        float borderWidth;
        simd_float4 borderColor;
    };

    struct Shell {
        Shell(Renderer& renderer, float width, float height, float x = 0.0, float y = 0.0, simd_float4 color={0,0,0,1}, float cornerRadius = 0.0);
        
        void update(const LayoutBox& layoutBox);
        void buildPipeline(MTL::RenderPipelineState*& pipeline);
        MTL::RenderPipelineState* getPipeline();
        void encode(MTL::RenderCommandEncoder* encoder);
        const Bounds& bounds() const;
        bool contains(simd_float2 point) const;
        ~Shell();
        
        Renderer& renderer;
        
        MTL::Buffer* quadPointsBuffer;
        MTL::Buffer* uniformsBuffer;
        MTL::Buffer* frameInfoBuffer;
        
        // properties
        simd_float4 color;
        simd_float4 borderColor;
        float cornerRadius = 0.0;
        float borderWidth = 0.0;
        
        // layout
        float height;
        float width;
        float x;
        float y;
        
        Bounds elementBounds;
        simd_float2 halfExtent;
        simd_float2 center;
        
    };

}
