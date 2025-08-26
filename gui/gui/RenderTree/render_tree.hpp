//
//  render_tree.hpp
//  gui
//
//  Created by Taanish Reja on 8/21/25.
//


// ownership dynamics
// render tree owns all nodes
// nodes merely hold ptr to other nodes

// when we add or delete a node, the render tree handles this, not nodes

#pragma once
#include "metal_imports.hpp"
#include <memory>
#include <set>
#include "drawable.hpp"
#include <print>
#include <algorithm>
#include <any>
#include <functional>
#include "events.hpp"
#include "sdf_helpers.hpp"

// goals; tree that starts from null root (the primary view)
// inserted based on parent node and z that is relative to the parent

struct RenderNode;

using EventHandler = std::function<void(RenderNode&, const Event&)>;

struct RenderNodeComparator {
    bool operator()(const std::unique_ptr<RenderNode>& a, const std::unique_ptr<RenderNode>& b) const;
};

struct RenderNode {
    RenderNode();
    
    RenderNode(Drawable& drawable, RenderNode* parent);
    
    void update();
    
    void encode(MTL::RenderCommandEncoder* encoder) const;
    
    void render(MTL::RenderCommandEncoder* encoder) const;
    
    void changeZIndex(int zIndex);
    
    template <EventType E, typename F>
    void addEventHandler(F&& f);

    void dispatch(const Event& event);
    
    RenderNode* parent;
    
    std::vector<std::unique_ptr<RenderNode>> children;
    
    std::unique_ptr<Drawable> drawable;
    
    int zIndex;
    
    uint64_t insertionId;
    
    std::unordered_map<EventType, std::vector<EventHandler>> handlers;
};

template <EventType E, typename F>
void RenderNode::addEventHandler(F&& f)
{
    EventHandler wrapper = [fn = std::forward<F>(f)](RenderNode& self, const Event& event){
        if constexpr (E == EventType::Click) {
            auto& mousePayload = std::get<MousePayload>(event.payload);
            
            if (!self.drawable->contains({mousePayload.x, mousePayload.y}))
                return;
        }
        
        
        fn(self, std::get<event_payload_t<E>>(event.payload));
    };
    
    handlers[E].push_back(std::move(wrapper));
}

struct RenderTree {
    RenderTree();

    void update();
    void render(MTL::RenderCommandEncoder* encoder) const;
    
    RenderNode* insertNode(std::unique_ptr<Drawable> drawable, RenderNode* parent);
    void deleteNode(RenderNode* node);
    
    void reparent(RenderNode* node, RenderNode* newParent);
    
    void dispatch(const Event& event);
    
    std::unique_ptr<RenderNode> root;
    std::unordered_map<uint64_t, RenderNode*> nodeMap;
    
    size_t nodes;
    uint64_t nextInsertionId;
};
