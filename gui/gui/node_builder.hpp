#pragma once

#include "element.hpp"
#include "div.hpp"
#include "events.hpp"
#include "renderer.hpp"
#include "image.hpp"
#include "text.hpp"
#include "new_arch.hpp"
#include <algorithm>
#include <memory>
#include <print>
#include "context_manager.hpp"

namespace NewArch {
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
        }

        
        
        static void reparent(TreeNode* newParent, TreeNode* child) {
            auto& siblings = child->parent->children;
            auto it = std::find_if(siblings.begin(), siblings.end(), [&](auto& elem){
                return elem.get() == child;
            });

            if (it == siblings.end()) return;

            std::unique_ptr<TreeNode> moved = std::move(*it);
            siblings.erase(it);

            newParent->attach_child(std::move(moved));
        }

        template <typename... Children>
        NodeBuilder& operator()(Children&&... args) {
            TreeStack::pushTree(&this->renderTree);
            (reparent(this->node, args.node), ...);
            TreeStack::popTree();
            return *this;
        }

        // --- Shared property setters (write to node->shared) ---

        NodeBuilder<E,P>& position(Position position) {
            node->shared.position = position;
            return *this;
        }

        NodeBuilder<E,P>& display(Display display) {
            node->shared.display = display;
            return *this;
        }

        NodeBuilder<E,P>& width(Size width) {
            node->shared.width = width;
            return *this;
        }

        NodeBuilder<E,P>& height(Size height) {
            node->shared.height = height;
            return *this;
        }

        NodeBuilder<E,P>& top(Size top) {
            node->shared.top = top;
            return *this;
        }

        NodeBuilder<E,P>& right(Size right) {
            node->shared.right = right;
            return *this;
        }

        NodeBuilder<E,P>& bottom(Size bottom) {
            node->shared.bottom = bottom;
            return *this;
        }

        NodeBuilder<E,P>& left(Size left) {
            node->shared.left = left;
            return *this;
        }

        NodeBuilder<E,P>& flexDirection(FlexDirection direction) {
            node->shared.flexDirection = direction;
            return *this;
        }

        NodeBuilder& justifyContent(JustifyContent justifyContent) { 
            node->shared.justifyContent = justifyContent; 
            return *this; 
        }

        NodeBuilder& alignItems(AlignItems alignItems) {
            node->shared.alignItems = alignItems;
            return *this;
        }

        NodeBuilder& flexWrap(FlexWrap wrap) {
            node->shared.flexWrap = wrap;
            return *this;
        }

        NodeBuilder& alignContent(AlignContent ac) {
            node->shared.alignContent = ac;
            return *this;
        }

        NodeBuilder& alignSelf(AlignSelf as) {
            node->shared.alignSelf = as;
            return *this;
        }

        NodeBuilder<E,P>& flexGrow(Size grow) {
            node->shared.flexGrow = grow;
            return *this;
        }

        NodeBuilder<E,P>& flexGap(Size gap) {
            node->shared.flexGap = gap;
            return *this;
        }

        NodeBuilder<E,P>& flexShrink(Size shrink) {
            node->shared.flexShrink = shrink;
            return *this;
        }

        NodeBuilder<E,P>& cornerRadius(Size radius) {
            node->shared.cornerRadius = radius;
            return *this;
        }

        NodeBuilder<E,P>& borderWidth(Size width) {
            node->shared.borderWidth = width;
            return *this;
        }

        NodeBuilder<E,P>& borderColor(simd_float4 color) {
            node->shared.borderColor = color;
            return *this;
        }

        NodeBuilder<E,P>& padding(Size padding) {
            node->shared.padding = padding;
            node->shared.paddingTop = padding;
            node->shared.paddingRight = padding;
            node->shared.paddingBottom = padding;
            node->shared.paddingLeft = padding;
            return *this;
        }

        NodeBuilder<E,P>& paddingTop(Size padding) {
            node->shared.paddingTop = padding;
            return *this;
        }

        NodeBuilder<E,P>& paddingRight(Size padding) {
            node->shared.paddingRight = padding;
            return *this;
        }

        NodeBuilder<E,P>& paddingBottom(Size padding) {
            node->shared.paddingBottom = padding;
            return *this;
        }

        NodeBuilder<E,P>& paddingLeft(Size padding) {
            node->shared.paddingLeft = padding;
            return *this;
        }

        NodeBuilder<E,P>& margin(float margin) {
            return this->margin(Size::px(margin));
        }

        NodeBuilder<E,P>& margin(Size margin) {
            node->shared.margin = margin;
            return *this;
        }

        NodeBuilder<E,P>& marginTop(float margin) {
            return this->marginTop(Size::px(margin));
        }

        NodeBuilder<E,P>& marginTop(Size margin) {
            node->shared.marginTop = margin;
            return *this;
        }

        NodeBuilder<E,P>& marginRight(float margin) {
            return this->marginRight(Size::px(margin));
        }

        NodeBuilder<E,P>& marginRight(Size margin) {
            node->shared.marginRight = margin;
            return *this;
        }

        NodeBuilder<E,P>& marginBottom(float margin) {
            return this->marginBottom(Size::px(margin));
        }

        NodeBuilder<E,P>& marginBottom(Size margin) {
            node->shared.marginBottom = margin;
            return *this;
        }

        NodeBuilder<E,P>& marginLeft(float margin) {
            return this->marginLeft(Size::px(margin));
        }

        NodeBuilder<E,P>& marginLeft(Size margin) {
            node->shared.marginLeft = margin;
            return *this;
        }

        // --- Element-specific setters (write to typed descriptor) ---

        NodeBuilder<E,P>& color(simd_float4 color) requires HasColor<typename E::DescriptorType> {
            auto* elem = static_cast<ElemT*>(node->element.get());
            auto& desc = elem->element.getDescriptor();
            desc.color = color;
            return *this;
        }

        NodeBuilder<E,P>& text(const std::string& text) requires HasText<typename E::DescriptorType> {
            auto* elem = static_cast<ElemT*>(node->element.get());
            auto& desc = elem->element.getDescriptor();
            desc.text = text;
            return *this;
        }

        NodeBuilder<E,P>& font(const std::string& fontPath) requires HasFont<typename E::DescriptorType> {
            auto* elem = static_cast<ElemT*>(node->element.get());
            auto& desc = elem->element.getDescriptor();
            desc.font = fontPath;
            return *this;
        }

        NodeBuilder<E,P>& fontSize(Size size) requires HasFontSize<typename E::DescriptorType> {
            auto* elem = static_cast<ElemT*>(node->element.get());
            auto& desc = elem->element.getDescriptor();
            desc.fontSize = size;
            return *this;
        }


        using NodeBuilderEventHandler = std::function<void(typename E::DescriptorType& descriptor, const Event& event)>;

        NodeBuilder<E,P>& addEventListener(EventType type, NodeBuilderEventHandler handler) {
            auto stableNode = node;

            auto func = [stableNode, handler](const Event& event){ 
                auto elem = static_cast<ElemT*>(stableNode->element.get());
                auto& desc = elem->element.getDescriptor();

                handler(desc, event);                
            };

            node->addEventListener(type, func);
            return *this;
        }

        using ElemT = Element<E,P, typename E::StorageType, typename E::DescriptorType, typename E::UniformsType>;
    };

    NodeBuilder<Div<DivStorage>, DivProcessor<DivStorage, DivUniforms>> div();
    NodeBuilder<Div<DivStorage>, DivProcessor<DivStorage, DivUniforms>> div(Size width, Size height, simd_float4 color);
    NodeBuilder<Text<TextStorage>, TextProcessor<TextStorage, TextUniforms>> text(const std::string& text, 
                 Size fontSize = Size::pt(24.0f), simd_float4 color = {1, 1, 1, 1}, 
                 const std::string& font = "/System/Library/Fonts/Supplemental/Arial.ttf");
    NodeBuilder<Image<ImageStorage>, ImageProcessor<ImageStorage, ImageUniforms>> image(const std::string& path,
                    Size width = Size::px(0), Size height = Size::px(0));
}