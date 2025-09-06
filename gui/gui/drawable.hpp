//
//  drawable.hpp
//  gui
//
//  Created by Taanish Reja on 8/20/25.
//

#pragma once

#include "metal_imports.hpp"
#include "layout_box.hpp"

struct Bounds {
    simd_float2 topLeft;
    simd_float2 bottomRight;
};

template <typename T>
concept Drawable = requires(T t, MTL::RenderCommandEncoder* encoder, simd_float2 pt, const LayoutBox& layoutBox) {
    { t.getPipeline() } -> std::same_as<MTL::RenderPipelineState*>;
    
    { t.update(layoutBox) } -> std::same_as<void>;
    { t.encode(encoder) } -> std::same_as<void>;
    
    { t.bounds() } -> std::same_as<const Bounds&>;
    { t.contains(pt) } -> std::same_as<bool>;
    
    { t.x } -> std::convertible_to<float>;
    { t.y } -> std::convertible_to<float>;
};
