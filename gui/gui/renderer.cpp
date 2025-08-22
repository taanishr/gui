//
//  renderer.cpp
//  font-rendering
//
//  Created by Taanish Reja on 7/21/25.
//

#include "renderer.hpp"
#include "ui.hpp"

Renderer::Renderer(MTL::Device* device, MTK::View* view):
    device{device},
    view{view},
    commandQueue(device->newCommandQueue()),
    frameSemaphore{MaxOutstandingFrameCount},
    renderTree{}
{
    makeResources();
}

void Renderer::makeResources() {
    FT_Init_FreeType(&(this->ft));
//    renderables.push_back(std::make_unique<Text>(*this, 10.0, 25.0, 24.0, simd_float3{1,1,1}));
//    renderables.push_back(std::make_unique<Shell>(*this, 100.0, 100.0, 256.0, 256.0, simd_float4{0,0,0.5,0.5}));
//    SelectedString::textBlock = dynamic_cast<Text*>(renderables[0].get());
    
    auto root = renderTree.root.get();
    auto textBlock = renderTree.insertNode(std::make_unique<Text>(*this, 10.0, 25.0, 24.0, simd_float3{1,1,1}), root);
    renderTree.insertNode(std::make_unique<Shell>(*this, 100.0, 100.0, 256.0, 256.0, simd_float4{0,0,0.5,0.5}), root);
    
    SelectedString::textBlock = dynamic_cast<Text*>(textBlock->renderable.get());
}

void Renderer::draw() {
    NS::AutoreleasePool* autoreleasePool = NS::AutoreleasePool::alloc()->init();
    
    this->frameSemaphore.acquire();
    
    MTL::CommandBuffer* commandBuffer = commandQueue->commandBuffer();
    MTL::RenderPassDescriptor* renderPassDescriptor = view->currentRenderPassDescriptor();
    MTL::RenderCommandEncoder* renderCommandEncoder = commandBuffer->renderCommandEncoder(renderPassDescriptor);

//    for (auto& renderable: renderables) {
//        renderable->update();
//        renderable->encode(renderCommandEncoder);
//    }
    
    renderTree.update();
    renderTree.render(renderCommandEncoder);

    renderCommandEncoder->endEncoding();
    
    std::function<void(MTL::CommandBuffer*)> completedHandler = [this](MTL::CommandBuffer* commandBuffer){
        this->frameSemaphore.release();
    };
    
    commandBuffer->addCompletedHandler(completedHandler);
    commandBuffer->presentDrawable(view->currentDrawable());
    commandBuffer->commit();
    
    autoreleasePool->release();
}

FrameInfo Renderer::getFrameInfo() {
    auto frameDimensions = this->view->drawableSize();

    return {.width=static_cast<float>(frameDimensions.width)/2.0f, .height=static_cast<float>(frameDimensions.height)/2.0f};
}


Renderer::~Renderer() {
    commandQueue->release();
    renderPipelineState->release();
    FT_Done_FreeType(ft);
}
