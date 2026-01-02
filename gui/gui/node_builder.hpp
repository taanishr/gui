#pragma once

#include "element.hpp"
#include "div.hpp"
#include "image.hpp"
#include "text.hpp"
#include "new_arch.hpp"
#include <memory>
#include <mutex>
#include <print>

namespace NewArch {
    DivProcessor<DivStorage, DivUniforms>& getDivProcessor(UIContext& ctx);
    ImageProcessor<ImageStorage, ImageUniforms>& getImageProcessor(UIContext& ctx);
    TextProcessor<TextStorage, TextUniforms>& getTextProcessor(UIContext& ctx);

    struct NodeBuilder {
        RenderTree& renderTree;
        UIContext& ctx;
        TreeNode* node;

        template <ElementType E, typename P>
        NodeBuilder(UIContext& ctx, RenderTree& tree, E elem, P& proc): 
            ctx{ctx},
            renderTree{tree}
        {
            auto root = renderTree.getRoot();

            auto n = std::make_unique<TreeNode>(
                ctx, std::move(elem), proc
            );
            
            std::println("root ptr: {}", reinterpret_cast<void*>(root));
            
            this->node = n.get();
            
            root->attach_child(std::move(n));
        }

        template <typename... Children>
        NodeBuilder& operator()(Children&... args) {
           (node->attach_child(args->node), ...);
            return *this;
        }
    };

    NodeBuilder div(UIContext& ctx, RenderTree& tree, float width, float height, simd_float4 color);
    NodeBuilder text(UIContext& ctx, RenderTree& tree, const std::string& text, 
                 float fontSize = 24.0f, simd_float4 color = {1, 1, 1, 1}, 
                 const std::string& font = "/System/Library/Fonts/Supplemental/Arial.ttf");
    NodeBuilder image(UIContext& ctx, RenderTree& tree, const std::string& path,
                    float width = 0.0f, float height = 0.0f);
}