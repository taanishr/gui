
#include "context_manager.hpp"

namespace NewArch {
    std::once_flag NewArch::ContextManager::initFlag;
    std::optional<UIContext> NewArch::ContextManager::context = std::nullopt;

    UIContext& ContextManager::initContext(MTL::Device* device, MTK::View* view) {
        std::call_once(ContextManager::initFlag, [&](){
            ContextManager::context.emplace(device, view);
        });

        return *ContextManager::context;
    }

    UIContext& ContextManager::getContext() {
        assert(ContextManager::context.has_value());
        return *ContextManager::context;
    }

 
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
}