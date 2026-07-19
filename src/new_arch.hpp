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
#include "text_bidi.hpp"
#include <concepts>
#include <cstdint>
#include <optional>
#include <ranges>
#include <span>
#include <memory>
#include <format>
#include "AppKit_Extensions.hpp"
#include <any>
#include <unordered_map>
#include <string>
#include <utility>
#include <vector>

class Renderer;

namespace layout {


    struct ResolvedMargins {
        float top, right, bottom, left;
    };

    
    using FragmentID = uint64_t;

    struct Measured {
        FragmentID id;
        std::expected<float, style::SizeResolveFailure> explicitWidth{
            std::unexpected(style::SizeResolveFailure::Auto)
        };
        std::expected<float, style::SizeResolveFailure> explicitHeight{
            std::unexpected(style::SizeResolveFailure::Auto)
        };
    };

    struct Atomized {
        FragmentID id;
        std::vector<Atom> atoms;
        std::vector<Atom> drawableAtoms;
        bool usesDrawableAtoms{};
    };

    struct Placed {
        FragmentID id;
        std::vector<AtomPlacement> placements;
    };

    struct LineFragment {
        float width{};
        size_t atomStart{};
        size_t atomCount{};
        size_t textByteStart{};
        size_t textByteLength{};
        uint8_t bidiLevel{};
        size_t lineBoxIndex{};
        size_t fragmentIndex{};  // index within lineBox.fragmentOffsets
    };

    struct LineBox {
        size_t fragmentCount{}; // number of fragments
        std::vector<float> fragmentOffsets{}; // relative x where fragment is placed in line box
        float width{}; // width of line box (width of all fragments)
        float currentFragmentOffset{};

        void pushFragment(const LineFragment& fragment);
    };

    struct InlineFragmentRange {
        size_t start{};
        size_t count{};
    };

    struct InlineFormattingContext {
        std::vector<LineFragment> fragments;
        std::vector<LineBox> lineBoxes;
        std::vector<InlineFragmentRange> childFragments;
    };

    struct InlineFormattingInput {
        std::shared_ptr<const InlineFormattingContext> context;
        InlineFragmentRange fragments;

        std::span<const LineFragment> lineFragments() const {
            if (!context || fragments.count == 0) return {};
            return std::span{context->fragments}.subspan(fragments.start, fragments.count);
        }

        std::span<const LineBox> lineBoxes() const {
            if (!context) return {};
            return context->lineBoxes;
        }
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



}

namespace style {
    enum class Position {
        Absolute,
        Fixed,
        Static,
        Relative,
    };

    enum class Display {
        Block,
        Inline,
        Flex,
        Grid
    };

    enum class FlexDirection {
        Row,
        Col,
        RowReverse,
        ColReverse
    };

    enum class JustifyContent {
        FlexStart,
        FlexEnd,
        Center,
        SpaceBetween,
        SpaceAround,
        SpaceEvenly
    };

    enum class AlignItems {
        Stretch,
        FlexStart,
        FlexEnd,
        Center,
    };

    enum class FlexWrap {
        NoWrap,
        Wrap,
        WrapReverse
    };

    enum class AlignContent {
        Stretch,
        FlexStart,
        FlexEnd,
        Center,
        SpaceBetween,
        SpaceAround,
        SpaceEvenly
    };

    enum class AlignSelf {
        Auto,
        Stretch,
        FlexStart,
        FlexEnd,
        Center
    };

    enum class JustifyItems {
        Stretch,
        Start,
        End,
        Center
    };

    enum class JustifySelf {
        Auto,
        Stretch,
        Start,
        End,
        Center
    };

    enum class Overflow {
        Visible,
        Hidden,
        Scroll
    };

    enum class PointerEvents {
        Auto,
        None
    };

    enum class WhiteSpace {
        Normal,
        NoWrap,
        Pre,
        PreWrap
    };

    enum class WordBreak {
        Normal,
        BreakAll
    };

    enum class TextAlign {
        Start,
        Left,
        Center,
        Right
    };

    struct TextOverflow {
        enum class Mode {
            Clip,
            Ellipsis,
            Custom
        };

        Mode mode{Mode::Clip};
        std::string ending{};

        static TextOverflow clip() {
            return {};
        }

        static TextOverflow ellipsis() {
            return {.mode = Mode::Ellipsis, .ending = "\xE2\x80\xA6"};
        }

        static TextOverflow custom(std::string ending) {
            return {.mode = Mode::Custom, .ending = std::move(ending)};
        }

        bool drawsEnding() const {
            return mode != Mode::Clip && !ending.empty();
        }
    };

    struct ClipUniform {
        simd_float2 rectCenter{};
        simd_float2 halfExtent{};
        simd_float2 cornerRadius{};
    };

