//
//  render_tree.cpp
//  gui
//
//  Created by Taanish Reja on 8/21/25.
//

#include "render_tree.hpp"

bool RenderNodeComparator::operator()(const std::unique_ptr<RenderNode>& a, const std::unique_ptr<RenderNode>& b) const
{
    if (a->zIndex != b->zIndex)
        return a->zIndex < b->zIndex;
    
    return a->insertionId < b->insertionId;
}

RenderNode::RenderNode()
{}

void RenderNode::update()
{
    if (drawable)
        drawable->update();
    
    std::sort(children.begin(), children.end(), RenderNodeComparator{});
    for (const auto& child: children)
        child->update();
}


void RenderNode::encode(MTL::RenderCommandEncoder* encoder) const
{
    if (drawable)
        drawable->encode(encoder);
}

void RenderNode::render(MTL::RenderCommandEncoder* encoder) const
{
    if (drawable) {
        encode(encoder);
    }
        
    for (const auto& child: children)
        child->render(encoder);
}

void RenderNode::changeZIndex(int zIndex) {
    this->zIndex = this->parent->zIndex + zIndex;
}

void RenderNode::dispatch(const Event& event) {
    auto it = handlers.find(event.type);
    
    if (it != handlers.end())
        for (auto& h : it->second) h(*this, event);
    
    for (auto& child : children) child->dispatch(event);
}

RenderTree::RenderTree():
    root{std::make_unique<RenderNode>()}
{
    root->zIndex = 0;
}


void RenderTree::update() {
    root->update();
}

void RenderTree::render(MTL::RenderCommandEncoder* encoder) const
{
    root->render(encoder);
}

RenderNode* RenderTree::insertNode(std::unique_ptr<Drawable> drawable, RenderNode *parent)
{
    auto newNode = std::make_unique<RenderNode>();
    newNode->parent = parent;
    newNode->zIndex = parent->zIndex;
    newNode->drawable = std::move(drawable);
    newNode->insertionId = nextInsertionId;
    
    parent->children.push_back(std::move(newNode));
    auto& back = parent->children.back();
    
    nodeMap[nextInsertionId] = back.get();
    
    ++nodes;
    ++nextInsertionId;

    
    return back.get();
}

void RenderTree::deleteNode(RenderNode* node)
{
    std::erase_if(node->parent->children, [node](const std::unique_ptr<RenderNode>& child){
        return node == child.get();
    });
    
    nodeMap.erase(node->insertionId);
    
    --nodes;
}

void RenderTree::reparent(RenderNode* node, RenderNode* newParent)
{
    auto curr = node->parent->children.begin();
    auto end = node->parent->children.end();
    for (; curr != end; ++curr) {
        if ((*curr).get() == node) {
            newParent->children.push_back(std::move(*curr));
            node->parent->children.erase(curr);
            node->parent = newParent;
            break;
        }
    }
}


void RenderTree::dispatch(const Event& event)
{
    root->dispatch(event);
}
