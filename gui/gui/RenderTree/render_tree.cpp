//
//  render_tree.cpp
//  gui
//
//  Created by Taanish Reja on 8/21/25.
//

#include "render_tree.hpp"

RenderTree* RenderTree::current = nullptr;

bool RenderNodeComparator::operator()(const std::unique_ptr<RenderNodeBase>& a, const std::unique_ptr<RenderNodeBase>& b) const
{
    if (a->globalZIndex != b->globalZIndex)
        return a->globalZIndex < b->globalZIndex;

    return a->insertionId < b->insertionId;
}


RenderTree::RenderTree():
    root{std::make_unique<RenderNode<RootDrawable>>()}
{
    
    root->globalZIndex = 0;
    root->localZIndex = 0;
    
}

void RenderTree::flatten(RenderNodeBase* node, int parentZ) {
    node->globalZIndex = parentZ + node->localZIndex;
    
    this->drawList.push_back(node);
    
    for (auto& child : node->children)
        flatten(child.get(), node->globalZIndex);
}

void RenderTree::update() {
    root->update();
    
    drawList.clear();
    flatten(root.get(), 0);
    std::sort(drawList.begin(), drawList.end(), [](RenderNodeBase* a, RenderNodeBase* b){
        if (a->globalZIndex != b->globalZIndex)
            return a->globalZIndex < b->globalZIndex;

        return a->insertionId < b->insertionId;
    });
}

void RenderTree::render(MTL::RenderCommandEncoder* encoder) const
{
//    root->render(encoder);
    for (auto node: drawList) {
        node->encode(encoder);
    }
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
