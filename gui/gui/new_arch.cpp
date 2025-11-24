//
//  new_arch.cpp
//  gui
//
//  Created by Taanish Reja on 11/20/25.
//

#include "new_arch.hpp"
#include "renderer.hpp"

namespace NewArch {
    DrawableBuffer::DrawableBuffer(MTL::Device* device, unsigned int bufferId, unsigned int size):
        bufferId{bufferId}
    {
        buffer = device->newBuffer(size, MTL::ResourceStorageModeShared);
    }

    DrawableBuffer::~DrawableBuffer() {
        buffer->release();
    }

    DrawableBufferAllocator::DrawableBufferAllocator(MTL::Device* device):
        device{device},
        buffers{}
    {}
    
    BufferHandle DrawableBufferAllocator::allocate(unsigned int size) {
        buffers.emplace(nextId, DrawableBuffer{device, nextId, size});
        return nextId++;
        
        return 0;
    }

    MTL::Buffer* DrawableBufferAllocator::get(BufferHandle handle) {
        auto it = buffers.find(handle);
        
        if (it != buffers.end()) {
            auto db = it->second;
            return db.buffer;
        }
        
        return nullptr;
    }

    Shell::Shell(Renderer& renderer, simd_float4 color, float cornerRadius):
        renderer{renderer},
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
        static int debugCtr = 0;
        
        auto frameInfo = renderer.getFrameInfo();
        
        // TODO
        
//        std::array<QuadPoint, 6> quadPoints {{
//            {.position={layout.computedX,layout.computedY}},
//            {.position={layout.computedX+layout.computedWidth,layout.computedY}},
//            {.position={layout.computedX,layout.computedY+layout.computedHeight}},
//            {.position={layout.computedX,layout.computedY+layout.computedHeight}},
//            {.position={layout.computedX+layout.computedWidth,layout.computedY}},
//            {.position={layout.computedX+layout.computedWidth,layout.computedY+layout.computedHeight}},
//        }};
//        
//        Uniforms uniforms { .color=color,
//                            .rectCenter = layout.center,
//                            .halfExtent = layout.halfExtent,
//                            .cornerRadius = cornerRadius,
//                            .borderColor = borderColor,
//                            .borderWidth = borderWidth};
//
//        
//        
//        std::memcpy(this->frameInfoBuffer->contents(), &frameInfo, sizeof(FrameInfo));
//        std::memcpy(this->uniformsBuffer->contents(), &uniforms, sizeof(Uniforms));
//                    
//        std::memcpy(this->quadPointsBuffer->contents(), quadPoints.data(), 6*sizeof(QuadPoint));
//        
//        debugCtr += 1;
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

    const DrawableSize& Shell::measure() const
    {
        static DrawableSize intrinsicSize {0,0};
        return intrinsicSize;
    }

    const FragmentTemplate Shell::atomize() const {
        std::vector<Atom> atoms;
        return FragmentTemplate {
                .atoms = std::vector<Atom>(),
                .width = 0,
                .height = 0
        };
    }

    bool Shell::contains(simd_float2 point) const
    {
        // TODO
        return true;
    }


    Shell::~Shell() {
        this->quadPointsBuffer->release();
        this->uniformsBuffer->release();
        this->frameInfoBuffer->release();
    }
        
}
