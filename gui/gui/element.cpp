#include "element.hpp"
#include "fragment_types.hpp"
#include "new_arch.hpp"
#include "printers.hpp"
#include <algorithm>
#include <any>
#include <cstdint>
#include <memory>
#include <print>
#include <ranges>
#include <simd/vector_types.h>
#include <unordered_map>
#include <ranges>


namespace NewArch {
    ElementBase::~ElementBase() {};

    // Helper functions for descriptor field access
    std::optional<Position> getPosition(TreeNode* node) {
        std::any request{DescriptorPayload{GetField{.name = "position"}}};
        auto resp = node->element->request(RequestTarget::Descriptor, request);
        if (resp.has_value()) {
            return std::any_cast<Position>(resp);
        }
        return std::nullopt;
    }

    std::optional<Display> getDisplay(TreeNode* node) {
        std::any request{DescriptorPayload{GetField{.name = "display"}}};
        auto resp = node->element->request(RequestTarget::Descriptor, request);
        if (resp.has_value()) {
            return std::any_cast<Display>(resp);
        }
        return std::nullopt;
    }

    std::optional<Size> getMarginTop(TreeNode* node) {
        std::any request{DescriptorPayload{GetField{.name = "marginTop"}}};
        auto resp = node->element->request(RequestTarget::Descriptor, request);
        if (resp.has_value()) {
            return std::any_cast<Size>(resp);
        }
        return std::nullopt;
    }

    std::optional<Size> getMarginBottom(TreeNode* node) {
        std::any request{DescriptorPayload{GetField{.name = "marginBottom"}}};
        auto resp = node->element->request(RequestTarget::Descriptor, request);
        if (resp.has_value()) {
            return std::any_cast<Size>(resp);
        }
        return std::nullopt;
    }

    std::optional<Size> getPaddingTop(TreeNode* node) {
        std::any request{DescriptorPayload{GetField{.name = "paddingTop"}}};
        auto resp = node->element->request(RequestTarget::Descriptor, request);
        if (resp.has_value()) {
            return std::any_cast<Size>(resp);
        }
        return std::nullopt;
    }

    std::optional<Size> getPaddingBottom(TreeNode* node) {
        std::any request{DescriptorPayload{GetField{.name = "paddingBottom"}}};
        auto resp = node->element->request(RequestTarget::Descriptor, request);
        if (resp.has_value()) {
            return std::any_cast<Size>(resp);
        }
        return std::nullopt;
    }

    std::optional<Size> getMarginLeft(TreeNode* node) {
        std::any request{DescriptorPayload{GetField{.name = "marginLeft"}}};
        auto resp = node->element->request(RequestTarget::Descriptor, request);
        if (resp.has_value()) {
            return std::any_cast<Size>(resp);
        }
        return std::nullopt;
    }

    std::optional<Size> getMarginRight(TreeNode* node) {
        std::any request{DescriptorPayload{GetField{.name = "marginRight"}}};
        auto resp = node->element->request(RequestTarget::Descriptor, request);
        if (resp.has_value()) {
            return std::any_cast<Size>(resp);
        }
        return std::nullopt;
    }

    std::optional<std::string> getText(TreeNode* node) {
        std::any request{DescriptorPayload{GetField{.name = "text"}}};
        auto resp = node->element->request(RequestTarget::Descriptor, request);
        if (resp.has_value()) {
            return std::any_cast<std::string>(resp);
        }
        return std::nullopt;
    }

    std::optional<float> getWidth(TreeNode* node) {
        std::any request{DescriptorPayload{GetField{.name = "width"}}};
        auto resp = node->element->request(RequestTarget::Descriptor, request);
        if (resp.has_value()) {
            return std::any_cast<float>(resp);
        }
        return std::nullopt;
    }

    std::optional<Size> getMargin(TreeNode* node) {
        std::any request{DescriptorPayload{GetField{.name = "margin"}}};
        auto resp = node->element->request(RequestTarget::Descriptor, request);
        if (resp.has_value()) {
            return std::any_cast<Size>(resp);
        }
        return std::nullopt;
    }

