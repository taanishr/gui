#include "node_builder.hpp"
#include "div.hpp"
#include "tree_manager.hpp"
#include <print>

namespace NewArch {
    NodeBuilder<Div<DivStorage>, DivProcessor<DivStorage, DivUniforms>> div()
    {
        auto& ctx = ContextManager::getContext();
        auto& proc = getDivProcessor(ctx);
        Div elem {ctx};

        auto currTree = TreeStack::getCurrentTree();

        return NodeBuilder(ctx, *currTree, std::move(elem), proc);
    }

    NodeBuilder<Div<DivStorage>, DivProcessor<DivStorage, DivUniforms>> div(Size width, Size height, simd_float4 color)
    {
        auto& ctx = ContextManager::getContext();
        auto& proc = getDivProcessor(ctx);
        Div elem {ctx};
        elem.getDescriptor().color = color;

        auto currTree = TreeStack::getCurrentTree();

        auto builder = NodeBuilder(ctx, *currTree, std::move(elem), proc);
        builder.node->shared.width = width;
        builder.node->shared.height = height;
        return builder;
    }

    NodeBuilder<Text<TextStorage>, TextProcessor<TextStorage, TextUniforms>> text(const std::u32string& text,
                 Size fontSize, simd_float4 color,
                 const std::string& font)
    {
        auto& ctx = ContextManager::getContext();
        auto& proc = getTextProcessor(ctx);
        Text elem{ctx};
        auto& desc = elem.getDescriptor();
        desc.text = text;
        desc.fontSize = fontSize;
        desc.color = color;
        desc.font = font;

        auto currTree = TreeStack::getCurrentTree();

        auto builder = NodeBuilder(ctx, *currTree, std::move(elem), proc);
        builder.node->shared.display = Display::Inline;
        return builder;
    }

    NodeBuilder<Image<ImageStorage>, ImageProcessor<ImageStorage, ImageUniforms>> image(const std::string& path,
                    Size width, Size height)
    {
        auto& ctx = ContextManager::getContext();
        auto& proc = getImageProcessor(ctx);
        Image elem{ctx};
        elem.getDescriptor().path = path;

        auto currTree = TreeStack::getCurrentTree();

        auto builder = NodeBuilder(ctx, *currTree, std::move(elem), proc);
        builder.node->shared.width = width;
        builder.node->shared.height = height;
        return builder;
    }

    NodeBuilder<SVG<SVGStorage>, SVGProcessor<SVGStorage, SVGUniforms>> svg(const std::string& path,
                    Size width, Size height)
    {
        auto& ctx = ContextManager::getContext();
        auto& proc = getSVGProcessor(ctx);
        SVG elem{ctx};
        elem.getDescriptor().path = path;

        auto currTree = TreeStack::getCurrentTree();

        auto builder = NodeBuilder(ctx, *currTree, std::move(elem), proc);
        builder.node->shared.width = width;
        builder.node->shared.height = height;
        return builder;
    }
}
