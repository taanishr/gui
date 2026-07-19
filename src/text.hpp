//
//  Untitled.hpp
//  gui
//
//  Created by Taanish Reja on 12/19/25.
//

#pragma once
#include "frame_buffered_buffer.hpp"
#include "element.hpp"
#include "renderer_constants.hpp"
#include "freetype.hpp"
#include "glyphs.hpp"
#include "glyphCache.hpp"
#include <mutex>
#include <optional>
#include <print>
#include "new_arch.hpp"
#include "overloaded.hpp"
#include "utf8.hpp"
#include "textShaper.hpp"
#include <any>
#include <vector>

namespace elements {
    using layout::Atomized;
    using layout::Constraints;
    using layout::Direction;
    using layout::Finalized;
    using layout::LayoutResult;
    using layout::Measured;
    using layout::Placed;
    using layout::toLayoutInput;
    using runtime::HitTestContext;
    using runtime::UIContext;
    using style::ClipUniform;
    using style::SharedDescriptor;
    using style::Size;
    using style::Unit;
    using style::WhiteSpace;
    using style::WordBreak;

    struct TextPoint {
        simd_float2 point;
        simd_float2 shapingOffset;
        int metadataIndex;
        int id;
    };

    struct TextRequestPayload {
        
    };
}

template <>
struct std::formatter<elements::TextPoint> : std::formatter<float>
{
    auto format(const elements::TextPoint& v, format_context& ctx) const
    {
            return std::format_to(ctx.out(), "(x: {}, y: {}, ox: {}, oy: {}, mi: {} id: {})", v.point.x, v.point.y, v.shapingOffset.x, v.shapingOffset.y, v.metadataIndex, v.id);
    }
};

namespace elements {
    struct TextDescriptor {
        TextDescriptor();

        std::any request(std::any const& payloadAny) {
            if (!payloadAny.has_value()) return std::any{};
            if (auto payloadPtr = std::any_cast<DescriptorPayload>(&payloadAny)) {
                return std::visit(Overloaded{
                    [this](GetFull const&) -> std::any { return std::any(*this); },
                    [this](GetField const& f) -> std::any {
                        if (f.name == "text")     return std::any{this->text};
                        if (f.name == "font")     return std::any{this->font};
                        if (f.name == "fontSize") return std::any{this->fontSize};
                        if (f.name == "color")    return std::any{this->color};
                        if (f.name == "whiteSpace") return std::any{this->whiteSpace};
                        if (f.name == "wordBreak")  return std::any{this->wordBreak};
                        return std::any{};
                    }
                }, *payloadPtr);
            }
            return std::any{};
        }

        std::string text;
        std::string font;
        simd_float4 color;
        Size fontSize;
        std::optional<float> lineHeight;
        WhiteSpace whiteSpace{WhiteSpace::Normal};
        WordBreak wordBreak{WordBreak::Normal};
    };

    struct TextUniforms {
        simd_float4 color;
        float fontSize;
        uint32_t numClips;
    };


    struct TextStorage {
        TextStorage(UIContext& ctx):
            atomsBuffer{ctx.allocator, 6 * sizeof(TextPoint) * 4, MaxOutstandingFrameCount},
            drawablePointsBuffer{ctx.allocator, 6 * sizeof(TextPoint) * 4, MaxOutstandingFrameCount},
            placementsBuffer{ctx.allocator, sizeof(simd_float2) * 4, MaxOutstandingFrameCount},
            uniformsBuffer{ctx.allocator, sizeof(TextUniforms), MaxOutstandingFrameCount},
            metadataBuffer{ctx.allocator,sizeof(int) * 16, MaxOutstandingFrameCount },
            clipsBuffer{ctx.allocator, sizeof(ClipUniform) * 4, MaxOutstandingFrameCount}
        {}

        FrameBufferedBuffer<TextPoint> atomsBuffer;
        FrameBufferedBuffer<TextPoint> drawablePointsBuffer;
        FrameBufferedBuffer<simd_float2> placementsBuffer;
        FrameBufferedBuffer<TextUniforms> uniformsBuffer;

        FrameBufferedBuffer<int> metadataBuffer;
        FrameBufferedBuffer<ClipUniform> clipsBuffer;
        size_t sourceMetadataCount{};
        ShapedRun shapedRun;
    };

    template <typename S = TextStorage>
    struct Text {
        static constexpr std::string_view elementName = "Text";

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

