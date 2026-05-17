#pragma once

#include "element.hpp"
#include "div.hpp"
#include "events.hpp"
#include "renderer.hpp"
#include "image.hpp"
#include "svg.hpp"
#include "text.hpp"
#include "new_arch.hpp"
#include <algorithm>
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


    template <ElementType E, typename P>
        requires ProcessorType<P, typename E::StorageType, typename E::DescriptorType, typename E::UniformsType>
    struct NodeBuilder {
        RenderTree& renderTree;
        UIContext& ctx;
        TreeNode* node;
        std::vector<NodeBuilder> children;

        NodeBuilder(UIContext& ctx, RenderTree& tree, E elem, P& proc): 
            ctx{ctx},
            renderTree{tree}
        {
            auto root = renderTree.getRoot();

            auto n = std::make_unique<TreeNode>(
                ctx, std::move(elem), proc
            );
            
            this->node = n.get();

            root->attach_child(std::move(n));
            renderTree.markDirty(root, layoutDirtyBits() | DirtyBits::PaintOrder);
        }

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
                auto* oldParent = reparent(this->node, args.node);
                if (oldParent) {
                    renderTree.markDirty(oldParent, layoutDirtyBits() | DirtyBits::PaintOrder);
                    renderTree.markDirty(this->node, layoutDirtyBits() | DirtyBits::PaintOrder);
                }
            }()), ...);
            return *this;
        }

        NodeBuilder<E,P>& position(Position position) {
            node->shared.position = position;
            markDirty(layoutDirtyBits());
            return *this;
        }

        NodeBuilder<E,P>& display(Display display) {
            node->shared.display = display;
            markDirty(layoutDirtyBits());
            return *this;
        }

        NodeBuilder<E,P>& width(Size width) {
            node->shared.width = width;
            markDirty(layoutDirtyBits());
            return *this;
        }

        NodeBuilder<E,P>& height(Size height) {
            node->shared.height = height;
            markDirty(layoutDirtyBits());
            return *this;
        }

        NodeBuilder<E,P>& top(Size top) {
            node->shared.top = top;
            markDirty(layoutDirtyBits());
            return *this;
        }

        NodeBuilder<E,P>& right(Size right) {
            node->shared.right = right;
            markDirty(layoutDirtyBits());
            return *this;
        }

        NodeBuilder<E,P>& bottom(Size bottom) {
            node->shared.bottom = bottom;
            markDirty(layoutDirtyBits());
            return *this;
        }

        NodeBuilder<E,P>& left(Size left) {
            node->shared.left = left;
            markDirty(layoutDirtyBits());
            return *this;
        }

        NodeBuilder<E,P>& flexDirection(FlexDirection direction) {
            node->shared.flexDirection = direction;
            markDirty(layoutDirtyBits());
            return *this;
        }

        NodeBuilder& justifyContent(JustifyContent justifyContent) { 
            node->shared.justifyContent = justifyContent; 
            markDirty(layoutDirtyBits());
            return *this; 
        }

        NodeBuilder& alignItems(AlignItems alignItems) {
            node->shared.alignItems = alignItems;
            markDirty(layoutDirtyBits());
            return *this;
        }

        NodeBuilder& flexWrap(FlexWrap wrap) {
            node->shared.flexWrap = wrap;
            markDirty(layoutDirtyBits());
            return *this;
        }

        NodeBuilder& alignContent(AlignContent ac) {
            node->shared.alignContent = ac;
            markDirty(layoutDirtyBits());
            return *this;
        }

        NodeBuilder& alignSelf(AlignSelf as) {
            node->shared.alignSelf = as;
            markDirty(layoutDirtyBits());
            return *this;
        }

        NodeBuilder& overflow(Overflow overflow) {
            node->shared.overflow = overflow;
            markDirty(layoutDirtyBits());
            return *this;
        }

        NodeBuilder<E,P>& flexGrow(Size grow) {
            node->shared.flexGrow = grow;
            markDirty(layoutDirtyBits());
            return *this;
        }

        NodeBuilder<E,P>& flexGap(Size gap) {
            node->shared.flexGap = gap;
            markDirty(layoutDirtyBits());
            return *this;
        }

        NodeBuilder<E,P>& flexShrink(Size shrink) {
            node->shared.flexShrink = shrink;
            markDirty(layoutDirtyBits());
            return *this;
        }

        NodeBuilder& gridTemplateColumns(std::vector<Size> tracks) {
            node->shared.gridTemplateColumns = std::move(tracks);
            markDirty(layoutDirtyBits());
            return *this;
        }

        NodeBuilder& gridTemplateRows(std::vector<Size> tracks) {
            node->shared.gridTemplateRows = std::move(tracks);
            markDirty(layoutDirtyBits());
            return *this;
        }

        NodeBuilder& gridColumnGap(Size gap) {
            node->shared.gridColumnGap = gap;
            markDirty(layoutDirtyBits());
            return *this;
        }

        NodeBuilder& gridRowGap(Size gap) {
            node->shared.gridRowGap = gap;
            markDirty(layoutDirtyBits());
            return *this;
        }

        NodeBuilder& gridColumn(int start, int end = 0) {
            node->shared.gridPlacement.colStart = start;
            node->shared.gridPlacement.colEnd = end;
            markDirty(layoutDirtyBits());
            return *this;
        }

        NodeBuilder& gridRow(int start, int end = 0) {
            node->shared.gridPlacement.rowStart = start;
            node->shared.gridPlacement.rowEnd = end;
            markDirty(layoutDirtyBits());
            return *this;
        }

        NodeBuilder<E,P>& cornerRadius(Size radius) {
            node->shared.cornerRadius = radius;
            markDirty(DirtyBits::Finalize);
            return *this;
        }

        NodeBuilder<E,P>& borderWidth(Size width) {
            node->shared.borderWidth = width;
            markDirty(DirtyBits::Finalize);
            return *this;
        }

        NodeBuilder<E,P>& borderColor(simd_float4 color) {
            node->shared.borderColor = color;
            markDirty(DirtyBits::Finalize);
            return *this;
        }

        NodeBuilder<E,P>& padding(Size padding) {
            node->shared.padding = padding;
            node->shared.paddingTop = padding;
            node->shared.paddingRight = padding;
            node->shared.paddingBottom = padding;
            node->shared.paddingLeft = padding;
            markDirty(layoutDirtyBits());
            return *this;
        }

        NodeBuilder<E,P>& paddingTop(Size padding) {
            node->shared.paddingTop = padding;
            markDirty(layoutDirtyBits());
            return *this;
        }

        NodeBuilder<E,P>& paddingRight(Size padding) {
            node->shared.paddingRight = padding;
            markDirty(layoutDirtyBits());
            return *this;
        }

        NodeBuilder<E,P>& paddingBottom(Size padding) {
            node->shared.paddingBottom = padding;
            markDirty(layoutDirtyBits());
            return *this;
        }

        NodeBuilder<E,P>& paddingLeft(Size padding) {
            node->shared.paddingLeft = padding;
            markDirty(layoutDirtyBits());
            return *this;
        }

        NodeBuilder<E,P>& margin(float margin) {
            return this->margin(Size::px(margin));
        }

        NodeBuilder<E,P>& margin(Size margin) {
            node->shared.margin = margin;
            markDirty(layoutDirtyBits());
            return *this;
        }

        NodeBuilder<E,P>& marginTop(float margin) {
            return this->marginTop(Size::px(margin));
        }

        NodeBuilder<E,P>& marginTop(Size margin) {
            node->shared.marginTop = margin;
            markDirty(layoutDirtyBits());
            return *this;
        }

        NodeBuilder<E,P>& marginRight(float margin) {
            return this->marginRight(Size::px(margin));
        }

        NodeBuilder<E,P>& marginRight(Size margin) {
            node->shared.marginRight = margin;
            markDirty(layoutDirtyBits());
            return *this;
        }

        NodeBuilder<E,P>& marginBottom(float margin) {
            return this->marginBottom(Size::px(margin));
        }

        NodeBuilder<E,P>& marginBottom(Size margin) {
            node->shared.marginBottom = margin;
            markDirty(layoutDirtyBits());
            return *this;
        }

        NodeBuilder<E,P>& marginLeft(float margin) {
            return this->marginLeft(Size::px(margin));
        }

        NodeBuilder<E,P>& marginLeft(Size margin) {
            node->shared.marginLeft = margin;
            markDirty(layoutDirtyBits());
            return *this;
        }

        // --- Element-specific setters (write to typed descriptor) ---

        NodeBuilder<E,P>& color(simd_float4 color) requires HasColor<typename E::DescriptorType> {
            auto* elem = static_cast<ElemT*>(node->element.get());
            auto& desc = elem->element.getDescriptor();
            desc.color = color;
            markDirty(DirtyBits::Finalize);
            return *this;
        }

        NodeBuilder<E,P>& text(const std::u32string& text) requires HasText<typename E::DescriptorType> {
            auto* elem = static_cast<ElemT*>(node->element.get());
            auto& desc = elem->element.getDescriptor();
            desc.text = text;
            markDirty(textDirtyBits());
            return *this;
        }

        NodeBuilder<E,P>& text(const std::string& text) requires HasText<typename E::DescriptorType> {
            auto* elem = static_cast<ElemT*>(node->element.get());
            auto& desc = elem->element.getDescriptor();
            desc.text.assign(text.begin(), text.end());
            markDirty(textDirtyBits());
            return *this;
        }

        NodeBuilder<E,P>& font(const std::string& fontPath) requires HasFont<typename E::DescriptorType> {
            auto* elem = static_cast<ElemT*>(node->element.get());
            auto& desc = elem->element.getDescriptor();
            desc.font = fontPath;
            markDirty(textDirtyBits());
            return *this;
        }

        NodeBuilder<E,P>& fontSize(Size size) requires HasFontSize<typename E::DescriptorType> {
            auto* elem = static_cast<ElemT*>(node->element.get());
            auto& desc = elem->element.getDescriptor();
            desc.fontSize = size;
            markDirty(textDirtyBits());
            return *this;
        }


        using NodeBuilderEventHandler = std::function<void(typename E::DescriptorType& descriptor, const Event& event)>;

        NodeBuilder<E,P>& addEventListener(EventType type, NodeBuilderEventHandler handler) {
            auto stableNode = node;
            auto* tree = &renderTree;

            auto func = [stableNode, tree, handler](const Event& event){
                auto elem = static_cast<ElemT*>(stableNode->element.get());
                auto& desc = elem->element.getDescriptor();

                handler(desc, event);

                if constexpr (HasText<typename E::DescriptorType>) {
                    tree->markDirty(stableNode, NodeBuilder<E,P>::textDirtyBits());
                } else {
                    tree->markDirty(stableNode, DirtyBits::Finalize);
                }
            };

            node->addEventListener(type, func);
            return *this;
        }

        using ElemT = Element<E,P, typename E::StorageType, typename E::DescriptorType, typename E::UniformsType>;
    };

    NodeBuilder<Div<DivStorage>, DivProcessor<DivStorage, DivUniforms>> div();
    NodeBuilder<Div<DivStorage>, DivProcessor<DivStorage, DivUniforms>> div(Size width, Size height, simd_float4 color);
    NodeBuilder<Text<TextStorage>, TextProcessor<TextStorage, TextUniforms>> text(const std::u32string& text, 
                 Size fontSize = Size::pt(24.0f), simd_float4 color = {1, 1, 1, 1}, 
                 const std::string& font = "/System/Library/Fonts/Supplemental/Arial.ttf");
    NodeBuilder<Image<ImageStorage>, ImageProcessor<ImageStorage, ImageUniforms>> image(const std::string& path,
                    Size width = Size::px(0), Size height = Size::px(0));
    NodeBuilder<SVG<SVGStorage>, SVGProcessor<SVGStorage, SVGUniforms>> svg(const std::string& path,
                    Size width = Size::px(0), Size height = Size::px(0));
}
