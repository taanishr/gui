//
//  new_arch.cpp
//  gui
//
//  Created by Taanish Reja on 11/20/25.
//

#include "new_arch.hpp"

namespace NewArch {
    UIContext::UIContext(MTL::Device* device, MTK::View* view):
        device{device}, view{view}, allocator{DrawableBufferAllocator{device}}
    {};

    void ShellUniforms::init_shape_dep(float width, float height) {
        this->halfExtent = {
            width / 2.0f,
            height / 2.0f
        };
    }

    void ShellUniforms::init_layout_dep(Layout& layout) {
        this->rectCenter = {
            layout.x + halfExtent.x,
            layout.y + halfExtent.y,
        };
    }

    Shell::Shell(UIContext& ctx, float height, float width):
        ctx{ctx}, height{height}, width{width}
    {}

    void Shell::buildPipeline(MTL::RenderPipelineState*& pipeline) {
        MTL::Library* defaultLibrary = ctx.device->newDefaultLibrary();
        MTL::RenderPipelineDescriptor* renderPipelineDescriptor = MTL::RenderPipelineDescriptor::alloc()->init();
        
        // set up vertex descriptor
        MTL::VertexDescriptor* vertexDescriptor = MTL::VertexDescriptor::alloc()->init();
        vertexDescriptor->attributes()->object(0)->setFormat(MTL::VertexFormat::VertexFormatFloat2);
        vertexDescriptor->attributes()->object(0)->setOffset(0);
        vertexDescriptor->attributes()->object(0)->setBufferIndex(0);
        
        vertexDescriptor->attributes()->object(1)->setFormat(MTL::VertexFormat::VertexFormatUInt);
        vertexDescriptor->attributes()->object(1)->setOffset(sizeof(simd_float2));
        vertexDescriptor->attributes()->object(1)->setBufferIndex(0);

        vertexDescriptor->layouts()->object(0)->setStride(sizeof(AtomPoint));
        
        renderPipelineDescriptor->setVertexDescriptor(vertexDescriptor);

        
        // set up vertex function
        MTL::Function* vertexFunction = defaultLibrary->newFunction(NS::String::string("vertex_shell", NS::UTF8StringEncoding));
        renderPipelineDescriptor->setVertexFunction(vertexFunction);

        // color attachments
        renderPipelineDescriptor->colorAttachments()->object(0)->setPixelFormat(ctx.view->colorPixelFormat());
        renderPipelineDescriptor->colorAttachments()->object(0)->setBlendingEnabled(true);
        renderPipelineDescriptor->colorAttachments()->object(0)->setAlphaBlendOperation(MTL::BlendOperationAdd);
        renderPipelineDescriptor->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
        renderPipelineDescriptor->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
        renderPipelineDescriptor->colorAttachments()->object(0)->setSourceAlphaBlendFactor(MTL::BlendFactorSourceAlpha);
        renderPipelineDescriptor->colorAttachments()->object(0)->setDestinationAlphaBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
        
        
        renderPipelineDescriptor->setDepthAttachmentPixelFormat(ctx.view->depthStencilPixelFormat());
        

        // set up fragment function
        MTL::Function* fragmentFunction = defaultLibrary->newFunction(NS::String::string("fragment_shell", NS::UTF8StringEncoding));
        renderPipelineDescriptor->setFragmentFunction(fragmentFunction);
        
        
        NS::Error* error = nullptr;
        pipeline = ctx.device->newRenderPipelineState(renderPipelineDescriptor, &error);
        
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


    std::vector<Atom> Shell::atomize() {
        std::vector<Atom> atoms {};
        
        // prepare buffer
        unsigned long bufferLen = 6*sizeof(AtomPoint);
        auto bufferHandle = ctx.allocator.allocate(bufferLen);
        auto buf = ctx.allocator.get(bufferHandle);
        
        std::array<AtomPoint, 6> shell_points {{
            {{0,0}, 0},
            {{width,0}, 0},
            {{0,height}, 0},
            {{0,height}, 0},
            {{width,0}, 0},
            {{width,height}, 0},
        }};
        
        std::memcpy(buf->contents(), shell_points.data(), shell_points.size() * sizeof(AtomPoint));
        
        // finish allocating atom
        Atom atom;
        atom.bufferHandle = bufferHandle;
        atom.bufferOffset = 0;
        atom.length = bufferLen;
        atom.width = width;
        atom.height = height;
        
        atoms.push_back(atom);
        
        return atoms;
    }

    LayoutEngine::LayoutEngine(UIContext& ctx):
        ctx{ctx}
    {}

    Encoder::Encoder(UIContext& ctx):
        ctx{ctx}
    {
        // set up frame info buffer
        auto frameDimensions = ctx.view->drawableSize();
        FrameInfo fi {.width=static_cast<float>(frameDimensions.width)/2.0f, .height=static_cast<float>(frameDimensions.height)/2.0f};
        this->frameInfo = ctx.allocator.allocate(sizeof(frameInfo));
        auto fiBuf = ctx.allocator.get(this->frameInfo);
        std::memcpy(fiBuf->contents(), &fi, sizeof(frameInfo));
    }
}