        std::any request(RequestTarget target, std::any& payload) {
            switch (target) {
                case RequestTarget::Descriptor: 
                {
                    return desc.request(payload);
                }
                case RequestTarget::TextShaping:
                    return &fragment.fragmentStorage.shapedRun;
                default: {
                    return std::any{};
                }
            }
        }
    
        TextDescriptor desc;
        Fragment<S> fragment;

        using StorageType = S;
        using UniformsType = TextUniforms;
        using DescriptorType = TextDescriptor;
    };

    template <typename S = TextStorage, typename U = TextUniforms>
    struct TextProcessor {
        TextProcessor(UIContext& ctx):
            glyphCache{},
            lastGlyphsBufferOffset{0},
            glyphBuffer{ctx.allocator.allocate(4096)},
            ctx{ctx}
        {
            // FT_Init_FreeType(&(this->ft));
            // FT_Done_FreeType(ft);
            
            // glyphCache = GlyphCache{this->ft};
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
            vertexDescriptor->attributes()->object(1)->setOffset(sizeof(simd_float2) * 2);
            vertexDescriptor->attributes()->object(1)->setBufferIndex(0);
        
            vertexDescriptor->attributes()->object(2)->setFormat(MTL::VertexFormat::VertexFormatInt);
            vertexDescriptor->attributes()->object(2)->setOffset(sizeof(simd_float2) * 2 + sizeof(int));
            vertexDescriptor->attributes()->object(2)->setBufferIndex(0);

            vertexDescriptor->attributes()->object(3)->setFormat(MTL::VertexFormat::VertexFormatFloat2);
            vertexDescriptor->attributes()->object(3)->setOffset(sizeof(simd_float2));
            vertexDescriptor->attributes()->object(3)->setBufferIndex(0);
    
        
            vertexDescriptor->layouts()->object(0)->setStride(sizeof(TextPoint));
        
            renderPipelineDescriptor->setVertexDescriptor(vertexDescriptor);
        
        
            // set up vertex function
            MTL::Function* vertexFunction = defaultLibrary->newFunction(NS::String::string("vertex_text", NS::UTF8StringEncoding));
            renderPipelineDescriptor->setVertexFunction(vertexFunction);
        
            renderPipelineDescriptor->colorAttachments()->object(0)->setPixelFormat(ctx.view->colorPixelFormat());
            renderPipelineDescriptor->colorAttachments()->object(0)->setBlendingEnabled(true);
            renderPipelineDescriptor->colorAttachments()->object(0)->setAlphaBlendOperation(MTL::BlendOperationAdd);
            renderPipelineDescriptor->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorOne);
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
            static std::once_flag initFlag;
            static MTL::RenderPipelineState* pipeline = nullptr;
        
            std::call_once(initFlag, [&](){
                buildPipeline(pipeline);
            });
        
            return pipeline;
        }
        
        Measured measure(Fragment<S>& fragment, Constraints& constraints, SharedDescriptor& shared, TextDescriptor& desc) {
            Measured measured;

            measured.id = fragment.id;
            measured.explicitWidth  = 0.0;
            measured.explicitHeight = 0.0;

            return measured;
        }

        std::array<TextPoint, 6> makeAtomPoints(const Quad& quad, int metadataIndex, int id, simd_float2 shapingOffset = {}) {
            return std::array<TextPoint,6>{
                TextPoint{ .point = quad.topLeft, .shapingOffset = shapingOffset, .metadataIndex = metadataIndex, .id = id },
                TextPoint{ .point = simd_float2{ quad.bottomRight.x, quad.topLeft.y }, .shapingOffset = shapingOffset, .metadataIndex = metadataIndex, .id = id },
                TextPoint{ .point = simd_float2{ quad.topLeft.x,    quad.bottomRight.y }, .shapingOffset = shapingOffset, .metadataIndex = metadataIndex, .id = id },
                TextPoint{ .point = simd_float2{ quad.topLeft.x,    quad.bottomRight.y }, .shapingOffset = shapingOffset, .metadataIndex = metadataIndex, .id = id },
                TextPoint{ .point = simd_float2{ quad.bottomRight.x, quad.topLeft.y }, .shapingOffset = shapingOffset, .metadataIndex = metadataIndex, .id = id },
                TextPoint{ .point = quad.bottomRight, .shapingOffset = shapingOffset, .metadataIndex = metadataIndex, .id = id }
            };
        }

