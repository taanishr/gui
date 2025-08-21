//
//  renderer.cpp
//  font-rendering
//
//  Created by Taanish Reja on 7/21/25.
//

#include "renderer.hpp"

float linearInterpolation(float x1, float y1, float x2, float y2) {
    return (y2-y1)/(x2-x1);
}

Renderer::Renderer(MTL::Device* device, MTK::View* view):
    device{device},
    view{view},
    commandQueue(device->newCommandQueue()),
    frameSemaphore{maxOutstandingFrameCount}
{
    makeResources();
}

void Renderer::makeResources() {
    FT_Init_FreeType(&(this->ft));
    textBlocks.push_back(std::make_unique<Text>(device, view, ft, 10.0, 25.0, 20.0, simd_float3{1,1,1}));
//    textBlocks[0]->setText("will this work");
    SelectedString::textBlock = dynamic_cast<Text*>(textBlocks[0].get());
}

void Renderer::draw() {
    NS::AutoreleasePool* autoreleasePool = NS::AutoreleasePool::alloc()->init();
    
    this->frameSemaphore.acquire();
    
    MTL::CommandBuffer* commandBuffer = commandQueue->commandBuffer();
    MTL::RenderPassDescriptor* renderPassDescriptor = view->currentRenderPassDescriptor();
    MTL::RenderCommandEncoder* renderCommandEncoder = commandBuffer->renderCommandEncoder(renderPassDescriptor);

    for (auto& textBlock: textBlocks) {
        textBlock->update();
        textBlock->encode(renderCommandEncoder);
    }

    renderCommandEncoder->endEncoding();
    
    std::function<void(MTL::CommandBuffer*)> completedHandler = [this](MTL::CommandBuffer* commandBuffer){
        this->frameSemaphore.release();
    };
    
    commandBuffer->addCompletedHandler(completedHandler);
    commandBuffer->presentDrawable(view->currentDrawable());
    commandBuffer->commit();
    
    autoreleasePool->release();
}

Renderer::~Renderer() {
    commandQueue->release();
    renderPipelineState->release();
    FT_Done_FreeType(ft);
}
