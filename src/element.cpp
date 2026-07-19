#include "element.hpp"
#include "fragment_types.hpp"
#include "sizing.hpp"
#include "new_arch.hpp"
#include "utf8.hpp"
#include "textShaper.hpp"
#include <algorithm>
#include <any>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <simd/vector_types.h>
#include <string>
#include <unordered_map>


namespace elements {
    ElementBase::~ElementBase() {};
}

namespace tree {
    using elements::DescriptorPayload;
    using elements::GetField;
    using elements::RequestTarget;
    using elements::isTextWhitespace;
    using layout::Constraints;
    using layout::LayoutEngine;
    using layout::LayoutInput;
    using layout::LineBox;
    using layout::LineFragment;
    using layout::ResolvedMargins;
    using style::Display;
    using style::Position;
    using style::Size;
    using style::WhiteSpace;
    using style::WordBreak;

    // Element-specific requests still use the request system
    std::optional<std::string> getText(TreeNode* node) {
        std::any request{DescriptorPayload{GetField{.name = "text"}}};
        auto resp = node->element->request(RequestTarget::Descriptor, request);
        if (resp.has_value()) {
            return std::any_cast<std::string>(resp);
        }
        return std::nullopt;
    }

    Result<std::vector<std::optional<bidi::TextBidiInput>>>
    prepareChildBidiInputs(
        TreeNode* parent,
        layout::Direction baseDirection
    ) {
        std::vector<std::optional<bidi::TextBidiInput>> inputs(parent->children.size());

        size_t sequenceStart = 0;
        while (sequenceStart < parent->children.size()) {
            while (sequenceStart < parent->children.size() &&
                   !getText(parent->children[sequenceStart].get()).has_value()) {
                ++sequenceStart;
            }
            if (sequenceStart == parent->children.size()) break;

            size_t sequenceEnd = sequenceStart;
            std::string paragraph;
            std::vector<size_t> childStarts;
            while (sequenceEnd < parent->children.size()) {
                auto childText = getText(parent->children[sequenceEnd].get());
                if (!childText.has_value()) break;
                childStarts.push_back(paragraph.size());
                std::string bidiText = *childText;
                const auto whiteSpace = getWhiteSpace(parent->children[sequenceEnd].get())
                    .value_or(WhiteSpace::Normal);
                if (whiteSpace == WhiteSpace::Normal || whiteSpace == WhiteSpace::NoWrap) {
                    for (char& byte : bidiText) {
                        if (byte == '\r' || byte == '\n') byte = ' ';
                    }
                }
                paragraph += bidiText;
                ++sequenceEnd;
            }

            auto resolvedContext = bidi::TextBidiContext::create(
                std::move(paragraph),
                baseDirection == layout::Direction::rtl
                    ? bidi::BidiBaseDirection::Rtl
                    : bidi::BidiBaseDirection::Ltr
            );
            if (!resolvedContext) return std::unexpected{resolvedContext.error()};
            auto context = std::make_shared<bidi::TextBidiContext>(
                std::move(*resolvedContext));

            for (size_t i = sequenceStart; i < sequenceEnd; ++i) {
                const size_t childStart = childStarts[i - sequenceStart];
                const size_t childLength = getText(parent->children[i].get())->size();
                const size_t childEnd = childStart + childLength;
                std::vector<bidi::TextShapingRun> childRuns;
                for (const auto& run : context->runs()) {
                    const size_t runEnd = run.byteStart + run.byteLength;
                    const size_t start = std::max(childStart, run.byteStart);
                    const size_t end = std::min(childEnd, runEnd);
                    if (start >= end) continue;
                    childRuns.push_back({
                        .byteStart = start - childStart,
                        .byteLength = end - start,
                        .level = run.level,
                        .scriptTag = run.scriptTag
                    });
                }
                inputs[i] = bidi::TextBidiInput{
                    .context = context,
                    .paragraphByteStart = childStart,
                    .byteLength = childLength,
                    .runs = std::move(childRuns)
                };
            }
            sequenceStart = sequenceEnd;
        }

        return inputs;
    }

    std::optional<WhiteSpace> getWhiteSpace(TreeNode* node) {
        std::any request{DescriptorPayload{GetField{.name = "whiteSpace"}}};
        auto resp = node->element->request(RequestTarget::Descriptor, request);
        if (resp.has_value()) {
            return std::any_cast<WhiteSpace>(resp);
        }
        return std::nullopt;
    }