        ShapedRun appendTextAtoms(
            std::string_view text,
            const TextDescriptor& desc,
            bool collapseWhitespace,
            bool preserveLineFeeds,
            std::vector<Atom>& atoms,
            std::vector<TextPoint>& points,
            std::vector<int>& metadata,
            std::span<const bidi::TextShapingRun> bidiRuns
        ) {
            float fontSize = 0.0;
            if (desc.fontSize.unit == Unit::Pt) {
                fontSize = desc.fontSize.resolveOr(Size::px(0.0f));
            }

            float scale = fontSize / BASE_PIXEL_HEIGHT;
            float defaultLineHeight = glyphCache.lineHeight(desc.font) / FT_PIXEL_CF * scale;
            float resolvedLineHeight = defaultLineHeight * desc.lineHeight.value_or(1.0f);

            std::string renderedText{text};
            auto codepoints = utf8::codePoints(text);
            std::vector<uint8_t> collapsedWhitespace(text.size(), false);
            bool previousLogicalCodepointWasWhitespace = false;
            for (const auto& codepoint : codepoints) {
                const bool whitespace = isTextWhitespace(codepoint.value);
                collapsedWhitespace[codepoint.byteOffset] =
                    collapseWhitespace &&
                    whitespace &&
                    previousLogicalCodepointWasWhitespace;

                if (collapseWhitespace && whitespace) {
                    renderedText[codepoint.byteOffset] = ' ';
                }

                previousLogicalCodepointWasWhitespace = whitespace;
            }

            ShapedRun shapedRun;
            for (const auto& run : bidiRuns) {
                auto sourceRun = textShaper.shape(
                    renderedText,
                    run.byteStart,
                    run.byteLength,
                    desc.font,
                    run.isRtl() ? TextDirection::Rtl : TextDirection::Ltr,
                    run.scriptTag
                );

                const size_t atomBase = atoms.size();
                for (auto cluster : sourceRun.clusters) {
                    cluster.glyphStart += atomBase;
                    shapedRun.clusters.push_back(std::move(cluster));
                }

                shapedRun.runs.push_back({
                    .byteStart = run.byteStart,
                    .byteLength = run.byteLength,
                    .glyphStart = atomBase,
                    .glyphCount = sourceRun.glyphs.size(),
                    .bidiLevel = run.level
                });

                for (const auto& shapedGlyph : sourceRun.glyphs) {
                    uint32_t codepoint = utf8::at(text, shapedGlyph.byteOffset).value;
                    Atom atom;
                    bool sourceLineFeed = codepoint == U'\r' || codepoint == U'\n';

                    if (preserveLineFeeds && sourceLineFeed) {
                        int metadataIndex = metadata.size();
                        metadata.push_back(0);
                        metadata.push_back(static_cast<int>(CurveType::Quadratic));
                        metadata.push_back(0);

                        Quad emptyQuad {
                            .topLeft = {0.0f, 0.0f},
                            .bottomRight = {0.0f, 0.0f}
                        };
                        auto atomPts = makeAtomPoints(emptyQuad, metadataIndex, atoms.size());
                        points.insert(points.end(), atomPts.begin(), atomPts.end());

                        atom.atomBufferHandle = glyphBuffer.handle();
                        atom.length = sizeof(TextPoint) * 6;
                        atom.offset = (points.size() - 6) * sizeof(TextPoint);
                        atom.width = 0;
                        atom.height = defaultLineHeight;
                        atom.lineHeight = resolvedLineHeight;
                        bool followsCarriageReturn = shapedGlyph.byteOffset > 0 &&
                            text[shapedGlyph.byteOffset - 1] == '\r';
                        atom.placeOnNewLine = !(codepoint == U'\n' && followsCarriageReturn);

                        atoms.push_back(atom);
                        continue;
                    } else if (codepoint) {
                        atom.canPlaceOnNewLine = true;
                    }

                    GlyphQuery glyphQuery { shapedGlyph.glyphId, desc.font };

                    auto glyph = glyphCache.retrieve(glyphQuery);
                    size_t pointsLenBytes = glyph.points.size() * sizeof(simd_float2);
                    size_t offset = 0;

                    // highly unoptimized coarse lock, for now
                    {
                        std::lock_guard<std::mutex> lock(glyphBufferMutex);
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

                        offset = offset_it->second;
                    }

                    int metadataIndex = metadata.size();

                    metadata.push_back(offset);
                    metadata.push_back(static_cast<int>(glyph.curveType));
                    metadata.push_back(glyph.numContours);
                    for (auto& contourSize : glyph.contourSizes) {
                        metadata.push_back(contourSize);
                    }

                    simd_float2 shapingOffset{
                        shapedGlyph.xOffset,
                        -shapedGlyph.yOffset
                    };
                    auto atomPts = makeAtomPoints(glyph.quad, metadataIndex, atoms.size(), shapingOffset);

                    points.insert(points.end(), atomPts.begin(), atomPts.end());

                    atom.atomBufferHandle = glyphBuffer.handle();
                    atom.length = sizeof(TextPoint) * 6;
                    atom.offset = (points.size() - 6) * sizeof(TextPoint);
                    float glyphWidth = shapedGlyph.xAdvance / FT_PIXEL_CF * scale;
                    atom.width = collapsedWhitespace[shapedGlyph.byteOffset]
                        ? 0.0f
                        : glyphWidth;
                    atom.height = defaultLineHeight;
                    atom.lineHeight = resolvedLineHeight;

                    atoms.push_back(atom);
                }
            }
            std::ranges::sort(shapedRun.clusters, {}, &ShapedCluster::byteOffset);
            return shapedRun;
        }

