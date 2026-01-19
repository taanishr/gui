#include "tree_manager.hpp"

TreeContextGuard::TreeContextGuard(): 
    tree{}
{
    handle = TreeManager::addTree(&tree);
};

TreeContextGuard::~TreeContextGuard()
{
    TreeManager::deleteTree(handle);
};

std::map<TreeHandle, NewArch::RenderTree*> TreeManager::trees {};

TreeHandle TreeManager::currHandle = 0;
TreeHandle TreeManager::nextHandle = 0;

TreeHandle TreeManager::addTree(NewArch::RenderTree* tree) {
    currHandle = nextHandle++;
    trees[currHandle] = tree;

    return currHandle;
}

void TreeManager::deleteTree(TreeHandle handle) {
    if (trees.contains(handle)) {
        trees.erase(handle);
    }
}

NewArch::RenderTree* TreeManager::getTree(TreeHandle handle) {
    if (trees.contains(handle)) {
        return trees[handle];
    }

    return nullptr;
}

TreeHandle TreeManager::getCurrentTreeHandle() {
    return currHandle;
}