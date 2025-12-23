//
//  Untitled.hpp
//  gui
//
//  Created by Taanish Reja on 12/19/25.
//

#pragma once
#include "metal_imports.hpp"
#include "freetype.hpp"
#include "glyphs.hpp"
#include "frame_info.hpp"
#include "glyphCache.hpp"
#include <ranges>
#include "new_arch.hpp"

namespace NewArch {
    struct TextPoint {
        simd_float2 point;
        int metadataIndex;
        int id;
    };
}

template <>
struct std::formatter<NewArch::TextPoint> : std::formatter<float>
{
    auto format(const NewArch::TextPoint& v, format_context& ctx) const
    {
        return std::format_to(ctx.out(), "(x: {}, y: {}, mi: {} id: {})", v.point.x, v.point.y, v.metadataIndex, v.id);
    }
};

namespace NewArch {
    struct TextDescriptor {
        std::string text;
        std::string font;
        simd_float4 color;
        float fontSize;
    };

    struct TextUniforms {
        simd_float4 color;
    };

    struct TextStorage {
        TextStorage(UIContext& ctx):
            atomsBuffer{ ctx.allocator.allocate(6 * sizeof(TextPoint) * 4) },
            placementsBuffer{ ctx.allocator.allocate(sizeof(simd_float2) * 4) },
            uniformsBuffer{ ctx.allocator.allocate(sizeof(TextUniforms)) },
            metadataBuffer{ ctx.allocator.allocate(sizeof(int) * 16) }
        {}

        DrawableBuffer atomsBuffer;
        DrawableBuffer placementsBuffer;
        DrawableBuffer uniformsBuffer;

        DrawableBuffer metadataBuffer;
    };

    template <typename S = TextStorage>
    struct Text {
        Text(UIContext& ctx):
            desc{},
            fragment{ctx}
        {}
        
        TextDescriptor& getDescriptor()
        {
            return desc;
        }
        
        Fragment<S>& getFragment()
        {
            return fragment;
        }
        
        TextDescriptor desc;
        Fragment<S> fragment;
    };

    template <typename S = TextStorage, typename U = TextUniforms>
    struct TextProcessor {
        TextProcessor(UIContext& ctx):
            glyphCache{nullptr},
            lastGlyphsBufferOffset{0},
            glyphBuffer{ctx.allocator.allocate(4096)},
            ctx{ctx}
        {
            FT_Init_FreeType(&(this->ft));
            
            glyphCache = GlyphCache{this->ft};
        }
        
