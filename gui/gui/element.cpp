#include "element.hpp"
#include "text.hpp"
#include "new_arch.hpp"
#include "printers.hpp"
#include <any>
#include <cstdint>
#include <map>
#include <memory>
#include <print>
#include <simd/vector_types.h>
#include <unordered_map>


    
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
        
        // std::println("root ptr: {}", reinterpret_cast<void*>(root));
        preLayoutPhase(root, frameInfo, rootConstraints);
        
        // for (auto& chain : collapsed) {
        //     std
        // }

        layoutPhase(root, frameInfo, rootConstraints);
        root->calculateGlobalZIndex(0);

        // std::println("\n\n");
        
        Parallel::for_each(allNodes.begin(), allNodes.end(),
            [&](TreeNode* node) {
                node->placed = node->element->place(rootConstraints, *node->measured, 
                                                    *node->atomized, *node->layout);
            }
        );
        
        Parallel::for_each(allNodes.begin(), allNodes.end(),
            [&](TreeNode* node) {
                node->finalized = node->element->finalize(rootConstraints, *node->measured,
                                                        *node->atomized, *node->placed);
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

        childConstraints.maxWidth = measured.explicitWidth;
        childConstraints.maxHeight = measured.explicitHeight;

        // measure margins now?

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

    void RenderTree::precomputeMargins(TreeNode* node, Constraints& constraints) {
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
        childConstraints.maxWidth = measured.explicitWidth;
        childConstraints.maxHeight = measured.explicitHeight;
        childConstraints.frameInfo = constraints.frameInfo;

        for (auto& child : node->children) {
            precomputeMargins(child.get(), childConstraints);
        }
    }

    void RenderTree::preLayoutPhase(TreeNode* node, const FrameInfo& frameInfo, Constraints& constraints) {
        collapsedChainMap.clear();
        nextChainId = 0;

        buildCollapsedChains(node, collapsedChainMap, nextChainId, nullptr, nullptr);

        precomputeMargins(node, constraints);
    }

    std::tuple<std::vector<std::vector<LineFragment>>, std::vector<LineBox>> buildInlineBoxes(TreeNode* node, Constraints& childConstraints) {
        // precompute line fragments & line boxes
        bool prevInline = false;
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

                // construct current fragment.
                runningWidth += marginLeft;

                while (idx < text.size() && idx < atoms.size()) {
                    char ch = text[idx];

                    if (ch != ' ') {
                        runningWidth += atoms[idx].width;
                        runningAtomCount++;
                        idx++;
                        
                        if (idx < text.size())
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
                            .lineBoxIndex = currentLineBoxIndex,
                            .fragmentIndex = currentLineBox.fragmentCount
                        };

                        childLineFragments.push_back(lineFragment);
                        currentLineBox.pushFragment(lineFragment);
                    
                        childrenLineBoxes.push_back(currentLineBox);
                        currentLineBox = {};
                        currentLineBoxIndex++;

                        runningWidth = 0.0;
                        runningAtomCount = 0;

                    }
                
                }

                if (runningAtomCount > 0) {                        
                    runningWidth += marginRight;

                    LineFragment lineFragment {
                        .width = runningWidth,
                        .atomCount = runningAtomCount,
                        .lineBoxIndex = currentLineBoxIndex,
                        .fragmentIndex = currentLineBox.fragmentCount
                    };

                    childLineFragments.push_back(lineFragment);
                    currentLineBox.pushFragment(lineFragment);

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

    // DONE SERIALLY
    void RenderTree::layoutPhase(TreeNode* node, const FrameInfo& frameInfo, Constraints& constraints) {
        assert(node->measured.has_value());
        assert(node->atomized.has_value());
        assert(node->preLayout.has_value());

        auto& measured = *node->measured;
        auto& atomized = *node->atomized;
   
        constraints.resolvedMargins = node->preLayout->resolvedMargins;

        auto layout = node->element->layout(constraints, measured, atomized);
        node->layout = layout;

        auto childConstraints = layout.childConstraints;

        // If this node is positioned, it becomes the containing block for absolute descendants
        auto position = getPosition(node);
        if (position.has_value() && *position != Position::Static) {
            childConstraints.absoluteContainingBlock = {
                .origin = {layout.computedBox.x, layout.computedBox.y},
                .width = layout.computedBox.width,
                .height = layout.computedBox.height
            };
        } else {
            childConstraints.absoluteContainingBlock = constraints.absoluteContainingBlock;
        }

        auto&& [childrenLineFragments, childrenLineBoxes] = buildInlineBoxes(node, childConstraints);
        
        // auto flatViewFragments = childrenLineFragments | std::views::join;

        // for (auto& fragment : flatViewFragments) {
        //     std::println("fragment lb: {} fragment idx: {}", fragment.lineBoxIndex, fragment.fragmentIndex);
        // }

        for (uint64_t i = 0; i < node->children.size(); ++i) {
            auto& child = node->children[i];
            auto childAsPtr = child.get();

            // Pass precomputed line fragments and line boxes to child
            childConstraints.lineFragments = childrenLineFragments[i];
            childConstraints.lineBoxes = childrenLineBoxes;


            childConstraints.inheritedProperties = constraints.inheritedProperties;
            layoutPhase(childAsPtr, frameInfo, childConstraints);
            auto childLayout = *childAsPtr->layout;

            if (!childLayout.outOfFlow) {
                childConstraints.cursor = childLayout.siblingCursor;
                childConstraints.edgeIntent = childLayout.edgeIntent;
            }
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
        auto& placed = *node->placed;
        auto finalized = node->element->finalize(constraints, measured, atomized, placed);
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
