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

        for (auto& child : node->children) {
            measurePhase(child.get(), childConstraints);
        }
    }


    // consider safer way of accessing cache?
    void RenderTree::atomizePhase(TreeNode* node, Constraints& constraints) {
        // assert(renderCache.measured.contains(node->id) && 
        //    "atomizePhase called before measurePhase");
        assert(node->measured.has_value() && 
           "atomizePhase called before measurePhase");


        auto& measured  = *node->measured;
        auto atomized = node->element->atomize(constraints, measured);
        node->atomized = atomized;
    }

    void buildCollapsedChains(std::unordered_map<ChainID, CollapsedChain>& chainMap, ChainID& nextChainId, CollapsedChain& collapsedTopChain, CollapsedChain& collapsedBottomChain, TreeNode* node, int depth = 0) {
        TreeNode* firstInFlowCollapsableChild = nullptr;
        TreeNode* lastInFlowCollapsableChild = nullptr;

        std::any positionRequest {DescriptorPayload{
            GetField{
                .name="position"
            },
        }};

        std::any displayRequest {DescriptorPayload{
            GetField{
                .name="display"
            },
        }};

        for (auto& child : node->children) {
            auto positionResp = child->element->request(RequestTarget::Descriptor, positionRequest);
            auto displayResp = child->element->request(RequestTarget::Descriptor, displayRequest);

            if (positionResp.has_value() && displayResp.has_value()) {
                auto position = std::any_cast<Position>(positionResp);
                auto display = std::any_cast<Display>(displayResp);

                if (position == Position::Static && display == Display::Block) {
                    if (firstInFlowCollapsableChild == nullptr) firstInFlowCollapsableChild = child.get();
                    lastInFlowCollapsableChild = child.get();
                }
            }
        }

        std::any marginTopRequest {DescriptorPayload{
            GetField{
                .name="marginTop"
            },
        }};

        std::any marginBottomRequest {DescriptorPayload{
            GetField{
                .name="marginBottom"
            },
        }};

        std::any paddingTopRequest {DescriptorPayload{
            GetField{
                .name="paddingTop"
            }
        }};

        std::any paddingBottomRequest {DescriptorPayload{
            GetField{
                .name="paddingBottom"
            }
        }};

        auto currPositionResp = node->element->request(RequestTarget::Descriptor, positionRequest);
        bool currOutOfFlow = false;

        if (currPositionResp.has_value()) {
            auto pos = std::any_cast<Position>(currPositionResp);
            currOutOfFlow = pos == NewArch::Position::Absolute || pos == NewArch::Position::Fixed;
        }

        if (currOutOfFlow) {
            firstInFlowCollapsableChild = nullptr;
            lastInFlowCollapsableChild = nullptr;
        }

        auto nodePaddingTopResp = node->element->request(RequestTarget::Descriptor, paddingTopRequest);
        auto nodePaddingBottomResp = node->element->request(RequestTarget::Descriptor, paddingBottomRequest);
        bool nodeBlocksTopChain = nodePaddingTopResp.has_value();
        bool nodeBlocksBottomChain = nodePaddingBottomResp.has_value();

        if (nodeBlocksTopChain) {
            firstInFlowCollapsableChild = nullptr;
        }

        if (nodeBlocksBottomChain) {
            lastInFlowCollapsableChild = nullptr;
        }

        if (firstInFlowCollapsableChild) {
            auto resp = firstInFlowCollapsableChild->element->request(RequestTarget::Descriptor, marginTopRequest);

            if (resp.has_value()) {
                auto margin = std::any_cast<Size>(resp);
                if (margin.value > collapsedTopChain.intent.value) {
                    collapsedTopChain.intent = margin;
                }
            }
            collapsedTopChain.depth++;
        } else {
            chainMap[collapsedTopChain.id] = collapsedTopChain;
        }

        if (lastInFlowCollapsableChild) {
            auto resp = lastInFlowCollapsableChild->element->request(RequestTarget::Descriptor, marginBottomRequest);

            if (resp.has_value()) {
                auto margin = std::any_cast<Size>(resp);
                if (margin.value > collapsedBottomChain.intent.value) {
                    collapsedBottomChain.intent = margin;
                }
            }
            collapsedBottomChain.depth++;
        } else {
            chainMap[collapsedBottomChain.id] = collapsedBottomChain;
        }

        for (auto& child : node->children) {
            auto rawChild = child.get();

            // Check child's padding to determine if it blocks chain propagation
            auto childPaddingTopResp = rawChild->element->request(RequestTarget::Descriptor, paddingTopRequest);

            auto childPaddingBottomResp = rawChild->element->request(RequestTarget::Descriptor, paddingBottomRequest);

            if (rawChild == firstInFlowCollapsableChild) {
                // First child: belongs to collapsedTopChain, gets new bottom chain
                rawChild->marginMetadata.topChainId = collapsedTopChain.id;

                auto marginResp = rawChild->element->request(RequestTarget::Descriptor, marginBottomRequest);
                Size margin {};
                if (marginResp.has_value()) {
                    margin = std::any_cast<Size>(marginResp);
                }

                CollapsedChain newCollapsedBottomChain {
                    .id = nextChainId++,
                    .root = rawChild,
                    .intent = margin,
                    .depth = 1
                };
                rawChild->marginMetadata.bottomChainId = newCollapsedBottomChain.id;

                buildCollapsedChains(chainMap, nextChainId, collapsedTopChain, newCollapsedBottomChain, rawChild, depth + 1);

            } else if (rawChild == lastInFlowCollapsableChild) {
                // Last child: gets new top chain, belongs to collapsedBottomChain
                auto marginResp = rawChild->element->request(RequestTarget::Descriptor, marginTopRequest);
                Size margin {};
                if (marginResp.has_value()) {
                    margin = std::any_cast<Size>(marginResp);
                }

                CollapsedChain newCollapsedTopChain {
                    .id = nextChainId++,
                    .root = rawChild,
                    .intent = margin,
                    .depth = 1
                };
                rawChild->marginMetadata.topChainId = newCollapsedTopChain.id;
                rawChild->marginMetadata.bottomChainId = collapsedBottomChain.id;

                buildCollapsedChains(chainMap, nextChainId, newCollapsedTopChain, collapsedBottomChain, rawChild, depth + 1);
            } else {
                // Middle child or out-of-flow: gets fresh chains for both
                auto topResp = rawChild->element->request(RequestTarget::Descriptor, marginTopRequest);
                auto botResp = rawChild->element->request(RequestTarget::Descriptor, marginBottomRequest);

                Size marginTop {};
                Size marginBottom {};
                if (topResp.has_value()) {
                    marginTop = std::any_cast<Size>(topResp);
                }
                if (botResp.has_value()) {
                    marginBottom = std::any_cast<Size>(botResp);
                }

                CollapsedChain newCollapsedTopChain {
                    .id = nextChainId++,
                    .root = rawChild,
                    .intent = marginTop,
                    .depth = 1
                };

                CollapsedChain newCollapsedBottomChain {
                    .id = nextChainId++,
                    .root = rawChild,
                    .intent = marginBottom,
                    .depth = 1
                };

                rawChild->marginMetadata.topChainId = newCollapsedTopChain.id;
                rawChild->marginMetadata.bottomChainId = newCollapsedBottomChain.id;

                buildCollapsedChains(chainMap, nextChainId, newCollapsedTopChain, newCollapsedBottomChain, rawChild, depth + 1);
            }
        }
    }

    void RenderTree::preLayoutPhase(TreeNode* node, const FrameInfo& frameInfo, Constraints& constraints) {
        collapsedChainMap.clear();
        nextChainId = 0;

        for (auto& child : node->children) {
            auto rawChild = child.get();

            CollapsedChain topChain {
                .id = nextChainId++,
                .root = rawChild,
                .intent = Size{},
                .depth = 1,
            };

            CollapsedChain bottomChain {
                .id = nextChainId++,
                .root = rawChild,
                .intent = Size{},
                .depth = 1,
            };

            // Parent sets child's chainIds before recursing
            rawChild->marginMetadata.topChainId = topChain.id;
            rawChild->marginMetadata.bottomChainId = bottomChain.id;

            buildCollapsedChains(collapsedChainMap, nextChainId, topChain, bottomChain, rawChild);
        }

        // std::println("\n=== COLLAPSED CHAINS SUMMARY ===");
        // std::println("Total chains: {}", collapsedChainMap.size());

        // for (auto& [id, chain] : collapsedChainMap) {
        //     std::println("  [id={}] root node id: {}, intent: {:.2f} depth: {}", id, chain.root->id, chain.intent.value, chain.depth);
        // }
    }

    // DONE SERIALLY
    void RenderTree::layoutPhase(TreeNode* node, const FrameInfo& frameInfo, Constraints& constraints) {
        assert(node->measured.has_value() && 
            "layoutPhase called before measurePhase");
        assert(node->atomized.has_value() && 
            "layoutPhase called before atomizePhase");

        auto& measured = *node->measured;
        auto& atomized = *node->atomized;
        
        constraints.replacedAttributes = {};

        if (node->marginMetadata.topChainId.has_value()) {
            auto topChain = collapsedChainMap[node->marginMetadata.topChainId.value()];
            if (topChain.depth > 1) {
                if (node == topChain.root) {
                    constraints.replacedAttributes.marginTop = topChain.intent;
                }else {
                    constraints.replacedAttributes.marginTop = Size{};
                }
            }
        }

        if (node->marginMetadata.bottomChainId.has_value()) {
            auto bottomChain = collapsedChainMap[node->marginMetadata.bottomChainId.value()];
            if (bottomChain.depth > 1) {
                if (node == bottomChain.root) {
                    constraints.replacedAttributes.marginBottom = bottomChain.intent;
                }else {
                    constraints.replacedAttributes.marginBottom = Size{};
                }
            }
        }

        auto layout = node->element->layout(constraints, measured, atomized);
        node->layout = layout;

        auto childConstraints =  layout.childConstraints;

        // precompute line fragments & line boxes
        bool prevInline = false;
        bool trailingLineBox = false;
        std::vector<LineBox> childrenLineBoxes;
        std::vector<std::vector<LineFragment>> childrenLineFragments;


        LineBox currentLineBox {};
        size_t currentLineBoxIndex = 0;

        for (uint64_t i = 0; i < node->children.size(); ++i) {
            auto& child = node->children[i];

            std::vector<LineFragment> childLineFragments;

            // request
            DescriptorPayload rawPayload {
                GetField{
                    .name="text"
                },
            };
            std::any payload = rawPayload;
            auto resp = child->element->request(RequestTarget::Descriptor, payload);

            // value check
            if (resp.has_value()) {
                auto text = std::any_cast<std::string>(resp);

                float runningWidth = 0.0;
                size_t runningAtomCount = 0;
                bool isFirstLineFragment = true;

                auto& atoms = child->atomized->atoms;
                size_t idx = 0;

                // if prevInline; add to current line box
                // else, make new linebox
                // what to do with trailing line boxes?
                // how do we know we have a trailing line box


                // checks for gaps
                
                if (i > 0 && !prevInline) {
                    childrenLineBoxes.push_back(currentLineBox);
                    currentLineBox = {};
                    currentLineBoxIndex++;
                }

                while (idx < text.size() && idx < atoms.size()) {
                    char ch = text[idx];

                    if (ch != ' ') {
                        // Non-space: accumulate
                        runningWidth += atoms[idx].width;
                        runningAtomCount++;
                        idx++;
                    } else {
                        // Space: add all consecutive spaces to current line fragment
                        while (idx < text.size() && text[idx] == ' ') {
                            runningWidth += atoms[idx].width;
                            runningAtomCount++;
                            idx++;
                        }
                        // Push line fragment with trailing spaces
                        if (runningAtomCount > 0) {
                            LineFragment lineFragment {
                                .width = runningWidth,
                                .atomCount = runningAtomCount,
                                .collapsable = isFirstLineFragment,
                                .lineBoxIndex = currentLineBoxIndex,
                                .fragmentIndex = currentLineBox.fragmentCount
                            };

                            childLineFragments.push_back(lineFragment);
                            currentLineBox.pushFragment(lineFragment);
                            childrenLineBoxes.push_back(currentLineBox);
                            currentLineBox = {};
                            currentLineBoxIndex++;
                            trailingLineBox = false;
                            isFirstLineFragment = false;
                        }
                        runningWidth = 0.0;
                        runningAtomCount = 0;
                    }
                }

                // Push final line fragment if any
                if (runningAtomCount > 0) {
                    LineFragment lineFragment {
                        .width = runningWidth,
                        .atomCount = runningAtomCount,
                        .collapsable = isFirstLineFragment,
                        .lineBoxIndex = currentLineBoxIndex,
                        .fragmentIndex = currentLineBox.fragmentCount
                    };

                    childLineFragments.push_back(lineFragment);
                    currentLineBox.pushFragment(lineFragment);
                    trailingLineBox = true;
                }
                prevInline = true;
            }else {
                prevInline = false;
            }

            childrenLineFragments.push_back(childLineFragments);
            // std::println("child line boxes: {} child line fragments: {}", childrenLineBoxes.size(), childLineFragments.size());
        }

        if (trailingLineBox) {
            childrenLineBoxes.push_back(currentLineBox);
        }

        
        auto flatViewFragments = childrenLineFragments | std::views::join;

        for (auto& fragment : flatViewFragments) {
            std::println("fragment lb: {} fragment idx: {}", fragment.lineBoxIndex, fragment.fragmentIndex);
        }

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
        assert(node->measured.has_value() && 
            "finalizePhase: missing measured");
        assert(node->atomized.has_value() && 
            "finalizePhase: missing atomized");
        assert(node->layout.has_value() && 
            "finalizePhase: missing layout");
        
        auto& measured = *node->measured;
        auto& atomized = *node->atomized;
        auto& layout = *node->layout;

        auto placed = node->element->place(constraints, measured, atomized, layout);
        node->placed = placed;
    }

    void RenderTree::finalizePhase(TreeNode* node, Constraints& constraints) {
        // I thought runtime checks for private methods were stupid, but I also am not superhuman and LLMs can hallucinate,
        // so debug asserts never hurt
        assert(node->measured.has_value() && 
            "finalizePhase: missing measured");
        assert(node->atomized.has_value() && 
            "finalizePhase: missing atomized");
        assert(node->finalized.has_value() && 
            "finalizePhase: missing placed");
        
            
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