        void buildPipeline(MTL::RenderPipelineState*& pipeline) {
            MTL::Library* defaultLibrary = ctx.device->newDefaultLibrary();
            MTL::RenderPipelineDescriptor* renderPipelineDescriptor = MTL::RenderPipelineDescriptor::alloc()->init();
        
            // set up vertex descriptor
            MTL::VertexDescriptor* vertexDescriptor = MTL::VertexDescriptor::alloc()->init();
            vertexDescriptor->attributes()->object(0)->setFormat(MTL::VertexFormat::VertexFormatFloat2);
            vertexDescriptor->attributes()->object(0)->setOffset(0);
            vertexDescriptor->attributes()->object(0)->setBufferIndex(0);
        
            vertexDescriptor->attributes()->object(1)->setFormat(MTL::VertexFormat::VertexFormatInt);
            vertexDescriptor->attributes()->object(1)->setOffset(sizeof(simd_float2));
            vertexDescriptor->attributes()->object(1)->setBufferIndex(0);
        
            vertexDescriptor->attributes()->object(2)->setFormat(MTL::VertexFormat::VertexFormatInt);
            vertexDescriptor->attributes()->object(2)->setOffset(sizeof(simd_float2) + sizeof(int));
            vertexDescriptor->attributes()->object(2)->setBufferIndex(0);
    
        
            vertexDescriptor->layouts()->object(0)->setStride(sizeof(TextPoint));
        
            renderPipelineDescriptor->setVertexDescriptor(vertexDescriptor);
        
        
            // set up vertex function
            MTL::Function* vertexFunction = defaultLibrary->newFunction(NS::String::string("vertex_text", NS::UTF8StringEncoding));
            renderPipelineDescriptor->setVertexFunction(vertexFunction);
        
            renderPipelineDescriptor->colorAttachments()->object(0)->setPixelFormat(ctx.view->colorPixelFormat());
            renderPipelineDescriptor->colorAttachments()->object(0)->setBlendingEnabled(true);
            renderPipelineDescriptor->colorAttachments()->object(0)->setAlphaBlendOperation(MTL::BlendOperationAdd);
            renderPipelineDescriptor->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
            renderPipelineDescriptor->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
            renderPipelineDescriptor->colorAttachments()->object(0)->setSourceAlphaBlendFactor(MTL::BlendFactorSourceAlpha);
            renderPipelineDescriptor->colorAttachments()->object(0)->setDestinationAlphaBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
        
        
            renderPipelineDescriptor->setDepthAttachmentPixelFormat(ctx.view->depthStencilPixelFormat());
        
        
            // set up fragment function
            MTL::Function* fragmentFunction = defaultLibrary->newFunction(NS::String::string("fragment_text", NS::UTF8StringEncoding));
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
        
        Measured measure(Fragment<S>& fragment, Constraints& constraints, TextDescriptor& desc) {
            Measured measured;
            measured.id = fragment.id;
            
            measured.explicitWidth  = 0.0;
            measured.explicitHeight = 0.0;

            return measured;
        }
        
        std::array<TextPoint, 6> makeAtomPoints(const Quad& quad, int metadataIndex, int id) {
            return std::array<TextPoint,6>{
                TextPoint{ .point = quad.topLeft, .metadataIndex = metadataIndex, .id = id },
                TextPoint{ .point = simd_float2{ quad.bottomRight.x, quad.topLeft.y }, .metadataIndex = metadataIndex, .id = id },
                TextPoint{ .point = simd_float2{ quad.topLeft.x,    quad.bottomRight.y }, .metadataIndex = metadataIndex, .id = id },
                TextPoint{ .point = simd_float2{ quad.topLeft.x,    quad.bottomRight.y }, .metadataIndex = metadataIndex, .id = id },
                TextPoint{ .point = simd_float2{ quad.bottomRight.x, quad.topLeft.y }, .metadataIndex = metadataIndex, .id = id },
                TextPoint{ .point = quad.bottomRight, .metadataIndex = metadataIndex, .id = id }
            };
        }
        
        Atomized atomize(Fragment<S>& fragment, Constraints&, TextDescriptor& desc, Measured&) {
            std::vector<Atom> atoms;
            std::vector<TextPoint> allAtomPoints;
            std::vector<int> metadata;
            allAtomPoints.reserve(desc.text.size() * 6);

            for (size_t i = 0; i < desc.text.size(); ++i) {
                char ch = desc.text[i];
                Atom atom;

                GlyphQuery glyphQuery { ch, desc.font, .fontSize = desc.fontSize };
                auto glyph = glyphCache.retrieve(glyphQuery);

                size_t pointsLenBytes = glyph.points.size() * sizeof(simd_float2);

                auto offset_it = glyphBufferOffsets.find(glyphQuery);
                if (offset_it == glyphBufferOffsets.end()) {
                    auto rawGlyphBuffer = glyphBuffer.get();
                    size_t requiredSize = lastGlyphsBufferOffset + pointsLenBytes;

                    if (requiredSize > rawGlyphBuffer->length()) {
                        ctx.allocator.resize(glyphBuffer, requiredSize);
                        rawGlyphBuffer = glyphBuffer.get();
                    }

                    auto copyStart =
                        reinterpret_cast<std::byte*>(rawGlyphBuffer->contents()) + lastGlyphsBufferOffset;

                    std::memcpy(copyStart, glyph.points.data(), pointsLenBytes);

                    glyphBufferOffsets[glyphQuery] = lastGlyphsBufferOffset / sizeof(simd_float2);
                    offset_it = glyphBufferOffsets.find(glyphQuery);
                    lastGlyphsBufferOffset += pointsLenBytes;
                }

                size_t offset = offset_it->second;

                int metadataIndex = metadata.size();

                metadata.push_back(offset);
                metadata.push_back(glyph.numContours);
                for (auto& contourSize : glyph.contourSizes) {
                    metadata.push_back(contourSize);
                }

                auto atomPts = makeAtomPoints(glyph.quad, metadataIndex, i);
                allAtomPoints.insert(allAtomPoints.end(), atomPts.begin(), atomPts.end());

                atom.atomBufferHandle = glyphBuffer.handle();
                atom.length = sizeof(TextPoint) * 6;
                atom.offset = i * sizeof(TextPoint) * 6;
                atom.width = (glyph.quad.bottomRight.x - glyph.quad.topLeft.x) / 64.0f;
                atom.height = (glyph.quad.topLeft.y - glyph.quad.bottomRight.y) / 64.0f;

                atoms.push_back(atom);
            }

            if (!allAtomPoints.empty()) {
                size_t neededAtomsBytes = allAtomPoints.size() * sizeof(TextPoint);
                auto atomsBuf = fragment.fragmentStorage.atomsBuffer.get();

                if (neededAtomsBytes > atomsBuf->length()) {
                    ctx.allocator.resize(fragment.fragmentStorage.atomsBuffer, neededAtomsBytes);
                    atomsBuf = fragment.fragmentStorage.atomsBuffer.get();
                }

                std::memcpy(
                    reinterpret_cast<std::byte*>(atomsBuf->contents()),
                    allAtomPoints.data(),
                    neededAtomsBytes
                );
            }

            size_t neededMetaBytes = metadata.size() * sizeof(int);

            auto metaBuf = fragment.fragmentStorage.metadataBuffer.get();

            if (neededMetaBytes > metaBuf->length()) {
                ctx.allocator.resize(fragment.fragmentStorage.metadataBuffer, neededMetaBytes);
                metaBuf = fragment.fragmentStorage.metadataBuffer.get();
            }

            std::memcpy(
                reinterpret_cast<std::byte*>(metaBuf->contents()),
                metadata.data(),
                neededMetaBytes
            );
            

            return Atomized{ .id = fragment.id, .atoms = atoms };
        }

        Placed place(Fragment<S>& fragment, Constraints& constraints, TextDescriptor& desc, Measured& measured, Atomized& atomized) {
            std::vector<AtomPlacement> placements;
            
            LayoutInput li;
            
            li.display = Display::Inline;
            li.position = Position::Relative;

            auto lr = ctx.layoutEngine.resolve(constraints, li, atomized);
            auto offsets = lr.atomOffsets;

            auto placementsBuffer = fragment.fragmentStorage.placementsBuffer.get();
            size_t bufferLen = offsets.size() * sizeof(simd_float2);
            
            
            if (bufferLen > placementsBuffer->length()) {
                ctx.allocator.resize(fragment.fragmentStorage.placementsBuffer, bufferLen*2);
                placementsBuffer = fragment.fragmentStorage.atomsBuffer.get();
            }
            
            std::memcpy(placementsBuffer->contents(), offsets.data(), bufferLen);
        

            for (int i = 0; i < offsets.size(); ++i) {
                placements.push_back({
                    .placementBufferHandle = fragment.fragmentStorage.placementsBuffer.handle(),
                    .x = offsets[i].x,
                    .y = offsets[i].y
                });
            }

            return Placed{ .id = fragment.id, .placements = placements };
        }
        
        Finalized<U> finalize(Fragment<S>& fragment, Constraints&, TextDescriptor& desc, Measured& measured, Atomized& atomized, Placed& placed) {
            TextUniforms uniforms {
                .color = desc.color
            };

            std::memcpy(fragment.fragmentStorage.uniformsBuffer.get()->contents(), &uniforms, sizeof(TextUniforms));

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

            auto atomBuf = fragment.fragmentStorage.atomsBuffer.get();
            auto placementBuf = fragment.fragmentStorage.placementsBuffer.get();
            auto frameInfoBuf = ctx.frameInfoBuffer.get();
            auto metaBuf = fragment.fragmentStorage.metadataBuffer.get();
            auto uniformsBuf = fragment.fragmentStorage.uniformsBuffer.get();
            auto bezierBuf = glyphBuffer.get();

            encoder->setVertexBuffer(atomBuf, 0, 0);
            encoder->setVertexBuffer(placementBuf, 0, 1);
            encoder->setVertexBuffer(frameInfoBuf, 0, 2);

            encoder->setFragmentBuffer(bezierBuf, 0, 0);
            encoder->setFragmentBuffer(metaBuf, 0, 1);
            encoder->setFragmentBuffer(uniformsBuf, 0, 2);

            const auto& atoms = finalized.atomized.atoms;
            encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0), atoms.size()*6);

        }
        
        ~TextProcessor() {
//            FT_Done_Face(face);
            FT_Done_FreeType(ft);
        }

        // shared glyph cache wrapper?
        // retrivial methods, stores buffer with all glyphs/ligatures, standardizes everything, etc... Turn this into a struct later
//        FT_Face face; // share library/face?
        GlyphCache glyphCache;
        size_t lastGlyphsBufferOffset;
        std::unordered_map<GlyphQuery, size_t, GlyphQueryHash> glyphBufferOffsets;
        DrawableBuffer glyphBuffer;
        FT_Library ft;
        
        UIContext& ctx;
    };
}


