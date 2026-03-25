//
//  renderer.cpp
//  font-rendering
//
//  Created by Taanish Reja on 7/21/25.
//

#include "renderer.hpp"
#include "element.hpp"
#include "div.hpp"
#include "tree_manager.hpp"
#include "new_arch.hpp"
#include <simd/vector_types.h>
#include "index.hpp"

Renderer* Renderer::current = nullptr;

Renderer::Renderer(MTL::Device* device, MTK::View* view):
    device{device},
    view{view},
    commandQueue(device->newCommandQueue()),
    frameSemaphore{MaxOutstandingFrameCount},
    ctx{NewArch::ContextManager::getContext()},
    layoutEngine{},
    divProcessor{ctx},
    div{ctx},
    imgProcessor{ctx},
    txtProcessor{ctx},
    img{ctx},
    txt{ctx},
    rootTree{}
{
    auto rootElem = Div(ctx);
    rootElem.getDescriptor().color = simd_float4{0,0,0,0};

    auto* rootNode = rootTree.createRoot(ctx, std::move(rootElem), NewArch::getDivProcessor(ctx));
    rootNode->shared.width = NewArch::Size::percent(1.0);
    rootNode->shared.height = NewArch::Size::percent(1.0);
    makeResources();
}

MTL::DepthStencilState* Renderer::getDefaultDepthStencilState()
{
    static MTL::DepthStencilState* defaultDepthStencilState = nullptr;
    
    if (!defaultDepthStencilState) {
        MTL::DepthStencilDescriptor* depthDescriptor = MTL::DepthStencilDescriptor::alloc()->init();
        depthDescriptor->setDepthCompareFunction(MTL::CompareFunctionLessEqual);
        depthDescriptor->setDepthWriteEnabled(true);
        defaultDepthStencilState = device->newDepthStencilState(depthDescriptor);
        depthDescriptor->release();
    }
    
    return defaultDepthStencilState;
}

void Renderer::makeResources()
{
    FT_Init_FreeType(&(this->ft));

    TreeStack::pushTree(&rootTree);

    index();
}

void Renderer::draw() {
    NS::AutoreleasePool* autoreleasePool = NS::AutoreleasePool::alloc()->init();
    
    this->frameSemaphore.acquire();
    
    MTL::CommandBuffer* commandBuffer = commandQueue->commandBuffer();
    MTL::RenderPassDescriptor* renderPassDescriptor = view->currentRenderPassDescriptor();
    MTL::RenderCommandEncoder* renderCommandEncoder = commandBuffer->renderCommandEncoder(renderPassDescriptor);
    // renderCommandEncoder->setDepthStencilState(getDefaultDepthStencilState());

    auto ts1 = clock.now();
    rootTree.update(getFrameInfo());
    rootTree.render(renderCommandEncoder);
    
    auto ts2 = clock.now();
    auto micros = std::chrono::duration_cast<std::chrono::microseconds>(ts2 - ts1).count();

    windowSum -= samples[sampleIndex % WINDOW_SIZE];
    samples[sampleIndex % WINDOW_SIZE] = micros;
    windowSum += micros;

    ++sampleIndex;
    ++totalSamples;

    if (totalSamples % 100 == 0) {
        int count = std::min(totalSamples, WINDOW_SIZE);
        double avgMs = (windowSum / static_cast<double>(count)) / 1000.0;
        // std::println("Rolling avg (last {} frames): {:.3f} ms", count, avgMs);
    }

    renderCommandEncoder->endEncoding();
    
    std::function<void(MTL::CommandBuffer*)> completedHandler = [this](MTL::CommandBuffer* commandBuffer){
        this->frameSemaphore.release();
        ctx.frameIndex += 1;
    };

  
    commandBuffer->addCompletedHandler(completedHandler);
    commandBuffer->presentDrawable(view->currentDrawable());
    commandBuffer->commit();
    
    autoreleasePool->release();
}

FrameInfo Renderer::getFramePixelSize() {
    auto frameDimensions = this->view->drawableSize();

    return {.width=static_cast<float>(frameDimensions.width), .height=static_cast<float>(frameDimensions.height)};
}

FrameInfo Renderer::getFrameInfo() {
    auto frameDimensions = this->view->drawableSize();
    auto scale = AppKit_Extensions::getContentScaleFactor(reinterpret_cast<void*>(this->view));

    return {.width=static_cast<float>(frameDimensions.width)/2.0f, .height=static_cast<float>(frameDimensions.height)/2.0f, .scale = scale};
}

void Renderer::makeCurrent() {
    current = this;
}

bool Renderer::hasActiveRenderer() {
    return current != nullptr;
}

Renderer& Renderer::active()
{
    return *current;
}


Renderer::~Renderer() {
    commandQueue->release();
    FT_Done_FreeType(ft);
}







