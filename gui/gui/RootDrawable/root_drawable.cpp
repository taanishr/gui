//
//  root_drawable.cpp
//  gui
//
//  Created by Taanish Reja on 8/25/25.
//

#include "root_drawable.hpp"

using namespace RootRender;

void RootDrawable::update(const DefaultLayout& layoutBox) {};
MTL::RenderPipelineState* RootDrawable::getPipeline() { return nullptr; }
void RootDrawable::encode(MTL::RenderCommandEncoder* encoder) {};

const DrawableSize& RootDrawable::measure() const {
    static DrawableSize intrinsicSize {0,0};
    return intrinsicSize;
}

const DefaultLayout& RootDrawable::layout() const { return *(this->rootLayout); }
bool RootDrawable::contains(simd_float2 point) const { return false; };

