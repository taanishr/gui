//
//  renderer.hpp
//  font-rendeirng
//
//  Created by Taanish Reja on 7/21/25.
//

#include "metal_imports.hpp"
#include "bezier.hpp"
#include <semaphore>

constexpr int maxOutstandingFrameCount = 3;
constexpr float resolution = 24;

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
};