        Atomized atomize(Fragment<S>& fragment, Constraints& constraints, SharedDescriptor& shared, TextDescriptor& desc, Measured&) {
            std::vector<Atom> atoms;
            std::vector<TextPoint> allAtomPoints;
            std::vector<int> metadata;
            allAtomPoints.reserve(desc.text.size() * 6);

            bool collapseWhitespace =
                desc.whiteSpace == WhiteSpace::Normal ||
                desc.whiteSpace == WhiteSpace::NoWrap;
            bool preserveLineFeeds =
                desc.whiteSpace == WhiteSpace::Pre ||
                desc.whiteSpace == WhiteSpace::PreWrap;

            fragment.fragmentStorage.shapedRun = appendTextAtoms(
                desc.text,
                desc,
                collapseWhitespace,
                preserveLineFeeds,
                atoms,
                allAtomPoints,
                metadata,
                constraints.textBidiInput.value().runs
            );

            if (!allAtomPoints.empty()) {
                size_t neededAtomsBytes = allAtomPoints.size() * sizeof(TextPoint);

                fragment.fragmentStorage.atomsBuffer.write(ctx.frameIndex, allAtomPoints.data(), neededAtomsBytes);
            }

            size_t neededMetaBytes = metadata.size() * sizeof(int);

            if (!metadata.empty()) {
                fragment.fragmentStorage.metadataBuffer.write(ctx.frameIndex, metadata.data(), neededMetaBytes);
            }

            fragment.fragmentStorage.sourceMetadataCount = metadata.size();

            return Atomized{ .id = fragment.id, .atoms = std::move(atoms) };
        }

        LayoutResult layout(Fragment<S>& fragment, Constraints& constraints, SharedDescriptor& shared, TextDescriptor& desc, Measured& measured, Atomized& atomized) {
            auto li = toLayoutInput(shared, measured);
            auto lr = ctx.layoutEngine.resolve(constraints, li, atomized);
            lr.inlineFormatting = constraints.inlineFormatting;
            return lr;
        }

