
#pragma once

#include "div.hpp"
#include "node_builder.hpp"
#include "render_tree.hpp"
#include "sizing.hpp"
#include <cstddef>
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
        std::string nodeIdLabel();
        std::string nodeTypeLabel();
        std::string hitStackLabel();
        std::string nodeStyleLabel();
        std::string nodePositionLabel();
        std::string nodeWidthLabel();
        std::string nodeHeightLabel();
        std::string nodeLocalBoxLabel();
        std::string nodeScrollLabel();
        std::string nodeZIndexLabel();
        std::string nodePathLabel();
        std::string mouseXLabel();
        std::string mouseYLabel();
        NodeBuilder<Div<DivStorage>, DivProcessor<DivStorage, DivUniforms>> visualizer();

        std::optional<EventType> lastEventType;
        std::optional<MousePayload> lastMouse;
        std::optional<KeyboardPayload> lastKeyboard;
        std::optional<ScrollPayload> lastScroll;
        NodeBuilder<Text<TextStorage>, TextProcessor<TextStorage, TextUniforms>> mouseXText;
        NodeBuilder<Text<TextStorage>, TextProcessor<TextStorage, TextUniforms>> mouseYText;
        NodeBuilder<Text<TextStorage>, TextProcessor<TextStorage, TextUniforms>> nodeIDText;
        NodeBuilder<Text<TextStorage>, TextProcessor<TextStorage, TextUniforms>> nodeTypeText;
        NodeBuilder<Text<TextStorage>, TextProcessor<TextStorage, TextUniforms>> hitStackText;
        NodeBuilder<Text<TextStorage>, TextProcessor<TextStorage, TextUniforms>> nodeStyleText;
        NodeBuilder<Text<TextStorage>, TextProcessor<TextStorage, TextUniforms>> nodePositionText;
        NodeBuilder<Text<TextStorage>, TextProcessor<TextStorage, TextUniforms>> nodeWText;
        NodeBuilder<Text<TextStorage>, TextProcessor<TextStorage, TextUniforms>> nodeHText;
        NodeBuilder<Text<TextStorage>, TextProcessor<TextStorage, TextUniforms>> nodeLocalBoxText;
        NodeBuilder<Text<TextStorage>, TextProcessor<TextStorage, TextUniforms>> nodeScrollText;
        NodeBuilder<Text<TextStorage>, TextProcessor<TextStorage, TextUniforms>> nodeZIndexText;
        NodeBuilder<Text<TextStorage>, TextProcessor<TextStorage, TextUniforms>> nodePathText;
        NodeBuilder<Div<DivStorage>, DivProcessor<DivStorage, DivUniforms>> visualizerState;
        uint64_t inspectorNodeID;
        std::size_t selectedHitIndex = 0;
        std::size_t hitNodeCount = 0;
        std::optional<uint64_t> topNodeID;
        std::optional<uint64_t> htNodeID;
        std::optional<std::string> htNodeElementType;
        std::optional<std::string> htNodeDisplay;
        std::optional<std::string> htNodePosition;
        std::optional<std::string> htNodeOverflow;
        std::optional<float> htNodeX;
        std::optional<float> htNodeY;
        std::optional<float> htNodeW;
        std::optional<float> htNodeH;
        std::optional<float> htNodeLocalX;
        std::optional<float> htNodeLocalY;
        std::optional<float> htNodeLocalW;
        std::optional<float> htNodeLocalH;
        std::optional<float> htNodeScrollX;
        std::optional<float> htNodeScrollY;
        std::optional<uint64_t> htNodeZIndex;
        std::optional<std::string> htNodePath;
    };
};