    std::optional<WordBreak> getWordBreak(TreeNode* node) {
        std::any request{DescriptorPayload{GetField{.name = "wordBreak"}}};
        auto resp = node->element->request(RequestTarget::Descriptor, request);
        if (resp.has_value()) {
            return std::any_cast<WordBreak>(resp);
        }
        return std::nullopt;
    }

    ShapedRun* getShapedRun(TreeNode* node) {
        std::any request;
        auto response = node->element->request(RequestTarget::TextShaping, request);
        return std::any_cast<ShapedRun*>(response);
    }
    

    uint64_t TreeNode::nextId = 0;

    std::vector<TreeNode*> collectAllNodes(TreeNode* root) {
        std::vector<TreeNode*> nodes;
        std::function<void(TreeNode*)> collect = [&](TreeNode* node) {
            if (!node) return;
            nodes.push_back(node);
            for (auto& child : node->children) {
                collect(child.get());
            }
        };
        collect(root);
        return nodes;
    }

    void pushRunFragments(
        const ShapedRun& shapedRun,
        const std::vector<Atom>& atoms,
        size_t clusterStart,
        size_t clusterEnd,
        float totalWidth,
        std::vector<LineFragment>& fragments,
        LineBox& lineBox,
        size_t lineBoxIndex
    ) {
        if (clusterStart == clusterEnd) return;

        float glyphWidth = 0.0f;
        for (size_t i = clusterStart; i < clusterEnd; ++i) {
            const auto& cluster = shapedRun.clusters[i];
            for (size_t glyph = 0; glyph < cluster.glyphCount; ++glyph) {
                glyphWidth += atoms[cluster.glyphStart + glyph].width;
            }
        }
        const float trailingWidth = totalWidth - glyphWidth;
        const auto& finalCluster = shapedRun.clusters[clusterEnd - 1];
        const size_t finalByteEnd = finalCluster.byteOffset + finalCluster.byteLength;

        for (const auto& run : shapedRun.runs) {
            const size_t runEnd = run.byteStart + run.byteLength;
            size_t atomStart = atoms.size();
            size_t atomEnd = 0;
            size_t byteStart = 0;
            size_t byteEnd = 0;
            float width = 0.0f;
            bool foundCluster = false;

            for (size_t i = clusterStart; i < clusterEnd; ++i) {
                const auto& cluster = shapedRun.clusters[i];
                if (cluster.byteOffset < run.byteStart || cluster.byteOffset >= runEnd) {
                    continue;
                }

                if (!foundCluster) {
                    byteStart = cluster.byteOffset;
                    foundCluster = true;
                }
                byteEnd = cluster.byteOffset + cluster.byteLength;
                atomStart = std::min(atomStart, cluster.glyphStart);
                atomEnd = std::max(atomEnd, cluster.glyphStart + cluster.glyphCount);
                for (size_t glyph = 0; glyph < cluster.glyphCount; ++glyph) {
                    width += atoms[cluster.glyphStart + glyph].width;
                }
            }

            if (!foundCluster) continue;
            if (byteEnd == finalByteEnd) width += trailingWidth;

            LineFragment fragment{
                .width = width,
                .atomStart = atomStart,
                .atomCount = atomEnd - atomStart,
                .textByteStart = byteStart,
                .textByteLength = byteEnd - byteStart,
                .bidiLevel = run.bidiLevel,
                .lineBoxIndex = lineBoxIndex,
                .fragmentIndex = lineBox.fragmentCount
            };
            fragments.push_back(fragment);
            lineBox.pushFragment(fragment);
        }
    }

    void reorderLineFragments(layout::InlineFormattingContext& context) {
        for (size_t lineIndex = 0; lineIndex < context.lineBoxes.size(); ++lineIndex) {
            auto& lineBox = context.lineBoxes[lineIndex];
            std::vector<LineFragment*> fragments;
            int maximumLevel = 0;
            int minimumOddLevel = -1;

            for (auto& fragment : context.fragments) {
                if (fragment.lineBoxIndex != lineIndex) continue;
                fragments.push_back(&fragment);
                maximumLevel = std::max(maximumLevel, static_cast<int>(fragment.bidiLevel));
                if ((fragment.bidiLevel & 1u) != 0 &&
                    (minimumOddLevel == -1 || fragment.bidiLevel < minimumOddLevel)) {
                    minimumOddLevel = fragment.bidiLevel;
                }
            }

            if (minimumOddLevel != -1) {
                for (int level = maximumLevel; level >= minimumOddLevel; --level) {
                    size_t start = 0;
                    while (start < fragments.size()) {
                        while (start < fragments.size() &&
                               fragments[start]->bidiLevel < level) {
                            ++start;
                        }

                        size_t end = start;
                        while (end < fragments.size() &&
                               fragments[end]->bidiLevel >= level) {
                            ++end;
                        }

                        std::reverse(
                            fragments.begin() + start,
                            fragments.begin() + end
                        );
                        start = end;
                    }
                }
            }

            float offset = 0.0f;
            for (const auto* fragment : fragments) {
                lineBox.fragmentOffsets[fragment->fragmentIndex] = offset;
                offset += fragment->width;
            }
            assert(std::abs(offset - lineBox.width) < 0.001f);
        }
    }

