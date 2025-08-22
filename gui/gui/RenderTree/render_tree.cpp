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
    if (renderable)
        renderable->update();
    
    for (const auto& child: children)
        child->update();
}


void RenderNode::encode(MTL::RenderCommandEncoder* encoder) const{
    if (renderable)
        renderable->encode(encoder);
}

void RenderNode::render(MTL::RenderCommandEncoder* encoder) const {
    if (renderable) {
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


void RenderTree::update() {
    root->update();
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
    
    parent->children.push_back(std::move(newNode));
    auto& back = parent->children.back();
    
    std::sort(parent->children.begin(), parent->children.end(), RenderNodeComparator{});
    
    return back.get();
}

void RenderTree::deleteNode(RenderNode* node)
{
    std::erase_if(node->parent->children, [node](const std::unique_ptr<RenderNode>& child){
        return node == child.get();
    });
}
