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
    makeCurrent();
    FT_Init_FreeType(&(this->ft));
    
    auto onInput = [](auto& self, const auto& payload){
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
    
    div().h(100.0).w(100.0).cornerRadius(50.0).x(256.0).y(256.0).color(RGB{0,0,128,0.5}).on<EventType::Click>(onClick)(
        text("").fontSize(24.0).x(10.0).y(25.0).on<EventType::KeyboardDown>(onInput),
        div().h(100.0).w(100.0).y(128.0).color(RGB{128,0,0,0.5})(
             div().h(50.0).w(50.0).y(324.0).x(128.0).cornerRadius(25).color(RGB{255,255,255,1}).borderColor(RGB{125,0,0,0.5}).borderWidth(2)
        ),
//        image("/Users/treja/Downloads/lemickey.jpg").x(200).y(500).w(200).h(231.75)
        image("/Users/treja/build_journal/Screenshot 2025-07-19 at 8.06.55 PM.png").x(200).y(500).w(275).h(100).cornerRadius(16).borderWidth(2).borderColor(RGB(50,0,126,1))
    );
    
//    div().h(200).w(150).cornerRadius(20).x(256-75).y(256+100).color({0,0,0.5,1}).on<EventType::Click>(onClick)(
//        text("Login").fontSize(24).x(256-50).y(256-75),
//        div().h(20).w(100).cornerRadius(8).x(256-50).y(256).color({1,1,1,1}),
//        div().h(20).w(100).cornerRadius(8).x(256-50).y(256+40).color({1,1,1,1})
//    );
//    
    
    
    // api improvements
    // define root elem which passes renderer to children? or actually, automatically determine if parent node builder has an element
    // figure out how to go beyond concepts to cutomize elements more? i.e. specific operator to access drawable
    /*
    root(
        div(100.0,100.0,50).x(256.0).y(256.0).color({0,0,0.5,0.5}).on<EventType::Click>(onClick)
        (
            text(24.0,10,25).on<EventType::KeyboardDown>(inputListener),
            div(100.0,100.0).y(128.0).color(simd_float4{})
        )
     )
     
     */
    
    
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

void Renderer::makeCurrent() {
    current = this;
}

Renderer& Renderer::active()
{
    return *current;
}


Renderer::~Renderer() {
    commandQueue->release();
    FT_Done_FreeType(ft);
}
