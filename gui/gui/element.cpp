#include "element.hpp"
#include "text.hpp"
#include "new_arch.hpp"
#include "printers.hpp"
#include <any>
#include <cstdint>
#include <print>
#include <simd/vector_types.h>


    
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
        
        layoutPhase(root, frameInfo, rootConstraints);
        root->calculateGlobalZIndex(0);
        
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

    void RenderTree::layoutPreprocess(TreeNode* node, const FrameInfo& frameInfo, Constraints& constraints) {
        // do asserts here
        
        TreeNode* firstInFlowChild = nullptr;

        TreeNode* lastInFlowChild = nullptr;

        DescriptorPayload rawPayload {
            GetField{
                .name="Position"
            },
        };

        std::any payload {rawPayload};

        for (auto& child : node->children) {
            // if request says in flow (not absolute/fixed)
                // if firstInFlowChild == nullptr; set
                // set lastInFlowChild (want last one)


            auto resp = child->element->request(RequestTarget::Descriptor, payload);

            if (resp.has_value()) {
                auto flow = std::any_cast<Position>(resp);

                if (flow == Position::Relative) {
                    if (firstInFlowChild == nullptr) firstInFlowChild = child.get();
                    lastInFlowChild = child.get();
                }
            }
        }

        // if firstInFlowChild == nullptr (both are null)
            // check if size == 0 (useless condition, here for clarity)
                // if in flow
                    // this->collapseWithAncestor = true
                // return
            // otherwise, still preprocess children
                // find first/last child; preprocess
                // this->collapseWithAncestor = false
                // return

        
        if (firstInFlowChild) {
            firstInFlowChild->treeConstraints.collapseWithAncestor = true;
            lastInFlowChild->treeConstraints.collapseWithAncestor = true;
        }

        for (auto& child : node->children) {
            layoutPreprocess(child.get(), frameInfo, constraints);
        }
    }

    // DONE SERIALLY
    void RenderTree::layoutPhase(TreeNode* node, const FrameInfo& frameInfo, Constraints& constraints) {
        assert(node->measured.has_value() && 
            "layoutPhase called before measurePhase");
        assert(node->atomized.has_value() && 
            "layoutPhase called before atomizePhase");

        auto& measured = *node->measured;
        auto& atomized = *node->atomized;
        

        // check constraints.parentMarginCollapses (only true if relative)
            // if true 
            // set first child's parentCollapses to true


        auto layout = node->element->layout(constraints, measured, atomized);
        node->layout = layout;

        auto childConstraints =  layout.childConstraints;

        // precompute lineboxes
        std::vector<std::vector<Line>> childrenLineboxes;

        // check first child
            // request margin ?

        // update margin ? of child and parent if it has margin and parent has no padding?

        for (uint64_t i = 0; i < node->children.size(); ++i) {
            auto& child = node->children[i];

            std::vector<Line> childLineboxes;

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
                bool isFirstLinebox = true;

                auto& atoms = child->atomized->atoms;
                size_t idx = 0;

                while (idx < text.size() && idx < atoms.size()) {
                    char ch = text[idx];

                    if (ch != ' ') {
                        // Non-space: accumulate
                        runningWidth += atoms[idx].width;
                        runningAtomCount++;
                        idx++;
                    } else {
                        // Space: add all consecutive spaces to current linebox
                        while (idx < text.size() && text[idx] == ' ') {
                            runningWidth += atoms[idx].width;
                            runningAtomCount++;
                            idx++;
                        }
                        // Push linebox with trailing spaces
                        if (runningAtomCount > 0) {
                            childLineboxes.push_back({
                                .width = runningWidth,
                                .atomCount = runningAtomCount,
                                .collapsable = isFirstLinebox
                            });
                            isFirstLinebox = false;
                        }
                        runningWidth = 0.0;
                        runningAtomCount = 0;
                    }
                }

                // Push final linebox if any
                if (runningAtomCount > 0) {
                    childLineboxes.push_back({
                        .width = runningWidth,
                        .atomCount = runningAtomCount,
                        .collapsable = isFirstLinebox
                    });
                }
            }

            childrenLineboxes.push_back(childLineboxes);
        }
        
        for (uint64_t i = 0; i < node->children.size(); ++i) {
            auto& child = node->children[i];
            auto childAsPtr = child.get();

            // Pass precomputed lineboxes to child
            childConstraints.lineboxes = childrenLineboxes[i];

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
