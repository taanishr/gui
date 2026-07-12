#pragma once

#include "element.hpp"
#include "div.hpp"
#include "events.hpp"
#include "fonts.hpp"
#include "renderer.hpp"
#include "image.hpp"
#include "svg.hpp"
#include "text.hpp"
#include "new_arch.hpp"
#include <algorithm>
#include <cstdint>
#include <memory>
#include <print>
#include "context_manager.hpp"

namespace elements {
    using runtime::ContextManager;
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
    using style::Overflow;
    using style::Position;
    using style::Size;
    using style::WhiteSpace;
    using style::WordBreak;
    using tree::RenderTree;
    using tree::DirtyBits;
    using tree::TreeNode;

    // Element-specific concepts (only for properties that vary by element type)
    template<typename T>
    concept HasText = requires { std::declval<T&>().text; };

    template<typename T>
    concept HasFont = requires { std::declval<T&>().font; };

    template<typename T>
    concept HasColor = requires { std::declval<T&>().color; };

    template<typename T>
    concept HasFontSize = requires { std::declval<T&>().fontSize; };

    template<typename T>
    concept HasLineHeight = requires { std::declval<T&>().lineHeight; };

    template<typename T>
    concept HasWhiteSpace = requires { std::declval<T&>().whiteSpace; };

    template<typename T>
    concept HasWordBreak = requires { std::declval<T&>().wordBreak; };

    template <typename Derived, ElementType E, typename P>
        requires ProcessorType<P, typename E::StorageType, typename E::DescriptorType, typename E::UniformsType>
    struct NodeMutation {
    protected:
        RenderTree& renderTree;
        TreeNode* node;

    public:
        NodeMutation(RenderTree& tree, TreeNode* node):
            renderTree{tree},
            node{node}
        {}

        void markDirty() {
            renderTree.markDirty();
        }

        void markDirty(DirtyBits bits) {
            renderTree.markDirty(node, bits);
        }

        static constexpr DirtyBits layoutDirtyBits() {
            return DirtyBits::Measure | DirtyBits::Atomize | DirtyBits::Layout |
                DirtyBits::PostLayout | DirtyBits::Place | DirtyBits::Finalize;
        }

        static constexpr DirtyBits textDirtyBits() {
            return DirtyBits::Atomize | DirtyBits::Layout |
                DirtyBits::PostLayout | DirtyBits::Place | DirtyBits::Finalize;
        }

        Derived& self() {
            return static_cast<Derived&>(*this);
        }

        const Derived& self() const {
            return static_cast<const Derived&>(*this);
        }

        Position position() const {
            return node->shared.position;
        }

        Derived& position(Position position) {
            node->shared.position = position;
            markDirty(layoutDirtyBits());
            return self();
        }

        Display display() const {
            return node->shared.display;
        }

        Derived& display(Display display) {
            node->shared.display = display;
            markDirty(layoutDirtyBits());
            return self();
        }

        std::optional<Size> width() const {
            return node->shared.width;
        }

        Derived& width(Size width) {
            node->shared.width = width;
            markDirty(layoutDirtyBits());
            return self();
        }

        std::optional<Size> minWidth() const {
            return node->shared.minWidth;
        }

        Derived& minWidth(Size width) {
            node->shared.minWidth = width;
            markDirty(layoutDirtyBits());
            return self();
        }

        std::optional<Size> maxWidth() const {
            return node->shared.maxWidth;
        }

        Derived& maxWidth(Size width) {
            node->shared.maxWidth = width;
            markDirty(layoutDirtyBits());
            return self();
        }

        std::optional<Size> height() const {
            return node->shared.height;
        }

        Derived& height(Size height) {
            node->shared.height = height;
            markDirty(layoutDirtyBits());
            return self();
        }

        std::optional<Size> minHeight() const {
            return node->shared.minHeight;
        }

        Derived& minHeight(Size height) {
            node->shared.minHeight = height;
            markDirty(layoutDirtyBits());
            return self();
        }

        std::optional<Size> maxHeight() const {
            return node->shared.maxHeight;
        }

        Derived& maxHeight(Size height) {
            node->shared.maxHeight = height;
            markDirty(layoutDirtyBits());
            return self();
        }

        std::optional<Size> top() const {
            return node->shared.top;
        }

        Derived& top(Size top) {
            node->shared.top = top;
            markDirty(layoutDirtyBits());
            return self();
        }

        std::optional<Size> right() const {
            return node->shared.right;
        }

        Derived& right(Size right) {
            node->shared.right = right;
            markDirty(layoutDirtyBits());
            return self();
        }

        std::optional<Size> bottom() const {
            return node->shared.bottom;
        }

        Derived& bottom(Size bottom) {
            node->shared.bottom = bottom;
            markDirty(layoutDirtyBits());
            return self();
        }

