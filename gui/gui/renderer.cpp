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


// things to sort out:

// ideal api
/*
    root()(
        div().flex().color(simd_float3{0.5,0.5,0.3}).on<Click>([](RenderNode& self, KeyboardPayload payload){})
        (
            div(),
            text().on<KeyboardDown>(lambda)
        )
    );
 */


void Renderer::makeResources()
{
    FT_Init_FreeType(&(this->ft));
    
    auto inputListener = [](auto& self, const auto& payload){
        auto drawable = self.drawable.get();

        if (payload.ch == '\x7F') {
            drawable->removeChar();
        }else {
            drawable->addChar(payload.ch);
        }
    };
    
    auto onClick = [](auto& self, const auto& payload) {
        auto drawable = self.drawable.get();

        drawable->color = simd_float4{0,0.5,0,0.5};
    };
    
    div(*this, 100.0, 100.0, 256.0, 256.0, simd_float4{0,0,0.5,0.5}, 50.0).on<EventType::Click>(onClick)
    (
        text(*this, "", 24.0, 10, 25).on<EventType::KeyboardDown>(inputListener),
        div(*this, 100.0, 100.0, 0, 128.0, simd_float4{0.5,0,0,0.5})
    );
    
    
//    auto s1 = renderTree.insertNode(std::make_unique<Shell>(*this, 100.0, 100.0, 256.0, 256.0, simd_float4{0,0,0.5,0.5}, 50.0), root);
//    
//    auto textBlock = renderTree.insertNode(std::make_unique<Text>(*this, 10.0, 25.0, 24.0, simd_float3{1,1,1}), root);
//    
//    renderTree.insertNode(std::make_unique<Shell>(*this, 100.0, 100.0, 0, 128.0, simd_float4{0.5,0,0,0.5}), root);
//    
//    textBlock->addEventHandler<EventType::KeyboardDown>([](auto& self, const auto& payload){
//        auto drawable = self.drawable.get();
//    
//        if (payload.ch == '\x7F') {
//            drawable->removeChar();
//        }else {
//            drawable->addChar(payload.ch);
//        }
//    });
//    
////    textBlock->drawable.get()->setText("This is a prerendered scene");
//
//    s1->addEventHandler<EventType::Click>([](auto& self, const auto& payload) {
//        auto drawable = self.drawable.get();
//    
//        drawable->color = simd_float4{0,0.5,0,0.5};
//    });

    
}

void Renderer::draw() {
    NS::AutoreleasePool* autoreleasePool = NS::AutoreleasePool::alloc()->init();
    
    this->frameSemaphore.acquire();
    
    MTL::CommandBuffer* commandBuffer = commandQueue->commandBuffer();
    MTL::RenderPassDescriptor* renderPassDescriptor = view->currentRenderPassDescriptor();
    MTL::RenderCommandEncoder* renderCommandEncoder = commandBuffer->renderCommandEncoder(renderPassDescriptor);
    renderCommandEncoder->setDepthStencilState(getDefaultDepthStencilState());
    
//    auto ts1 = clock.now();
//    
//    renderTree.update();
//    renderTree.render(renderCommandEncoder);
//    
//    auto ts2 = clock.now();
//    auto micros = std::chrono::duration_cast<std::chrono::microseconds>(ts2 - ts1).count();
//    

////    std::println("Update+Render: {} us", micros);

    auto ts1 = clock.now();

    renderTree.update();
    renderTree.render(renderCommandEncoder);

    auto ts2 = clock.now();
    auto micros = std::chrono::duration_cast<std::chrono::microseconds>(ts2 - ts1).count();

    ++numSamples;
    totalMicros += micros;  // running sum in µs

    if (numSamples % 100 == 0) {
        double avgMs = (totalMicros / static_cast<double>(numSamples)) / 1000.0;
        std::cout << "Average time over " << numSamples << " samples: "
                  << avgMs << " ms\n";
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

FrameInfo Renderer::getFramePixelSize() {
    auto frameDimensions = this->view->drawableSize();

    return {.width=static_cast<float>(frameDimensions.width), .height=static_cast<float>(frameDimensions.height)};
}

FrameInfo Renderer::getFrameInfo() {
    auto frameDimensions = this->view->drawableSize();

    return {.width=static_cast<float>(frameDimensions.width)/2.0f, .height=static_cast<float>(frameDimensions.height)/2.0f};
}


Renderer::~Renderer() {
    commandQueue->release();
    FT_Done_FreeType(ft);
}
