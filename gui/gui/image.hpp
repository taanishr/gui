//
//  image.hpp
//  gui
//
//  Created by Taanish Reja on 12/19/25.
//

#pragma once
#include "fragment_types.hpp"
#include "printers.hpp"
#include "metal_imports.hpp"
#include "frame_info.hpp"
#include <ranges>
#include "MTKTexture_loader.hpp"
#include <format>
#include "new_arch.hpp"

namespace NewArch {
    struct ImagePoint {
        simd_float2 position;
        simd_float2 uv;
        unsigned int id;
    };

    struct ImageDescriptor {
        ImageDescriptor();

        float width  = 0.0f;
        float height = 0.0f;
        std::string path;
        float cornerRadius = 0.0f;
        float borderWidth = 0.0f;
        simd_float4 borderColor = {0,0,0,1};

        Display display;
        Position position; 
    };

    struct ImageStyleUniforms {
        float cornerRadius;
        float borderWidth;
        simd_float4 borderColor;
    };

    struct ImageGeometryUniforms {
        simd_float2 rectCenter;
        simd_float2 halfExtent;
    };

    struct ImageUniforms {
        ImageStyleUniforms style;
        ImageGeometryUniforms geometry;
    };

    struct ImageStorage {
        ImageStorage(UIContext& ctx):
            atomsBuffer{ctx.allocator.allocate(6 * sizeof(ImagePoint))},
            placementsBuffer{ctx.allocator.allocate(sizeof(simd_float2))},
            uniformsBuffer{ctx.allocator.allocate(sizeof(ImageUniforms))}
        {}

        DrawableBuffer atomsBuffer;
        DrawableBuffer placementsBuffer;
        DrawableBuffer uniformsBuffer;

        NS::SharedPtr<MTL::Texture> texture;
        simd_float2 intrinsicSize {0.0f, 0.0f};
    };

    template <typename S = ImageStorage>
    struct Image {
        Image(UIContext& ctx):
            desc{},
            fragment{ctx}
        {}
        
        ImageDescriptor& getDescriptor()
        {
            return desc;
        }
        
        Fragment<S>& getFragment()
        {
            return fragment;
        }
        
        ImageDescriptor desc;
        Fragment<S> fragment;
    };

    template <typename S = ImageStorage, typename U = ImageUniforms>
    struct ImageProcessor {
        ImageProcessor(UIContext& ctx):
            ctx{ctx}
        {}

        void buildPipeline(MTL::RenderPipelineState*& pipeline)
        {
            MTL::Library* defaultLibrary = ctx.device->newDefaultLibrary();
            MTL::RenderPipelineDescriptor* renderPipelineDescriptor =
                MTL::RenderPipelineDescriptor::alloc()->init();

            MTL::VertexDescriptor* vertexDescriptor = MTL::VertexDescriptor::alloc()->init();

            vertexDescriptor->attributes()->object(0)->setFormat(MTL::VertexFormatFloat2);
            vertexDescriptor->attributes()->object(0)->setOffset(0);
            vertexDescriptor->attributes()->object(0)->setBufferIndex(0);

            vertexDescriptor->attributes()->object(1)->setFormat(MTL::VertexFormatFloat2);
            vertexDescriptor->attributes()->object(1)->setOffset(sizeof(simd_float2));
            vertexDescriptor->attributes()->object(1)->setBufferIndex(0);

            vertexDescriptor->attributes()->object(2)->setFormat(MTL::VertexFormatUInt);
            vertexDescriptor->attributes()->object(2)->setOffset(sizeof(simd_float2) * 2);
            vertexDescriptor->attributes()->object(2)->setBufferIndex(0);

            vertexDescriptor->layouts()->object(0)->setStride(sizeof(ImagePoint));
            renderPipelineDescriptor->setVertexDescriptor(vertexDescriptor);

            MTL::Function* vertexFunction =
                defaultLibrary->newFunction(NS::String::string("vertex_image", NS::UTF8StringEncoding));
            renderPipelineDescriptor->setVertexFunction(vertexFunction);

            MTL::Function* fragmentFunction =
                defaultLibrary->newFunction(NS::String::string("fragment_image", NS::UTF8StringEncoding));
            renderPipelineDescriptor->setFragmentFunction(fragmentFunction);

            renderPipelineDescriptor->colorAttachments()->object(0)->setPixelFormat(ctx.view->colorPixelFormat());
            renderPipelineDescriptor->colorAttachments()->object(0)->setBlendingEnabled(true);
            renderPipelineDescriptor->colorAttachments()->object(0)->setAlphaBlendOperation(MTL::BlendOperationAdd);
            renderPipelineDescriptor->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
            renderPipelineDescriptor->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
            renderPipelineDescriptor->colorAttachments()->object(0)->setSourceAlphaBlendFactor(MTL::BlendFactorSourceAlpha);
            renderPipelineDescriptor->colorAttachments()->object(0)->setDestinationAlphaBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);