        std::optional<Size> left() const {
            return node->shared.left;
        }

        Derived& left(Size left) {
            node->shared.left = left;
            markDirty(layoutDirtyBits());
            return self();
        }

        FlexDirection flexDirection() const {
            return node->shared.flexDirection;
        }

        Derived& flexDirection(FlexDirection direction) {
            node->shared.flexDirection = direction;
            markDirty(layoutDirtyBits());
            return self();
        }

        JustifyContent justifyContent() const {
            return node->shared.justifyContent;
        }

        Derived& justifyContent(JustifyContent justifyContent) {
            node->shared.justifyContent = justifyContent;
            markDirty(layoutDirtyBits());
            return self();
        }

        AlignItems alignItems() const {
            return node->shared.alignItems;
        }

        Derived& alignItems(AlignItems alignItems) {
            node->shared.alignItems = alignItems;
            markDirty(layoutDirtyBits());
            return self();
        }

        FlexWrap flexWrap() const {
            return node->shared.flexWrap;
        }

        Derived& flexWrap(FlexWrap wrap) {
            node->shared.flexWrap = wrap;
            markDirty(layoutDirtyBits());
            return self();
        }

        AlignContent alignContent() const {
            return node->shared.alignContent;
        }

        Derived& alignContent(AlignContent ac) {
            node->shared.alignContent = ac;
            markDirty(layoutDirtyBits());
            return self();
        }

        AlignSelf alignSelf() const {
            return node->shared.alignSelf;
        }

        Derived& alignSelf(AlignSelf as) {
            node->shared.alignSelf = as;
            markDirty(layoutDirtyBits());
            return self();
        }

        Overflow overflow() const {
            return node->shared.overflow;
        }

        Derived& overflow(Overflow overflow) {
            node->shared.overflow = overflow;
            markDirty(layoutDirtyBits());
            return self();
        }

        Size flexGrow() const {
            return node->shared.flexGrow;
        }

        Derived& flexGrow(Size grow) {
            node->shared.flexGrow = grow;
            markDirty(layoutDirtyBits());
            return self();
        }

        Size flexGap() const {
            return node->shared.flexGap;
        }

        Derived& flexGap(Size gap) {
            node->shared.flexGap = gap;
            markDirty(layoutDirtyBits());
            return self();
        }

        Size flexShrink() const {
            return node->shared.flexShrink;
        }

        Derived& flexShrink(Size shrink) {
            node->shared.flexShrink = shrink;
            markDirty(layoutDirtyBits());
            return self();
        }

        const std::vector<Size>& gridTemplateColumns() const {
            return node->shared.gridTemplateColumns;
        }

        Derived& gridTemplateColumns(std::vector<Size> tracks) {
            node->shared.gridTemplateColumns = std::move(tracks);
            markDirty(layoutDirtyBits());
            return self();
        }

        const std::vector<Size>& gridTemplateRows() const {
            return node->shared.gridTemplateRows;
        }

        Derived& gridTemplateRows(std::vector<Size> tracks) {
            node->shared.gridTemplateRows = std::move(tracks);
            markDirty(layoutDirtyBits());
            return self();
        }

        Size gridColumnGap() const {
            return node->shared.gridColumnGap;
        }

        Derived& gridColumnGap(Size gap) {
            node->shared.gridColumnGap = gap;
            markDirty(layoutDirtyBits());
            return self();
        }

        Size gridRowGap() const {
            return node->shared.gridRowGap;
        }

        Derived& gridRowGap(Size gap) {
            node->shared.gridRowGap = gap;
            markDirty(layoutDirtyBits());
            return self();
        }

        GridPlacement gridPlacement() const {
            return node->shared.gridPlacement;
        }

        Derived& gridColumn(int start, int end = 0) {
            node->shared.gridPlacement.colStart = start;
            node->shared.gridPlacement.colEnd = end;
            markDirty(layoutDirtyBits());
            return self();
        }

        Derived& gridRow(int start, int end = 0) {
            node->shared.gridPlacement.rowStart = start;
            node->shared.gridPlacement.rowEnd = end;
            markDirty(layoutDirtyBits());
            return self();
        }

        Size cornerRadius() const {
            return node->shared.cornerRadius;
        }

        Derived& cornerRadius(Size radius) {
            node->shared.cornerRadius = radius;
            markDirty(DirtyBits::Finalize);
            return self();
        }

        Size borderWidth() const {
            return node->shared.borderWidth;
        }

        Derived& borderWidth(Size width) {
            node->shared.borderWidth = width;
            markDirty(DirtyBits::Finalize);
            return self();
        }

        simd_float4 borderColor() const {
            return node->shared.borderColor;
        }

