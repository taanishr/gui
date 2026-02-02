#include "node_builder.hpp"
#include "div.hpp"
#include "tree_manager.hpp"

namespace NewArch {
    NodeBuilder<Div<DivStorage>, DivProcessor<DivStorage, DivUniforms>> div(Size width, Size height, simd_float4 color)
    {
        auto& ctx = ContextManager::getContext();
        auto& proc = getDivProcessor(ctx);
        Div elem {ctx};
        auto& desc = elem.getDescriptor();
        desc.width = width;
        desc.height = height;
        desc.color = color;
        // alter descriptor and fragment via elem.getDescriptor() and elem.getFragment() (mutable references)



        auto currTree = TreeStack::getCurrentTree();

        return NodeBuilder(ctx, *currTree, std::move(elem), proc);
    }

    NodeBuilder<Text<TextStorage>, TextProcessor<TextStorage, TextUniforms>> text(const std::string& text, 
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
        
        return NodeBuilder(ctx, *currTree, std::move(elem), proc);
    }

    NodeBuilder<Image<ImageStorage>, ImageProcessor<ImageStorage, ImageUniforms>> image(const std::string& path,
                    Size width, Size height)
    {
        auto& ctx = ContextManager::getContext();
        auto& proc = getImageProcessor(ctx);
        Image elem{ctx};
        auto& desc = elem.getDescriptor();
        desc.path = path;
        desc.width = width;
        desc.height = height;

        auto currTree = TreeStack::getCurrentTree();

        return NodeBuilder(ctx, *currTree, std::move(elem), proc);
    }
}