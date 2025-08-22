//
//  renderable.cpp
//  gui
//
//  Created by Taanish Reja on 8/21/25.
//

#include "render_tree.hpp"

bool RenderNodeComparator::operator()(const std::unique_ptr<RenderNode>& a, const std::unique_ptr<RenderNode>& b) const
{
    if (a->zIndex != b->zIndex)
        return a->zIndex < b->zIndex;
    
    return a.get() < b.get();
}

RenderNode::RenderNode()
{}

void RenderNode::update() {
    if (renderable) // find a better way to express this contract
        renderable->update();
}


void RenderNode::encode(MTL::RenderCommandEncoder* encoder) {
    if (renderable)
        renderable->encode(encoder);
}

void RenderNode::render(MTL::RenderCommandEncoder* encoder) {
    if (renderable) {
        update();
        encode(encoder);
    }
        
    for (const auto& child: children)
        child->render(encoder);
}

RenderTree::RenderTree():
    root{std::make_unique<RenderNode>()}
{
    root->zIndex = 0;
}

void RenderTree::render(MTL::RenderCommandEncoder* encoder) const {
    root->render(encoder);
}

RenderNode* RenderTree::insertNode(std::unique_ptr<Renderable> renderable, RenderNode *parent)
{
    auto newNode = std::make_unique<RenderNode>();
    newNode->parent = parent;
    newNode->zIndex = parent->zIndex;
    newNode->renderable = std::move(renderable);
    
    auto [it, _] = parent->children.insert(std::move(newNode));
    
    return (*it).get();
}

void RenderTree::deleteNode(RenderNode* node)
{
    std::erase_if(node->parent->children, [node](const std::unique_ptr<RenderNode>& child){
        return node == child.get();
    });
}
