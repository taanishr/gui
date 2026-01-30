#include "element.hpp"
#include "text.hpp"
#include "new_arch.hpp"
#include "printers.hpp"
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

    // DONE SERIALLY
    void RenderTree::layoutPhase(TreeNode* node, const FrameInfo& frameInfo, Constraints& constraints) {
        assert(node->measured.has_value() && 
            "layoutPhase called before measurePhase");
        assert(node->atomized.has_value() && 
            "layoutPhase called before atomizePhase");
            
        auto& measured = *node->measured;
        auto& atomized = *node->atomized;
        
        auto layout = node->element->layout(constraints, measured, atomized);
        node->layout = layout;

        auto childConstraints =  layout.childConstraints;

        // childConstraints.origin = constraints.origin; // add static check later

        // start with prev = parent
        // increasingly change prev as we see in flow elements
        // generate margin negoatiations function

        // std::println("in layoutPhase: layout.childConstraints.origin.x: {}", (float)layout.childConstraints.origin.x);

        // precompute lineboxes
        std::vector<Line> lineBoxes;
        for (auto& child : node->children) {
            // LineRequest lr {
            //     .atomized = atomized,
            //     // .descriptor = node->
            // };

            // if (node->element->request(std::any &payload)) {

            // }

        }

        for (auto& child : node->children) {
            auto childAsPtr = child.get();
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
