#pragma once

#include "sdf_helpers.hpp"
#include "metal_imports.hpp"
#include "new_arch.hpp"
#include <concepts>
#include <any>
#include <cstdint>
#include <algorithm>
#include <unordered_map>
#include "frame_info.hpp"
#include <optional>
#include <print>
#include <simd/vector_types.h>
#include <string_view>
#include "parallel.hpp"
#include "events.hpp"
#include "printers.hpp"
#include <numeric> 

namespace elements {
    using layout::Atomized;
    using layout::Constraints;
    using layout::Finalized;
    using layout::LayoutResult;
    using layout::Measured;
    using layout::Placed;
    using runtime::HitTestContext;
    using runtime::UIContext;
    using style::SharedDescriptor;

    constexpr bool isTextWhitespace(char32_t codepoint) noexcept {
        return codepoint == U' '  ||
               codepoint == U'\t' ||
               codepoint == U'\r' ||
               codepoint == U'\n';
    }

    enum class RequestTarget {
        Descriptor,
        TextShaping,
    };

    template<typename E>
    concept ElementType = requires(E& e, RequestTarget target, std::any& payload) {
        e.getDescriptor();
        e.getFragment();
        { e.request(target, payload) } -> std::same_as<std::any>;

        typename E::StorageType;
        typename E::DescriptorType;
        typename E::UniformsType;
    } && DescriptorType<typename E::DescriptorType>;

    template<typename P, typename S, typename D, typename U>
    concept ProcessorType = requires(
        P& proc,
        Fragment<S>& fragment,
        Constraints& constraints,
        SharedDescriptor& shared,
        D& desc,
        Measured& measured,
        Atomized& atomized,
        Placed& placed,
        Finalized<U>& finalized,
        LayoutResult layout,
        MTL::RenderCommandEncoder* encoder
    ) {
        { proc.measure(fragment, constraints, shared, desc) } -> std::same_as<Measured>;
        { proc.atomize(fragment, constraints, shared, desc, measured) } -> std::same_as<Atomized>;
        { proc.layout(fragment, constraints, shared, desc, measured, atomized) } -> std::same_as<LayoutResult>;

        { proc.postLayout(fragment, constraints, shared, desc, measured, atomized, layout) } -> std::same_as<Atomized>;

        { proc.place(fragment, constraints, shared, desc, measured, atomized, layout) } -> std::same_as<Placed>;

        { proc.finalize(fragment, constraints, shared, desc, measured, atomized, layout, placed) } -> std::same_as<Finalized<U>>;
        { proc.setupHitTestFunction() } -> std::same_as<std::function<bool(HitTestContext<U>&, simd_float2)>>;
        proc.encode(encoder, fragment, finalized);
    };

    struct ElementBase {
        virtual Measured measure(Constraints& constraints, SharedDescriptor& shared) = 0;
        virtual Atomized atomize(Constraints& constraints, SharedDescriptor& shared, Measured& measured) = 0;
        virtual LayoutResult layout(Constraints& constraints, SharedDescriptor& shared, Measured& measured, Atomized& atomized) = 0;
        virtual Atomized postLayout(Constraints& constraints, SharedDescriptor& shared, Measured& measured, Atomized& atomized, LayoutResult& layout) = 0;
        virtual Placed place(Constraints& constraints, SharedDescriptor& shared, Measured& measured, Atomized& atomized, LayoutResult& layout) = 0;
        virtual std::any finalize(Constraints& constraints, SharedDescriptor& shared, Measured& measured, Atomized& atomized, LayoutResult& layout, Placed& placed) = 0;
        virtual std::any request(RequestTarget target, std::any& payload) = 0;
        virtual void encode(MTL::RenderCommandEncoder* encoder, std::any& finalized) = 0;
        virtual std::string_view elementTypeName() const = 0;
        virtual bool preciseHitTest(simd_float2 point, const LayoutResult& layout, const std::any& finalized) {
            return true;
        }

        virtual ~ElementBase() = 0;
    };

    template <ElementType E, typename P, typename S, typename D, typename U>
        requires ProcessorType<P, S, D, U>
    struct Element : ElementBase {
        Element(UIContext& ctx, E&& elem, P& proc):
            element{std::move(elem)}, processor{proc}
        {
            hitTestFunction = processor.setupHitTestFunction();
        }

