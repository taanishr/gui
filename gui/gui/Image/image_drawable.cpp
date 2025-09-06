//
//  image.cpp
//  gui
//
//  Created by Taanish Reja on 8/27/25.
//

#include "image_drawable.hpp"
#include "renderer.hpp"

using namespace ImageRender;

ImageDrawable::ImageDrawable(Renderer& renderer, const std::string& path):
    renderer{renderer},
    path{path},
    x{0},
    y{0},
    width{0},
    height{0},
    cornerRadius{0},
    borderWidth{0},
    borderColor{simd_float4{0,0,0,1}}
{
    this->texture = NS::TransferPtr(
        MTKTextures::createTexture(getTextureLoader(), path)
    );
    
    this->quadPointsBuffer = renderer.device->newBuffer(6*sizeof(QuadPoint), MTL::StorageModeShared);
    this->frameInfoBuffer = renderer.device->newBuffer(sizeof(FrameInfo), MTL::ResourceStorageModeShared);
    this->uniformsBuffer = renderer.device->newBuffer(sizeof(Uniforms), MTL::ResourceStorageModeShared);
}

MTKTextures::MTKTextureLoader& ImageDrawable::getTextureLoader() {
    static auto textureLoader = MTKTextures::MTKTextureLoader{renderer.device};
    return textureLoader;
}

void ImageDrawable::buildPipeline(MTL::RenderPipelineState*& pipeline)
{
    MTL::Library* defaultLibrary = renderer.device->newDefaultLibrary();
    MTL::RenderPipelineDescriptor* renderPipelineDescriptor = MTL::RenderPipelineDescriptor::alloc()->init();
    
    // set up vertex descriptor
    MTL::VertexDescriptor* vertexDescriptor = MTL::VertexDescriptor::alloc()->init();
    vertexDescriptor->attributes()->object(0)->setFormat(MTL::VertexFormat::VertexFormatFloat2);
    vertexDescriptor->attributes()->object(0)->setOffset(0);
    vertexDescriptor->attributes()->object(0)->setBufferIndex(0);
    
    vertexDescriptor->attributes()->object(1)->setFormat(MTL::VertexFormat::VertexFormatFloat2);
    vertexDescriptor->attributes()->object(1)->setOffset(sizeof(simd_float2));
    vertexDescriptor->attributes()->object(1)->setBufferIndex(0);
    
    vertexDescriptor->layouts()->object(0)->setStride(sizeof(QuadPoint));
    
    renderPipelineDescriptor->setVertexDescriptor(vertexDescriptor);

    
    // set up vertex function
    MTL::Function* vertexFunction = defaultLibrary->newFunction(NS::String::string("vertex_image", NS::UTF8StringEncoding));
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
    MTL::Function* fragmentFunction = defaultLibrary->newFunction(NS::String::string("fragment_image", NS::UTF8StringEncoding));
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

MTL::RenderPipelineState* ImageDrawable::getPipeline()
{
    static MTL::RenderPipelineState* pipeline = nullptr;
    
    if (!pipeline)
        buildPipeline(pipeline);
    
    return pipeline;
}

void ImageDrawable::buildSampler(MTL::SamplerState*& samplerState)
{
    MTL::SamplerDescriptor* samplerDescriptor = MTL::SamplerDescriptor::alloc()->init();
    
    samplerDescriptor->setNormalizedCoordinates(true);
    samplerDescriptor->setMagFilter(MTL::SamplerMinMagFilterLinear);
    samplerDescriptor->setMinFilter(MTL::SamplerMinMagFilterLinear);
    samplerDescriptor->setSAddressMode(MTL::SamplerAddressMode::SamplerAddressModeClampToZero);
    samplerDescriptor->setTAddressMode(MTL::SamplerAddressMode::SamplerAddressModeClampToZero);
    
    samplerState = renderer.device->newSamplerState(samplerDescriptor);
    
    samplerDescriptor->release();
}

MTL::SamplerState* ImageDrawable::getSampler()
{
    static MTL::SamplerState* sampler = nullptr;

    if (!sampler)
        buildSampler(sampler);
    
    return sampler;
}

void ImageDrawable::update(const LayoutBox& layoutBox) {
    auto frameInfo = renderer.getFrameInfo();
    
    simd_float2 drawOffset {layoutBox.x, frameInfo.height - layoutBox.y};
    
    std::array<QuadPoint, 6> quadPoints {{
        {.position={drawOffset.x,drawOffset.y}, .uv={0,0}},
        {.position={drawOffset.x+layoutBox.width,drawOffset.y}, .uv={1,0}},
        {.position={drawOffset.x,drawOffset.y+layoutBox.height}, .uv{0,1}},
        {.position={drawOffset.x,drawOffset.y+layoutBox.height}, .uv{0,1}},
        {.position={drawOffset.x+layoutBox.width,drawOffset.y}, .uv={1,0}},
        {.position={drawOffset.x+layoutBox.width,drawOffset.y+layoutBox.height}, .uv={1,1}},
    }};
    
    this->center = {drawOffset.x + layoutBox.width/2.0f, drawOffset.y + layoutBox.height/2.0f};
    this->halfExtent = {layoutBox.width / 2.0f, layoutBox.height / 2.0f};
    
    Uniforms uniforms { .rectCenter = center,
                        .halfExtent = halfExtent,
                        .cornerRadius = cornerRadius,
                        .borderWidth = borderWidth,
                        .borderColor = borderColor};
    
    elementBounds = {.topLeft = {drawOffset.x, drawOffset.y},
        .bottomRight ={drawOffset.x + layoutBox.width, drawOffset.y + layoutBox.height}};
    
    std::memcpy(this->frameInfoBuffer->contents(), &frameInfo, sizeof(FrameInfo));
    std::memcpy(quadPointsBuffer->contents(), quadPoints.data(), sizeof(QuadPoint)*quadPoints.size());
    std::memcpy(this->uniformsBuffer->contents(), &uniforms, sizeof(Uniforms));
}

void ImageDrawable::encode(MTL::RenderCommandEncoder* encoder)
{
    auto pipeline = getPipeline();
    
    encoder->setRenderPipelineState(pipeline);
    
    auto sampler = getSampler();
    
    encoder->setVertexBuffer(this->quadPointsBuffer, 0, 0);
    encoder->setVertexBuffer(this->frameInfoBuffer, 0, 1);
    
    encoder->setFragmentBuffer(this->uniformsBuffer, 0, 0);
    encoder->setFragmentTexture(texture.get(), 0);
    encoder->setFragmentSamplerState(sampler, 0);
    
    encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::Integer(0), 6);
}

const Bounds& ImageDrawable::bounds() const
{
    return elementBounds;
}

bool ImageDrawable::contains(simd_float2 point) const
{
    simd_float2 localPoint {point.x - center.x, point.y - center.y};
    return rounded_rect_sdf(localPoint, halfExtent, 0) < 0;
}

ImageDrawable::~ImageDrawable() {
    uniformsBuffer->release();
    frameInfoBuffer->release();
    quadPointsBuffer->release();
}

