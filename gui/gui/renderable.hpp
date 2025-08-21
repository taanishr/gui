//
//  renderable.hpp
//  gui
//
//  Created by Taanish Reja on 8/20/25.
//

#pragma once

#include "metal_imports.hpp"

class Renderable {
public:
    virtual void update() = 0;
    virtual MTL::RenderPipelineState* getPipeline() = 0;
    virtual void encode(MTL::RenderCommandEncoder* encoder) = 0;
    virtual ~Renderable() = default;

private:
    virtual void buildPipeline(MTL::RenderPipelineState*& pipeline) = 0;
};
