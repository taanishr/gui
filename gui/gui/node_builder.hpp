#include "element.hpp"
#include "div.hpp"
#include "image.hpp"
#include "text.hpp"
#include "new_arch.hpp"
#include <memory>
#include <mutex>

namespace NewArch {
    DivProcessor<DivStorage, DivUniforms>& getDivProcessor(UIContext& ctx);
    ImageProcessor<ImageStorage, ImageUniforms>& getImageProcessor(UIContext& ctx);
    TextProcessor<TextStorage, TextUniforms>& getTextProcessor(UIContext& ctx);

    struct NodeBuilder {
        RenderTree& renderTree;
        UIContext& ctx;
        TreeNode* node;

        template <ElementType E>
        NodeBuilder(UIContext& ctx, RenderTree& tree, E elem): 
            ctx{ctx},
            renderTree{tree}
        {
            auto root = renderTree.getRoot();
            auto& proc = getDivProcessor(ctx);

            auto n = std::make_unique<TreeNode>(
                ctx, std::move(elem), proc
            );
            
            node = n.get();
            root->attach_child(std::move(n));
        }

        template <typename... Children>
        NodeBuilder& operator()(Children&... args) {
           (node->attach_child(args->node), ...);
            return *this;
        }
    };

    NodeBuilder div(UIContext& ctx, RenderTree& tree, float width, float height, simd_float4 color);
    NodeBuilder image();
    NodeBuilder text();

}