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

class Renderer;

namespace RootRender {
    struct RootDrawable {
        void update();
        MTL::RenderPipelineState* getPipeline();
        void encode(MTL::RenderCommandEncoder* encoder);
        const Bounds& bounds() const;
        bool contains(simd_float2 point) const;
        
        static Bounds elementBounds;
        
        float x;
        float y;
        
        simd_float4 color;
    };
}
