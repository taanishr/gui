#include "node_builder.hpp"
#include "div.hpp"

namespace NewArch {
    DivProcessor<DivStorage, DivUniforms>& getDivProcessor(UIContext& ctx) {
        std::once_flag initFlag;
        static std::optional<DivProcessor<DivStorage, DivUniforms>> div_proc;

        std::call_once(initFlag, [&](){
            div_proc.emplace(ctx);
        });

        return *div_proc;
    }

    ImageProcessor<ImageStorage, ImageUniforms>& getImageProcessor(UIContext& ctx) {
        std::once_flag initFlag;
        static std::optional<ImageProcessor<ImageStorage, ImageUniforms>> img_proc;

        std::call_once(initFlag, [&](){
            img_proc.emplace(ctx);
        });

        return *img_proc;
    }

    TextProcessor<TextStorage, TextUniforms>& getTextProcessor(UIContext& ctx) {
        std::once_flag initFlag;
        static std::optional<TextProcessor<TextStorage, TextUniforms>> txt_proc;

        std::call_once(initFlag, [&](){
            txt_proc.emplace(ctx);
        });

        return *txt_proc;
    }

    

    NodeBuilder div(UIContext& ctx, RenderTree& tree, float width, float height, simd_float4 color)
    {
        auto& proc = getDivProcessor(ctx);
        Div elem {ctx};
        auto& desc = elem.getDescriptor();
        desc.width = width;
        desc.height = height;
        desc.color = color;
        // alter descriptor and fragment via elem.getDescriptor() and elem.getFragment() (mutable references)
        return NodeBuilder(ctx, tree, std::move(elem), proc);
    }

    NodeBuilder text(UIContext& ctx, RenderTree& tree, const std::string& text, 
                 float fontSize, simd_float4 color, 
                 const std::string& font)
    {
        auto& proc = getTextProcessor(ctx);
        Text elem{ctx};
        auto& desc = elem.getDescriptor();
        desc.text = text;
        desc.fontSize = fontSize;
        desc.color = color;
        desc.font = font;
        
        return NodeBuilder(ctx, tree, std::move(elem), proc);
    }

    NodeBuilder image(UIContext& ctx, RenderTree& tree, const std::string& path,
                    float width, float height)
    {
        auto& proc = getImageProcessor(ctx);
        Image elem{ctx};
        auto& desc = elem.getDescriptor();
        desc.path = path;
        desc.width = width;
        desc.height = height;
        
        return NodeBuilder(ctx, tree, std::move(elem), proc);
    }
}