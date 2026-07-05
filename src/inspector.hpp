
#pragma once

#include "div.hpp"
#include "node_builder.hpp"
#include "render_tree.hpp"
#include "sizing.hpp"
#include <cstdint>
#include <limits>
#include <optional>
#include <simd/vector_types.h>
#include <string>

#ifndef GUI_ENABLE_INSPECTOR
#define GUI_ENABLE_INSPECTOR 0
#endif

inline constexpr bool GUI_INSPECTOR_ENABLED = GUI_ENABLE_INSPECTOR;
constexpr uint64_t DEBUG_Z_INDEX = std::numeric_limits<uint64_t>::max();

namespace Inspector {
    using namespace elements;
    using namespace runtime;

    struct Inspector {
        Inspector();
        void observe(const Event& event);

        simd_float2 getMouseState();
        std::u32string mouseXLabel();
        std::u32string mouseYLabel();
        NodeBuilder<Div<DivStorage>, DivProcessor<DivStorage, DivUniforms>> visualizer();

        std::optional<EventType> lastEventType;
        std::optional<MousePayload> lastMouse;
        std::optional<KeyboardPayload> lastKeyboard;
        std::optional<ScrollPayload> lastScroll;
        NodeBuilder<Text<TextStorage>, TextProcessor<TextStorage, TextUniforms>> mouseXText;
        NodeBuilder<Text<TextStorage>, TextProcessor<TextStorage, TextUniforms>> mouseYText;
        NodeBuilder<Div<DivStorage>, DivProcessor<DivStorage, DivUniforms>> visualizerState;
    };
};
