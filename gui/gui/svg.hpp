//
//  SVG.hpp
//  gui
//
//  Created by Taanish Reja on 4/5/2026.
//

#pragma once
#include "fragment_types.hpp"
#include "frame_buffered_buffer.hpp"
#include "element.hpp"
#include "renderer_constants.hpp"
#include <format>
#include <optional>
#include <map>
#include <print>
#include <simd/vector_types.h>
#include "new_arch.hpp"
#include <any>
#include <utility>
#include <resvg.h>

namespace NewArch {
    struct SVGPoint {
        simd_float2 position;
        simd_float2 uv;
        unsigned int id;
    };

    struct SVGDescriptor {
        SVGDescriptor();

        std::any request(std::any payload) {
            return std::any{};
        }

        std::string path;
    };

    struct SVGStyleUniforms {
        simd_float2 cornerRadius;
        float borderWidth;
        simd_float4 borderColor;
    };

    struct SVGGeometryUniforms {
        simd_float2 rectCenter;
        simd_float2 halfExtent;
    };

    struct SVGUniforms {
        SVGStyleUniforms style;
        SVGGeometryUniforms geometry;
        uint32_t numClips;
    };

    struct SVGStorage {
        SVGStorage(UIContext& ctx):
            atomsBuffer{ctx.allocator, 6*sizeof(SVGPoint), MaxOutstandingFrameCount},
            placementsBuffer{ctx.allocator, sizeof(simd_float2), MaxOutstandingFrameCount},
            uniformsBuffer{ctx.allocator, 6*sizeof(SVGUniforms), MaxOutstandingFrameCount},
            clipsBuffer{ctx.allocator, sizeof(ClipUniform) * 4, MaxOutstandingFrameCount}
        {}

        ~SVGStorage() {
            if (tree) resvg_tree_destroy(tree);
        }
        SVGStorage(SVGStorage&& other):
            atomsBuffer{std::move(other.atomsBuffer)},
            placementsBuffer{std::move(other.placementsBuffer)},
            uniformsBuffer{std::move(other.uniformsBuffer)},
            clipsBuffer{std::move(other.clipsBuffer)},
            tree{std::exchange(other.tree, nullptr)},
            textureCache{std::move(other.textureCache)},
            activeBucket{other.activeBucket},
            lastRenderedSize{other.lastRenderedSize},
            intrinsicSize{other.intrinsicSize}
        {}

        FrameBufferedBuffer<SVGPoint> atomsBuffer;
        FrameBufferedBuffer<simd_float2> placementsBuffer;
        FrameBufferedBuffer<SVGUniforms> uniformsBuffer;
        FrameBufferedBuffer<ClipUniform> clipsBuffer;

        resvg_render_tree* tree {nullptr};
        std::map<uint32_t, NS::SharedPtr<MTL::Texture>> textureCache;
        uint32_t activeBucket {0};
        simd_float2 lastRenderedSize {0.0f, 0.0f};
        simd_float2 intrinsicSize {0.0f, 0.0f};

        MTL::Texture* getActiveTexture() {
            auto it = textureCache.find(activeBucket);
            if (it != textureCache.end()) return it->second.get();
            return nullptr;
        }
    };

    template <typename S = SVGStorage>
    struct SVG {
        SVG(UIContext& ctx):
            desc{},
            fragment{ctx}
        {}

        SVGDescriptor& getDescriptor()
        {
            return desc;
        }

        Fragment<S>& getFragment()
        {
            return fragment;
        }

        std::any request(RequestTarget target, std::any payload) {
            switch (target) {
                case RequestTarget::Descriptor:
                {
                    return desc.request(payload);
                }
                default: {
                    return std::any{};
                }
            }
        }

        SVGDescriptor desc;
        Fragment<S> fragment;

        using StorageType = S;
        using UniformsType = SVGUniforms;
        using DescriptorType = SVGDescriptor;
    };

    template <typename S = SVGStorage, typename U = SVGUniforms>
    struct SVGProcessor {
        SVGProcessor(UIContext& ctx):
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

