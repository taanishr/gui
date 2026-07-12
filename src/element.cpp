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

    const ShapedRun* getShapedRun(TreeNode* node) {
        std::any request;
        auto response = node->element->request(RequestTarget::TextShaping, request);
        if (response.has_value()) {
            return std::any_cast<const ShapedRun*>(response);
        }
        return nullptr;
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

    void appendTextLineFragments(
        const std::string& text,
        const ShapedRun& shapedRun,
        const std::vector<Atom>& atoms,
        WhiteSpace whiteSpace,
        WordBreak wordBreak,
        ResolvedMargins margins,
        float availableWidth,
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
        size_t runningAtomStart = 0;
        size_t runningAtomCount = 0;
        size_t idx = 0;

        auto clusterWidth = [&](const ShapedCluster& cluster) {
            float width = 0.0f;
            for (size_t i = 0; i < cluster.glyphCount; ++i) {
                width += atoms[cluster.glyphStart + i].width;
            }
            return width;
        };

        auto clusterCodepoint = [&](const ShapedCluster& cluster) {
            return utf8::at(text, cluster.byteOffset).value;
        };

        while (idx < shapedRun.clusters.size()) {
            const auto& cluster = shapedRun.clusters[idx];
            char32_t ch = clusterCodepoint(cluster);
            const auto& firstAtom = atoms[cluster.glyphStart];
            float width = clusterWidth(cluster);

            if (preserveLineFeeds && firstAtom.placeOnNewLine) {
                runningWidth += width + margins.right;
                runningAtomCount += cluster.glyphCount;

                LineFragment frag{
                    .width = runningWidth,
                    .atomStart = runningAtomStart,
                    .atomCount = runningAtomCount
                };

                if (allowSoftWrap && lastFragmentHasBreakOpportunity && currentLineBox.fragmentCount > 0 && currentLineBox.width + runningWidth > availableWidth) {
                    lineBoxes.push_back(currentLineBox);
                    currentLineBox = {};
                    currentLineBoxIndex++;
                }

                frag.lineBoxIndex = currentLineBoxIndex;
                frag.fragmentIndex = currentLineBox.fragmentCount;
                fragments.push_back(frag);
                currentLineBox.pushFragment(frag);

                lineBoxes.push_back(currentLineBox);
                currentLineBox = {};
                currentLineBoxIndex++;
                lastFragmentHasBreakOpportunity = false;

                runningWidth = 0.0;
                runningAtomCount = 0;
                idx++;
                runningAtomStart = idx < shapedRun.clusters.size()
                    ? shapedRun.clusters[idx].glyphStart
                    : atoms.size();
                continue;
            }

            if (breakInsideWords && !isTextWhitespace(ch)) {
                float prospectiveWidth = currentLineBox.width + runningWidth + width;

                if (prospectiveWidth > availableWidth &&
                    (currentLineBox.fragmentCount > 0 || runningAtomCount > 0)) {
                    bool hadPendingAtoms = runningAtomCount > 0;
                    if (runningAtomCount > 0) {
                        LineFragment frag{
                            .width = runningWidth,
                            .atomStart = runningAtomStart,
                            .atomCount = runningAtomCount,
                            .lineBoxIndex = currentLineBoxIndex,
                            .fragmentIndex = currentLineBox.fragmentCount
                        };
                        fragments.push_back(frag);
                        currentLineBox.pushFragment(frag);
                    }

                    lineBoxes.push_back(currentLineBox);
                    currentLineBox = {};
                    currentLineBoxIndex++;
                    lastFragmentHasBreakOpportunity = false;
                    runningWidth = hadPendingAtoms ? 0.0f : margins.left;
                    runningAtomCount = 0;
                    runningAtomStart = cluster.glyphStart;
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
                   isTextWhitespace(clusterCodepoint(shapedRun.clusters[idx]))) {
                const auto& whitespace = shapedRun.clusters[idx];
                runningWidth += clusterWidth(whitespace);
                runningAtomCount += whitespace.glyphCount;
                idx++;
            }

            runningWidth += margins.right;

            LineFragment frag{
                .width = runningWidth,
                .atomStart = runningAtomStart,
                .atomCount = runningAtomCount
            };

            if (allowSoftWrap && lastFragmentHasBreakOpportunity && currentLineBox.fragmentCount > 0 && currentLineBox.width + runningWidth > availableWidth) {
                lineBoxes.push_back(currentLineBox);
                currentLineBox = {};
                currentLineBoxIndex++;
            }

            frag.lineBoxIndex = currentLineBoxIndex;
            frag.fragmentIndex = currentLineBox.fragmentCount;
            fragments.push_back(frag);
            currentLineBox.pushFragment(frag);
            lastFragmentHasBreakOpportunity = true;

            runningWidth = 0.0;
            runningAtomCount = 0;
            runningAtomStart = idx < shapedRun.clusters.size()
                ? shapedRun.clusters[idx].glyphStart
                : atoms.size();
        }

        if (runningAtomCount > 0) {
            runningWidth += margins.right;

            LineFragment frag{
                .width = runningWidth,
                .atomStart = runningAtomStart,
                .atomCount = runningAtomCount
            };

            if (allowSoftWrap && lastFragmentHasBreakOpportunity && currentLineBox.fragmentCount > 0 && currentLineBox.width + runningWidth > availableWidth) {
                lineBoxes.push_back(currentLineBox);
                currentLineBox = {};
                currentLineBoxIndex++;
            }

            frag.lineBoxIndex = currentLineBoxIndex;
            frag.fragmentIndex = currentLineBox.fragmentCount;
            fragments.push_back(frag);
            currentLineBox.pushFragment(frag);
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

            float width = node->shared.width.has_value()
                ? node->shared.width->resolveOr(0.0f)
                : 0.0f;
            if (width > 0) {
                contentWidth = width;
            } else {
                contentWidth = constraints.availableWidth;
            }

            LayoutInput li{
                .position = position,
                .display = display,
                .width = width,
                .height = 0,
                .marginTop = marginTop,
                .marginRight = marginRight,
                .marginBottom = marginBottom,
                .marginLeft = marginLeft,
            };

            margins = LayoutEngine::resolveAutoMargins(li, constraints.replacedAttributes, constraints.availableWidth, contentWidth);
        } else {
            margins = {
                .top = marginTop.resolveOr(0.0f, 0.0f),
                .right = marginRight.resolveOr(0.0f, 0.0f),
                .bottom = marginBottom.resolveOr(0.0f, 0.0f),
                .left = marginLeft.resolveOr(0.0f, 0.0f),
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

    std::pair<std::vector<LineFragment>, std::vector<LineBox>> buildIsolatedInlineBoxes(TreeNode* node, float maxWidth) {
        std::vector<LineFragment> fragments;
        std::vector<LineBox> lineBoxes;
        LineBox currentLineBox{};
        size_t currentLineBoxIndex = 0;
        bool lastFragmentHasBreakOpportunity = false;

        auto textResp = getText(node);
        if (textResp.has_value()) {
            auto shapedRun = getShapedRun(node);
            if (!shapedRun) return {fragments, lineBoxes};
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
                fragments,
                lineBoxes,
                currentLineBox,
                currentLineBoxIndex,
                lastFragmentHasBreakOpportunity
            );
        }

        if (currentLineBox.fragmentCount > 0)
            lineBoxes.push_back(currentLineBox);

        return {fragments, lineBoxes};
    }

    std::tuple<std::vector<std::vector<LineFragment>>, std::vector<LineBox>> buildInlineBoxes(TreeNode* node, Constraints& childConstraints) {
        bool prevInline = false;
        std::vector<LineBox> childrenLineBoxes;
        std::vector<std::vector<LineFragment>> childrenLineFragments;
        LineBox currentLineBox {};
        size_t currentLineBoxIndex = 0;
        bool lastFragmentHasBreakOpportunity = false;


        for (uint64_t i = 0; i < node->children.size(); ++i) {
            auto& child = node->children[i];

            std::vector<LineFragment> childLineFragments;

            auto textResp = getText(child.get());

            if (textResp.has_value()) {
                auto shapedRun = getShapedRun(child.get());
                if (!shapedRun) {
                    childrenLineFragments.push_back({});
                    continue;
                }
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
                    childLineFragments,
                    childrenLineBoxes,
                    currentLineBox,
                    currentLineBoxIndex,
                    lastFragmentHasBreakOpportunity
                );

                prevInline = true;
            }else {
                prevInline = false;
            }

            childrenLineFragments.push_back(childLineFragments);
        }

        if (currentLineBox.fragmentCount > 0) {
            childrenLineBoxes.push_back(currentLineBox);
        }

        return {childrenLineFragments, childrenLineBoxes};
    }
}