    bool shouldTakeSoftBreak(
        layout::AxisResolution widthResolution,
        bool hasBreakOpportunity,
        bool lineHasContent,
        float prospectiveWidth,
        float availableWidth
    ) {
        if (!hasBreakOpportunity || !lineHasContent) return false;
        if (widthResolution == layout::AxisResolution::MinContent) return true;
        if (widthResolution == layout::AxisResolution::MaxContent) return false;
        return prospectiveWidth > availableWidth;
    }

    void appendTextLineFragments(
        const std::string& text,
        const ShapedRun& shapedRun,
        const std::vector<Atom>& atoms,
        WhiteSpace whiteSpace,
        WordBreak wordBreak,
        ResolvedMargins margins,
        float availableWidth,
        layout::AxisResolution widthResolution,
        std::vector<LineFragment>& fragments,
        std::vector<LineBox>& lineBoxes,
        LineBox& currentLineBox,
        size_t& currentLineBoxIndex,
        bool& lastFragmentHasBreakOpportunity
    ) {
        const bool preserveLineFeeds =
            whiteSpace == WhiteSpace::Pre ||
            whiteSpace == WhiteSpace::PreWrap;
        const bool allowSoftWrap =
            whiteSpace == WhiteSpace::Normal ||
            whiteSpace == WhiteSpace::PreWrap;
        const bool breakInsideWords =
            allowSoftWrap && wordBreak == WordBreak::BreakAll;

        float runningWidth = margins.left;
        size_t runningAtomCount = 0;
        size_t runningClusterStart = 0;
        size_t idx = 0;

        while (idx < shapedRun.clusters.size()) {
            const auto& cluster = shapedRun.clusters[idx];
            char32_t ch = cluster.codepoint();
            const auto& firstAtom = atoms[cluster.glyphStart];
            float width = 0.0f;
            for (size_t i = 0; i < cluster.glyphCount; ++i) {
                width += atoms[cluster.glyphStart + i].width;
            }

            if (preserveLineFeeds && firstAtom.placeOnNewLine) {
                runningWidth += width + margins.right;
                runningAtomCount += cluster.glyphCount;

                if (allowSoftWrap && shouldTakeSoftBreak(
                        widthResolution,
                        lastFragmentHasBreakOpportunity,
                        currentLineBox.fragmentCount > 0,
                        currentLineBox.width + runningWidth,
                        availableWidth
                    )) {
                    lineBoxes.push_back(currentLineBox);
                    currentLineBox = {};
                    currentLineBoxIndex++;
                }

                pushRunFragments(
                    shapedRun,
                    atoms,
                    runningClusterStart,
                    idx + 1,
                    runningWidth,
                    fragments,
                    currentLineBox,
                    currentLineBoxIndex
                );

                lineBoxes.push_back(currentLineBox);
                currentLineBox = {};
                currentLineBoxIndex++;
                lastFragmentHasBreakOpportunity = false;

                runningWidth = 0.0;
                runningAtomCount = 0;
                idx++;
                runningClusterStart = idx;
                continue;
            }

            if (breakInsideWords && !isTextWhitespace(ch)) {
                float prospectiveWidth = currentLineBox.width + runningWidth + width;

                if (shouldTakeSoftBreak(
                        widthResolution,
                        true,
                        currentLineBox.fragmentCount > 0 || runningAtomCount > 0,
                        prospectiveWidth,
                        availableWidth
                    )) {
                    bool hadPendingAtoms = runningAtomCount > 0;
                    if (runningAtomCount > 0) {
                        pushRunFragments(
                            shapedRun,
                            atoms,
                            runningClusterStart,
                            idx,
                            runningWidth,
                            fragments,
                            currentLineBox,
                            currentLineBoxIndex
                        );
                    }

                    lineBoxes.push_back(currentLineBox);
                    currentLineBox = {};
                    currentLineBoxIndex++;
                    lastFragmentHasBreakOpportunity = false;
                    runningWidth = hadPendingAtoms ? 0.0f : margins.left;
                    runningAtomCount = 0;
                    runningClusterStart = idx;
                }

                runningWidth += width;
                runningAtomCount += cluster.glyphCount;
                idx++;
                continue;
            }

            if (!isTextWhitespace(ch)) {
                runningWidth += width;
                runningAtomCount += cluster.glyphCount;
                idx++;
                continue;
            }

            while (idx < shapedRun.clusters.size() &&
                   isTextWhitespace(shapedRun.clusters[idx].codepoint())) {
                const auto& whitespace = shapedRun.clusters[idx];
                for (size_t i = 0; i < whitespace.glyphCount; ++i) {
                    runningWidth += atoms[whitespace.glyphStart + i].width;
                }
                runningAtomCount += whitespace.glyphCount;
                idx++;
            }

            runningWidth += margins.right;

            if (allowSoftWrap && shouldTakeSoftBreak(
                    widthResolution,
                    lastFragmentHasBreakOpportunity,
                    currentLineBox.fragmentCount > 0,
                    currentLineBox.width + runningWidth,
                    availableWidth
                )) {
                lineBoxes.push_back(currentLineBox);
                currentLineBox = {};
                currentLineBoxIndex++;
            }

            pushRunFragments(
                shapedRun,
                atoms,
                runningClusterStart,
                idx,
                runningWidth,
                fragments,
                currentLineBox,
                currentLineBoxIndex
            );
            lastFragmentHasBreakOpportunity = true;

            runningWidth = 0.0;
            runningAtomCount = 0;
            runningClusterStart = idx;
        }

        if (runningAtomCount > 0) {
            runningWidth += margins.right;

            if (allowSoftWrap && shouldTakeSoftBreak(
                    widthResolution,
                    lastFragmentHasBreakOpportunity,
                    currentLineBox.fragmentCount > 0,
                    currentLineBox.width + runningWidth,
                    availableWidth
                )) {
                lineBoxes.push_back(currentLineBox);
                currentLineBox = {};
                currentLineBoxIndex++;
            }

            pushRunFragments(
                shapedRun,
                atoms,
                runningClusterStart,
                shapedRun.clusters.size(),
                runningWidth,
                fragments,
                currentLineBox,
                currentLineBoxIndex
            );
            lastFragmentHasBreakOpportunity = false;
        }
    }
    

