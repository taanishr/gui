#include "node_builder.hpp"
#include "div.hpp"

namespace NewArch {
    DivProcessor<DivStorage, DivUniforms>& getDivProcessor(UIContext& ctx) {
        std::once_flag initFlag;
        static std::optional<DivProcessor<DivStorage, DivUniforms>> proc;

        std::call_once(initFlag, [&](){
            proc.emplace(ctx);
        });

        return *proc;
    }

    ImageProcessor<ImageStorage, ImageUniforms>& getImageProcessor(UIContext& ctx) {
        std::once_flag initFlag;
        static std::optional<ImageProcessor<ImageStorage, ImageUniforms>> proc;

        std::call_once(initFlag, [&](){
            proc.emplace(ctx);
        });

        return *proc;
    }

    TextProcessor<TextStorage, TextUniforms>& getTextProcessor(UIContext& ctx) {
        std::once_flag initFlag;
        static std::optional<TextProcessor<TextStorage, TextUniforms>> proc;

        std::call_once(initFlag, [&](){
            proc.emplace(ctx);
        });

        return *proc;
    }


    NodeBuilder div(UIContext& ctx, RenderTree& tree, float width, float height, simd_float4 color)
    {
        auto& proc = getDivProcessor(ctx);
        // Element<Div<DivStorage>, DivProcessor<DivStorage, DivUniforms>, DivStorage, DivDescriptor, DivUniforms> elem {ctx, {ctx}, proc};
        // auto& rawElem = elem.element;
        // auto& desc = rawElem.getDescriptor();
        // desc.width = width;
        // desc.height = height;
        // desc.color = color;
        Div elem {ctx};
        // alter descriptor and fragment via elem.getDescriptor() and elem.getFragment() (mutable references)
        return NodeBuilder(ctx, tree, std::move(elem));
    }
}