#include "tree_manager.hpp"

namespace NewArch {
    thread_local std::vector<RenderTree*> TreeStack::treeStack {};
}