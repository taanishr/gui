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
#include "renderable.hpp"
#include <print>
#include <algorithm>

// goals; tree that starts from null root (the primary view)
// inserted based on parent node and z that is relative to the parent

struct RenderNode;

struct RenderNodeComparator {
    bool operator()(const std::unique_ptr<RenderNode>& a, const std::unique_ptr<RenderNode>& b) const;
};


struct RenderNode {
    RenderNode();
    
    RenderNode(Renderable& renderable, RenderNode* parent);
    
    void update();
    
    void encode(MTL::RenderCommandEncoder* encoder) const;
    
    void render(MTL::RenderCommandEncoder* encoder) const;
    
    RenderNode* parent;
    
    std::vector<std::unique_ptr<RenderNode>> children;
    
    std::unique_ptr<Renderable> renderable;
    
    int zIndex;
};

struct RenderTree {
    RenderTree();

    void update();
    void render(MTL::RenderCommandEncoder* encoder) const;
    
    RenderNode* insertNode(std::unique_ptr<Renderable> renderable, RenderNode* parent);
    void deleteNode(RenderNode* node);
    
    std::unique_ptr<RenderNode> root;
};