    struct GridPlacement {
        int colStart{0};  // 1-based line number, 0 = auto
        int colEnd{0};    // 0 = colStart+1 (span 1)
        int rowStart{0};
        int rowEnd{0};
    };

    struct SharedDescriptor {
        Position position{Position::Static};
        Display display{Display::Block};

        Size width{Size::autoSize()};
        Size height{Size::autoSize()};
        Size minWidth{Size::autoSize()};
        Size minHeight{Size::autoSize()};
        std::optional<Size> maxWidth, maxHeight;
        std::optional<Size> top, left, bottom, right;

        Size margin{};
        std::optional<Size> marginLeft, marginRight, marginTop, marginBottom;

        Size padding{};
        std::optional<Size> paddingLeft, paddingRight, paddingTop, paddingBottom;

        FlexDirection flexDirection{FlexDirection::Row};
        JustifyContent justifyContent{JustifyContent::FlexStart};
        AlignItems alignItems{AlignItems::Stretch};
        FlexWrap flexWrap{FlexWrap::NoWrap};
        AlignContent alignContent{AlignContent::Stretch};
        AlignSelf alignSelf{AlignSelf::Auto};
        JustifyItems justifyItems{JustifyItems::Stretch};
        JustifySelf justifySelf{JustifySelf::Auto};
        Size flexGrow{Size::px(0.0)};
        Size flexShrink{Size::px(1.0)};
        Size flexGap{Size::px(0)};

        // Grid container properties
        std::vector<Size> gridTemplateColumns{};
        std::vector<Size> gridTemplateRows{};
        Size gridColumnGap{Size::px(0)};
        Size gridRowGap{Size::px(0)};

        // Grid item properties
        GridPlacement gridPlacement{};

        Size cornerRadius{};
        Size borderWidth{};
        simd_float4 borderColor{0,0,0,1};

        Overflow overflow {Overflow::Visible};
        PointerEvents pointerEvents {PointerEvents::Auto};
        TextOverflow textOverflow{};
        std::optional<TextAlign> textAlign{};
    };
}

namespace layout {
    using style::AlignContent;
    using style::AlignItems;
    using style::AlignSelf;
    using style::ClipUniform;
    using style::Display;
    using style::FlexDirection;
    using style::FlexWrap;
    using style::JustifyContent;
    using style::JustifyItems;
    using style::JustifySelf;
    using style::Position;
    using style::SharedDescriptor;
    using style::Size;
    using style::TextOverflow;
    using style::TextAlign;

    struct EdgeIntent {
        Display edgeDisplayMode{Display::Block};
        float intent{};
        bool collapsable {};
    };

    enum class Direction {
        ltr,
        rtl
    };

    struct ReplacedAttributes {
        std::optional<Size> marginTop{}; 
        std::optional<Size> marginBottom{}; 
    };

    struct InheritedProperties {
        Direction direction{Direction::ltr};
        TextAlign textAlign{TextAlign::Start};
    };

    struct ContainingBlock {
        simd_float2 origin{};
        float width{};
        float height{};
    };

    enum class AxisResolution {
        Final,
        MinContent,
        MaxContent,
        Deferred
    };

    struct Constraints {
        simd_float2 origin{};
        simd_float2 cursor{};
        float availableWidth{};
        float availableHeight{};

        InheritedProperties inheritedProperties{};

        FrameInfo frameInfo{}; // viewport size (for fixed)
        ContainingBlock absoluteContainingBlock{}; // for absolute: nearest positioned ancestor

        EdgeIntent edgeIntent{};

        InlineFormattingInput inlineFormatting {};
        std::optional<bidi::TextBidiInput> textBidiInput;

        ReplacedAttributes replacedAttributes {};
        ResolvedMargins resolvedMargins {};
        float prevInlineHeight{};
        std::vector<ClipUniform> clipUniforms {};
        std::optional<TextOverflow> textOverflow{};

        bool shrinkWidthToFit{false};
        bool shrinkHeightToFit{false};
        AxisResolution widthResolution{AxisResolution::Final};
        AxisResolution heightResolution{AxisResolution::Final};
    };

    struct LayoutInput {
        Position position;
        Display display;

        std::expected<float, style::SizeResolveFailure> width{
            std::unexpected(style::SizeResolveFailure::Auto)
        };
        std::expected<float, style::SizeResolveFailure> height{
            std::unexpected(style::SizeResolveFailure::Auto)
        };
        Size minWidth{Size::autoSize()};
        Size minHeight{Size::autoSize()};
        std::optional<Size> maxWidth, maxHeight;
        
        std::optional<Size> top, left, bottom, right;

        std::optional<Direction> direction; // overwrites inherited if specified
        std::optional<TextAlign> textAlign;

        Size marginTop, marginRight, marginBottom, marginLeft;