    void precomputeMargins(TreeNode* node, Constraints& constraints, std::unordered_map<ChainID, CollapsedChain>& collapsedChainMap) {
        constraints.replacedAttributes = {};

        if (node->preLayout->marginMetadata.topChainId.has_value()) {
            auto topChain = collapsedChainMap[node->preLayout->marginMetadata.topChainId.value()];
            if (topChain.depth > 1) {
                if (node == topChain.root) {
                    constraints.replacedAttributes.marginTop = topChain.intent;
                } else {
                    constraints.replacedAttributes.marginTop = Size{};
                }
            }
        }

        if (node->preLayout->marginMetadata.bottomChainId.has_value()) {
            auto bottomChain = collapsedChainMap[node->preLayout->marginMetadata.bottomChainId.value()];
            if (bottomChain.depth > 1) {
                if (node == bottomChain.root) {
                    constraints.replacedAttributes.marginBottom = bottomChain.intent;
                } else {
                    constraints.replacedAttributes.marginBottom = Size{};
                }
            }
        }

        auto position = node->getPosition();
        auto display = node->getDisplay();

        Size marginTop = node->getMarginTop();
        Size marginRight = node->getMarginRight();
        Size marginBottom = node->getMarginBottom();
        Size marginLeft = node->getMarginLeft();

        ResolvedMargins margins{};

        if (position == Position::Static && display == Display::Block) {
            // Block normal flow: use resolveAutoMargins
            // Compute content dimensions from atoms
            float contentWidth = 0;
            float contentHeight = 0;
            for (auto& atom : node->atomized->atoms) {
                contentWidth += atom.width;
                contentHeight = std::max(contentHeight, atom.height);
            }

            auto resolvedWidth = node->shared.width.resolve(
                Size::px(constraints.availableWidth)
            );
            contentWidth = resolvedWidth.value_or(constraints.availableWidth);

            LayoutInput li{
                .position = position,
                .display = display,
                .width = resolvedWidth,
                .height = std::unexpected(
                    style::SizeResolveFailure::Auto
                ),
                .marginTop = marginTop,
                .marginRight = marginRight,
                .marginBottom = marginBottom,
                .marginLeft = marginLeft,
            };

            margins = LayoutEngine::resolveAutoMargins(li, constraints.replacedAttributes, constraints.availableWidth, contentWidth);
        } else {
            margins = {
                .top = marginTop.resolveOr(Size::px(0.0f), 0.0f),
                .right = marginRight.resolveOr(Size::px(0.0f), 0.0f),
                .bottom = marginBottom.resolveOr(Size::px(0.0f), 0.0f),
                .left = marginLeft.resolveOr(Size::px(0.0f), 0.0f),
            };
        }

        node->preLayout->resolvedMargins = margins;

        auto& measured = *node->measured;
        Constraints childConstraints{};
        childConstraints.availableWidth = measured.explicitWidth.value_or(constraints.availableWidth);
        childConstraints.availableHeight = measured.explicitHeight.value_or(constraints.availableHeight);
        childConstraints.frameInfo = constraints.frameInfo;

        for (auto& child : node->children) {
            precomputeMargins(child.get(), childConstraints, collapsedChainMap);
        }
    }

