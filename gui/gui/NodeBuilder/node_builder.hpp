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
concept HasWidth = requires(T t) {
    { t.width } -> std::convertible_to<float>;
};

template <typename T>
concept HasHeight = requires(T t) {
    { t.height } -> std::convertible_to<float>;
};

template <typename T>
concept HasCornerRadius = requires(T t) {
    { t.cornerRadius } -> std::convertible_to<float>;
};

template <typename T>
concept HasFontSize = requires(T t) {
    { t.fontSize } -> std::convertible_to<float>;
};


template <Drawable DrawableType>
struct NodeBuilder {
    RenderTree& renderTree;
    RenderNode<DrawableType>* node;
    
    NodeBuilder(RenderTree& renderTree, RenderNodeBase* parent, std::unique_ptr<DrawableType> drawable):
        renderTree{renderTree}
    {
        this->node = renderTree.insertNode(std::move(drawable), parent);
    }
    
    
    template <typename... Children>
    NodeBuilder<DrawableType>& operator()(Children&&... children) {
        (..., renderTree.reparent(this->node, children.node));
        return *this;
    }
    
    NodeBuilder<DrawableType>& x(float x) {
        node->drawable->x = x;
        return *this;
    }
    
    NodeBuilder<DrawableType>& y(float y) {
        node->drawable->y = y;
        return *this;
    }
    
    NodeBuilder<DrawableType>& w(float width) requires HasWidth<DrawableType> {
        node->drawable->width = width;
        return *this;
    }
    
    NodeBuilder<DrawableType>& h(float height) requires HasHeight<DrawableType> {
        node->drawable->height = height;
        return *this;
    }
    
    NodeBuilder<DrawableType>& color(simd_float4 color) requires HasColor<DrawableType> {
        node->drawable->color = color;
        return *this;
    }
    
    NodeBuilder<DrawableType>& borderColor(simd_float4 color) requires HasBorderColor<DrawableType> {
        node->drawable->borderColor = color;
        return *this;
    }
    
    
    NodeBuilder<DrawableType>& borderWidth(float borderWidth) requires HasBorderWidth<DrawableType> {
        node->drawable->borderWidth = borderWidth;
        return *this;
    }
    
    NodeBuilder<DrawableType>& fontSize(float fontSize) requires HasFontSize<DrawableType> {
        node->drawable->fontSize = fontSize;
        return *this;
    }
    
    NodeBuilder<DrawableType>& cornerRadius(float cornerRadius) requires HasCornerRadius<DrawableType> {
        node->drawable->cornerRadius = cornerRadius;
        return *this;
    }
    
    
    template <EventType E, typename F>
    NodeBuilder<DrawableType>& on(F&& f) {
        node->template addEventHandler<E>(f);
        return *this;
    }
    
    
};


// idea: node builder base with regular templates
// then add a node builder normal for each element (shell/text/etc...)
// problem; doesn't fix attr problem?
// solutions? define custom operator


NodeBuilder<Shell> div(float w = 0, float h = 0, float cornerRadius = 0, float x = 0, float y = 0, simd_float4 color = {0,0,0,1});

NodeBuilder<Text> text(const std::string& text, float fontSize = 24.0, float x = 0, float y = 0);

NodeBuilder<ImageDrawable> image(const std::string& path);
