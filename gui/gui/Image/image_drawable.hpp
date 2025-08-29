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

class Renderer;

namespace ImageRender {
    struct QuadPoint {
        simd_float2 position;
        simd_float2 uv;
    };


    struct ImageDrawable {
        ImageDrawable(Renderer& renderer, const std::string& path);
        
        MTKTextures::MTKTextureLoader& getTextureLoader();
        
        void buildPipeline(MTL::RenderPipelineState*& pipeline);
        MTL::RenderPipelineState* getPipeline();
        
        void buildSampler(MTL::SamplerState*&);
        MTL::SamplerState* getSampler();
        
        void update();
        void encode(MTL::RenderCommandEncoder* encoder);
        
        const Bounds& bounds() const;
        bool contains(simd_float2 point) const;
        
        float x;
        float y;
        
        float height;
        float width;
        
        simd_float2 halfExtent;
        simd_float2 center;
        
        ~ImageDrawable();
        
        Bounds elementBounds;
    
        NS::SharedPtr<MTL::Texture> texture;
        MTL::Buffer* quadPointsBuffer;
        MTL::Buffer* frameInfoBuffer;
   
        std::string path;
        Renderer& renderer;
    
    };
}