        Measured measure(Constraints& constraints, SharedDescriptor& shared) override {
            return processor.measure(element.getFragment(), constraints, shared, element.getDescriptor());
        }

        Atomized atomize(Constraints& constraints, SharedDescriptor& shared, Measured& measured) override {
            return processor.atomize(element.getFragment(), constraints, shared, element.getDescriptor(), measured);
        }

        LayoutResult layout(Constraints& constraints, SharedDescriptor& shared, Measured& measured, Atomized& atomized) override {
            return processor.layout(element.getFragment(), constraints, shared, element.getDescriptor(), measured, atomized);
        }

        Atomized postLayout(Constraints& constraints, SharedDescriptor& shared, Measured& measured, Atomized& atomized, LayoutResult& layout) override {
            return processor.postLayout(element.getFragment(), constraints, shared, element.getDescriptor(), measured, atomized, layout);
        }

        Placed place(Constraints& constraints, SharedDescriptor& shared, Measured& measured, Atomized& atomized, LayoutResult& layout) override {
            return processor.place(element.getFragment(), constraints, shared, element.getDescriptor(), measured, atomized, layout);
        }

        std::any finalize(Constraints& constraints, SharedDescriptor& shared, Measured& measured, Atomized& atomized, LayoutResult& layout, Placed& placed) override {
            auto finalized = processor.finalize(element.getFragment(), constraints, shared, element.getDescriptor(), measured, atomized, layout, placed);
            auto finalizedErased = finalized;
            return finalizedErased;
        }

        std::any request(RequestTarget target, std::any& payload) override {
            return element.request(target, payload);
        };

        void encode(MTL::RenderCommandEncoder* encoder, std::any& finalizedErased) override {
            auto finalized = std::any_cast<Finalized<typename E::UniformsType>>(finalizedErased);
            return processor.encode(encoder, element.getFragment(), finalized);
        }

        std::string_view elementTypeName() const override {
            if constexpr (requires { E::elementName; }) {
                return E::elementName;
            }

            return "Unknown";
        }

        bool preciseHitTest(simd_float2 point, const LayoutResult& layout, const std::any& finalized) override {
            if (hitTestFunction) {
                HitTestContext<U> ctx {
                    .finalized = std::any_cast<Finalized<U>>(finalized),
                    .layout = layout
                };

                return hitTestFunction(ctx, point);
            }

            return true;
        }

        ~Element() override {};

        std::function<bool(HitTestContext<U>& context, simd_float2 testPoint)> hitTestFunction;
        E element;
        P& processor;
    };
}

namespace tree {
    using elements::Element;
    using elements::ElementBase;
    using elements::ElementType;
    using elements::ProcessorType;
    using layout::Atomized;
    using layout::ChainID;
    using layout::Constraints;
    using layout::LayoutResult;
    using layout::LineBox;
    using layout::LineFragment;
    using layout::Measured;
    using layout::Placed;
    using layout::PreLayoutResult;
    using runtime::Event;
    using runtime::EventType;
    using runtime::UIContext;
    using style::AlignContent;
    using style::AlignItems;
    using style::AlignSelf;
    using style::Display;
    using style::FlexDirection;
    using style::FlexWrap;
    using style::GridPlacement;
    using style::JustifyContent;
    using style::JustifyItems;
    using style::JustifySelf;
    using style::Overflow;
    using style::Position;
    using style::SharedDescriptor;
    using style::Size;

    struct TreeNode;

    enum class DirtyBits : uint32_t {
        None = 0,
        Measure = 1 << 0,
        Atomize = 1 << 1,
        Layout = 1 << 2,
        PostLayout = 1 << 3,
        Place = 1 << 4,
        Finalize = 1 << 5,
        PaintOrder = 1 << 6,
    };

    constexpr DirtyBits operator|(DirtyBits a, DirtyBits b) {
        return static_cast<DirtyBits>(std::to_underlying(a) | std::to_underlying(b));
    }

    constexpr DirtyBits operator&(DirtyBits a, DirtyBits b) {
        return static_cast<DirtyBits>(std::to_underlying(a) & std::to_underlying(b));
    }

