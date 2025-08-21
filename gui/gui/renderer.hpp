//
//  renderer.hpp
//  font-rendeirng
//
//  Created by Taanish Reja on 7/21/25.
//

#pragma once
#include "metal_imports.hpp"
#include "freetype.hpp"
#include "input_state.hpp"
#include <semaphore>
#include <iostream>
#include <ranges>
#include "frame_info.hpp"

constexpr int MaxOutstandingFrameCount = 2;

class Renderer {
public:
    Renderer(MTL::Device* device, MTK::View* view);
    ~Renderer();
    void updateConstants();
    void makeResources();
    void draw();
    FrameInfo getFrameInfo();
    
    MTL::Device* device;
    MTK::View* view;
    FT_Library ft;
    
private:
    MTL::CommandQueue* commandQueue;
    MTL::RenderPipelineState* renderPipelineState;
    std::counting_semaphore<MaxOutstandingFrameCount> frameSemaphore;
    std::vector<std::unique_ptr<Renderable>> renderables;
};