    std::optional<Size> getFlexGrow(TreeNode* node) {
        std::any request{DescriptorPayload{GetField{.name = "flexGrow"}}};
        auto resp = node->element->request(RequestTarget::Descriptor, request);
        if (resp.has_value()) {
            return std::any_cast<Size>(resp);
        }
        return std::nullopt;
    }

        std::optional<Size> getFlexShrink(TreeNode* node) {
        std::any request{DescriptorPayload{GetField{.name = "flexShrink"}}};
        auto resp = node->element->request(RequestTarget::Descriptor, request);
        if (resp.has_value()) {
            return std::any_cast<Size>(resp);
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
    

    // I have a render cache, develop some sort of caching policy that makes these useful
    void RenderTree::update(const FrameInfo& frameInfo) {
        auto root = getRoot();
        auto allNodes = collectAllNodes(root);


        rootCursor = simd_float2{0,0};
        rootConstraints = Constraints{
            .origin = simd_float2{0,0},
            .cursor = rootCursor,
            .maxWidth = frameInfo.width,
            .maxHeight = frameInfo.height,
            .frameInfo = frameInfo,
            .absoluteContainingBlock = {
                .origin = {0, 0},
                .width = frameInfo.width,
                .height = frameInfo.height
            },
        };

        // AHH APPLE CLANG DOESN'T SUPPORT EXECUTION POLICIES YET EXECUTE ME
        // Parallel::for_each(allNodes.begin(), allNodes.end(),
        //     [&](TreeNode* node) {
        //         node->measured = node->element->measure(rootConstraints);
        //     }
        // );

        measurePhase(root, rootConstraints);
        
        Parallel::for_each(allNodes.begin(), allNodes.end(),
            [&](TreeNode* node) {
                node->atomized = node->element->atomize(rootConstraints, *node->measured);
            }
        );
    
        // Layout pass
        // precompute margin metadata + intents
        preLayoutPhase(root, frameInfo, rootConstraints);
        // initial layout pass
        layoutPhase(root, frameInfo, rootConstraints);
        root->calculateGlobalZIndex(0);
        // postLayout: resolve global positions (serial, top-down) + reconcile atoms
        postLayoutPhase(root, frameInfo, rootConstraints, {0.0f, 0.0f}, {0.0f, 0.0f});

        Parallel::for_each(allNodes.begin(), allNodes.end(),
            [&](TreeNode* node) {
                node->placed = node->element->place(rootConstraints, *node->measured, 
                                                    *node->atomized, *node->layout);
            }
        );
        
        Parallel::for_each(allNodes.begin(), allNodes.end(),
            [&](TreeNode* node) {
                node->finalized = node->element->finalize(rootConstraints, *node->measured,
                                                        *node->atomized, *node->layout, *node->placed);
            }
        );
    }

    void RenderTree::render(MTL::RenderCommandEncoder* encoder) {
        auto root = getRoot();
        auto allNodes = collectAllNodes(root);

        std::sort(allNodes.begin(), allNodes.end(), [](TreeNode* a, TreeNode* b) {
            if (a->globalZIndex != b->globalZIndex) {
                return a->globalZIndex < b->globalZIndex;
            }
            return a->id < b->id;
        });
        
        // serially encoded; encoders are not thread safe
        for (auto node : allNodes) { 
            auto& finalized = node->finalized;
            node->element->encode(encoder, finalized);
        }
    }

    void RenderTree::measurePhase(TreeNode* node, Constraints& constraints) {
        auto measured = node->element->measure(constraints);
        node->measured = measured;
        
        Constraints childConstraints {};

        childConstraints.maxWidth = measured.explicitWidth.value_or(constraints.maxWidth);;
        childConstraints.maxHeight = measured.explicitHeight.value_or(constraints.maxHeight);

        for (auto& child : node->children) {
            measurePhase(child.get(), childConstraints);
        }
    }


    // consider safer way of accessing cache?
    void RenderTree::atomizePhase(TreeNode* node, Constraints& constraints) {
        // assert(renderCache.measured.contains(node->id) && 
        //    "atomizePhase called before measurePhase");
        assert(node->measured.has_value());


        auto& measured  = *node->measured;
        auto atomized = node->element->atomize(constraints, measured);
        node->atomized = atomized;
    }

    void buildCollapsedChains(
        TreeNode* node,
        std::unordered_map<ChainID, CollapsedChain>& chainMap,
        ChainID& nextChainId,
        CollapsedChain* collapsedTopChain,
        CollapsedChain* collapsedBottomChain
    )
    {
        // find first/last collapsable child
        TreeNode* firstInFlowCollapsableChild = nullptr;
        TreeNode* lastInFlowCollapsableChild = nullptr;

        for (auto& child : node->children) {
            auto position = getPosition(child.get());
            auto display = getDisplay(child.get());

            if (position.has_value() && display.has_value()) {
                if ((*position == Position::Static || *position == Position::Relative) && *display == Display::Block) {
                    if (firstInFlowCollapsableChild == nullptr) firstInFlowCollapsableChild = child.get();
                    lastInFlowCollapsableChild = child.get();
                }
            }
        }

        // check if out of flow
        auto currPosition = getPosition(node);
        bool currOutOfFlow = false;

        if (currPosition.has_value()) {
            currOutOfFlow = *currPosition == Position::Absolute || *currPosition == Position::Fixed;
        }

        if (currOutOfFlow) {
            firstInFlowCollapsableChild = nullptr;
            lastInFlowCollapsableChild = nullptr;
        }

        // skip continuation if padding defined
        bool nodeBlocksTopChain = getPaddingTop(node).has_value();
        bool nodeBlocksBottomChain = getPaddingBottom(node).has_value();

        if (nodeBlocksTopChain) {
            firstInFlowCollapsableChild = nullptr;
        }

        if (nodeBlocksBottomChain) {
            lastInFlowCollapsableChild = nullptr;
        }

        Size marginTop = getMarginTop(node).value_or(Size{});
        Size marginBottom = getMarginBottom(node).value_or(Size{});

        // check if has top chains; if not, create new ones (which will be propagated to first and last child)
        CollapsedChain newCollapsedTopChain;
        CollapsedChain newCollapsedBottomChain;
        CollapsedChain* propagatedTopChain = collapsedTopChain;
        CollapsedChain* propagatedBottomChain = collapsedBottomChain;

        if (!collapsedTopChain) {
            newCollapsedTopChain = {
                .id = nextChainId++,
                .root = node,
                .intent = marginTop,
                .depth = 1
            };

            propagatedTopChain = &newCollapsedTopChain;
        }else {
            if (marginTop.value > collapsedTopChain->intent.value) {
                collapsedTopChain->intent = marginTop;
            }
            
            collapsedTopChain->depth++;

            if (!firstInFlowCollapsableChild) {
                chainMap[collapsedTopChain->id] = *collapsedTopChain;
            }
        }

        if (!collapsedBottomChain) {
            newCollapsedBottomChain = {
                .id = nextChainId++,
                .root = node,
                .intent = marginBottom,
                .depth = 1
            };

            propagatedBottomChain = &newCollapsedBottomChain;
        }else {
            if (marginBottom.value > collapsedBottomChain->intent.value) {
                collapsedBottomChain->intent = marginBottom;
            }

            collapsedBottomChain->depth++;

            if (!lastInFlowCollapsableChild) {
                chainMap[collapsedBottomChain->id] = *collapsedBottomChain;
            }
        }

        for (auto& child : node->children) {
            auto rawChild = child.get();

            if (rawChild == firstInFlowCollapsableChild) {
                buildCollapsedChains(rawChild, chainMap, nextChainId, propagatedTopChain, nullptr);
            }else if (rawChild == lastInFlowCollapsableChild) {
                buildCollapsedChains(rawChild, chainMap, nextChainId, nullptr, propagatedBottomChain);
            }else {
                buildCollapsedChains(rawChild, chainMap, nextChainId, nullptr, nullptr);
            }
        }

        MarginMetadata marginMetadata {
            .topChainId = propagatedTopChain->id,
            .bottomChainId = propagatedBottomChain->id
        };

        node->preLayout = PreLayoutResult{};

        node->preLayout->marginMetadata = marginMetadata;
    }

    void precomputeMargins(TreeNode* node, Constraints& constraints, std::unordered_map<ChainID, CollapsedChain>& collapsedChainMap) {
        // Build replacedAttributes from collapse chain (same logic as layoutPhase)
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

        // Determine position and display
        auto position = getPosition(node).value_or(Position::Static);
        auto display = getDisplay(node).value_or(Display::Block);

        // Get margin values from descriptor (with fallback to default margin)
        Size defaultMargin = getMargin(node).value_or(Size{});
        Size marginTop = getMarginTop(node).value_or(defaultMargin);
        Size marginRight = getMarginRight(node).value_or(defaultMargin);
        Size marginBottom = getMarginBottom(node).value_or(defaultMargin);
        Size marginLeft = getMarginLeft(node).value_or(defaultMargin);

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

            float width = getWidth(node).value_or(0.0f);
            if (width > 0) {
                contentWidth = width;
            } else {
                contentWidth = constraints.maxWidth;
            }

            // Build a temporary LayoutInput for resolveAutoMargins
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
            // Inline, fixed, absolute: use simple resolution
            margins = {
                .top = marginTop.resolveOr(0.0f, 0.0f),
                .right = marginRight.resolveOr(0.0f, 0.0f),
                .bottom = marginBottom.resolveOr(0.0f, 0.0f),
                .left = marginLeft.resolveOr(0.0f, 0.0f),
            };
        }

        node->preLayout->resolvedMargins = margins;

        // Recurse for children with updated constraints
        auto& measured = *node->measured;
        Constraints childConstraints{};
        childConstraints.maxWidth = measured.explicitWidth.value_or(constraints.maxWidth);
        childConstraints.maxHeight = measured.explicitHeight.value_or(constraints.maxHeight);
        childConstraints.frameInfo = constraints.frameInfo;

        for (auto& child : node->children) {
            precomputeMargins(child.get(), childConstraints, collapsedChainMap);
        }
    }

    void RenderTree::preLayoutPhase(TreeNode* node, const FrameInfo& frameInfo, Constraints& constraints) {
        collapsedChainMap.clear();
        nextChainId = 0;

        buildCollapsedChains(node, collapsedChainMap, nextChainId, nullptr, nullptr);

        precomputeMargins(node, constraints, collapsedChainMap);
    }

    std::tuple<std::vector<std::vector<LineFragment>>, std::vector<LineBox>> buildInlineBoxes(TreeNode* node, Constraints& childConstraints) {
        // precompute line fragments & line boxes
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

    // std::vector<float> buildFlexContext(TreeNode* node, Constraints& childConstraints) {
    //     // contract: ensure is flex before caling this method
    //     // basic algorithm; not finalized becuase must run in measurement pahse

    //     // flex row impl
    //     float maxWidth = childConstraints.maxWidth;

    //     // get flex shrink and grow as transforms
        // float totalWidth {};
        // float shrinkScaledTotalWidth {};
        // float growthScaledTotalWidth {};

        // std::vector<float> childWidths;
        // std::vector<float> shrinkScaledChildWidths;
        // std::vector<float> growthScaledChildWidths;

        // for (auto& child : node->children) {
        //     auto rawChild = child.get();
        //     float childWidth = rawChild->measured->explicitWidth;

        //     auto flexGrow = getFlexGrow(rawChild);
        //     float resolvedFlexGrow = flexGrow->resolveOr(0.0, 0.0);

        //     auto flexShrink = getFlexShrink(rawChild);
        //     float resolvedFlexShrink = flexShrink->resolveOr(0.0, 1.0);

        //     childWidths.push_back(childWidth);

        //     if (resolvedFlexShrink > 0.0) {
        //         shrinkScaledChildWidths.push_back(childWidth * resolvedFlexShrink);
        //         shrinkScaledTotalWidth += childWidth * resolvedFlexShrink;
        //     }else {
        //         shrinkScaledChildWidths.push_back(0.0);
        //     }

        //     if (resolvedFlexGrow > 0.0 ){
        //         growthScaledChildWidths.push_back(childWidth * resolvedFlexGrow);
        //         growthScaledTotalWidth += childWidth * resolvedFlexGrow;
        //     }else {
        //         growthScaledChildWidths.push_back(0.0);
        //     }

        //     totalWidth += childWidth;
        // }
        

        // std::vector<float> flexWidths;

        // float spaceRemaining = maxWidth - totalWidth;

        // if (spaceRemaining > 0) {
        //     // grow
        //     for (int i = 0; i < childWidths.size(); ++i) {
        //         if (growthScaledChildWidths[i] > 0) {
        //             flexWidths.push_back(
        //                 childWidths[i] + (growthScaledChildWidths[i] / growthScaledTotalWidth) * spaceRemaining
        //             );
        //         }else {
        //             flexWidths.push_back(childWidths[i]);
        //         }
        //     }
        // }else if (spaceRemaining < 0) {
        //     for (int i = 0; i < childWidths.size(); ++i) {
        //         if (shrinkScaledChildWidths[i] > 0) {
        //             flexWidths.push_back(
        //                 childWidths[i] + (shrinkScaledChildWidths[i] / shrinkScaledTotalWidth) * spaceRemaining
        //             );
        //         }else {
        //             flexWidths.push_back(childWidths[i]);
        //         }
        //     }
        // }else {
        //     for (int i = 0; i < childWidths.size(); ++i) {
        //         flexWidths.push_back(childWidths[i]);
        //     }
        // }

    //     return flexWidths;
    // }

    void RenderTree::layoutPhase(TreeNode* node, const FrameInfo& frameInfo, Constraints& constraints) {
        assert(node->measured.has_value());
        assert(node->atomized.has_value());
        assert(node->preLayout.has_value());

        auto& measured = *node->measured;
        auto& atomized = *node->atomized;
        auto& prelayout = *node->preLayout;

        // attach precomputed margins and resolve existing node layout
        constraints.resolvedMargins = prelayout.resolvedMargins;
        auto layout = node->element->layout(constraints, measured, atomized);

        // prepare child constraints
        auto childConstraints = layout.childConstraints;
        auto position = getPosition(node);
        if (position.has_value() && *position != Position::Static) {
            childConstraints.absoluteContainingBlock = {
                .origin = {0.0f, 0.0f},
                .width = layout.computedBox.width,
                .height = layout.computedBox.height
            };
        } else {
            childConstraints.absoluteContainingBlock = constraints.absoluteContainingBlock;
        }

        // construct line boxes (if any)
        auto&& [childrenLineFragments, childrenLineBoxes] = buildInlineBoxes(node, childConstraints);
        childConstraints.lineBoxes = childrenLineBoxes;


        // prepare post layout context
        float minY = layout.childConstraints.origin.y;
        float maxY = layout.childConstraints.origin.y;
        float minX = layout.childConstraints.origin.x;
        float maxX = layout.childConstraints.origin.x;

        auto display = getDisplay(node).value_or(Display::Block);

        if (display == Display::Flex) {
            std::vector<LayoutResult*> flexChildrenLayouts;

            float totalWidth {};
            float shrinkScaledTotalWidth {};
            float growthScaledTotalWidth {};

            std::vector<float> childWidths;
            std::vector<float> shrinkScaledChildWidths;
            std::vector<float> growthScaledChildWidths;

            for (uint64_t i = 0; i < node->children.size(); ++i) {
                auto& child = node->children[i];
                auto childAsPtr = child.get();

                childConstraints.lineFragments = childrenLineFragments[i];
                childConstraints.inheritedProperties = constraints.inheritedProperties;

                layoutPhase(childAsPtr, frameInfo, childConstraints);
                auto& childLayout = *childAsPtr->layout;

                if (childLayout.outOfFlow) {
                    continue;
                }

                float childWidth = childLayout.computedBox.width;

                auto flexGrow = getFlexGrow(childAsPtr);
                float resolvedFlexGrow = flexGrow->resolveOr(0.0, 0.0);

                auto flexShrink = getFlexShrink(childAsPtr);
                float resolvedFlexShrink = flexShrink->resolveOr(0.0, 1.0);

                childWidths.push_back(childWidth);

                if (resolvedFlexShrink > 0.0) {
                    shrinkScaledChildWidths.push_back(childWidth * resolvedFlexShrink);
                    shrinkScaledTotalWidth += childWidth * resolvedFlexShrink;
                }else {
                    shrinkScaledChildWidths.push_back(0.0);
                }

                if (resolvedFlexGrow > 0.0 ){
                    growthScaledChildWidths.push_back(resolvedFlexGrow);
                    growthScaledTotalWidth += resolvedFlexGrow;
                }else {
                    growthScaledChildWidths.push_back(0.0);
                }

                totalWidth += childWidth;
            }

            std::vector<float> flexWidths;

            float spaceRemaining = measured.explicitWidth.value_or(constraints.maxWidth) - totalWidth;

            // std::println("maxWidth: {}, totalWidth: {}, space remaining: {}", constraints.maxWidth, totalWidth, spaceRemaining);

            if (spaceRemaining > 0) {
                // grow
                for (int i = 0; i < childWidths.size(); ++i) {
                    if (growthScaledChildWidths[i] > 0) {
                        flexWidths.push_back(
                            childWidths[i] + (growthScaledChildWidths[i] / growthScaledTotalWidth) * spaceRemaining
                        );
                    }else {
                        flexWidths.push_back(childWidths[i]);
                    }
                }
            }else if (spaceRemaining < 0) {
                for (int i = 0; i < childWidths.size(); ++i) {
                    if (shrinkScaledChildWidths[i] > 0) {
                        flexWidths.push_back(
                            childWidths[i] + (shrinkScaledChildWidths[i] / shrinkScaledTotalWidth) * spaceRemaining
                        );
                    }else {
                        flexWidths.push_back(childWidths[i]);
                    }
                }
            }else {
                for (int i = 0; i < childWidths.size(); ++i) {
                    flexWidths.push_back(childWidths[i]);
                }
            }

            float accumulatedX = 0;
            
            for (int i = 0; i < node->children.size(); ++i) {
                auto& child = node->children[i];
                auto childAsPtr = child.get();

                Constraints flexChildConstraints = childConstraints;
                flexChildConstraints.maxWidth = flexWidths[i];
                flexChildConstraints.origin.x = accumulatedX;
                flexChildConstraints.cursor = {0, 0};
                flexChildConstraints.lineFragments = childrenLineFragments[i];
                flexChildConstraints.inheritedProperties = constraints.inheritedProperties;

                childAsPtr->measured->explicitWidth = flexWidths[i];

                layoutPhase(childAsPtr, frameInfo, flexChildConstraints);
                auto& childLayout = *childAsPtr->layout;

                if (childLayout.outOfFlow) {
                    continue;
                }

                maxX = std::max(maxX, childLayout.computedBox.x + childLayout.computedBox.width);
                maxY = std::max(maxY, childLayout.computedBox.y + childLayout.consumedHeight);

                accumulatedX += flexWidths[i];
            }

        }else {
            for (uint64_t i = 0; i < node->children.size(); ++i) {
                auto& child = node->children[i];
                auto childAsPtr = child.get();

                childConstraints.lineFragments = childrenLineFragments[i];
                childConstraints.inheritedProperties = constraints.inheritedProperties;
                layoutPhase(childAsPtr, frameInfo, childConstraints);
                auto childLayout = *childAsPtr->layout;

                if (!childLayout.outOfFlow) {
                    childConstraints.cursor = childLayout.siblingCursor;
                    childConstraints.edgeIntent = childLayout.edgeIntent;
                    
                    maxX = std::max(maxX, childLayout.computedBox.x + childLayout.computedBox.width);
                    maxY = std::max(maxY, childLayout.computedBox.y + childLayout.consumedHeight);
                }
            }
        }

        // resize width/height of underspecified elements
        if (!measured.explicitWidth.has_value()) {
            if (*position != Position::Static)
                layout.computedBox.width = maxX - minX + layout.resolvedPadding.left + layout.resolvedPadding.right;
        }

        if (!measured.explicitHeight.has_value()) {
            layout.computedBox.height = maxY - minY + layout.resolvedPadding.top + layout.resolvedPadding.bottom;
            layout.consumedHeight = layout.computedBox.height;
        }

        // finalize layout of node
        node->layout = layout;
    }

    void RenderTree::postLayoutPhase(TreeNode* node, const FrameInfo& frameInfo, Constraints& constraints,
                                      simd_float2 parentGlobalOrigin, simd_float2 absBlockGlobalOrigin) {
        assert(node->measured.has_value());
        assert(node->atomized.has_value());
        assert(node->layout.has_value());

        auto& layout = *node->layout;
        auto position = getPosition(node).value_or(Position::Static);

        auto& dp = layout.deferredPosition;
        if (dp.needsRightResolution) {
            float newX = dp.containingBlockWidth - dp.marginRight - layout.computedBox.width - dp.resolvedRight;
            float deltaX = newX - layout.computedBox.x;
            layout.computedBox.x = newX;
            for (auto& offset : layout.atomOffsets) {
                offset.x += deltaX;
            }
        }
        if (dp.needsBottomResolution) {
            float newY = dp.containingBlockHeight - dp.marginBottom - layout.computedBox.height - dp.resolvedBottom;
            float deltaY = newY - layout.computedBox.y;
            layout.computedBox.y = newY;
            for (auto& offset : layout.atomOffsets) {
                offset.y += deltaY;
            }
        }

        simd_float2 baseOrigin;
        if (position == Position::Fixed) {
            baseOrigin = {0.0f, 0.0f};
        } else if (position == Position::Absolute) {
            baseOrigin = absBlockGlobalOrigin;
        } else {
            baseOrigin = parentGlobalOrigin;
        }

        layout.computedBox.x += baseOrigin.x;
        layout.computedBox.y += baseOrigin.y;
        for (auto& offset : layout.atomOffsets) {
            offset.x += baseOrigin.x;
            offset.y += baseOrigin.y;
        }
        node->globalOffset = baseOrigin;

        node->atomized = node->element->postLayout(constraints, *node->measured,
                                                    *node->atomized, layout);

        simd_float2 currContentOrigin = {
            layout.computedBox.x + layout.resolvedPadding.left,
            layout.computedBox.y + layout.resolvedPadding.top
        };

        simd_float2 childAbsBlockOrigin = absBlockGlobalOrigin;
        if (position != Position::Static) {
            childAbsBlockOrigin = currContentOrigin;
        }

        for (auto& child : node->children) {
            postLayoutPhase(child.get(), frameInfo, constraints,
                           currContentOrigin, childAbsBlockOrigin);
        }
    }

    void RenderTree::placePhase(TreeNode* node, const FrameInfo& frameInfo, Constraints& constraints) {
        assert(node->measured.has_value());
        assert(node->atomized.has_value());
        assert(node->layout.has_value());
        
        auto& measured = *node->measured;
        auto& atomized = *node->atomized;
        auto& layout = *node->layout;

        auto placed = node->element->place(constraints, measured, atomized, layout);
        node->placed = placed;
    }

    void RenderTree::finalizePhase(TreeNode* node, Constraints& constraints) {
        // I thought runtime checks for private methods were stupid, but I also am not superhuman and LLMs can hallucinate,
        // so debug asserts never hurt
        assert(node->measured.has_value());
        assert(node->atomized.has_value());
        assert(node->finalized.has_value());
        
            
        auto& measured =  *node->measured;
        auto& atomized = *node->atomized;
        auto& layout = *node->layout;
        auto& placed = *node->placed;
        auto finalized = node->element->finalize(constraints, measured, atomized, layout, placed);
        node->finalized = finalized;
    }

    TreeNode* RenderTree::hitTestRecursive(TreeNode* node, simd_float2 point) {
        if (!node) return nullptr;
        
        std::vector<TreeNode*> sortedChildren;
        for (auto& child : node->children) {
            sortedChildren.push_back(child.get());
        }
        std::sort(sortedChildren.begin(), sortedChildren.end(), 
            [](TreeNode* a, TreeNode* b) {
                if (a->globalZIndex != b->globalZIndex) {
                    return a->globalZIndex > b->globalZIndex;
                }
                return a->id > b->id;
            });
        
        for (auto* child : sortedChildren) {
            TreeNode* hit = hitTestRecursive(child, point);
            if (hit) return hit;
        }
        
        if (node->contains(point)) {
            return node;
        }
        
        return nullptr;
    }
}