            vertexDescriptor->layouts()->object(0)->setStride(sizeof(SVGPoint));
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
            renderPipelineDescriptor->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorOne);
            renderPipelineDescriptor->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
            renderPipelineDescriptor->colorAttachments()->object(0)->setSourceAlphaBlendFactor(MTL::BlendFactorOne);
            renderPipelineDescriptor->colorAttachments()->object(0)->setDestinationAlphaBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);

            renderPipelineDescriptor->setDepthAttachmentPixelFormat(ctx.view->depthStencilPixelFormat());

            NS::Error* error = nullptr;
            pipeline = ctx.device->newRenderPipelineState(renderPipelineDescriptor, &error);
            if (error != nullptr)
                std::println("error in SVG pipeline creation: {}", error->localizedDescription()->utf8String());

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

        static constexpr uint32_t BucketSize = 64;

        static uint32_t quantize(float value) {
            uint32_t bucket = static_cast<uint32_t>((value + BucketSize / 2.0f) / BucketSize) * BucketSize;
            return std::max(bucket, BucketSize);
        }

        NS::SharedPtr<MTL::Texture> createTextureFromPixmap(char* pixmap, uint32_t w, uint32_t h) {
            auto* desc = MTL::TextureDescriptor::texture2DDescriptor(
                MTL::PixelFormatRGBA8Unorm,
                w, h, false
            );
            desc->setUsage(MTL::TextureUsageShaderRead);
            desc->setStorageMode(MTL::StorageModeShared);

            auto texture = NS::TransferPtr(ctx.device->newTexture(desc));
            if (texture) {
                MTL::Region region = MTL::Region::Make2D(0, 0, w, h);
                texture->replaceRegion(region, 0, pixmap, w * 4);
            }
            return texture;
        }

        void loadDocument(Fragment<S>& fragment, const std::string& path) {
            auto& storage = fragment.fragmentStorage;
            if (storage.tree) return;

            auto* opt = resvg_options_create();
            int32_t err = resvg_parse_tree_from_file(path.c_str(), opt, &storage.tree);
            resvg_options_destroy(opt);

            if (err != RESVG_OK || !storage.tree) {
                std::println("resvg: failed to parse {}, error={}", path, err);
                storage.tree = nullptr;
                return;
            }

            auto size = resvg_get_image_size(storage.tree);
            storage.intrinsicSize = { size.width, size.height };
        }

        void loadTexture(Fragment<S>& fragment, float width, float height) {
            auto& storage = fragment.fragmentStorage;
            if (!storage.tree) return;

            uint32_t bucket = quantize(width);
            if (bucket == storage.activeBucket && storage.getActiveTexture()) return;

            if (auto it = storage.textureCache.find(bucket); it != storage.textureCache.end()) {
                storage.activeBucket = bucket;
                storage.lastRenderedSize = {width, height};
                return;
            }

            float aspect = storage.intrinsicSize.y / storage.intrinsicSize.x;

            uint32_t renderW = bucket * this->ctx.frameInfo.scale;
            uint32_t renderH = static_cast<uint32_t>(bucket * aspect) * this->ctx.frameInfo.scale;

            float scaleX = static_cast<float>(renderW) / storage.intrinsicSize.x;
            float scaleY = static_cast<float>(renderH) / storage.intrinsicSize.y;

            resvg_transform transform = resvg_transform_identity();
            transform.a = scaleX;
            transform.d = scaleY;

            std::vector<char> pixmap(renderW * renderH * 4, 0);
            resvg_render(storage.tree, transform, renderW, renderH, pixmap.data());

            auto texture = createTextureFromPixmap(pixmap.data(), renderW, renderH);
            if (texture) {
                storage.textureCache[bucket] = std::move(texture);
                storage.activeBucket = bucket;
                storage.lastRenderedSize = {width, height};
            }
        }

        Measured measure(Fragment<S>& fragment, Constraints& constraints, SharedDescriptor& shared, SVGDescriptor& desc) {
            Measured measured;
            measured.id = fragment.id;

            if (!desc.path.empty()) {
                loadDocument(fragment, desc.path);
            }

            SizeResolutionContext sizeCtx {
                .position = shared.position,
                .parentConstraints = constraints,
                .top = shared.top,
                .right = shared.right,
                .bottom = shared.bottom,
                .left = shared.left,
                .requestedWidth = shared.width,
                .requestedHeight = shared.height,
                .availableWidth = constraints.maxWidth,
                .availableHeight = constraints.maxHeight
            };

            ResolvedSize resolvedSize = resolveSize(sizeCtx);

            auto resolvedWidth = resolvedSize.width;
            auto resolvedHeight = resolvedSize.height;

            if (!shared.width || !shared.height) {
                const auto& intrinsic = fragment.fragmentStorage.intrinsicSize;
                if (intrinsic.x > 0.0f && intrinsic.y > 0.0f) {
                    resolvedWidth = intrinsic.x;
                    resolvedHeight = intrinsic.y;
                }
            }

            float w = resolvedWidth.value_or(0.0f);
            float h = resolvedHeight.value_or(0.0f);

            if (w > 0.0f && h > 0.0f) {
                loadTexture(fragment, w, h);
            }

            measured.explicitWidth = resolvedWidth;
            measured.explicitHeight = resolvedHeight;

            return measured;
        }

        Atomized atomize(Fragment<S>& fragment, Constraints&, SharedDescriptor& shared, SVGDescriptor& desc, Measured& measured) {
            std::vector<Atom> atoms;

            float width = measured.explicitWidth.value_or(0.0);
            float height = measured.explicitHeight.value_or(0.0);

            size_t bufferLen = 6 * sizeof(SVGPoint);

            std::array<SVGPoint, 6> points {{
                {{0, 0},                {0, 0}, 0},
                {{width, 0},            {1, 0}, 0},
                {{0, height},           {0, 1}, 0},
                {{0, height},           {0, 1}, 0},
                {{width, 0},            {1, 0}, 0},
                {{width, height},       {1, 1}, 0}
            }};

            fragment.fragmentStorage.atomsBuffer.write(ctx.frameIndex, points.data(), bufferLen);

            Atom atom;
            atom.atomBufferHandle = fragment.fragmentStorage.atomsBuffer.getBufferHandle(ctx.frameIndex);
            atom.offset = 0;
            atom.length = bufferLen;
            atom.width = width;
            atom.height = height;
            atoms.push_back(atom);

            return Atomized{ .id = fragment.id, .atoms = atoms };
        }

        LayoutResult layout(Fragment<S>& fragment, Constraints& constraints, SharedDescriptor& shared, SVGDescriptor& desc, Measured& measured, Atomized& atomized) {
            auto li = toLayoutInput(shared, measured);
            auto lr = ctx.layoutEngine.resolve(constraints, li, atomized);
            return lr;
        }

        Atomized postLayout(Fragment<S>& fragment, Constraints&, SharedDescriptor& shared, SVGDescriptor& desc, Measured& measured, Atomized& atomized, LayoutResult& layout) {
            std::vector<Atom> atoms {};
            
            float width = layout.computedBox.width;
            float height = layout.computedBox.height;

            size_t bufferLen = 6*sizeof(SVGPoint);
            
            std::array<SVGPoint, 6> atomPoints {{
                {{0, 0},           {0, 0}, 0},
                {{width, 0},       {1, 0}, 0},
                {{0, height},      {0, 1}, 0},
                {{0, height},      {0, 1}, 0},
                {{width, 0},       {1, 0}, 0},
                {{width, height},  {1, 1}, 0}
            }};
            
            fragment.fragmentStorage.atomsBuffer.write(ctx.frameIndex, atomPoints.data(), bufferLen);
            
            Atom atom;
            atom.atomBufferHandle = fragment.fragmentStorage.atomsBuffer.getBufferHandle(0);
            atom.offset = 0;
            atom.length = bufferLen;
            atom.width = width;
            atom.height = height;
            
            atoms.push_back(atom);
            
            return Atomized{
                .id = fragment.id,
                .atoms = atoms
            };
        };

        Placed place(Fragment<S>& fragment, Constraints& constraints, SharedDescriptor& shared, SVGDescriptor& desc, Measured&, Atomized& atomized, LayoutResult& lr) {
            std::vector<AtomPlacement> placements;

            auto offsets = lr.atomOffsets;

            size_t bufferLen = offsets.size() * sizeof(simd_float2);
            if (bufferLen > 0) {
                fragment.fragmentStorage.placementsBuffer.write(ctx.frameIndex, offsets.data(), bufferLen);
            }

            for (int i = 0; i < offsets.size(); ++i) {
                placements.push_back({
                    .placementBufferHandle = fragment.fragmentStorage.placementsBuffer.getBufferHandle(ctx.frameIndex),
                    .x = offsets[i].x,
                    .y = offsets[i].y
                });
            }

            return Placed{ .id = fragment.id, .placements = placements };
        }

        Finalized<U> finalize(Fragment<S>& fragment, Constraints& constraints, SharedDescriptor& shared, SVGDescriptor& desc, Measured& measured, Atomized& atomized, LayoutResult& layout, Placed& placed) {
            float borderWidth = 0.0;

            if (shared.borderWidth.unit == Unit::Px) {
                borderWidth = shared.borderWidth.resolveOr(constraints.maxWidth);
            }

            simd_float2 cornerRadius {
                shared.cornerRadius.resolveOr(layout.computedBox.width),
                shared.cornerRadius.resolveOr(layout.computedBox.height)
            };

            SVGStyleUniforms styleUniforms {
                .cornerRadius = cornerRadius,
                .borderWidth = borderWidth,
                .borderColor = shared.borderColor
            };

            SVGGeometryUniforms geometryUniforms;

            if (placed.placements.size() > 0) {
                auto offset = placed.placements.front();
                simd_float2 halfExtent { layout.computedBox.width / 2.0f, layout.computedBox.height / 2.0f };
                simd_float2 rectCenter { offset.x + halfExtent.x, offset.y + halfExtent.y };

                geometryUniforms.rectCenter = rectCenter;
                geometryUniforms.halfExtent = halfExtent;
            }

            SVGUniforms uniforms {
                .style = styleUniforms,
                .geometry = geometryUniforms,
                .numClips = static_cast<uint32_t>(layout.clipUniforms.size())
            };

            fragment.fragmentStorage.uniformsBuffer.write(ctx.frameIndex, &uniforms, sizeof(SVGUniforms));
            fragment.fragmentStorage.clipsBuffer.write(
                ctx.frameIndex,
                layout.clipUniforms.data(),
                sizeof(ClipUniform) * layout.clipUniforms.size()
            );
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

            auto atomBuf = fragment.fragmentStorage.atomsBuffer.getBuffer(ctx.frameIndex);
            auto atomPlacementBuf = fragment.fragmentStorage.placementsBuffer.getBuffer(ctx.frameIndex);
            auto frameInfoBuf = ctx.frameInfoBuffer.get();
            auto uniformsBuf = fragment.fragmentStorage.uniformsBuffer.getBuffer(ctx.frameIndex);
            auto clipsBuf = fragment.fragmentStorage.clipsBuffer.getBuffer(ctx.frameIndex);

            encoder->setVertexBuffer(atomBuf, 0, 0);
            encoder->setVertexBuffer(atomPlacementBuf, 0, 1);
            encoder->setVertexBuffer(frameInfoBuf, 0, 2);

            encoder->setFragmentBuffer(uniformsBuf, 0, 0);
            encoder->setFragmentBuffer(clipsBuf, 0, 1);

            if (auto* tex = fragment.fragmentStorage.getActiveTexture()) {
                encoder->setFragmentTexture(tex, 0);
            }

            encoder->setFragmentSamplerState(sampler, 0);
            encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0), 6);
        }

        std::function<bool(HitTestContext<U>& context, simd_float2 testPoint)> setupHitTestFunction() {
            auto hitTestFunction = [](HitTestContext<U>& context, simd_float2 testPoint){
                return true;
            };

            return hitTestFunction;
        }

        UIContext& ctx;
    };
}
