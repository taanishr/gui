//
//  renderer.cpp
//  font-rendering
//
//  Created by Taanish Reja on 7/21/25.
//

#include "renderer.hpp"
#include "ui.hpp"

Renderer* Renderer::current = nullptr;

Renderer::Renderer(MTL::Device* device, MTK::View* view):
    device{device},
    view{view},
    commandQueue(device->newCommandQueue()),
    frameSemaphore{MaxOutstandingFrameCount},
    renderTree{},
    ctx{device, view},
    layoutEngine{},
    divProcessor{ctx},
    div{ctx},
    imgProcessor{ctx},
    img{ctx}
{
    
    makeResources();
}

GlyphCache& Renderer::glyphCache() {
    static GlyphCache cache {ft};
    return cache;
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
    auto& divDesc = div.getDescriptor();
    divDesc.width = 100;
    divDesc.height = 100;
    divDesc.color = simd_float4{0.5,0.0,0.0,1.0};
    
    auto& imgDesc = img.getDescriptor();
    imgDesc.width = 286;
    imgDesc.height = 147;
    imgDesc.path = "/Users/treja/Downloads/coarsening.png";
}

void Renderer::draw() {
    NS::AutoreleasePool* autoreleasePool = NS::AutoreleasePool::alloc()->init();
    
    this->frameSemaphore.acquire();
    
    MTL::CommandBuffer* commandBuffer = commandQueue->commandBuffer();
    MTL::RenderPassDescriptor* renderPassDescriptor = view->currentRenderPassDescriptor();
    MTL::RenderCommandEncoder* renderCommandEncoder = commandBuffer->renderCommandEncoder(renderPassDescriptor);
    renderCommandEncoder->setDepthStencilState(getDefaultDepthStencilState());

    auto ts1 = clock.now();
    NewArch::Constraints divConstraints;
    divConstraints.x = 100;
    divConstraints.y = 100;
    
    auto measured = divProcessor.measure(div.getFragment(), divConstraints, div.getDescriptor());
    auto atomized = divProcessor.atomize(div.getFragment(), divConstraints, div.getDescriptor(), measured);
    auto placed = divProcessor.place(div.getFragment(), divConstraints, div.getDescriptor(), measured, atomized);
    auto finalized = divProcessor.finalize(div.getFragment(), divConstraints, div.getDescriptor(), measured, atomized, placed);
    divProcessor.encode(renderCommandEncoder, div.getFragment(), finalized);
    
    NewArch::Constraints imgConstraints;
    imgConstraints.x = 50;
    imgConstraints.y = 300;
    
    auto iMeasured = imgProcessor.measure(img.getFragment(), imgConstraints, img.getDescriptor());
    auto iAtomized = imgProcessor.atomize(img.getFragment(), imgConstraints, img.getDescriptor(), iMeasured);
    auto iPlaced = imgProcessor.place(img.getFragment(), imgConstraints, img.getDescriptor(), iMeasured, iAtomized);
    auto iFinalized = imgProcessor.finalize(img.getFragment(), imgConstraints, img.getDescriptor(), iMeasured, iAtomized, iPlaced);
    imgProcessor.encode(renderCommandEncoder, img.getFragment(), iFinalized);

    
    auto ts2 = clock.now();
    auto micros = std::chrono::duration_cast<std::chrono::microseconds>(ts2 - ts1).count();

    ++numSamples;
    totalMicros += micros;  // running sum in µs

//    if (numSamples % 100 == 0) {
//        double avgMs = (totalMicros / static_cast<double>(numSamples)) / 1000.0;
//        std::println("Average time over {} samples: {} ms", numSamples, avgMs);
//    }
    
    renderCommandEncoder->endEncoding();
    
    
    std::function<void(MTL::CommandBuffer*)> completedHandler = [this](MTL::CommandBuffer* commandBuffer){
        this->frameSemaphore.release();
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

    return {.width=static_cast<float>(frameDimensions.width)/2.0f, .height=static_cast<float>(frameDimensions.height)/2.0f};
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
