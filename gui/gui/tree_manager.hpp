#pragma once

#include "element.hpp"
#include <print>
#include <vector>

namespace NewArch {
    struct TreeStack {
        static void pushTree(RenderTree* tree) {
            treeStack.push_back(tree);
        }
        
        static void popTree() {
            // std::println("popping");
            assert(!treeStack.empty());
            treeStack.pop_back();
        }
        
        static RenderTree* getCurrentTree() {
            assert(!treeStack.empty());
            return treeStack.back();
        }
        
        static thread_local std::vector<RenderTree*> treeStack;
    };
}