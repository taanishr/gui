#pragma once

#include "new_arch.hpp"
#include "div.hpp"
#include "image.hpp"
#include "svg.hpp"
#include "text.hpp"
#include <mutex>

namespace runtime {
    using elements::DivProcessor;
    using elements::DivStorage;
    using elements::DivUniforms;
    using elements::ImageProcessor;
    using elements::ImageStorage;
    using elements::ImageUniforms;
    using elements::SVGProcessor;
    using elements::SVGStorage;
    using elements::SVGUniforms;
    using elements::TextProcessor;
    using elements::TextStorage;
    using elements::TextUniforms;

    struct ContextManager {
        static UIContext& initContext(MTL::Device* device, MTK::View* view);
        static UIContext& getContext();

        static std::once_flag initFlag;
        static std::optional<UIContext> context;
    };

    DivProcessor<DivStorage, DivUniforms>& getDivProcessor(UIContext& ctx);
    ImageProcessor<ImageStorage, ImageUniforms>& getImageProcessor(UIContext& ctx);
    TextProcessor<TextStorage, TextUniforms>& getTextProcessor(UIContext& ctx);
    SVGProcessor<SVGStorage, SVGUniforms>& getSVGProcessor(UIContext& ctx);
}
