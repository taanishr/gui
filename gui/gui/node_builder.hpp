#pragma once

#include "element.hpp"
#include "div.hpp"
#include "image.hpp"
#include "text.hpp"
#include "new_arch.hpp"
#include <algorithm>
#include <memory>

namespace NewArch {
    template<typename T>
    concept HasDisplay = requires { std::declval<T&>().display; };

    template<typename T>
    concept HasPosition = requires { std::declval<T&>().position; };
    
    template<typename T>
    concept HasText = requires { std::declval<T&>().text; };

    template<typename T>
    concept HasFont = requires { std::declval<T&>().font; };

    template<typename T>
    concept HasColor = requires { std::declval<T&>().color; };

    template<typename T>
    concept HasFontSize = requires { std::declval<T&>().fontSize; };

    template<typename T>
    concept HasCornerRadius = requires { std::declval<T&>().cornerRadius; };

    template<typename T>
    concept HasBorderWidth = requires { std::declval<T&>().borderWidth; };

    template<typename T>
    concept HasBorderColor = requires { std::declval<T&>().borderColor; };

    template <typename T>
    concept HasTop = requires{ std::declval<T&>().top; };

    template <typename T>
    concept HasLeft = requires{ std::declval<T&>().left; };

    template <typename T>
    concept HasPadding = requires(T& t) { 
        { t.padding } -> std::convertible_to<float>;
        { t.paddingTop } -> std::same_as<std::optional<float>&>;
        { t.paddingRight } -> std::same_as<std::optional<float>&>;
        { t.paddingBottom } -> std::same_as<std::optional<float>&>;
        { t.paddingLeft } -> std::same_as<std::optional<float>&>;
    };