    constexpr DirtyBits operator~(DirtyBits bits) {
        return static_cast<DirtyBits>(~std::to_underlying(bits));
    }

    inline DirtyBits& operator|=(DirtyBits& a, DirtyBits b) {
        a = a | b;
        return a;
    }

    inline DirtyBits& operator&=(DirtyBits& a, DirtyBits b) {
        a = a & b;
        return a;
    }

    constexpr bool hasDirty(DirtyBits value, DirtyBits bits) {
        return (static_cast<uint32_t>(value & bits) != 0);
    }

    constexpr DirtyBits allPhaseDirtyBits() {
        return DirtyBits::Measure | DirtyBits::Atomize | DirtyBits::Layout |
            DirtyBits::PostLayout | DirtyBits::Place | DirtyBits::Finalize | DirtyBits::PaintOrder;
    }

    struct ConstraintsKey {
        std::size_t value{};

        bool operator==(const ConstraintsKey& other) const {
            return value == other.value;
        }
    };

    struct CollapsedChain {
        ChainID id;
        TreeNode* root;
        Size intent;
        int depth;
    };
    
    using EventHandler = std::function<void(Event&)>;

    struct TreeNode {
        template<ElementType E, typename P>
            requires ProcessorType<P, typename E::StorageType, typename E::DescriptorType, typename E::UniformsType>
        TreeNode(UIContext& ctx, E&& elem, P& processor)
            : element(std::make_unique<Element<E,P, typename E::StorageType, typename E::DescriptorType, typename E::UniformsType>>(ctx, std::move(elem), processor)),
            parent(nullptr),
            id(nextId++),
            localZIndex{0},
            globalZIndex{0},
            paintPreorderIndex{0},
            paintPostorderIndex{0}
        {}


        void addEventListener(EventType type, EventHandler handler) {
            eventHandlers[type].push_back(std::move(handler));
        }

        TreeNode* dispatch(Event& event) {
            auto it = eventHandlers.find(event.type);
            if (it != eventHandlers.end()) {
                for (auto& handler : it->second) {
                    if (event.propagationStopped) break;
                    handler(event);
                }
            }

            if (event.type == EventType::ScrollWheel && shared.overflow == Overflow::Scroll) {
                auto& scroll = event.get<EventType::ScrollWheel>();
                float maxScrollX = 0.0f;
                float maxScrollY = 0.0f;
                if (layout.has_value()) {
                    maxScrollX = std::max(0.0f, scrollContentSize.x - scrollViewportSize.x);
                    maxScrollY = std::max(0.0f, scrollContentSize.y - scrollViewportSize.y);
                }
                auto oldScrollOffset = scrollOffset;

                scrollOffset.x = std::clamp(scrollOffset.x - scroll.dx, 0.0f, maxScrollX);
                scrollOffset.y = std::clamp(scrollOffset.y - scroll.dy, 0.0f, maxScrollY);
                event.stopPropagation();
                if (oldScrollOffset.x != scrollOffset.x || oldScrollOffset.y != scrollOffset.y) {
                    return this;
                }
            }

            if (!event.propagationStopped && parent) {
                return parent->dispatch(event);
            }

            return nullptr;
        }

        void calculateGlobalZIndex(uint64_t parentGlobal) {
            globalZIndex = std::add_sat(parentGlobal, localZIndex);
            
            for (auto& child : children) {
                child->calculateGlobalZIndex(globalZIndex);
            }
        }
        
        void attach_child(std::unique_ptr<TreeNode>&& child) {
            if (!child) return;
            child->parent = this;
            children.push_back(std::move(child));
        }

        bool contains(simd_float2 point) const {
            if (!layout.has_value()) return false;
            
            auto& box = layout->computedBox;

            if (point.x < box.x || point.x > box.x + box.width ||
                point.y < box.y || point.y > box.y + box.height) {
                return false;
            }

            for (auto& clip : layout->clipUniforms) {
                if (rounded_rect_sdf(point - clip.rectCenter, clip.halfExtent, clip.cornerRadius) > 0.0f) {
                    return false;
                }
            }

            return element->preciseHitTest(point, layout.value(), finalized);
        }

