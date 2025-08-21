//
//  renderable.cpp
//  gui
//
//  Created by Taanish Reja on 8/21/25.
//

#include "render_tree.hpp"

RenderNode::RenderNode()
{}

void RenderNode::update() {
    if (renderable) // find a better way to express this contract
        renderable->update();
}

void RenderNode::render(MTL::RenderCommandEncoder* encoder) {
    if (renderable)
        renderable->encode(encoder);
}

RenderTree::RenderTree():
    root{std::make_unique<RenderNode>()}
{
    root->zIndex = 0;
}

void RenderTree::insertNode(std::unique_ptr<Renderable> renderable, RenderNode *parent)
{
    auto newNode = std::make_unique<RenderNode>();
    newNode->parent = parent;
    newNode->zIndex = parent->zIndex;
    newNode->renderable = std::move(renderable);
    
    parent->children.push_back(std::move(newNode));
}

void RenderTree::deleteNode(RenderNode* node)
{
    
    // rough sketch for deletion
    if (node->parent) {
        auto curr = node->parent->children.begin();
        auto end = node->parent->children.end();
        for (; curr != end; ++curr) {
            if (curr->get() == node) {
                node->parent->children.erase(curr);
                break;
            }
        }
    }
    
    node->children.clear();
}


void RenderTree::draw()
{
    // TBD
}
