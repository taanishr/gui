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
    encoder{ctx},
    layoutEngine{ctx}
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
}

/*
start with fragment template
 
multiple passes
 
Shell {
    initialize();
    std::vector<Atom> atomize();
    <- layout pass (independent) ->
    place(std::vector<AtomPlacements>):
    
    <- commit uniforms buffer ->
    FragmentTemplate& finalize(); -> passed to encoder
 
    FragmentTemplate& template;
}
 
 
 
 */


void Renderer::draw() {
    NS::AutoreleasePool* autoreleasePool = NS::AutoreleasePool::alloc()->init();
    
    this->frameSemaphore.acquire();
    
    MTL::CommandBuffer* commandBuffer = commandQueue->commandBuffer();
    MTL::RenderPassDescriptor* renderPassDescriptor = view->currentRenderPassDescriptor();
    MTL::RenderCommandEncoder* renderCommandEncoder = commandBuffer->renderCommandEncoder(renderPassDescriptor);
    renderCommandEncoder->setDepthStencilState(getDefaultDepthStencilState());

    auto ts1 = clock.now();
    
    NewArch::Shell sh {ctx, 200, 200};
    auto atoms = sh.atomize();
    
    NewArch::ShellUniforms uniforms;
    
    uniforms.color = simd_float4{0.5, 0.0, 0.0, 1.0};
    uniforms.borderColor = simd_float4{0.5, 0.0, 0.0, 1.0};
    uniforms.borderWidth = 0;
    uniforms.cornerRadius = 0;
    
    NewArch::Layout layout;
    layout.x = 100.0f;
    layout.y = 100.0f;
    
    uniforms.init_shape_dep(sh.width, sh.height);
    uniforms.init_layout_dep(layout);
    
    // pipeline
    // create shell object; this initializes buffers and sets up atoms
    // then, call layout engines place on atoms to get atom placements
    // then initialize the uniforms
    
    FragmentTemplate ft = layoutEngine.place(layout, uniforms, atoms);
    
    std::println("ft.atoms size: {}", ft.atoms.size());
    
    auto pipeline = sh.getPipeline();
    
    encoder.encode(renderCommandEncoder, pipeline, ft);
    
//    auto frameInfo = getFrameInfo();
//    renderTree.layout(frameInfo);
//    renderTree.position();
//    renderTree.update();
//    renderTree.render(renderCommandEncoder);
    
    
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