        Derived& borderColor(simd_float4 color) {
            node->shared.borderColor = color;
            markDirty(DirtyBits::Finalize);
            return self();
        }

        Size padding() const {
            return node->shared.padding;
        }

        Derived& padding(Size padding) {
            node->shared.padding = padding;
            node->shared.paddingTop = padding;
            node->shared.paddingRight = padding;
            node->shared.paddingBottom = padding;
            node->shared.paddingLeft = padding;
            markDirty(layoutDirtyBits());
            return self();
        }

        std::optional<Size> paddingTop() const {
            return node->shared.paddingTop;
        }

        Derived& paddingTop(Size padding) {
            node->shared.paddingTop = padding;
            markDirty(layoutDirtyBits());
            return self();
        }

        std::optional<Size> paddingRight() const {
            return node->shared.paddingRight;
        }

        Derived& paddingRight(Size padding) {
            node->shared.paddingRight = padding;
            markDirty(layoutDirtyBits());
            return self();
        }

        std::optional<Size> paddingBottom() const {
            return node->shared.paddingBottom;
        }

        Derived& paddingBottom(Size padding) {
            node->shared.paddingBottom = padding;
            markDirty(layoutDirtyBits());
            return self();
        }

        std::optional<Size> paddingLeft() const {
            return node->shared.paddingLeft;
        }

        Derived& paddingLeft(Size padding) {
            node->shared.paddingLeft = padding;
            markDirty(layoutDirtyBits());
            return self();
        }

        Derived& margin(float margin) {
            return this->margin(Size::px(margin));
        }

        Size margin() const {
            return node->shared.margin;
        }

        Derived& margin(Size margin) {
            node->shared.margin = margin;
            markDirty(layoutDirtyBits());
            return self();
        }

        Derived& marginTop(float margin) {
            return this->marginTop(Size::px(margin));
        }

        std::optional<Size> marginTop() const {
            return node->shared.marginTop;
        }

        Derived& marginTop(Size margin) {
            node->shared.marginTop = margin;
            markDirty(layoutDirtyBits());
            return self();
        }

        Derived& marginRight(float margin) {
            return this->marginRight(Size::px(margin));
        }

        std::optional<Size> marginRight() const {
            return node->shared.marginRight;
        }

        Derived& marginRight(Size margin) {
            node->shared.marginRight = margin;
            markDirty(layoutDirtyBits());
            return self();
        }

        Derived& marginBottom(float margin) {
            return this->marginBottom(Size::px(margin));
        }

        std::optional<Size> marginBottom() const {
            return node->shared.marginBottom;
        }

        Derived& marginBottom(Size margin) {
            node->shared.marginBottom = margin;
            markDirty(layoutDirtyBits());
            return self();
        }

        Derived& marginLeft(float margin) {
            return this->marginLeft(Size::px(margin));
        }

        std::optional<Size> marginLeft() const {
            return node->shared.marginLeft;
        }

        Derived& marginLeft(Size margin) {
            node->shared.marginLeft = margin;
            markDirty(layoutDirtyBits());
            return self();
        }

        Derived& zIndex(uint64_t zIndex) {
            node->localZIndex = zIndex; // may need to make this global lol and check nvm we good
            markDirty(DirtyBits::PaintOrder);
            return self();
        }

        simd_float4 color() const requires HasColor<typename E::DescriptorType> {
            return descriptor().color;
        }

        Derived& color(simd_float4 color) requires HasColor<typename E::DescriptorType> {
            descriptor().color = color;
            markDirty(DirtyBits::Finalize);
            return self();
        }

        const std::u32string& text() const requires HasText<typename E::DescriptorType> {
            return descriptor().text;
        }

        Derived& text(const std::u32string& text) requires HasText<typename E::DescriptorType> {
            descriptor().text = text;
            markDirty(textDirtyBits());
            return self();
        }

        Derived& text(const std::string& text) requires HasText<typename E::DescriptorType> {
            descriptor().text.assign(text.begin(), text.end());
            markDirty(textDirtyBits());
            return self();
        }

        const std::string& font() const requires HasFont<typename E::DescriptorType> {
            return descriptor().font;
        }

        Derived& font(const std::string& fontPath) requires HasFont<typename E::DescriptorType> {
            descriptor().font = fontPath;
            markDirty(textDirtyBits());
            return self();
        }

        Size fontSize() const requires HasFontSize<typename E::DescriptorType> {
            return descriptor().fontSize;
        }

        Derived& fontSize(Size size) requires HasFontSize<typename E::DescriptorType> {
            descriptor().fontSize = size;
            markDirty(textDirtyBits());
            return self();
        }

