//
//  render_tree.cpp
//  gui
//
//  Created by Taanish Reja on 8/21/25.
//

#include "render_tree.hpp"
bool RenderNodeComparator::operator()(const std::unique_ptr<RenderNodeBase>& a, const std::unique_ptr<RenderNodeBase>& b) const
{
    if (a->zIndex != b->zIndex)
        return a->zIndex < b->zIndex;

    return a->insertionId < b->insertionId;
}


RenderTree::RenderTree():
    root{std::make_unique<RenderNode<RootDrawable>>()}
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

void RenderTree::deleteNode(RenderNodeBase* node)
{
    std::erase_if(node->parent->children, [node](const std::unique_ptr<RenderNodeBase>& child){
        return node == child.get();
    });

    nodeMap.erase(node->insertionId);

    --nodes;
}

void RenderTree::reparent(RenderNodeBase* node, RenderNodeBase* newParent)
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

