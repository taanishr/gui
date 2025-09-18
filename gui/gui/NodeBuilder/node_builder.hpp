//
//  NodeBuilder.hpp
//  gui
//
//  Created by Taanish Reja on 8/25/25.
//

#pragma once
#include "drawable.hpp"
#include "renderer.hpp"
#include "metal_imports.hpp"
#include "color.hpp"

template <typename T>
concept HasColor = requires(T t) {
    { t.color } -> std::convertible_to<simd_float4>;
};

template <typename T>
concept HasBorderColor = requires(T t) {
    { t.borderColor } -> std::convertible_to<simd_float4>;
};

template <typename T>
concept HasBorderWidth = requires(T t) {
    { t.borderWidth } -> std::convertible_to<float>;
};

template <typename T>
concept HasCornerRadius = requires(T t) {
    { t.cornerRadius } -> std::convertible_to<float>;
};

template <typename T>
concept HasFontSize = requires(T t) {
    { t.fontSize } -> std::convertible_to<float>;
};

template <typename DrawableType, typename LayoutType = DefaultLayout>
    requires Drawable<DrawableType, LayoutType>
struct NodeBuilder {
    RenderTree& renderTree;
    RenderNode<DrawableType, LayoutType>* node;
    
    NodeBuilder(RenderTree& renderTree, RenderNodeBase* parent, std::unique_ptr<DrawableType> drawable):
        renderTree{renderTree}
    {
        this->node = renderTree.insertNode<DrawableType, LayoutType>(std::move(drawable), parent);
    }

    template <typename... Children>
    NodeBuilder<DrawableType, LayoutType>& operator()(Children&&... children) {
        (..., renderTree.reparent(children.node, this->node));
        return *this;
    }
    
    NodeBuilder<DrawableType, LayoutType>& x(float x) {
        node->layoutBox.setX(x);
        return *this;
    }
    
    NodeBuilder<DrawableType, LayoutType>& y(float y) {
        node->layoutBox.setY(y);
        return *this;
    }
    
    NodeBuilder<DrawableType, LayoutType>& w(float width) {
        node->layoutBox.setWidth(width);
        return *this;
    }
    
    NodeBuilder<DrawableType, LayoutType>& h(float height) {
        node->layoutBox.setHeight(height);
        return *this;
    }
    
    template <ColorType Color>
    NodeBuilder<DrawableType, LayoutType>& color(Color color) requires HasColor<DrawableType> {
        node->drawable->color = color.get();
        return *this;
    }
    
    template <ColorType Color>
    NodeBuilder<DrawableType, LayoutType>& borderColor(Color color) requires HasBorderColor<DrawableType> {
        node->drawable->borderColor = color.get();
        return *this;
    }
    
    
    NodeBuilder<DrawableType, LayoutType>& borderWidth(float borderWidth) requires HasBorderWidth<DrawableType> {
        node->drawable->borderWidth = borderWidth;
        return *this;
    }
    
    NodeBuilder<DrawableType, LayoutType>& fontSize(float fontSize) requires HasFontSize<DrawableType> {
        node->drawable->fontSize = fontSize;
        return *this;
    }
    
    NodeBuilder<DrawableType, LayoutType>& cornerRadius(float cornerRadius) requires HasCornerRadius<DrawableType> {
        node->drawable->cornerRadius = cornerRadius;
        return *this;
    }
    
    NodeBuilder<DrawableType, LayoutType>& block() {
        node->layoutBox.display = Block{};
        return *this;
    }
    
    NodeBuilder<DrawableType, LayoutType>& staticPos() {
        node->layoutBox.position = Position::Static;
        return *this;
    }
    
    NodeBuilder<DrawableType, LayoutType>& relativePos() {
        node->layoutBox.position = Position::Relative;
        return *this;
    }
    
    NodeBuilder<DrawableType, LayoutType>& absolutePos() {
        node->layoutBox.position = Position::Absolute;
        return *this;
    }
    
    
    
    template <EventType E, typename F>
    NodeBuilder<DrawableType, LayoutType>& on(F&& f) {
        node->template addEventHandler<E>(f);
        return *this;
    }
    
    
};


// idea: node builder base with regular templates
// then add a node builder normal for each element (shell/text/etc...)
// problem; doesn't fix attr problem?
// solutions? define custom operator

NodeBuilder<Shell, ShellLayout> div(float cornerRadius = 0, simd_float4 color = {0,0,0,1});

NodeBuilder<Text, TextLayout> text(const std::string& text, float fontSize = 24.0, float x = 0, float y = 0);

NodeBuilder<ImageDrawable, ImageLayout> image(const std::string& path);

using Element = std::variant<NodeBuilder<Shell, ShellLayout>,NodeBuilder<Text, TextLayout>,NodeBuilder<ImageDrawable, ImageLayout>>;
