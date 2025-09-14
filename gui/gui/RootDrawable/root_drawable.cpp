//
//  root_drawable.cpp
//  gui
//
//  Created by Taanish Reja on 8/25/25.
//

#include "root_drawable.hpp"

using namespace RootRender;

//Bounds RootDrawable::elementBounds = {.topLeft = {0,0}, .bottomRight = {0,0}};

void RootDrawable::update(const DefaultLayout& layoutBox) {};
MTL::RenderPipelineState* RootDrawable::getPipeline() { return nullptr; }
void RootDrawable::encode(MTL::RenderCommandEncoder* encoder) {};
//const Bounds& RootDrawable::bounds() const { return elementBounds; };

const DrawableSize& RootDrawable::measure() const {
    static DrawableSize intrinsicSize {0,0};
    return intrinsicSize;
}

const DefaultLayout& RootDrawable::layout() const { return *(this->rootLayout); }
bool RootDrawable::contains(simd_float2 point) const { return false; };

