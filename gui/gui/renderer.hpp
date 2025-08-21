//
//  renderer.hpp
//  font-rendeirng
//
//  Created by Taanish Reja on 7/21/25.
//

#ifndef RENDERER_H
#define RENDERER_H
#include "metal_imports.hpp"
#include "freetype.hpp"
#include "input_state.hpp"
#include <semaphore>
#include "text.hpp"
#include <iostream>
#include <ranges>
#include "shell.hpp"

constexpr int maxOutstandingFrameCount = 2;

struct Constants {
    unsigned long nPoints;
    unsigned long numContours;
};

class Renderer {
public:
    Renderer(MTL::Device* device, MTK::View* view);
    ~Renderer();
    void updateConstants();
    void makeResources();
    void draw();
private:
    MTL::Device* device;
    MTL::CommandQueue* commandQueue;
    MTL::RenderPipelineState* renderPipelineState;
    MTK::View* view;
    std::counting_semaphore<maxOutstandingFrameCount> frameSemaphore;
    FT_Library ft;
    std::vector<std::unique_ptr<Renderable>> renderables;
};

#endif
