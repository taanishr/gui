//
//  shell.cpp
//  gui
//
//  Created by Taanish Reja on 8/20/25.
//

#include "shell.hpp"
#include "renderer.hpp"

using namespace ShellRender;

Shell::Shell(Renderer& renderer, float width, float height, float x, float y, simd_float4 color, float cornerRadius):
    renderer{renderer},
    width{width},
    height{height},
    x{x},
    y{y},
    color{color},
    borderColor{color},
    cornerRadius{cornerRadius},
    borderWidth{0}
{
    this->quadPointsBuffer = renderer.device->newBuffer(sizeof(simd_float2)*6, MTL::ResourceStorageModeShared);
    this->uniformsBuffer = renderer.device->newBuffer(sizeof(Uniforms),  MTL::ResourceStorageModeShared);
    this->frameInfoBuffer = renderer.device->newBuffer(sizeof(FrameInfo), MTL::ResourceStorageModeShared);
}

void Shell::update() {
    auto frameInfo = renderer.getFrameInfo();
    
    simd_float2 drawOffset {x, frameInfo.height - y};
    
    std::array<QuadPoint, 6> quadPoints {{
        {.position={drawOffset.x,drawOffset.y}},
        {.position={drawOffset.x+width,drawOffset.y}},
        {.position={drawOffset.x,drawOffset.y+height}},
        {.position={drawOffset.x,drawOffset.y+height}},
        {.position={drawOffset.x+width,drawOffset.y}},
        {.position={drawOffset.x+width,drawOffset.y+height}},
    }};
    
    elementBounds = {.topLeft = {drawOffset.x, drawOffset.y},
        .bottomRight ={drawOffset.x + width, drawOffset.y + height}};
    
    this->center = {drawOffset.x + width/2.0f, drawOffset.y + height/2.0f};
    this->halfExtent = {width / 2.0f, height / 2.0f};
    
    Uniforms uniforms { .color=color,
                        .rectCenter = center,
                        .halfExtent = halfExtent,
                        .cornerRadius = cornerRadius,
                        .borderColor = borderColor,
                        .borderWidth = borderWidth};
    
//    std::println("corner radius: {}, halfExtent x: {}, halfExtent y: {}", cornerRadius, width / 2.0f, height / 2.0f);
    
    
    
    std::memcpy(this->frameInfoBuffer->contents(), &frameInfo, sizeof(FrameInfo));
    std::memcpy(this->uniformsBuffer->contents(), &uniforms, sizeof(Uniforms));
                
    std::memcpy(this->quadPointsBuffer->contents(), quadPoints.data(), 6*sizeof(QuadPoint));
}

void Shell::buildPipeline(MTL::RenderPipelineState*& pipeline) {
    MTL::Library* defaultLibrary = renderer.device->newDefaultLibrary();
    MTL::RenderPipelineDescriptor* renderPipelineDescriptor = MTL::RenderPipelineDescriptor::alloc()->init();
    
    // set up vertex descriptor
    MTL::VertexDescriptor* vertexDescriptor = MTL::VertexDescriptor::alloc()->init();
    vertexDescriptor->attributes()->object(0)->setFormat(MTL::VertexFormat::VertexFormatFloat2);
    vertexDescriptor->attributes()->object(0)->setOffset(0);
    vertexDescriptor->attributes()->object(0)->setBufferIndex(0);

    vertexDescriptor->layouts()->object(0)->setStride(sizeof(QuadPoint));
    
    renderPipelineDescriptor->setVertexDescriptor(vertexDescriptor);

    
    // set up vertex function
    MTL::Function* vertexFunction = defaultLibrary->newFunction(NS::String::string("vertex_shell", NS::UTF8StringEncoding));
    renderPipelineDescriptor->setVertexFunction(vertexFunction);

    //
    renderPipelineDescriptor->colorAttachments()->object(0)->setPixelFormat(renderer.view->colorPixelFormat());
    renderPipelineDescriptor->colorAttachments()->object(0)->setBlendingEnabled(true);
    renderPipelineDescriptor->colorAttachments()->object(0)->setAlphaBlendOperation(MTL::BlendOperationAdd);
    renderPipelineDescriptor->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
    renderPipelineDescriptor->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
    renderPipelineDescriptor->colorAttachments()->object(0)->setSourceAlphaBlendFactor(MTL::BlendFactorSourceAlpha);
    renderPipelineDescriptor->colorAttachments()->object(0)->setDestinationAlphaBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
    
    
    renderPipelineDescriptor->setDepthAttachmentPixelFormat(renderer.view->depthStencilPixelFormat());
    

    // set up fragment function
    MTL::Function* fragmentFunction = defaultLibrary->newFunction(NS::String::string("fragment_shell", NS::UTF8StringEncoding));
    renderPipelineDescriptor->setFragmentFunction(fragmentFunction);
    
    
    NS::Error* error = nullptr;
    pipeline = renderer.device->newRenderPipelineState(renderPipelineDescriptor, &error);
    
    if (error != nullptr)
        std::println("error in pipeline creation: {}", error->localizedDescription()->utf8String());
    
    
    defaultLibrary->release();
    renderPipelineDescriptor->release();
    vertexDescriptor->release();
    vertexFunction->release();
}

MTL::RenderPipelineState* Shell::getPipeline() {
    static MTL::RenderPipelineState* pipeline = nullptr;
    
    if (!pipeline)
        buildPipeline(pipeline);
    
    return pipeline;
}

void Shell::encode(MTL::RenderCommandEncoder* encoder) {
    auto pipeline = getPipeline();
    encoder->setRenderPipelineState(pipeline);

    encoder->setVertexBuffer(this->quadPointsBuffer, 0, 0);
    encoder->setVertexBuffer(this->frameInfoBuffer, 0, 1);
    
    encoder->setFragmentBuffer(this->uniformsBuffer, 0, 0);
    
    encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0), 6);
}


const Bounds& Shell::bounds() const
{
    return elementBounds;
}

bool Shell::contains(simd_float2 point) const
{
    simd_float2 localPoint {point.x - center.x, point.y - center.y};
    return rounded_rect_sdf(localPoint, halfExtent, cornerRadius) < 0;
}

Shell::~Shell() {
    this->quadPointsBuffer->release();
    this->uniformsBuffer->release();
    this->frameInfoBuffer->release();
}
