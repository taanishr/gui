#pragma once

#include "element.hpp"
#include "render_tree.hpp"
#include <print>
#include <vector>

namespace tree {
    struct TreeStack {
        static void pushTree(RenderTree* tree) {
            treeStack.push_back(tree);
        }
        
        static void popTree() {
            assert(!treeStack.empty());
            treeStack.pop_back();
        }
        
        static RenderTree* getCurrentTree() {
            assert(!treeStack.empty());
            return treeStack.back();
        }
        
        static std::vector<RenderTree*> treeStack;
    };
}