    DivProcessor<DivStorage, DivUniforms>& getDivProcessor(UIContext& ctx);
    ImageProcessor<ImageStorage, ImageUniforms>& getImageProcessor(UIContext& ctx);
    TextProcessor<TextStorage, TextUniforms>& getTextProcessor(UIContext& ctx);

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
           (reparent(this->node, args.node), ...);
            return *this;
        }

        NodeBuilder<E,P>& position(Position position) requires HasPosition<typename E::DescriptorType> {
            auto* elem = static_cast<ElemT*>(node->element.get());
            auto& rawElem = elem->element;
            auto& desc = rawElem.getDescriptor();
            desc.position = position;
            return *this;
        }
        
        NodeBuilder<E,P>& borderColor(simd_float4 color) requires HasBorderColor<typename E::DescriptorType> {
            auto* elem = static_cast<ElemT*>(node->element.get());
            auto& rawElem = elem->element;
            auto& desc = rawElem.getDescriptor();
            desc.borderColor = color;
            return *this;
        }

        NodeBuilder<E,P>& color(simd_float4 color) requires HasColor<typename E::DescriptorType> {
            auto* elem = static_cast<ElemT*>(node->element.get());
            auto& rawElem = elem->element;
            auto& desc = rawElem.getDescriptor();
            desc.color = color;
            return *this;
        }

        NodeBuilder<E,P>& text(const std::string& text) requires HasText<typename E::DescriptorType> {
            auto* elem = static_cast<ElemT*>(node->element.get());
            auto& rawElem = elem->element;
            auto& desc = rawElem.getDescriptor();
            desc.text = text;
            return *this;
        }

        NodeBuilder<E,P>& font(const std::string& fontPath) requires HasFont<typename E::DescriptorType> {
            auto* elem = static_cast<ElemT*>(node->element.get());
            auto& rawElem = elem->element;
            auto& desc = rawElem.getDescriptor();
            desc.font = fontPath;
            return *this;
        }

        NodeBuilder<E,P>& fontSize(float size) requires HasFontSize<typename E::DescriptorType> {
            auto* elem = static_cast<ElemT*>(node->element.get());
            auto& rawElem = elem->element;
            auto& desc = rawElem.getDescriptor();
            desc.fontSize = size;
            return *this;
        }

        NodeBuilder<E,P>& cornerRadius(float radius) requires HasCornerRadius<typename E::DescriptorType> {
            auto* elem = static_cast<ElemT*>(node->element.get());
            auto& rawElem = elem->element;
            auto& desc = rawElem.getDescriptor();
            desc.cornerRadius = radius;
            return *this;
        }

        NodeBuilder<E,P>& borderWidth(float width) requires HasBorderWidth<typename E::DescriptorType> {
            auto* elem = static_cast<ElemT*>(node->element.get());
            auto& rawElem = elem->element;
            auto& desc = rawElem.getDescriptor();
            desc.borderWidth = width;
            return *this;
        }

        NodeBuilder<E,P>& top(float top) requires HasTop<typename E::DescriptorType> {
            auto* elem = static_cast<ElemT*>(node->element.get());
            auto& rawElem = elem->element;
            auto& desc = rawElem.getDescriptor();
            desc.top = top;
            return *this;
        }

        NodeBuilder<E,P>& left(float left) requires HasLeft<typename E::DescriptorType> {
            auto* elem = static_cast<ElemT*>(node->element.get());
            auto& rawElem = elem->element;
            auto& desc = rawElem.getDescriptor();
            desc.left = left;
            return *this;
        }

        NodeBuilder<E,P>& padding(float padding) requires HasPadding<typename E::DescriptorType> {
            auto* elem = static_cast<ElemT*>(node->element.get());
            auto& rawElem = elem->element;
            auto& desc = rawElem.getDescriptor();
            desc.padding = padding;
            return *this;
        }

                NodeBuilder<E,P>& paddingTop(float padding) requires HasPadding<typename E::DescriptorType> {
            auto* elem = static_cast<ElemT*>(node->element.get());
            auto& rawElem = elem->element;
            auto& desc = rawElem.getDescriptor();
            desc.paddingTop = padding;
            return *this;
        }

        NodeBuilder<E,P>& paddingRight(float padding) requires HasPadding<typename E::DescriptorType> {
            auto* elem = static_cast<ElemT*>(node->element.get());
            auto& rawElem = elem->element;
            auto& desc = rawElem.getDescriptor();
            desc.paddingRight = padding;
            return *this;
        }

        NodeBuilder<E,P>& paddingBottom(float padding) requires HasPadding<typename E::DescriptorType> {
            auto* elem = static_cast<ElemT*>(node->element.get());
            auto& rawElem = elem->element;
            auto& desc = rawElem.getDescriptor();
            desc.paddingBottom = padding;
            return *this;
        }

        NodeBuilder<E,P>& paddingLeft(float padding) requires HasPadding<typename E::DescriptorType> {
            auto* elem = static_cast<ElemT*>(node->element.get());
            auto& rawElem = elem->element;
            auto& desc = rawElem.getDescriptor();
            desc.paddingLeft = padding;
            return *this;
        }

        using ElemT = Element<E,P, typename E::StorageType, typename E::DescriptorType, typename E::UniformsType>;
    };

    NodeBuilder<Div<DivStorage>, DivProcessor<DivStorage, DivUniforms>> div(UIContext& ctx, RenderTree& tree, float width, float height, simd_float4 color);
    NodeBuilder<Text<TextStorage>, TextProcessor<TextStorage, TextUniforms>> text(UIContext& ctx, RenderTree& tree, const std::string& text, 
                 float fontSize = 24.0f, simd_float4 color = {1, 1, 1, 1}, 
                 const std::string& font = "/System/Library/Fonts/Supplemental/Arial.ttf");
    NodeBuilder<Image<ImageStorage>, ImageProcessor<ImageStorage, ImageUniforms>> image(UIContext& ctx, RenderTree& tree, const std::string& path,
                    float width = 0.0f, float height = 0.0f);
}