        Position getPosition() const { return shared.position; }
        Display getDisplay() const { return shared.display; }
        Size getMarginTop() const { return shared.marginTop.value_or(shared.margin); }
        Size getMarginBottom() const { return shared.marginBottom.value_or(shared.margin); }
        Size getMarginLeft() const { return shared.marginLeft.value_or(shared.margin); }
        Size getMarginRight() const { return shared.marginRight.value_or(shared.margin); }
        Size getMargin() const { return shared.margin; }
        std::optional<Size> getPaddingTop() const { return shared.paddingTop; }
        std::optional<Size> getPaddingBottom() const { return shared.paddingBottom; }
        Size getFlexGrow() const { return shared.flexGrow; }
        Size getFlexShrink() const { return shared.flexShrink; }
        FlexDirection getFlexDirection() const { return shared.flexDirection; }
        JustifyContent getJustifyContent() const { return shared.justifyContent; }
        AlignItems getAlignItems() const { return shared.alignItems; }
        Size getFlexGap() const { return shared.flexGap; }
        FlexWrap getFlexWrap() const { return shared.flexWrap; }
        AlignContent getAlignContent() const { return shared.alignContent; }
        AlignSelf getAlignSelf() const { return shared.alignSelf; }
        JustifyItems getJustifyItems() const { return shared.justifyItems; }
        JustifySelf getJustifySelf() const { return shared.justifySelf; }
        const std::vector<Size>& getGridTemplateColumns() const { return shared.gridTemplateColumns; }
        const std::vector<Size>& getGridTemplateRows() const { return shared.gridTemplateRows; }
        Size getGridColumnGap() const { return shared.gridColumnGap; }
        Size getGridRowGap() const { return shared.gridRowGap; }
        GridPlacement getGridPlacement() const { return shared.gridPlacement; }

        
        std::unique_ptr<ElementBase> element;
        TreeNode* parent = nullptr;
        std::vector<std::unique_ptr<TreeNode>> children;
        uint64_t id;
        uint64_t localZIndex;
        uint64_t globalZIndex;
        uint64_t paintPreorderIndex;
        uint64_t paintPostorderIndex;

        std::optional<Measured> measured;
        std::optional<Atomized> atomized;
        std::optional<PreLayoutResult> preLayout;
        std::optional<LayoutResult> layout;
        std::optional<bidi::TextBidiInput> textBidiInput;
        std::optional<Placed> placed;
        std::unordered_map<EventType, std::vector<EventHandler>> eventHandlers;
        std::any finalized;
        simd_float2 globalOffset {0.0f, 0.0f};
        simd_float2 scrollOffset {0.0f, 0.0f};
        simd_float2 scrollContentSize {0.0f, 0.0f};
        simd_float2 scrollViewportSize {0.0f, 0.0f};
        SharedDescriptor shared;
        DirtyBits dirtySelf{~DirtyBits::None};
        DirtyBits dirtySubtree{~DirtyBits::None};
        std::optional<ConstraintsKey> constraintsKey;

    private:
        static uint64_t nextId;
    };

    std::vector<TreeNode*> collectAllNodes(TreeNode* root);
    std::optional<std::string> getText(TreeNode* node);
    std::optional<style::WhiteSpace> getWhiteSpace(TreeNode* node);
    Result<std::vector<std::optional<bidi::TextBidiInput>>>
    prepareChildBidiInputs(
        TreeNode* parent,
        layout::Direction baseDirection
    );

    void precomputeMargins(TreeNode* node, Constraints& constraints, std::unordered_map<ChainID, CollapsedChain>& collapsedChainMap);
    
    // full blown inline context
    std::shared_ptr<layout::InlineFormattingContext> buildInlineBoxes(TreeNode* node, Constraints& childConstraints);

    // inline context calculated for a single child, independently of other siblings
    layout::InlineFormattingInput buildIsolatedInlineBoxes(
        TreeNode* node,
        float maxWidth,
        layout::AxisResolution widthResolution
    );

}

namespace std {
    template<>
    struct hash<tree::ConstraintsKey> {
        size_t operator()(const tree::ConstraintsKey& key) const noexcept {
            return key.value;
        }
    };
}