    layout::InlineFormattingInput buildIsolatedInlineBoxes(
        TreeNode* node,
        float maxWidth,
        layout::AxisResolution widthResolution
    ) {
        auto context = std::make_shared<layout::InlineFormattingContext>();
        auto& fragments = context->fragments;
        auto& lineBoxes = context->lineBoxes;
        LineBox currentLineBox{};
        size_t currentLineBoxIndex = 0;
        bool lastFragmentHasBreakOpportunity = false;

        auto textResp = getText(node);
        if (textResp.has_value()) {
            auto shapedRun = getShapedRun(node);
            auto margins = node->preLayout->resolvedMargins;
            auto text = *textResp;
            auto& atoms = node->atomized->atoms;
            appendTextLineFragments(
                text,
                *shapedRun,
                atoms,
                getWhiteSpace(node).value_or(WhiteSpace::Normal),
                getWordBreak(node).value_or(WordBreak::Normal),
                margins,
                maxWidth,
                widthResolution,
                fragments,
                lineBoxes,
                currentLineBox,
                currentLineBoxIndex,
                lastFragmentHasBreakOpportunity
            );
        }

        if (currentLineBox.fragmentCount > 0)
            lineBoxes.push_back(currentLineBox);

        reorderLineFragments(*context);

        const size_t fragmentCount = fragments.size();
        return {
            .context = std::move(context),
            .fragments = {.start = 0, .count = fragmentCount}
        };
    }

    std::shared_ptr<layout::InlineFormattingContext> buildInlineBoxes(TreeNode* node, Constraints& childConstraints) {
        bool prevInline = false;
        auto context = std::make_shared<layout::InlineFormattingContext>();
        auto& childrenLineBoxes = context->lineBoxes;
        auto& fragments = context->fragments;
        auto& childFragments = context->childFragments;
        LineBox currentLineBox {};
        size_t currentLineBoxIndex = 0;
        bool lastFragmentHasBreakOpportunity = false;


        for (uint64_t i = 0; i < node->children.size(); ++i) {
            auto& child = node->children[i];

            size_t childFragmentStart = fragments.size();

            auto textResp = getText(child.get());

            if (textResp.has_value()) {
                auto shapedRun = getShapedRun(child.get());
                auto margins = child->preLayout->resolvedMargins;
                auto text = *textResp;

                auto& atoms = child->atomized->atoms;

                if (i > 0 && !prevInline && currentLineBox.fragmentCount > 0) {
                    childrenLineBoxes.push_back(currentLineBox);
                    currentLineBox = {};
                    currentLineBoxIndex++;
                }

                appendTextLineFragments(
                    text,
                    *shapedRun,
                    atoms,
                    getWhiteSpace(child.get()).value_or(WhiteSpace::Normal),
                    getWordBreak(child.get()).value_or(WordBreak::Normal),
                    margins,
                    childConstraints.availableWidth,
                    childConstraints.widthResolution,
                    fragments,
                    childrenLineBoxes,
                    currentLineBox,
                    currentLineBoxIndex,
                    lastFragmentHasBreakOpportunity
                );

                prevInline = true;
            }else {
                prevInline = false;
            }

            childFragments.push_back({
                .start = childFragmentStart,
                .count = fragments.size() - childFragmentStart
            });
        }

        if (currentLineBox.fragmentCount > 0) {
            childrenLineBoxes.push_back(currentLineBox);
        }

        reorderLineFragments(*context);

        return context;
    }
}
