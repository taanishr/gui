//
//  renderer.hpp
//  font-rendeirng
//
//  Created by Taanish Reja on 7/21/25.
//

#pragma once
#include "metal_imports.hpp"
#include "freetype.hpp"
#include <semaphore>
#include <iostream>
#include <ranges>
#include "frame_info.hpp"
#include "render_tree.hpp"
#include "events.hpp"
#include <chrono>
#include "node_builder.hpp"

constexpr int MaxOutstandingFrameCount = 2;

class Renderer {
public:
    Renderer(MTL::Device* device, MTK::View* view);
    ~Renderer();
    MTL::DepthStencilState* getDefaultDepthStencilState();
    void updateConstants();
    void makeResources();
    void draw();
    FrameInfo getFramePixelSize();
    FrameInfo getFrameInfo();
    
    void makeCurrent();
    static Renderer& active();
    
    MTL::Device* device;
    MTK::View* view;
    FT_Library ft;
    
    RenderTree renderTree;
    
    std::chrono::high_resolution_clock clock {};
    
    long long numSamples = 0;
    long long totalMicros;
    
    static Renderer* current;
private:
    MTL::CommandQueue* commandQueue;
    std::counting_semaphore<MaxOutstandingFrameCount> frameSemaphore;
};
