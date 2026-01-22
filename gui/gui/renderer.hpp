//
//  renderer.hpp
//  font-rendeirng
//
//  Created by Taanish Reja on 7/21/25.
//

#pragma once
#include <semaphore>
#include "frame_info.hpp"
#include "new_arch.hpp"
#include "div.hpp"
#include "image.hpp"
#include <chrono>
#include "text.hpp"
#include "element.hpp"
#include "renderer_constants.hpp"
#include "context_manager.hpp"
#include "tree_manager.hpp"
#include "AppKit_Extensions.hpp"
#include "sizing.hpp"


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
    NewArch::UIContext& ctx;
    NewArch::LayoutEngine layoutEngine;
    NewArch::DivProcessor<> divProcessor;
    NewArch::ImageProcessor<> imgProcessor;
    NewArch::TextProcessor<> txtProcessor;
    NewArch::Div<> div;
    NewArch::Image<> img;
    NewArch::Text<> txt;
    NewArch::RenderTree rootTree;

    std::chrono::high_resolution_clock clock {};
    
    long long numSamples = 0;
    long long totalMicros;
    
    static Renderer* current;
private:
    MTL::CommandQueue* commandQueue;
    std::counting_semaphore<MaxOutstandingFrameCount> frameSemaphore;
};