            renderPipelineDescriptor->setDepthAttachmentPixelFormat(ctx.view->depthStencilPixelFormat());

            NS::Error* error = nullptr;
            pipeline = ctx.device->newRenderPipelineState(renderPipelineDescriptor, &error);
            if (error != nullptr)
                std::println("error in image pipeline creation: {}", error->localizedDescription()->utf8String());

            defaultLibrary->release();
            renderPipelineDescriptor->release();
            vertexDescriptor->release();
            vertexFunction->release();
            fragmentFunction->release();
        }

        MTL::RenderPipelineState* getPipeline() {
            static std::once_flag initFlag;
            static MTL::RenderPipelineState* pipeline = nullptr;
        
            std::call_once(initFlag, [&](){
                buildPipeline(pipeline);
            });
        
            return pipeline;
        }

        void buildSampler(MTL::SamplerState*& samplerState) {
            MTL::SamplerDescriptor* samplerDescriptor = MTL::SamplerDescriptor::alloc()->init();
            samplerDescriptor->setNormalizedCoordinates(true);
            samplerDescriptor->setMagFilter(MTL::SamplerMinMagFilterLinear);
            samplerDescriptor->setMinFilter(MTL::SamplerMinMagFilterLinear);
            samplerDescriptor->setSAddressMode(MTL::SamplerAddressMode::SamplerAddressModeClampToZero);
            samplerDescriptor->setTAddressMode(MTL::SamplerAddressMode::SamplerAddressModeClampToZero);
            samplerState = ctx.device->newSamplerState(samplerDescriptor);
            samplerDescriptor->release();
        }

        MTL::SamplerState* getSampler() {
            static MTL::SamplerState* sampler = nullptr;
            if (!sampler) buildSampler(sampler);
            return sampler;
        }
        
        MTKTextures::MTKTextureLoader& getTextureLoader() {
            static auto textureLoader = MTKTextures::MTKTextureLoader{ctx.device};
            return textureLoader;
        }


        void initializeTexture(Fragment<S>& fragment, const std::string& path) {
            auto& storage = fragment.fragmentStorage;


            if (storage.texture && storage.intrinsicSize.x > 0.0f) return;

            storage.texture = NS::TransferPtr(
                MTKTextures::createTexture(getTextureLoader(), path)
            );

            if (storage.texture) {
                storage.intrinsicSize = { (float)storage.texture->width(), (float)storage.texture->height() };
            } else {
                storage.intrinsicSize = { 0.0f, 0.0f };
            }

        }

        Measured measure(Fragment<S>& fragment, Constraints&, ImageDescriptor& desc) {
            Measured measured;
            measured.id = fragment.id;

            if (!desc.path.empty()) {
                initializeTexture(fragment, desc.path);
            }

            float width = desc.width;
            float height = desc.height;

            if (width == 0.0f || height == 0.0f) {
                auto intrinsic = fragment.fragmentStorage.intrinsicSize;
                if (intrinsic.x > 0.0f && intrinsic.y > 0.0f) {
                    if (width == 0.0f) width = intrinsic.x;
                    if (height == 0.0f) height = intrinsic.y;
                }
            }

            measured.explicitWidth = width;
            measured.explicitHeight = height;

            return measured;
        }