        Size paddingTop, paddingRight, paddingBottom, paddingLeft;

        bool hasHorizontalAutoMargins() const {
            return marginLeft.isAuto() && marginRight.isAuto();
        }
    };

    inline LayoutInput toLayoutInput(const SharedDescriptor& s, const Measured& m) {
        LayoutInput li;
        li.position = s.position;
        li.display = s.display;
        li.width = m.explicitWidth;
        li.height = m.explicitHeight;
        li.minWidth = s.minWidth;
        li.minHeight = s.minHeight;
        li.maxWidth = s.maxWidth;
        li.maxHeight = s.maxHeight;
        li.top = s.top;
        li.left = s.left;
        li.bottom = s.bottom;
        li.right = s.right;
        li.textAlign = s.textAlign;
        li.paddingTop = s.paddingTop.value_or(s.padding);
        li.paddingRight = s.paddingRight.value_or(s.padding);
        li.paddingBottom = s.paddingBottom.value_or(s.padding);
        li.paddingLeft = s.paddingLeft.value_or(s.padding);
        li.marginTop = s.marginTop.value_or(s.margin);
        li.marginRight = s.marginRight.value_or(s.margin);
        li.marginBottom = s.marginBottom.value_or(s.margin);
        li.marginLeft = s.marginLeft.value_or(s.margin);
        return li;
    }

    struct SizeResolutionContext {
        Position position;
        Constraints& parentConstraints;
        const std::optional<Size>&  top;
        const std::optional<Size>&  right;
        const std::optional<Size>&  bottom;
        const std::optional<Size>&  left;
        const Size& requestedWidth;
        const Size& requestedHeight;
        float availableWidth;
        float availableHeight;
    };

    struct PositionResolutionContext {
        simd_float2 currentCursor;
        const Constraints& constraints;
        const LayoutInput& layoutInput;
        const ResolvedMargins& margins;
    };


    struct ResolvedSize {
        std::expected<float, style::SizeResolveFailure> width{
            std::unexpected(style::SizeResolveFailure::Auto)
        };
        std::expected<float, style::SizeResolveFailure> height{
            std::unexpected(style::SizeResolveFailure::Auto)
        };
    };
    
    simd_float2 resolvePosition(const PositionResolutionContext& ctx);
    ResolvedSize resolveSize(const SizeResolutionContext& sizeContext);


    using ChainID = uint64_t;

    struct MarginMetadata {
        std::optional<ChainID> topChainId;
        std::optional<ChainID> bottomChainId;
    };  

    struct PreLayoutResult {
        MarginMetadata marginMetadata;
        ResolvedMargins resolvedMargins;
    };

    // Info needed to resolve right/bottom positioning in postLayout
    // (deferred because element size may not be known during layout)
    struct DeferredPositionInfo {
        bool needsRightResolution{};
        bool needsBottomResolution{};
        float containingBlockWidth{};
        float containingBlockHeight{};
        float resolvedRight{};   // already resolved from Size to float
        float resolvedBottom{};
        float marginRight{};
        float marginBottom{};
        Direction direction{Direction::ltr};
    };

    struct LayoutBox {
        float x, y;
        float width, height;
    };

    struct LayoutResult {
        std::vector<simd_float2> atomOffsets;
        std::vector<simd_float2> localAtomOffsets;
        std::vector<simd_float2> drawableAtomOffsets;
        InlineFormattingInput inlineFormatting;

        LayoutBox computedBox;
        LayoutBox localComputedBox;

        float consumedHeight; // how much height consumed
        float prevInlineHeight{};

        Constraints childConstraints; // child constraints

        simd_float2 siblingCursor;
        bool outOfFlow; // don't change siblings
        EdgeIntent edgeIntent;

        struct {
            float top{}, right{}, bottom{}, left{};
        } resolvedPadding;

        DeferredPositionInfo deferredPosition;
        std::vector<ClipUniform> clipUniforms {};
    };

    struct LayoutOutput {
        Measured measured;
        LayoutResult layout;
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

        // flex
        static LayoutResult layoutFlex(Constraints& constraints, simd_float2 currentCursor, LayoutInput& layoutInput, Atomized& atomized);


        static LayoutResult resolve(Constraints& constraints, LayoutInput& layoutInput, Atomized atomized);
    };
}

namespace runtime {
    struct UIContext {
        UIContext(MTL::Device* device, MTK::View* view);

        void updateView();

        MTL::Device* device;
        MTK::View* view;
        DrawableBufferAllocator allocator;
        layout::LayoutEngine layoutEngine;
        FrameInfo frameInfo;
        DrawableBuffer frameInfoBuffer;
        std::atomic<uint64_t> frameIndex{0};
    };
}

namespace elements {
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

    using layout::FragmentID;
    using runtime::UIContext;

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