        Atomized postLayout(Fragment<S>& fragment, Constraints& constraints, SharedDescriptor& shared, TextDescriptor& desc, Measured& measured, Atomized& atomized, LayoutResult& layout) {
            atomized.usesDrawableAtoms = false;
            if (!constraints.textOverflow->drawsEnding()) return atomized;

            const auto& overflowClip = constraints.clipUniforms.back();
            float visibleLeft = overflowClip.rectCenter.x - overflowClip.halfExtent.x;
            float visibleRight = overflowClip.rectCenter.x + overflowClip.halfExtent.x;
            bool isLtr = constraints.inheritedProperties.direction == Direction::ltr;

            atomized.drawableAtoms.clear();
            layout.drawableAtomOffsets.clear();

            auto sourcePoints = reinterpret_cast<TextPoint*>(
                fragment.fragmentStorage.atomsBuffer.getBuffer(ctx.frameIndex)->contents()
            );
            auto sourceMetadata = reinterpret_cast<int*>(
                fragment.fragmentStorage.metadataBuffer.getBuffer(ctx.frameIndex)->contents()
            );
            std::vector<TextPoint> drawablePoints;
            std::vector<int> drawableMetadata(
                sourceMetadata,
                sourceMetadata + fragment.fragmentStorage.sourceMetadataCount
            );

            bool endingAdded = false;
            for (const auto& lineFragment : layout.inlineFormatting.lineFragments()) {
                auto firstAtom = atomized.atoms.begin() + lineFragment.atomStart;
                auto firstOffset = layout.atomOffsets.begin() + lineFragment.atomStart;
                float fragmentLeft = firstOffset->x;
                float fragmentRight = fragmentLeft + lineFragment.width;
                atomized.usesDrawableAtoms = atomized.usesDrawableAtoms ||
                    (isLtr ? fragmentRight > visibleRight : fragmentLeft < visibleLeft);
                if (endingAdded && isLtr) continue;
                if (isLtr ? fragmentLeft >= visibleRight : fragmentRight <= visibleLeft) continue;

                size_t visibleStart = 0;
                size_t visibleCount = lineFragment.atomCount;
                float endingWidth = 0.0f;
                std::vector<Atom> endingAtoms;
                std::vector<TextPoint> endingPoints;
                std::vector<int> endingMetadata;

                auto endingBidi = bidi::TextBidiContext::create(
                    constraints.textOverflow->ending,
                    constraints.inheritedProperties.direction == Direction::rtl
                        ? bidi::BidiBaseDirection::Rtl
                        : bidi::BidiBaseDirection::Ltr
                );
                std::vector<bidi::TextShapingRun> endingRuns;
                if (endingBidi) {
                    auto resolvedRuns = endingBidi->runs();
                    endingRuns.assign(resolvedRuns.begin(), resolvedRuns.end());
                }

                appendTextAtoms(
                    constraints.textOverflow->ending,
                    desc,
                    true,
                    false,
                    endingAtoms,
                    endingPoints,
                    endingMetadata,
                    endingRuns
                );
                for (const Atom& atom : endingAtoms) endingWidth += atom.width;

                if (!endingAdded && (isLtr
                        ? fragmentRight + endingWidth > visibleRight
                        : fragmentLeft - endingWidth < visibleLeft)) {
                    visibleCount = 0;
                    if (isLtr) {
                        while (visibleCount < lineFragment.atomCount &&
                               firstOffset[visibleCount].x + firstAtom[visibleCount].width <=
                                   visibleRight - endingWidth) {
                            visibleCount++;
                        }
                    } else {
                        visibleStart = lineFragment.atomCount;
                        while (visibleStart > 0 &&
                               firstOffset[visibleStart - 1].x >= visibleLeft + endingWidth) {
                            visibleStart--;
                            visibleCount++;
                        }
                    }

                    if (visibleCount == 0) {
                        visibleCount = std::min<size_t>(1, lineFragment.atomCount);
                        visibleStart = isLtr ? 0 : lineFragment.atomCount - visibleCount;
                        endingAtoms.clear();
                        endingPoints.clear();
                        endingMetadata.clear();
                    }
                }

                atomized.drawableAtoms.insert(
                    atomized.drawableAtoms.end(),
                    firstAtom + visibleStart,
                    firstAtom + visibleStart + visibleCount
                );
                layout.drawableAtomOffsets.insert(
                    layout.drawableAtomOffsets.end(),
                    firstOffset + visibleStart,
                    firstOffset + visibleStart + visibleCount
                );
                drawablePoints.insert(
                    drawablePoints.end(),
                    sourcePoints + (lineFragment.atomStart + visibleStart) * 6,
                    sourcePoints + (lineFragment.atomStart + visibleStart + visibleCount) * 6
                );

                if (endingAtoms.empty() || visibleCount == lineFragment.atomCount) continue;

                size_t metadataOffset = drawableMetadata.size();
                for (auto& point : endingPoints) point.metadataIndex += metadataOffset;
                drawableMetadata.insert(
                    drawableMetadata.end(),
                    endingMetadata.begin(),
                    endingMetadata.end()
                );
                atomized.drawableAtoms.insert(
                    atomized.drawableAtoms.end(),
                    endingAtoms.begin(),
                    endingAtoms.end()
                );
                drawablePoints.insert(
                    drawablePoints.end(),
                    endingPoints.begin(),
                    endingPoints.end()
                );

                float endingX = isLtr
                    ? firstOffset[visibleCount - 1].x + firstAtom[visibleCount - 1].width
                    : firstOffset[visibleStart].x - endingWidth;
                float endingY = firstOffset[visibleStart].y;
                for (const Atom& atom : endingAtoms) {
                    layout.drawableAtomOffsets.push_back(simd_float2{endingX, endingY});
                    endingX += atom.width;
                }

                endingAdded = true;
            }

            for (size_t pointIndex = 0; pointIndex < drawablePoints.size(); ++pointIndex) {
                drawablePoints[pointIndex].id = pointIndex / 6;
            }
            fragment.fragmentStorage.drawablePointsBuffer.write(
                ctx.frameIndex,
                drawablePoints.data(),
                drawablePoints.size() * sizeof(TextPoint)
            );
            fragment.fragmentStorage.metadataBuffer.write(
                ctx.frameIndex,
                drawableMetadata.data(),
                drawableMetadata.size() * sizeof(int)
            );

            return atomized;
        };

