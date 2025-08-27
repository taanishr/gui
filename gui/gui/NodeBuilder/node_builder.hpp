//
//  NodeBuilder.hpp
//  gui
//
//  Created by Taanish Reja on 8/25/25.
//

#pragma once
#include "drawable.hpp"
#include "renderer.hpp"

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
    void operator()(Children&&... children) {
        (..., renderTree.reparent(this->node, children.node));
    }
    
    NodeBuilder<DrawableType>& x(float x) {
        node->drawable->x = x;
        return *this;
    }
    
    NodeBuilder<DrawableType>& y(float y) {
        node->drawable->y = y;
        return *this;
    }
    
    NodeBuilder<DrawableType>& color(simd_float4 color) {
        node->drawable->color = color;
        return *this;
    }
    
    template <EventType E, typename F>
    NodeBuilder<DrawableType>& on(F&& f) {
        node->template addEventHandler<E>(f);
        return *this;
    }
};

NodeBuilder<Shell> div(Renderer& renderer, float w, float h, float cornerRadius = 0, float x = 0, float y = 0, simd_float4 color = {0,0,0,1});

NodeBuilder<Text> text(Renderer& renderer, const std::string& text, float fontSize = 24.0, float x = 0, float y = 0);
