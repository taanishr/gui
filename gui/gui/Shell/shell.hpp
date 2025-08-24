//
//  shell.hpp
//  gui
//
//  Created by Taanish Reja on 8/20/25.
//

#pragma once
#include "metal_imports.hpp"
#include "printers.hpp"
#include "renderable.hpp"
#include <print>
#include "frame_info.hpp"

class Renderer;

namespace ShellRender {
    struct QuadPoint {
        simd_float2 position;
    };

    struct Uniforms {
        simd_float4 color;
    };

    struct Shell : public Renderable {
        Shell(Renderer& renderer, float width, float height, float x = 0.0, float y = 0.0, simd_float4 color={0,0,0,1});
        
        void update();
        void buildPipeline(MTL::RenderPipelineState*& pipeline);
        MTL::RenderPipelineState* getPipeline();
        void encode(MTL::RenderCommandEncoder* encoder);
        const Bounds& bounds();
        ~Shell();
        
        Renderer& renderer;
        
        MTL::Buffer* quadPointsBuffer;
        MTL::Buffer* uniformsBuffer;
        MTL::Buffer* frameInfoBuffer;
        
        // uniforms
        simd_float4 color;
        
        // size and positioning
        float height;
        float width;
        float x;
        float y;
        Bounds elementBounds;
        
    };

}
