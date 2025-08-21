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

// goals; tree that starts from null root (the primary view)
// inserted based on parent node and z that is relative to the parent

struct RenderNode;


struct RenderNode {
    // root ctor
    RenderNode();
    
    // regular ctor
    RenderNode(Renderable& renderable, RenderNode* parent);
    
    // update element
    void update();
    
    // render element
    void render(MTL::RenderCommandEncoder* encoder);
    
    // parent
    RenderNode* parent;
    
    // children
    std::vector<std::unique_ptr<RenderNode>> children; // figure out a comparator for this and change to set later
    
    std::unique_ptr<Renderable> renderable;
    
    int zIndex;
};

struct RenderTree {
    RenderTree();

    void draw();
    
    void insertNode(std::unique_ptr<Renderable> renderable, RenderNode* parent); // use move ctor
    void deleteNode(RenderNode* node);
    std::unique_ptr<RenderNode> root;
};


