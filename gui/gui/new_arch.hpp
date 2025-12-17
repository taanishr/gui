//
//  newarch.hpp
//  gui
//
//  Created by Taanish Reja on 11/20/25.
//

#pragma once
#include "fragment_types.hpp"
#include "metal_imports.hpp"
#include "frame_info.hpp"
#include <ranges>

using namespace std::ranges::views;

class Renderer;

namespace NewArch {
    struct UIContext {
        UIContext(MTL::Device* device, MTK::View* view);
        
        MTL::Device* device;
        MTK::View* view;
        DrawableBufferAllocator allocator;
    };

    struct Cursor {
        simd_float2 position;
    };

    enum LengthType {
        px=0,
        pct=1
    };

    struct Length {
        float value;
        LengthType type;
    };

    struct Layout {
        float parentWidth;
        float parentHeight;
        float x;
        float y;
    };

    struct ShellStyleUniforms {
        simd_float4 color;
        float cornerRadius;
        float borderWidth;
        simd_float4 borderColor;
    };

    struct ShellGeometryUniforms {
        simd_float2 rectCenter;
        simd_float2 halfExtent;
    };

    struct ShellUniforms {
        ShellStyleUniforms style;
        ShellGeometryUniforms geometry;
    };
        
    struct AtomPoint {
        simd_float2 point;
        unsigned int id;
    };


    // shell class does what?
    // initializes fragment template (shell-specific)
    // atomizes (shell-specific)
    // creates uniforms (shell-specific)
    // builds pipelines

    // layout engine does what?
    // takes fragment template and figures out atom placements.
    // should the layout engine resize the buffer? Should it call into a shell method that does that?

    // then the problems with text come up; should we just globablly share a text buffer and use indexed buffers? Then encoding becomes specific? Unless we make all draws indexed? Which makes sense, because it would save

    struct Shell {
        Shell(UIContext& ctx):
            ctx{ctx}
        {
            atomBufferHandle = ctx.allocator.allocate(6*sizeof(AtomPoint));
            placementBufferHandle = ctx.allocator.allocate(sizeof(simd_float2));
            uniformsBufferHandle = ctx.allocator.allocate(sizeof(ShellUniforms));
        }

        void buildPipeline(MTL::RenderPipelineState*& pipeline) {
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
        
        
        MTL::RenderPipelineState* getPipeline() {
            static MTL::RenderPipelineState* pipeline = nullptr;
            
            if (!pipeline)
                buildPipeline(pipeline);
            
            return pipeline;
        }
    
        // atomizes shell
        std::vector<Atom> atomize();
        void place(std::vector<AtomPlacements>& placements);
        FragmentTemplate& finalize();
        
        // Shell metadata
        // explicit dimensions
        float width;
        float height;
        
        simd_float4 color;
        float cornerRadius;
        float borderWidth;
        simd_float4 borderColor;
        
        // buffers
        BufferHandle atomBufferHandle;
        BufferHandle placementBufferHandle;
        BufferHandle uniformsBufferHandle;
        
        FragmentTemplate& fragTemplatae;;
        
        // ctx
        UIContext& ctx;
    };

    struct LayoutEngine {
        LayoutEngine(UIContext& ctx);
        
        
        std::vector<AtomPlacement> place(
            std::vector<Atom>& atoms,
            BufferHandle placementBufferHandle,
            Layout& layout
        )
        {
            auto placementBuffer = ctx.allocator.get(placementBufferHandle);
            
            
            simd_float2 cursor {
                layout.x,
                layout.y
            };
            
            // simple method first
            float running_width = 0.0;
            float running_height = 0.0;
            
            std::vector<simd_float2> raw_placements;
            std::vector<AtomPlacement> placements;
            
            for (auto i = 0; i < atoms.size(); ++i) {
                auto& curr_atom = atoms[i];
                AtomPlacement placement;
                placement.x = cursor.x;
                placement.y = cursor.y;
                placement.placementBufferHandle = placementBufferHandle;
                
                raw_placements.push_back(cursor);
                placements.push_back(placement);
                
                running_width += curr_atom.width;
                running_height = std::max(running_height, curr_atom.height);
                cursor.x += curr_atom.width;
            }
            
            std::memcpy(placementBuffer->contents(), raw_placements.data(), raw_placements.size()*sizeof(simd_float2));
            
            return placements;
        }
        
        UIContext& ctx;
    };


    struct Encoder {
        Encoder(UIContext& ctx);
        
        template <typename T>
        void encode(MTL::RenderCommandEncoder* renderCommandEncoder, MTL::RenderPipelineState* pipeline, FragmentTemplate<T>& ft) {
            renderCommandEncoder->setRenderPipelineState(pipeline);
            
            for (auto [atom, atomPlacement] : zip(ft.atoms, ft.atomPlacements)) {
                // vertex buffers
                auto atomBuf = ctx.allocator.get(atom.bufferHandle);
                auto atomPlacementBuf = ctx.allocator.get(atomPlacement.placementBufferHandle);
                auto frameInfoBuf = ctx.allocator.get(this->frameInfo);
                
                // fragment buffers
                auto uniformsBuf = ctx.allocator.get(ft.uniforms.uniformsBufferHandle);
                
                renderCommandEncoder->setVertexBuffer(atomBuf, 0, 0);
                renderCommandEncoder->setVertexBuffer(atomPlacementBuf, 0, 1);
                renderCommandEncoder->setVertexBuffer(frameInfoBuf, 0, 2);

                renderCommandEncoder->setFragmentBuffer(uniformsBuf, 0, 0);
                
                auto frameDimensions = ctx.view->drawableSize();
                FrameInfo fi {.width=static_cast<float>(frameDimensions.width)/2.0f, .height=static_cast<float>(frameDimensions.height)/2.0f};
                assert(std::memcmp(frameInfoBuf->contents(), &fi, sizeof(FrameInfo)) == 0);
                
                renderCommandEncoder->drawPrimitives(MTL::PrimitiveTypeTriangle, atom.bufferOffset, atom.length / sizeof(AtomPoint));
            }
        }
        
        BufferHandle frameInfo;
        UIContext& ctx;
    };

}
