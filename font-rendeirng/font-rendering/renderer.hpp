//
//  renderer.hpp
//  font-rendeirng
//
//  Created by Taanish Reja on 7/21/25.
//

#include "metal_imports.hpp"
#include "bezier.hpp"
#include "freetype.hpp"
#include "renderFace.hpp"
#include <semaphore>

constexpr int maxOutstandingFrameCount = 3;
constexpr float resolution = 25.0;
constexpr std::string_view fontPath {"/System/Library/Fonts/Supplemental/Arial.ttf"};

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
    MTL::Buffer* vertexBuffer;
    std::counting_semaphore<maxOutstandingFrameCount> frameSemaphore;
    std::vector<std::pair<long, long>> contourBounds;
    FT_Library ft;
};
