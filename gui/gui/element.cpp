#include "element.hpp"
#include "fragment_types.hpp"
#include "sizing.hpp"
#include "new_arch.hpp"
#include <algorithm>
#include <any>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <simd/vector_types.h>
#include <string>
#include <unordered_map>


namespace NewArch {
    ElementBase::~ElementBase() {};

    // Element-specific requests still use the request system
    std::optional<std::u32string> getText(TreeNode* node) {
        std::any request{DescriptorPayload{GetField{.name = "text"}}};
        auto resp = node->element->request(RequestTarget::Descriptor, request);
        if (resp.has_value()) {
            return std::any_cast<std::u32string>(resp);
        }
        return std::nullopt;
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
                contentWidth = constraints.maxWidth;
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

            margins = LayoutEngine::resolveAutoMargins(li, constraints.replacedAttributes, constraints.maxWidth, contentWidth);
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
        childConstraints.maxWidth = measured.explicitWidth.value_or(constraints.maxWidth);
        childConstraints.maxHeight = measured.explicitHeight.value_or(constraints.maxHeight);
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
            auto margins = node->preLayout->resolvedMargins;
            auto text = *textResp;
            auto& atoms = node->atomized->atoms;

            float runningWidth = margins.left;
            size_t runningAtomCount = 0;
            size_t idx = 0;

            while (idx < text.size() && idx < atoms.size()) {
                if (text[idx] != ' ') {
                    runningWidth += atoms[idx].width;
                    runningAtomCount++;
                    idx++;
                    continue;
                }

                while (idx < text.size() && text[idx] == ' ') {
                    runningWidth += atoms[idx].width;
                    runningAtomCount++;
                    idx++;
                }

                runningWidth += margins.right;

                LineFragment frag{.width = runningWidth, .atomCount = runningAtomCount};

                if (lastFragmentHasBreakOpportunity && currentLineBox.fragmentCount > 0 && currentLineBox.width + runningWidth > maxWidth) {
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
            }

            if (runningAtomCount > 0) {
                runningWidth += margins.right;
                LineFragment frag{.width = runningWidth, .atomCount = runningAtomCount};

                if (lastFragmentHasBreakOpportunity && currentLineBox.fragmentCount > 0 && currentLineBox.width + runningWidth > maxWidth) {
                    lineBoxes.push_back(currentLineBox);
                    currentLineBox = {};
                    currentLineBoxIndex++;
                }

                frag.lineBoxIndex = currentLineBoxIndex;
                frag.fragmentIndex = currentLineBox.fragmentCount;
                fragments.push_back(frag);
                currentLineBox.pushFragment(frag);
            }
        }

        if (currentLineBox.fragmentCount > 0)
            lineBoxes.push_back(currentLineBox);

        return {fragments, lineBoxes};
    }

    std::tuple<std::vector<std::vector<LineFragment>>, std::vector<LineBox>> buildInlineBoxes(TreeNode* node, Constraints& childConstraints) {
        bool prevInline = false;
        bool lastFragmentHasBreakOpportunity = false;
        std::vector<LineBox> childrenLineBoxes;
        std::vector<std::vector<LineFragment>> childrenLineFragments;
        LineBox currentLineBox {};
        size_t currentLineBoxIndex = 0;


        for (uint64_t i = 0; i < node->children.size(); ++i) {
            auto& child = node->children[i];

            std::vector<LineFragment> childLineFragments;

            auto textResp = getText(child.get());

            if (textResp.has_value()) {
                auto margins = child->preLayout->resolvedMargins;
                auto marginLeft = margins.left;
                auto marginRight = margins.right;
                auto text = *textResp;

                float runningWidth = 0.0;
                size_t runningAtomCount = 0;

                auto& atoms = child->atomized->atoms;
                size_t idx = 0;

                if (i > 0 && !prevInline && currentLineBox.fragmentCount > 0) {
                    childrenLineBoxes.push_back(currentLineBox);
                    currentLineBox = {};
                    currentLineBoxIndex++;
                }

                runningWidth += marginLeft;

                while (idx < text.size() && idx < atoms.size()) {
                    char ch = text[idx];

                    if (ch != ' ') {
                        runningWidth += atoms[idx].width;
                        runningAtomCount++;
                        idx++;
                        
                        continue;
                    }else {
                        while (idx < text.size() && text[idx] == ' ') {
                            runningWidth += atoms[idx].width;
                            runningAtomCount++;
                            idx++;
                        }

                        runningWidth += marginRight;

                        LineFragment lineFragment {
                            .width = runningWidth,
                            .atomCount = runningAtomCount,
                        };

                        if (lastFragmentHasBreakOpportunity && currentLineBox.fragmentCount > 0 && currentLineBox.width + runningWidth > childConstraints.maxWidth) {
                            childrenLineBoxes.push_back(currentLineBox);
                            currentLineBox = {};
                            currentLineBoxIndex++;
                        }

                        lineFragment.lineBoxIndex = currentLineBoxIndex;
                        lineFragment.fragmentIndex = currentLineBox.fragmentCount;

                        childLineFragments.push_back(lineFragment);
                        currentLineBox.pushFragment(lineFragment);
                        lastFragmentHasBreakOpportunity = true;

                        runningWidth = 0.0;
                        runningAtomCount = 0;
                    }
                }

                if (runningAtomCount > 0) {                        
                    runningWidth += marginRight;

                    LineFragment lineFragment {
                        .width = runningWidth,
                        .atomCount = runningAtomCount,
                    };

                    if (lastFragmentHasBreakOpportunity && currentLineBox.fragmentCount > 0 && currentLineBox.width + runningWidth > childConstraints.maxWidth) {
                        childrenLineBoxes.push_back(currentLineBox);
                        currentLineBox = {};
                        currentLineBoxIndex++;
                    }

                    lineFragment.lineBoxIndex = currentLineBoxIndex;
                    lineFragment.fragmentIndex = currentLineBox.fragmentCount;

                    childLineFragments.push_back(lineFragment);
                    currentLineBox.pushFragment(lineFragment);
                    lastFragmentHasBreakOpportunity = false;

                    runningWidth = 0.0;
                    runningAtomCount = 0;
                }

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
