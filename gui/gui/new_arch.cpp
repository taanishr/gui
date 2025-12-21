//
//  new_arch.cpp
//  gui
//
//  Created by Taanish Reja on 11/20/25.
//

#include "new_arch.hpp"

namespace NewArch {
    // pipeline specific
    UIContext::UIContext(MTL::Device* device, MTK::View* view):
        device{device},
        view{view},
        allocator{DrawableBufferAllocator{device}},
        layoutEngine{},
        frameInfoBuffer{allocator.allocate(sizeof(FrameInfo))}
    {
        auto frameDimensions = this->view->drawableSize();

        FrameInfo frameInfo {.width=static_cast<float>(frameDimensions.width)/2.0f, .height=static_cast<float>(frameDimensions.height)/2.0f};
        
        std::memcpy(frameInfoBuffer.get()->contents(), &frameInfo, sizeof(FrameInfo));
    };


    // --------------------------- Layout / positioning constraints ---------------------------
    //
    // NOTE: units are in frame pixels (layout space) unless stated otherwise. "Available
    // width" means the width inside the containing block after padding and borders are applied.
    //
    // ATOM LIFECYCLE (important):
    //  - Pre-atomize: produce indivisible content atoms (glyphs, images, shape quads). Each
    //    atom MUST include: ownerNodeId, intrinsicRect (w,h), styleId (resolved/inherited),
    //    and atom metadata index (glyph/SDF index).
    //  - Layout: consume content atoms (intrinsic sizes) to build line boxes and compute
    //    final layout boxes (content/padding/border) and clip rects. Do NOT emit new content
    //    atoms here.
    //  - Finalize: ONLY for multi-atom nodes (text), compute derived box-level atoms (e.g.
    //    a single background rect for the text node = union of placed glyph rects) and
    //    patch content atoms' final positions. Finalize must NOT affect layout results.
    //
    // ---------------------------------------------------------------------------------------


    // relative needs:
    /*
        - Participate in normal flow; influence siblings via the running cursor.
        - Maintain a running cursor (x,y) within the containing block during placement.
        - Block children:
            - Always start on a new line (cursor.x reset to line start).
            - Advance cursor.y by the block’s total outer height (content + padding + border).
        - Inline children:
            - Participate in inline formatting context.
            - Create/extend line boxes; place content atoms greedily into the current line box
              until "available width" is exceeded, then break to a new line box.
        - Affects siblings: in-flow; its final box must be included when determining subsequent
          in-flow placement.
    */


    // absolute needs:
    /*
        - Out-of-flow: does not affect siblings or the parent cursor.
        - Containing block selection:
            - nearest ancestor with position != static (i.e. positioned ancestor) -> containing block
            - otherwise -> root frame.
        - Position coordinates are relative to the containing block origin (top-left by default).
        - Internal layout (for its children) still uses inline/block semantics and the same
          line-box logic, but the "available width" is the containing block's inner width.
        - Shrink-to-fit for inline absolute boxes:
            - Compute minContentWidth and maxContentWidth for the node.
            - resolvedWidth = min( max(minContentWidth, availableWidth), maxContentWidth ).
            - If minContentWidth > availableWidth => inline internal layout degenerates to
              block-like stacking (vertical flow of line boxes).
        - Absolute elements must carry owner/style and be atomized as content atoms; any
          background/box rect for the absolute node is emitted in Finalize after layout.
    */


    // fixed needs:
    /*
        - Out-of-flow: does not affect siblings or the parent cursor.
        - Containing block: the viewport (screen). Coordinates are relative to viewport origin.
        - Requires awareness of viewport dimensions (screenWidth/screenHeight) as available space.
        - Internal layout respects inline vs block semantics; shrink-to-fit applies the same way:
            - If minContentWidth > viewportAvailableWidth => degenerate to block-like stacking.
        - Position and final atom placement are independent of scrolling of any containing block.
    */


    // inline needs:
    /*
        - Participate in inline formatting context inside the containing block.
        - Produce and extend line boxes (implement an initial greedy line-breaker).
        - Width behavior: content-determined for inline boxes; use shrink-to-fit when required
          (see absolute/fixed rules for formula).
        - Height is determined by line box stacking (sum of line heights + vertical gaps).
        - Content atoms (glyphs/images) are the unit of breaking; atom sizes are intrinsic and
          must be used by the line-box algorithm.
    */


    // block needs:
    /*
        - Always start on a new line in normal flow.
        - Default horizontal sizing: occupy the containing block's available width (i.e. do not
          shrink-to-fit for block-level elements unless explicitly sized).
        - Height may span multiple line boxes (wrapped inline content).
        - Visual box (background / border / radius) is a single box-level rect that spans the
          node's computed width and full height. This is emitted in Finalize (text-only or any
          multi-atom node that needs a box) and uses the node's resolvedStyle (background/border).
        - Content atoms do NOT define the block's visual extent; layout computes the block box
          then finalize emits the box atom that the renderer uses for background/border SDFs.
    */


    // ----------------------------- Additional precise invariants ----------------------------
    //
    // - Atoms never decide style: every atom references ownerNodeId -> renderer looks up the
    //   resolvedStyle/styleId for uniforms. Style inheritance must be resolved in the style pass.
    // - Stacking contexts/paint order: determine during layout/finalize (stacking context creation
    //   rules are separate). Finalize produces a paint-ordered atom list referencing node/ctx.
    // - Text-only finalization: only nodes that expanded into multiple atoms (e.g., text glyph runs)
    //   require emission of an additional box atom that is derived from layout (union of glyph rects).
    // - Coordinate spaces: layout positions are frame-space; fixed positions are viewport-space.
    // - Degeneration rule (explicit): "degenerate to block-like stacking" means inline formatting
    //   for that node falls back to vertical stacking of line boxes (the node remains out-of-flow).
    //
    // ------------------------------------------------------------------------------------------

    // relative, block/inline
    void resolveNormalFlow() {
        
    }
    
    // fixed and absolute, block/inline
    void resolveOutOfFlow() {
    
    }

    std::vector<simd_float2> LayoutEngine::resolve(Constraints& constraints, Atomized atomized)
    {
        std::vector<simd_float2> raw_placements;
    
        simd_float2 cursor {
            constraints.x,
            constraints.y
        };

        for (auto i = 0; i < atomized.atoms.size(); ++i) {
            auto& curr_atom = atomized.atoms[i];

            raw_placements.push_back(cursor);
            cursor.x += curr_atom.width;
        }
        
        return raw_placements;
    }
}
