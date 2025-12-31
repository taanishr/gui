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
#include "new_arch.hpp"
#include "div.hpp"
#include "image.hpp"
#include <chrono>
#include "glyphCache.hpp"
#include "text.hpp"

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
    static bool hasActiveRenderer();
    static Renderer& active();

    
    MTL::Device* device;
    MTK::View* view;
    FT_Library ft;
    NewArch::UIContext ctx;
    NewArch::LayoutEngine layoutEngine;
    NewArch::DivProcessor<> divProcessor;
    NewArch::ImageProcessor<> imgProcessor;
    NewArch::TextProcessor<> txtProcessor;
    NewArch::Div<> div;
    NewArch::Image<> img;
    NewArch::Text<> txt;



    std::chrono::high_resolution_clock clock {};
    
    long long numSamples = 0;
    long long totalMicros;
    
    static Renderer* current;
private:
    MTL::CommandQueue* commandQueue;
    std::counting_semaphore<MaxOutstandingFrameCount> frameSemaphore;
};
