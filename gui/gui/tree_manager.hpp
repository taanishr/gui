#include "element.hpp"
#include <cstdint>
#include <map>

using TreeHandle = uint64_t;

struct TreeContextGuard {
    TreeContextGuard(); // reg tree

    ~TreeContextGuard(); // dereg tree

    TreeHandle handle;
    NewArch::RenderTree tree;
};

struct TreeManager {
    TreeManager() = delete;

    static TreeHandle addTree(NewArch::RenderTree* tree);
    static void deleteTree(TreeHandle handle);

    static NewArch::RenderTree* getTree(TreeHandle handle);
    static TreeHandle getCurrentTreeHandle();

    static std::map<TreeHandle, NewArch::RenderTree*> trees;

    static TreeHandle currHandle;
    static TreeHandle nextHandle;
};