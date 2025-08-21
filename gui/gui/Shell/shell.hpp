//
//  shell.hpp
//  gui
//
//  Created by Taanish Reja on 8/20/25.
//

#include "metal_imports.hpp"
#include "printers.hpp"
#include "renderable.hpp"
#include <print>

struct ShellQuadPoint {
    simd_float2 position;
};

struct ShellUniforms {
    simd_float4 color;
};

struct Shell : public Renderable {
    Shell(MTL::Device* device, MTK::View* view, float width, float height, float x = 0.0, float y = 0.0, simd_float4 color={0,0,0,1});
    
    void update();
    void buildPipeline(MTL::RenderPipelineState*& pipeline);
    MTL::RenderPipelineState* getPipeline();
    void encode(MTL::RenderCommandEncoder* encoder);
    ~Shell();
    
    FrameInfo getFrameInfo();
    
    MTL::Device* device;
    MTK::View* view;
    
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
    
};

