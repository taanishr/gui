//
//  renderer.cpp
//  font-rendering
//
//  Created by Taanish Reja on 7/21/25.
//

#include "renderer.hpp"
#include <iostream>
#include <ranges>

std::vector<simd_float2> controlPoints {simd_float2{0,0},simd_float2{0.25,0.5},simd_float2{0.5,0}};

void printPoint(const simd_float2& pt, bool newLine) {
    std::string end = newLine ? "\n" : "";
    std::print("({},{}){}", pt[0], pt[1], end);
}

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
    renderPipelineDescriptor->colorAttachments()->object(0)->setBlendingEnabled(true);
    renderPipelineDescriptor->colorAttachments()->object(0)->setAlphaBlendOperation(MTL::BlendOperationAdd);
    renderPipelineDescriptor->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
    renderPipelineDescriptor->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
    renderPipelineDescriptor->colorAttachments()->object(0)->setSourceAlphaBlendFactor(MTL::BlendFactorSourceAlpha);
    renderPipelineDescriptor->colorAttachments()->object(0)->setDestinationAlphaBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
    
    renderPipelineDescriptor->setDepthAttachmentPixelFormat(this->view->depthStencilPixelFormat());
    

    // set up fragment function
    MTL::Function* fragmentFunction = defaultLibrary->newFunction(NS::String::string("fragment_main", NS::UTF8StringEncoding));
    renderPipelineDescriptor->setFragmentFunction(fragmentFunction);
    
    
    NS::Error* error = nullptr;
    this->renderPipelineState = this->device->newRenderPipelineState(renderPipelineDescriptor, &error);
    
    if (error != nullptr)
        std::println("error in pipeline creation: {}", error->localizedDescription()->utf8String());
    
    
    defaultLibrary->release();
    renderPipelineDescriptor->release();
    vertexDescriptor->release();
    vertexFunction->release();    
}

void Renderer::makeResources() {
    FT_Init_FreeType(&(this->ft));
    FT_New_Face(ft, fontPath.data(), 0, &(this->face));
    FT_Set_Pixel_Sizes(face, 0, 48);
    this->glyphQuadBuffer = this->device->newBuffer(sizeof(simd_float2)*4, MTL::StorageModeShared);
    this->glyphContoursBuffer = this->device->newBuffer(sizeof(simd_float2)*4096, MTL::StorageModeShared);
    this->constantsBuffer = this->device->newBuffer(sizeof(Constants), MTL::StorageModeShared);
    this->contourBoundsBuffer =  this->device->newBuffer(sizeof(ContourBounds)*256, MTL::StorageModeShared);
}

void Renderer::updateConstants() {
    std::vector<simd_float2> glyphQuadPoints {};
    std::vector<simd_float2> glyphOutlinePoints {};
    std::vector<std::vector<simd_float2>> contours {};
    std::vector<ContourBounds> contourBounds {};
    
    float penX = -8192.0;
    for (auto ch: str) {
        FT_Load_Char(face, ch, FT_LOAD_RENDER);
        auto chContours = drawContours(ch, this->face, fontPath, penX);
        contours.insert(contours.end(), chContours.begin(), chContours.end());
        penX += face->glyph->advance.x;
    }
    
    
    Constants c;
    
    c.scaling = 10000.0;
    
    c.nPoints = 0;
    
    c.numContours = contours.size();
    
    
    float minX = std::numeric_limits<float>::infinity();
    float maxX = -std::numeric_limits<float>::infinity();
    float minY = std::numeric_limits<float>::infinity();
    float maxY = -std::numeric_limits<float>::infinity();
    
    unsigned long contourStart = 0;
    
    for (auto& contour: contours) {
        c.nPoints += contour.size();
        glyphOutlinePoints.insert(glyphOutlinePoints.end(), contour.begin(), contour.end());
        
        for (auto pt: contour) {
            if (pt[0] < minX)
                minX = pt[0];
            
            if (pt[0] > maxX)
                maxX = pt[0];
            
            if (pt[1] < minY)
                minY = pt[1];
            
            if (pt[1] > maxY)
                maxY = pt[1];
        }
    
        contourBounds.push_back({.start = contourStart, .end = contourStart+contour.size()});
        contourStart += contour.size();
    }
    
    simd_float2 topLeft {minX, minY};
    simd_float2 topRight {maxX, minY};
    simd_float2 bottomLeft {minX, maxY};
    simd_float2 bottomRight {maxX, maxY};
    
    glyphQuadPoints.push_back(topLeft);
    glyphQuadPoints.push_back(topRight);
    glyphQuadPoints.push_back(bottomLeft);
    glyphQuadPoints.push_back(bottomRight);

    Constants* rawConstantsPtr = static_cast<Constants*>(constantsBuffer->contents());
    
    *rawConstantsPtr = c;
    
    std::memcpy(this->glyphQuadBuffer->contents(), glyphQuadPoints.data(), sizeof(simd_float2)*glyphQuadPoints.size());
    std::memcpy(this->glyphContoursBuffer->contents(), glyphOutlinePoints.data(), sizeof(simd_float2)*glyphOutlinePoints.size());
    std::memcpy(this->contourBoundsBuffer->contents(), contourBounds.data(), sizeof(contourBounds)*contourBounds.size());
}

void Renderer::draw() {
    NS::AutoreleasePool* autoreleasePool = NS::AutoreleasePool::alloc()->init();
    
    this->frameSemaphore.acquire();
    
    updateConstants();
    
    MTL::CommandBuffer* commandBuffer = commandQueue->commandBuffer();
    MTL::RenderPassDescriptor* renderPassDescriptor = view->currentRenderPassDescriptor();
    MTL::RenderCommandEncoder* renderCommandEncoder = commandBuffer->renderCommandEncoder(renderPassDescriptor);
    renderCommandEncoder->setRenderPipelineState(this->renderPipelineState);
    
    renderCommandEncoder->setVertexBuffer(this->glyphQuadBuffer, 0, 0);
    renderCommandEncoder->setVertexBuffer(this->constantsBuffer, 0, 1);
    
    renderCommandEncoder->setFragmentBuffer(this->glyphContoursBuffer, 0, 1);
    renderCommandEncoder->setFragmentBuffer(this->contourBoundsBuffer, 0, 2);
    renderCommandEncoder->setFragmentBuffer(this->constantsBuffer, 0, 3);
    
    renderCommandEncoder->drawPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangleStrip, NS::UInteger(0), NS::UInteger(4));
    
    renderCommandEncoder->endEncoding();
    
    std::function<void(MTL::CommandBuffer*)> completedHandler = [this](MTL::CommandBuffer* commandBuffer){
        this->frameSemaphore.release();
        this->quadBounds.clear();
    };
    
    commandBuffer->addCompletedHandler(completedHandler);
    commandBuffer->presentDrawable(view->currentDrawable());
    commandBuffer->commit();
    
    autoreleasePool->release();
}

Renderer::~Renderer() {
    commandQueue->release();
    glyphQuadBuffer->release();
    glyphContoursBuffer->release();
    constantsBuffer->release();
    contourBoundsBuffer->release();
    renderPipelineState->release();
    FT_Done_FreeType(ft);
    FT_Done_Face(face);
}
