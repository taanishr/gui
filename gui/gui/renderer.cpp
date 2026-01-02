//
//  renderer.cpp
//  font-rendering
//
//  Created by Taanish Reja on 7/21/25.
//

#include "renderer.hpp"
#include "node_builder.hpp"
#include "new_arch.hpp"
#include <print>
#include <simd/vector_types.h>

Renderer* Renderer::current = nullptr;

Renderer::Renderer(MTL::Device* device, MTK::View* view):
    device{device},
    view{view},
    commandQueue(device->newCommandQueue()),
    frameSemaphore{MaxOutstandingFrameCount},
    ctx{device, view},
    layoutEngine{},
    divProcessor{ctx},
    div{ctx},
    imgProcessor{ctx},
    txtProcessor{ctx},
    img{ctx},
    txt{ctx},
    tree()
{
    auto rootElem = NewArch::Div(ctx);
    auto& desc = rootElem.getDescriptor();
    desc.width = 300;
    desc.height = 300;
    desc.color = simd_float4{1,0,0,1};
    desc.cornerRadius = 0;

    tree.createRoot(ctx, std::move(rootElem), NewArch::getDivProcessor(ctx));
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
    auto& divDesc = div.getDescriptor();
    divDesc.position = NewArch::Position::Absolute;
    divDesc.width = 100;
    divDesc.height = 100;
    divDesc.color = simd_float4{0.5,0.0,0.0,1.0};
    
    auto& imgDesc = img.getDescriptor();
    imgDesc.width = 286;
    imgDesc.height = 147;
    imgDesc.path = "/Users/treja/Downloads/coarsening.png";
    
    auto& txtDesc = txt.getDescriptor();
    txtDesc.text = "bacfsdfsdfsfdfsdfdf";
    txtDesc.font = "/System/Library/Fonts/Supplemental/Arial.ttf";
    txtDesc.color = simd_float4{1,1,1,1};
    txtDesc.fontSize = 64.0;


    auto firstChild = NewArch::div(ctx, tree, 200, 200, simd_float4{0,1,0,1})(
        NewArch::text(ctx, tree, "hello world", 96.0)
    );

    std::println("first child: {}", reinterpret_cast<void*>(firstChild.node));
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
    divConstraints.origin = simd_float2{0.0,0.0};
    divConstraints.cursor = simd_float2{100.0,100.0};
    divConstraints.frameInfo = {
        .width = static_cast<float>(ctx.view->drawableSize().width / 2.0),
        .height = static_cast<float>(ctx.view->drawableSize().height / 2.0)
    };

    tree.update(getFrameInfo());
    tree.render(renderCommandEncoder);
    
    // auto measured = divProcessor.measure(div.getFragment(), divConstraints, div.getDescriptor());
    // auto atomized = divProcessor.atomize(div.getFragment(), divConstraints, div.getDescriptor(), measured);
    // auto layout = divProcessor.layout(div.getFragment(), divConstraints, div.getDescriptor(), measured, atomized);
    // auto placed = divProcessor.place(div.getFragment(), divConstraints, div.getDescriptor(), measured, atomized, layout);
    // auto finalized = divProcessor.finalize(div.getFragment(), divConstraints, div.getDescriptor(), measured, atomized, placed);
    // divProcessor.encode(renderCommandEncoder, div.getFragment(), finalized);
    
    // NewArch::Constraints imgConstraints;
    // imgConstraints.origin = simd_float2{0.0,0.0};
    // imgConstraints.cursor = simd_float2{50.0,300.0};
    // imgConstraints.frameInfo = {
    //     .width = static_cast<float>(ctx.view->drawableSize().width / 2.0),
    //     .height = static_cast<float>(ctx.view->drawableSize().height / 2.0)
    // };
    
    
    // auto iMeasured = imgProcessor.measure(img.getFragment(), imgConstraints, img.getDescriptor());
    // auto iAtomized = imgProcessor.atomize(img.getFragment(), imgConstraints, img.getDescriptor(), iMeasured);
    // auto iLayout = imgProcessor.layout(img.getFragment(), imgConstraints, img.getDescriptor(), iMeasured, iAtomized);
    // auto iPlaced = imgProcessor.place(img.getFragment(), imgConstraints, img.getDescriptor(), iMeasured, iAtomized, iLayout);
    // auto iFinalized = imgProcessor.finalize(img.getFragment(), imgConstraints, img.getDescriptor(), iMeasured, iAtomized, iPlaced);
    // imgProcessor.encode(renderCommandEncoder, img.getFragment(), iFinalized);

    // NewArch::Constraints txtConstraints;
    // txtConstraints.cursor = simd_float2{100.0,50.0};
    // txtConstraints.frameInfo = {
    //     .width = static_cast<float>(ctx.view->drawableSize().width / 2.0),
    //     .height = static_cast<float>(ctx.view->drawableSize().height / 2.0)
    // };
    
    // auto tMeasured = txtProcessor.measure(txt.getFragment(), txtConstraints, txt.getDescriptor());
    // auto tAtomized = txtProcessor.atomize(txt.getFragment(), txtConstraints, txt.getDescriptor(), tMeasured);
    // auto tLayout = txtProcessor.layout(txt.getFragment(), txtConstraints, txt.getDescriptor(), tMeasured, tAtomized);
    // auto tPlaced = txtProcessor.place(txt.getFragment(), txtConstraints, txt.getDescriptor(), tMeasured, tAtomized, tLayout);
    // auto tFinalized = txtProcessor.finalize(txt.getFragment(), txtConstraints, txt.getDescriptor(), tMeasured, tAtomized, tPlaced);
    // txtProcessor.encode(renderCommandEncoder, txt.getFragment(), tFinalized);
    
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







