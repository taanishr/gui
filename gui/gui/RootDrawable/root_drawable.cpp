//
//  root_drawable.cpp
//  gui
//
//  Created by Taanish Reja on 8/25/25.
//

#include "root_drawable.hpp"

using namespace RootRender;

Bounds RootDrawable::elementBounds = {.topLeft = {0,0}, .bottomRight = {0,0}};

void RootDrawable::update() {};
MTL::RenderPipelineState* RootDrawable::getPipeline() { return nullptr; }
void RootDrawable::encode(MTL::RenderCommandEncoder* encoder) {};
const Bounds& RootDrawable::bounds() const { return elementBounds; };
bool RootDrawable::contains(simd_float2 point) const { return false; };