        Atomized atomize(Fragment<S>& fragment, Constraints&, ImageDescriptor& desc, Measured& measured) {
            std::vector<Atom> atoms;

            float width = measured.explicitWidth;
            float height = measured.explicitHeight;

            auto atomsBuffer = fragment.fragmentStorage.atomsBuffer.get();
            size_t bufferLen = 6 * sizeof(ImagePoint);

            std::array<ImagePoint, 6> points {{
                {{0, 0},                {0, 0}, 0},
                {{width, 0},            {1, 0}, 0},
                {{0, height},           {0, 1}, 0},
                {{0, height},           {0, 1}, 0},
                {{width, 0},            {1, 0}, 0},
                {{width, height},       {1, 1}, 0}
            }};

            std::memcpy(atomsBuffer->contents(), points.data(), bufferLen);

            Atom atom;
            atom.atomBufferHandle = fragment.fragmentStorage.atomsBuffer.handle();
            atom.offset = 0;
            atom.length = bufferLen;
            atom.width = width;
            atom.height = height;
            atoms.push_back(atom);

            return Atomized{ .id = fragment.id, .atoms = atoms };
        }

        LayoutResult layout(Fragment<S>& fragment, Constraints& constraints, ImageDescriptor& desc, Measured& measured, Atomized& atomized) {
            LayoutInput li;
            li.display = desc.display;
            li.position = desc.position;
            auto lr = ctx.layoutEngine.resolve(constraints, li, atomized);

            return lr;
        }

        Placed place(Fragment<S>& fragment, Constraints& constraints, ImageDescriptor& desc, Measured&, Atomized& atomized, LayoutResult& lr) {
            std::vector<AtomPlacement> placements;

            // LayoutInput li;
            
            // li.display = desc.display;
            // li.position = desc.position;

            // auto lr = ctx.layoutEngine.resolve(constraints, li, atomized);
            auto offsets = lr.atomOffsets;
            
            auto placementsBuffer = fragment.fragmentStorage.placementsBuffer.get();
            size_t bufferLen = offsets.size() * sizeof(simd_float2);
            if (bufferLen > 0) {
                std::memcpy(placementsBuffer->contents(), offsets.data(), bufferLen);
            }

            for (int i = 0; i < offsets.size(); ++i) {
                placements.push_back({
                    .placementBufferHandle = fragment.fragmentStorage.placementsBuffer.handle(),
                    .x = offsets[i].x,
                    .y = offsets[i].y
                });
            }

            return Placed{ .id = fragment.id, .placements = placements };
        }

        Finalized<U> finalize(Fragment<S>& fragment, Constraints&, ImageDescriptor& desc, Measured& measured, Atomized& atomized, Placed& placed) {
            ImageStyleUniforms styleUniforms {
                .cornerRadius = desc.cornerRadius,
                .borderWidth = desc.borderWidth,
                .borderColor = desc.borderColor
            };

            ImageGeometryUniforms geometryUniforms;
            
            if (placed.placements.size() > 0) {
                auto offset = placed.placements.front();
                simd_float2 halfExtent { measured.explicitWidth / 2.0f, measured.explicitHeight / 2.0f };
                simd_float2 rectCenter { offset.x + halfExtent.x, offset.y + halfExtent.y };
                
                geometryUniforms.rectCenter = rectCenter;
                geometryUniforms.halfExtent = halfExtent;
            }

            ImageUniforms uniforms {
                .style = styleUniforms,
                .geometry = geometryUniforms
            };

            std::memcpy(fragment.fragmentStorage.uniformsBuffer.get()->contents(), &uniforms, sizeof(ImageUniforms));
            return Finalized<U> {
                .id = fragment.id,
                .atomized = atomized,
                .placed = placed,
                .uniforms = uniforms
            };
        }

        void encode(MTL::RenderCommandEncoder* encoder, Fragment<S>& fragment, Finalized<U>& finalized) {
            auto pipeline = getPipeline();
            encoder->setRenderPipelineState(pipeline);

            auto sampler = getSampler();

            auto atomBuf = fragment.fragmentStorage.atomsBuffer.get();
            auto atomPlacementBuf = fragment.fragmentStorage.placementsBuffer.get();
            auto frameInfoBuf = ctx.frameInfoBuffer.get();
            auto uniformsBuf = fragment.fragmentStorage.uniformsBuffer.get();

            encoder->setVertexBuffer(atomBuf, 0, 0);
            encoder->setVertexBuffer(atomPlacementBuf, 0, 1);
            encoder->setVertexBuffer(frameInfoBuf, 0, 2);

            encoder->setFragmentBuffer(uniformsBuf, 0, 0);

            if (fragment.fragmentStorage.texture) {
                encoder->setFragmentTexture(fragment.fragmentStorage.texture.get(), 0);
            }
            
            encoder->setFragmentSamplerState(sampler, 0);
            encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0), 6);
        }
        UIContext& ctx;
    };
}