        Derived& lineHeight(float multiplier) requires HasLineHeight<typename E::DescriptorType> {
            descriptor().lineHeight = multiplier;
            markDirty(textDirtyBits());
            return self();
        }

        Derived& whiteSpace(WhiteSpace value) requires HasWhiteSpace<typename E::DescriptorType> {
            descriptor().whiteSpace = value;
            markDirty(textDirtyBits());
            return self();
        }

        Derived& wordBreak(WordBreak value) requires HasWordBreak<typename E::DescriptorType> {
            descriptor().wordBreak = value;
            markDirty(textDirtyBits());
            return self();
        }

        using ElemT = Element<E,P, typename E::StorageType, typename E::DescriptorType, typename E::UniformsType>;

    private:
        typename E::DescriptorType& descriptor() {
            auto* elem = static_cast<ElemT*>(node->element.get());
            return elem->element.getDescriptor();
        }

        const typename E::DescriptorType& descriptor() const {
            auto* elem = static_cast<ElemT*>(node->element.get());
            return elem->element.getDescriptor();
        }
    };

    template <ElementType E, typename P>
        requires ProcessorType<P, typename E::StorageType, typename E::DescriptorType, typename E::UniformsType>
    struct EventNode : NodeMutation<EventNode<E,P>, E, P> {
        using Base = NodeMutation<EventNode<E,P>, E, P>;

        EventNode(RenderTree& tree, TreeNode* node):
            Base{tree, node}
        {}
    };

    template <ElementType E, typename P>
        requires ProcessorType<P, typename E::StorageType, typename E::DescriptorType, typename E::UniformsType>
    struct NodeBuilder : NodeMutation<NodeBuilder<E,P>, E, P> {
        using Base = NodeMutation<NodeBuilder<E,P>, E, P>;

        UIContext& ctx;
        std::vector<NodeBuilder> children;

        NodeBuilder(UIContext& ctx, RenderTree& tree, E elem, P& proc):
            Base{tree, nullptr},
            ctx{ctx}
        {
            auto root = this->renderTree.getRoot();

            auto n = std::make_unique<TreeNode>(
                ctx, std::move(elem), proc
            );

            this->node = n.get();

            root->attach_child(std::move(n));
            this->renderTree.markDirty(root, Base::layoutDirtyBits() | DirtyBits::PaintOrder);
        }


        static TreeNode* reparent(TreeNode* newParent, TreeNode* child) {
            auto* oldParent = child->parent;
            auto& siblings = child->parent->children;
            auto it = std::find_if(siblings.begin(), siblings.end(), [&](auto& elem){
                return elem.get() == child;
            });

            if (it == siblings.end()) return nullptr;

            std::unique_ptr<TreeNode> moved = std::move(*it);
            siblings.erase(it);

            newParent->attach_child(std::move(moved));
            return oldParent;
        }

        template <typename... Children>
        NodeBuilder& operator()(Children&&... args) {
            (([&] {
                auto* oldParent = reparent(this->node, args.treeNode());
                if (oldParent) {
                    this->renderTree.markDirty(oldParent, Base::layoutDirtyBits() | DirtyBits::PaintOrder);
                    this->renderTree.markDirty(this->node, Base::layoutDirtyBits() | DirtyBits::PaintOrder);
                }
            }()), ...);
            return *this;
        }


        using NodeBuilderEventHandler = std::function<void(EventNode<E,P>& node, Event& event)>;

        NodeBuilder<E,P>& addEventListener(EventType type, NodeBuilderEventHandler handler) {
            auto stableNode = this->node;
            auto* tree = &this->renderTree;

            auto func = [stableNode, tree, handler](Event& event){
                EventNode<E,P> eventNode{*tree, stableNode};
                handler(eventNode, event);
            };

            this->node->addEventListener(type, func);
            return *this;
        }

        TreeNode* treeNode() const {
            return this->node;
        }
    };

    NodeBuilder<Div<DivStorage>, DivProcessor<DivStorage, DivUniforms>> div();
    NodeBuilder<Div<DivStorage>, DivProcessor<DivStorage, DivUniforms>> div(Size width, Size height, simd_float4 color);
    NodeBuilder<Text<TextStorage>, TextProcessor<TextStorage, TextUniforms>> text(const std::u32string& text, 
                 Size fontSize = Size::pt(24.0f), simd_float4 color = {1, 1, 1, 1}, 
                 const std::string& font = Arial);
    NodeBuilder<Image<ImageStorage>, ImageProcessor<ImageStorage, ImageUniforms>> image(const std::string& path,
                    Size width = Size::px(0), Size height = Size::px(0));
    NodeBuilder<SVG<SVGStorage>, SVGProcessor<SVGStorage, SVGUniforms>> svg(const std::string& path,
                    Size width = Size::px(0), Size height = Size::px(0));
}