        Placed place(Fragment<S>& fragment, Constraints& constraints, SharedDescriptor& shared, TextDescriptor& desc, Measured& measured, Atomized& atomized, LayoutResult& lr) {
            std::vector<AtomPlacement> placements;
            
            const auto& offsets = atomized.usesDrawableAtoms
                ? lr.drawableAtomOffsets
                : lr.atomOffsets;


            size_t bufferLen = offsets.size() * sizeof(simd_float2);

            fragment.fragmentStorage.placementsBuffer.write(ctx.frameIndex, offsets.data(), bufferLen);

            for (int i = 0; i < offsets.size(); ++i) {
                placements.push_back({
                    .placementBufferHandle = fragment.fragmentStorage.placementsBuffer.getBufferHandle(ctx.frameIndex),
                    .x = offsets[i].x ,
                    .y = offsets[i].y
                });
            }

            return Placed{ .id = fragment.id, .placements = placements };
        }
        
        Finalized<U> finalize(Fragment<S>& fragment, Constraints&, SharedDescriptor& shared, TextDescriptor& desc, Measured& measured, Atomized& atomized, LayoutResult& layout, Placed& placed) {
            float fontSize;

            if (desc.fontSize.unit == Unit::Pt) {
                fontSize = desc.fontSize.resolveOr(Size::px(0.0f));
            }

            TextUniforms uniforms {
                .color = desc.color,
                .fontSize = fontSize,
                .numClips = static_cast<uint32_t>(layout.clipUniforms.size())
            };

            fragment.fragmentStorage.uniformsBuffer.write(ctx.frameIndex, &uniforms, sizeof(TextUniforms));
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

            auto frameInfoBuf = ctx.frameInfoBuffer.get();
            auto atomBuf = finalized.atomized.usesDrawableAtoms
                ? fragment.fragmentStorage.drawablePointsBuffer.getBuffer(ctx.frameIndex)
                : fragment.fragmentStorage.atomsBuffer.getBuffer(ctx.frameIndex);
            auto placementBuf = fragment.fragmentStorage.placementsBuffer.getBuffer(ctx.frameIndex);
            auto metaBuf = fragment.fragmentStorage.metadataBuffer.getBuffer(ctx.frameIndex);
            auto uniformsBuf = fragment.fragmentStorage.uniformsBuffer.getBuffer(ctx.frameIndex);
            auto clipsBuf = fragment.fragmentStorage.clipsBuffer.getBuffer(ctx.frameIndex);
            auto bezierBuf = glyphBuffer.get();

            encoder->setVertexBuffer(atomBuf, 0, 0);
            encoder->setVertexBuffer(placementBuf, 0, 1);
            encoder->setVertexBuffer(frameInfoBuf, 0, 2);
            encoder->setVertexBuffer(uniformsBuf, 0, 3);

            encoder->setFragmentBuffer(bezierBuf, 0, 0);
            encoder->setFragmentBuffer(metaBuf, 0, 1);
            encoder->setFragmentBuffer(uniformsBuf, 0, 2);
            encoder->setFragmentBuffer(clipsBuf, 0, 3);

            const auto& atoms = finalized.atomized.usesDrawableAtoms
                ? finalized.atomized.drawableAtoms
                : finalized.atomized.atoms;
            encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0), atoms.size()*6);

        }

        std::function<bool(HitTestContext<U>& context, simd_float2 testPoint)> setupHitTestFunction() {
            auto hitTestFunction = [](HitTestContext<U>& context, simd_float2 testPoint){
                return true;
            };

            return hitTestFunction;
        }

        ~TextProcessor() {}

        // shared glyph cache wrapper?
        // retrivial methods, stores buffer with all glyphs/ligatures, standardizes everything, etc... Turn this into a struct later
        GlyphCache glyphCache;
        TextShaper textShaper;
        size_t lastGlyphsBufferOffset;
        std::unordered_map<GlyphQuery, size_t, GlyphQueryHash> glyphBufferOffsets;
        DrawableBuffer glyphBuffer;
        std::mutex glyphBufferMutex;
        
        UIContext& ctx;
    };
}
