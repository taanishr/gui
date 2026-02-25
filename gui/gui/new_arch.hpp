//
//  newarch.hpp
//  gui
//
//  Created by Taanish Reja on 11/20/25.
//

#pragma once
#include "fragment_types.hpp"
#include "printers.hpp"
#include "metal_imports.hpp"
#include "frame_info.hpp"
#include "sizing.hpp"
#include <concepts>
#include <cstdint>
#include <optional>
#include <ranges>
#include <format>
#include "AppKit_Extensions.hpp"
#include <any>
#include <unordered_map>

using namespace std::ranges::views;

class Renderer;

namespace NewArch {
    using FragmentID = uint64_t;

    struct Measured {
        FragmentID id;
        float explicitWidth;
        float explicitHeight;
    };

    struct Atomized {
        FragmentID id;
        std::vector<Atom> atoms;
    };

    struct Placed {
        FragmentID id;
        std::vector<AtomPlacement> placements;
    };

    struct LineFragment {
        float width{};
        size_t atomCount{};
        bool collapsable{};
        size_t lineBoxIndex{};
        size_t fragmentIndex{};  // index within lineBox.fragmentOffsets
    };

    struct LineBox {
        size_t fragmentCount{}; // number of fragments
        std::vector<int> fragmentOffsets{}; // relative x where fragment is placed in line box
        float width{}; // width of line box (width of all fragments)
        float currentFragmentOffset{};

        void pushFragment(const LineFragment& fragment);
    };

    struct FinalizedBase {};

    template <typename U>
    struct Finalized : FinalizedBase {
        FragmentID id;
        Atomized atomized;
        Placed placed;
        U uniforms;
    };


    /* layout stuff */
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



    template<typename D>
    concept DescriptorType = requires(
        D d,
        std::any payload
    ) 
    {
        { d.request(payload) } -> std::same_as<std::any>;
    };

    struct GetFull {};
    struct GetField { std::string name; };
    using DescriptorPayload = std::variant<GetFull, GetField>;

    enum Position {
        Absolute,
        Fixed,
        Static,
    };

    enum Display {
        Block,
        Inline
    };

    struct EdgeIntent {
        Display edgeDisplayMode{Display::Block};
        float intent{};
        bool collapsable {};
    };

    enum class Direction {
        ltr,
        rtl
    };

    // descriptor defines attributes; attributes can be replaced based on the tree structure (i.e. )
    struct ReplacedAttributes {
        std::optional<Size> marginTop{}; 
        std::optional<Size> marginBottom{}; 
    };

     struct InheritedProperties {
        Direction direction{Direction::ltr};
    };

    struct Constraints {
        simd_float2 origin{};
        simd_float2 cursor{};
        float maxWidth{};
        float maxHeight{};
        InheritedProperties inheritedProperties{};
    
        FrameInfo frameInfo{}; // viewport size (for fixed)

        EdgeIntent edgeIntent{};

        std::vector<LineFragment> lineFragments {};
        std::vector<LineBox> lineBoxes {};

        ReplacedAttributes replacedAttributes {};
    };

    struct LayoutInput {
        Position position;
        Display display;

        float width, height;
        std::optional<Size> top, left, bottom, right;

        std::optional<Direction> direction; // overwrites inherited if specified

        Size marginTop, marginRight, marginBottom, marginLeft;

        Size paddingTop, paddingRight, paddingBottom, paddingLeft;

        bool hasHorizontalAutoMargins() const {
            return marginLeft.isAuto() && marginRight.isAuto();
        }
    };

    struct SizeResolutionContext {
        Position position;
        Constraints& parentConstraints;
        const std::optional<Size>&  top;
        const std::optional<Size>&  right;
        const std::optional<Size>&  bottom;
        const std::optional<Size>&  left;
        const std::optional<Size>& requestedWidth;
        const std::optional<Size>& requestedHeight;
        float availableWidth;
        float availableHeight;
    };

    struct ResolvedMargins {
        float top, right, bottom, left;
    };

    struct PositionResolutionContext {
        simd_float2 currentCursor;
        const Constraints& constraints;
        const LayoutInput& layoutInput;
        const ResolvedMargins& margins;
    };
    
    simd_float2 resolvePosition(const PositionResolutionContext& ctx);
    simd_float2 resolveSize(const SizeResolutionContext& positionContext);

    struct LayoutResult {
        std::vector<simd_float2> atomOffsets;
        
        struct {
            float x, y;
            float width, height;
        } computedBox;
        // for creating background atom in finalize for text? but doesn't make sense in case of border when regular inline...?

        float consumedHeight; // how much height consumed

        Constraints childConstraints; // child constraints
    
        simd_float2 siblingCursor;
        bool outOfFlow; // don't change siblings
        EdgeIntent edgeIntent;
    };

    
    struct LayoutEngine {
        static ResolvedMargins resolveAutoMargins(
            const LayoutInput& li,
            const ReplacedAttributes& replacedAttributes,
            float availableWidth,
            float contentWidth
        );

        // relative, block/inline
        static LayoutResult layoutBlockNormalFlow(Constraints& constraints, simd_float2 currentCursor, LayoutInput& layoutInput, Atomized& atomized);
        static LayoutResult layoutInlineNormalFlow(Constraints& constraints, simd_float2 currentCursor, LayoutInput& layoutInput, Atomized& atomized);
        static LayoutResult resolveNormalFlow(Constraints& constraints, simd_float2 currentCursor, LayoutInput& layoutInput, Atomized& atomized);

        // fixed and absolute, block/inline
        static LayoutResult layoutAbsolute(Constraints& constraints, simd_float2 currentCursor, LayoutInput& layoutInput, Atomized& atomized);
        static LayoutResult layoutFixed(Constraints& constraints, simd_float2 currentCursor, LayoutInput& layoutInput, Atomized& atomized);
        static LayoutResult resolveOutOfFlow(Constraints& constraints, simd_float2 currentCursor, LayoutInput& layoutInput, Atomized& atomized);
        static LayoutResult resolve(Constraints& constraints, LayoutInput& layoutInput, Atomized atomized);
    };

    // Rest of the stuff
    struct UIContext {
        UIContext(MTL::Device* device, MTK::View* view);

        void updateView();

        MTL::Device* device;
        MTK::View* view;
        DrawableBufferAllocator allocator;
        LayoutEngine layoutEngine;
        DrawableBuffer frameInfoBuffer;
        std::atomic<uint64_t> frameIndex{0};
    };

    template <typename S>
    struct Fragment {
        Fragment(UIContext& ctx):
            id{Fragment::nextId++},
            fragmentStorage{ctx}
        {}
        
        static FragmentID nextId;
        FragmentID id;
        S fragmentStorage;
    };

    template <typename S>
    FragmentID Fragment<S>::nextId = 0;
}
