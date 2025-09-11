//
//  drawable.hpp
//  gui
//
//  Created by Taanish Reja on 8/20/25.
//

#pragma once

#include "metal_imports.hpp"
#include "layout_box.hpp"


template <typename T, typename L>
concept Drawable = Layout<L> && requires(T t, MTL::RenderCommandEncoder* encoder, simd_float2 pt, const L& layout) {
    { t.getPipeline() } -> std::same_as<MTL::RenderPipelineState*>;
    
    { t.update(layout) } -> std::same_as<void>;
    { t.encode(encoder) } -> std::same_as<void>;
    
    { t.measure() } -> std::same_as<const DrawableSize&>;
    { t.layout() } -> std::same_as<const L&>;

    { t.contains(pt) } -> std::same_as<bool>;
};


