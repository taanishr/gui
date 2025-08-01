//
//  renderer.cpp
//  font-rendeirng
//
//  Created by Taanish Reja on 7/21/25.
//

#include "renderer.hpp"
#include <iostream>

std::vector<simd_float2> controlPoints {simd_float2{0,0},simd_float2{0.25,0.5},simd_float2{0.5,0}};

float linearInterpolation(float x1, float y1, float x2, float y2) {
    return (y2-y1)/(x2-x1);
}

Renderer::Renderer(MTL::Device* device, MTK::View* view):
    device{device},
    view{view},
    commandQueue(device->newCommandQueue()),
    frameSemaphore{1}
{
    makePipeline();
    makeResources();
}

void Renderer::makePipeline() {
    MTL::Library* defaultLibrary = device->newDefaultLibrary();
    MTL::RenderPipelineDescriptor* renderPipelineDescriptor = MTL::RenderPipelineDescriptor::alloc()->init();
    
    // set up vertex descriptor
    MTL::VertexDescriptor* vertexDescriptor = MTL::VertexDescriptor::alloc()->init();
    vertexDescriptor->attributes()->object(0)->setFormat(MTL::VertexFormat::VertexFormatFloat2);
    vertexDescriptor->attributes()->object(0)->setOffset(0);
    vertexDescriptor->attributes()->object(0)->setBufferIndex(0);
    
    vertexDescriptor->layouts()->object(0)->setStride(sizeof(float)*2);
    
    renderPipelineDescriptor->setVertexDescriptor(vertexDescriptor);
    
    // set up vertex function
    MTL::Function* vertexFunction = defaultLibrary->newFunction(NS::String::string("vertex_main", NS::UTF8StringEncoding));
    renderPipelineDescriptor->setVertexFunction(vertexFunction);
    
    renderPipelineDescriptor->colorAttachments()->object(0)->setPixelFormat(this->view->colorPixelFormat());
    renderPipelineDescriptor->setDepthAttachmentPixelFormat(this->view->depthStencilPixelFormat());
    
    // set up fragment function
    MTL::Function* fragmentFunction = defaultLibrary->newFunction(NS::String::string("fragment_main", NS::UTF8StringEncoding));
    renderPipelineDescriptor->setFragmentFunction(fragmentFunction);
    
    
    NS::Error* error = nullptr;
    this->renderPipelineState = this->device->newRenderPipelineState(renderPipelineDescriptor, &error);
    
    
    defaultLibrary->release();
    renderPipelineDescriptor->release();
    vertexDescriptor->release();
    vertexFunction->release();    
}

void Renderer::makeResources() {
    FT_Init_FreeType(&(this->ft));
    FT_New_Face(ft, fontPath.data(), 0, &(this->face));
    FT_Set_Pixel_Sizes(face, 1, 1);
    this->vertexBuffer = this->device->newBuffer(sizeof(simd_float2)*(4096), MTL::StorageModeShared);
}

void Renderer::updateConstants() {
    std::vector<simd_float2> interpolatedPoints {};
    std::vector<std::vector<simd_float2>> contours {};
    
    for (int ch = 0; ch < str.size(); ++ch) {
        auto chContours = drawContours(str.at(ch), this->ft, this->face, fontPath, resolution, ch*0.09, 0);
        contours.insert(contours.end(), chContours.begin(), chContours.end());
    }
    
    long contourStart = 0;
    for (std::vector<simd_float2>& contour : contours) {
        contourBounds.push_back({contourStart, contourStart+contour.size()});
        interpolatedPoints.insert(interpolatedPoints.end(), contour.begin(), contour.end());
        contourStart += contour.size();
    }
    
    
    std::memcpy(this->vertexBuffer->contents(), interpolatedPoints.data(), sizeof(simd_float2)*interpolatedPoints.size());
}

void Renderer::draw() {
    NS::AutoreleasePool* autoreleasePool = NS::AutoreleasePool::alloc()->init();
    
    this->frameSemaphore.acquire();
    
    updateConstants();
    
    MTL::CommandBuffer* commandBuffer = commandQueue->commandBuffer();
    MTL::RenderPassDescriptor* renderPassDescriptor = view->currentRenderPassDescriptor();
    MTL::RenderCommandEncoder* renderCommandEncoder = commandBuffer->renderCommandEncoder(renderPassDescriptor);
    renderCommandEncoder->setRenderPipelineState(this->renderPipelineState);
    renderCommandEncoder->setVertexBuffer(this->vertexBuffer, 0, 0);
    for (auto& cb : contourBounds) {
        renderCommandEncoder->drawPrimitives(MTL::PrimitiveType::PrimitiveTypeLineStrip, cb.first, cb.second-cb.first);
    }
    renderCommandEncoder->endEncoding();
    
    std::function<void(MTL::CommandBuffer*)> completedHandler = [this](MTL::CommandBuffer* commandBuffer){
        this->frameSemaphore.release();
        this->contourBounds.clear();
    };
    
    commandBuffer->addCompletedHandler(completedHandler);
    commandBuffer->presentDrawable(view->currentDrawable());
    commandBuffer->commit();
    
    autoreleasePool->release();
}

Renderer::~Renderer() {
    commandQueue->release();
    vertexBuffer->release();
    renderPipelineState->release();
    FT_Done_FreeType(ft);
    FT_Done_Face(face);
}
