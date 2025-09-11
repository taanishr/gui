//
//  image.hpp
//  gui
//
//  Created by Taanish Reja on 8/28/25.
//

#pragma once

#include <string>
#include "metal_imports.hpp"
#include "drawable.hpp"
#include "MTKTexture_loader.hpp"
#include "bounds.hpp"

class Renderer;

namespace ImageRender {
    struct QuadPoint {
        simd_float2 position;
        simd_float2 uv;
    };

    struct Uniforms {
        simd_float2 rectCenter;
        simd_float2 halfExtent;
        float cornerRadius;
        float borderWidth;
        simd_float4 borderColor;
    };


    struct ImageDrawable {
        ImageDrawable(Renderer& renderer, const std::string& path);
        
        MTKTextures::MTKTextureLoader& getTextureLoader();
        
        void buildPipeline(MTL::RenderPipelineState*& pipeline);
        MTL::RenderPipelineState* getPipeline();
        
        void buildSampler(MTL::SamplerState*&);
        MTL::SamplerState* getSampler();
        
        void update(const ImageLayout& layout);
        void encode(MTL::RenderCommandEncoder* encoder);
        
//        const Bounds& bounds() const;
        const ImageLayout& layout() const;
        const DrawableSize& measure() const;
        
        bool contains(simd_float2 point) const;
        
        // properties
        float cornerRadius;
        float borderWidth;
        simd_float4 borderColor;
        
        // layout
        const ImageLayout* imageLayout;
        
        ~ImageDrawable();
    
        NS::SharedPtr<MTL::Texture> texture;
        MTL::Buffer* quadPointsBuffer;
        MTL::Buffer* frameInfoBuffer;
        MTL::Buffer* uniformsBuffer;
        
        DrawableSize intrinsicSize;
   
        std::string path;
        Renderer& renderer;
    
    };
}
