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
    using FragmentID = uint64_t;

    struct Constraints {
        float maxWidth;
        float maxHeight;
        float x;
        float y;
    };

    template <typename S>
    struct Fragment {
        FragmentID id;
        S fragmentStorage;
    };

    struct DivDescriptor {
        float width;
        float height;
        
        simd_float4 color;
        float cornerRadius;
        float borderWidth;
        simd_float4 borderColor;
    };

    struct DivStorage {
        DrawableBuffer atomsBuffer;
        DrawableBuffer placementsBuffer;
        DrawableBuffer uniformsBuffer;
    };

    struct DivStyleUniforms {
        simd_float4 color;
        float cornerRadius;
        float borderWidth;
        simd_float4 borderColor;
    };

    struct DivGeometryUniforms {
        simd_float2 rectCenter;
        simd_float2 halfExtent;
    };

    struct DivUniforms {
        DivStyleUniforms style;
        DivGeometryUniforms geometry;
    };

    struct Measured {
        FragmentID id;
        float explicitWidth;
        float explicitHeight;
    };

    struct Atomized {
        FragmentID id;
        std::vector<Atom> atoms;
    };

    struct Placed {
        FragmentID id;
        std::vector<AtomPlacement> placements;
    };

    template <typename U>
    struct Finalized {
        FragmentID id;
        Atomized atomized;
        Placed placed;
        U Uniforms;
    };

    struct LayoutEngine {
        std::vector<simd_float2> resolve(Constraints& constraints, Atomized atomized);
    };


    struct UIContext {
        UIContext(MTL::Device* device, MTK::View* view);

        MTL::Device* device;
        MTK::View* view;
        DrawableBufferAllocator allocator;
        LayoutEngine layoutEngine;
        DrawableBuffer frameInfoBuffer;
    };
        

    template <typename S = DivStorage, typename U = DivUniforms>
    struct DivProcessor {
        DivProcessor(UIContext& ctx):
            ctx{ctx}
        {}
        
        // pipeline specific
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

        // placement/encoder/etc...
        Measured measure(Fragment<S> fragment, Constraints& constraints, DivDescriptor& desc /* constraints */) {
            Measured measured;
        
            // starting with just one type of measurement; for now, we keep it simple, just desc width and height
            measured.id = fragment.id;
            measured.explicitWidth = desc.width;
            measured.explicitHeight = desc.height;
            
            return measured;
        }
        // resolve percents as explicit width/height, also resolve explicit width/height (like divs default 100% width?)
        
        // won't be problematic since we know constraints before hand... so percents can be resolved easily
        // and also, we know 100% width so divs can be easily resolved. this makes sense
        
        // after we resolved the measurements, we can actually atomize... question is, do I need a new struct in between?
        Atomized atomize(Fragment<S> fragment, Constraints& constraints, DivDescriptor& desc, Measured& measured) {
            std::vector<Atom> atoms {};
            
            // get measurements
            float width = measured.explicitWidth;
            float height = measured.explicitHeight;
            
            // prepare buffer
            auto atomsBuffer = fragment.fragmentStorage.atomsBuffer.get();
            size_t bufferLen = 6*sizeof(AtomPoint);

            std::array<AtomPoint, 6> atomPoints {{
                {{0,0}, 0},
                {{width,0}, 0},
                {{0,height}, 0},
                {{0,height}, 0},
                {{width,0}, 0},
                {{width,height}, 0},
            }};

            // finish allocating atom
            Atom atom;
            atom.atomBufferHandle = fragment.fragmentStorage.atomsBuffer.handle();
            atom.offset = 0;
            atom.length = bufferLen;
            atom.width = width;
            atom.height = height;

            atoms.push_back(atom);
            
            std::memcpy(atomsBuffer->contents(), atomPoints.data(), bufferLen);

            return Atomized{
                .id = fragment.id,
                .atoms = atoms
            };
        }
        
        // TODO: Cache all this
        Placed place(Fragment<S> fragment, Constraints& constraints, DivDescriptor& desc, Measured& measured, Atomized& atomized) // also needs to take in constraints
        {
            std::vector<AtomPlacement> placements;
            
            // simply calls the layout engine; nothing implementation specific here
            auto offsets = ctx.layoutEngine.resolve(constraints, atomized);
            
            // copy placements into buffer
            auto placementsBuffer = fragment.fragmentStorage.placementsBuffer.get();
            size_t bufferLen = sizeof(simd_float2);
            std::memcpy(placementsBuffer->contents(), offsets.data(), bufferLen);
            
            // make the actual placement
            for (int i = 0; i < offsets.size(); ++i) {
                placements.push_back({
                    .placementBufferHandle = fragment.fragmentStorage.placementsBuffer.handle(),
                    .x = offsets[i].x,
                    .y = offsets[i].y
                });
            }
            
            return Placed{
                .id = fragment.id,
                .placements = placements
            };
        }
        
        // finalizes uniforms and makes final object
        Finalized<U> finalize(Fragment<S> fragment, Constraints& constraints, DivDescriptor& desc, Measured& measured, Atomized& atomized, Placed& placed)
        {
            // finalizes uniforms and return finalized object
            
            // style uniforms
            DivStyleUniforms styleUniforms{
                .color = desc.color,
                .cornerRadius = desc.cornerRadius,
                .borderWidth = desc.borderWidth,
                .borderColor = desc.borderColor
            };
            
            // geometry uniforms
            auto offset = placed.placements.front();
            simd_float2 halfExtent { desc.width / 2.0f, desc.height / 2.0f };
            simd_float2 rectCenter { offset.x + halfExtent.x, offset.y + halfExtent.y };
            DivGeometryUniforms geometryUniforms {
                .halfExtent = halfExtent,
                .rectCenter = rectCenter
            };
            
            DivUniforms uniforms {
                .style = styleUniforms,
                .geometry = geometryUniforms
            };
            
            return Finalized {
                .id = fragment.id,
                .atomized = atomized,
                .placed = placed,
                .uniforms = uniforms
            };
        }
        
        void encode(MTL::RenderCommandEncoder* encoder, Fragment<S> fragment, Finalized<U> finalized) {
            auto pipeline = getPipeline();
            encoder->setRenderPipelineState(pipeline);
            
            for (auto [atom, atomPlacement] : zip(finalized.atomized, finalized.placed)) {
                // vertex buffers
                auto atomBuf = fragment.fragmentStorage.atomsBuffer.get();
                auto atomPlacementBuf = fragment.fragmentStorage.placementsBuffer.get();
                auto frameInfoBuf = ctx.frameInfoBuffer.get();
                
                // fragment buffers
                auto uniformsBuf = fragment.fragmentStorage.uniformsBuffer.get();
                
                encoder->setVertexBuffer(atomBuf, 0, 0);
                encoder->setVertexBuffer(atomPlacementBuf, 0, 1);
                encoder->setVertexBuffer(frameInfoBuf, 0, 2);
                
                encoder->setFragmentBuffer(uniformsBuf, 0, 0);
            }
            
            
        }
        
        // Shared resources (optional)
        
        // Shared context
        UIContext& ctx;
    };

}
