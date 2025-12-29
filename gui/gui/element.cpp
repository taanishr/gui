#include "element.hpp"
#include "frame_info.hpp"
#include "new_arch.hpp"
#include <simd/vector_types.h>

namespace NewArch {
    uint64_t TreeNode::nextId = 0;

    // I have a render cache, develop some sort of caching policy that makes these useful
    void RenderTree::update(const FrameInfo& frameInfo) {
        auto root = getRoot();

        rootCursor = simd_float2{0,0};
        rootConstraints = Constraints{
            .origin = simd_float2{0,0},
            .cursor = rootCursor,
            .maxWidth = frameInfo.height,
            .maxHeight = frameInfo.width,
            .frameInfo = frameInfo,
        };
        
        measurePhase(root, rootConstraints);
        layoutPhase(root, frameInfo, rootConstraints);
        finalizePhase(root, rootConstraints);
    }

    void RenderTree::render(MTL::RenderCommandEncoder* encoder) {
        // encoder->setRenderPipelineState(const MTL::RenderPipelineState *pipelineState)
        
        
    }

    void RenderTree::measurePhase(TreeNode* node, Constraints& constraints) {
        auto measured = node->element->measure(constraints);
        renderCache.measured[node->id] = measured;
    }


    // consider safer way of accessing cache?
    void RenderTree::atomizePhase(TreeNode* node, Constraints& constraints) {
        assert(renderCache.measured.contains(node->id) && 
           "atomizePhase called before measurePhase");

        auto measured = renderCache.measured[node->id];
        auto atomized = node->element->atomize(constraints, measured);
        renderCache.atomized[node->id] = atomized;
    }


    void RenderTree::layoutPhase(TreeNode* node, const FrameInfo& frameInfo, Constraints& constraints) {
        assert(renderCache.measured.contains(node->id) && 
            "layoutPhase called before measurePhase");
        assert(renderCache.atomized.contains(node->id) && 
            "layoutPhase called before atomizePhase");
            
        auto measured = renderCache.measured[node->id];
        auto atomized = renderCache.atomized[node->id];
        auto layout = node->element->layout(constraints, measured, atomized);
        renderCache.layouts[node->id] = layout;
        auto placed = node->element->place(constraints, measured, atomized, layout);
        renderCache.placed[node->id] = placed;
    }

    void RenderTree::finalizePhase(TreeNode* node, Constraints& constraints) {
        // I thought runtime checks for private methods were stupid, but I also am not superhuman and LLMs can hallucinate,
        // so debug asserts never hurt
        assert(renderCache.measured.contains(node->id) && 
            "finalizePhase: missing measured");
        assert(renderCache.atomized.contains(node->id) && 
            "finalizePhase: missing atomized");
        assert(renderCache.placed.contains(node->id) && 
            "finalizePhase: missing placed");
        
            
        auto measured = renderCache.measured[node->id];
        auto atomized = renderCache.atomized[node->id];
        auto placed = renderCache.placed[node->id];
        auto finalized = node->element->finalize(constraints, measured, atomized, placed);
        renderCache.finalized[node->id] = finalized;
    }

}
