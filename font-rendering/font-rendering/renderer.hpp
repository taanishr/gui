//
//  renderer.hpp
//  font-rendeirng
//
//  Created by Taanish Reja on 7/21/25.
//

#ifndef RENDERER_H
#define RENDERER_H
#include "metal_imports.hpp"
#include "bezier.hpp"
#include "freetype.hpp"
#include "renderFace.hpp"
#include "inputState.hpp"
#include <semaphore>


constexpr std::string_view str {"hello world"};
constexpr int maxOutstandingFrameCount = 3;
constexpr std::string_view fontPath {"/System/Library/Fonts/Supplemental/Arial Bold.ttf"};


struct ContourBounds {
    unsigned long start;
    unsigned long end;
};

struct Constants {
    unsigned long nPoints;
    unsigned long numContours;
};

class Renderer {
public:
    Renderer(MTL::Device* device, MTK::View* view);
    ~Renderer();
    void updateConstants();
    void makePipeline();
    void makeResources();
    void draw();
private:
    MTL::Device* device;
    MTL::CommandQueue* commandQueue;
    MTL::RenderPipelineState* renderPipelineState;
    MTK::View* view;
    MTL::Buffer* glyphQuadBuffer;
    MTL::Buffer* glyphContoursBuffer;
    MTL::Buffer* constantsBuffer;
    MTL::Buffer* contourBoundsBuffer;
    std::counting_semaphore<maxOutstandingFrameCount> frameSemaphore;
    std::vector<std::pair<long, long>> quadBounds;
    FT_Library ft;
    FT_Face face;
};

#endif
