#pragma once

#include "new_arch.hpp"
#include "div.hpp"
#include "image.hpp"
#include "text.hpp"
#include <mutex>

namespace NewArch {
    struct ContextManager {
        static UIContext& initContext(MTL::Device* device, MTK::View* view);
        static UIContext& getContext();

        static std::once_flag initFlag;
        static std::optional<UIContext> context;
    };

    DivProcessor<DivStorage, DivUniforms>& getDivProcessor(UIContext& ctx);
    ImageProcessor<ImageStorage, ImageUniforms>& getImageProcessor(UIContext& ctx);
    TextProcessor<TextStorage, TextUniforms>& getTextProcessor(UIContext& ctx);
}