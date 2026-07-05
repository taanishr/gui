#include "inspector.hpp"
#include "events.hpp"
#include "new_arch.hpp"
#include "sizing.hpp"
#include <string>

namespace Inspector {
    Inspector::Inspector():
        mouseXText{text(mouseXLabel()).fontSize(style::Size::pt(12.0)).color(simd_float4{1.0,1.0,1.0,1.0})},
        mouseYText{text(mouseYLabel()).fontSize(style::Size::pt(12.0)).color(simd_float4{1.0,1.0,1.0,1.0})},
        visualizerState{
            div()
                .color(simd_float4{0,0,0,0.5})
                .height(style::Size::px(240))
                .width(style::Size::px(240))
                .position(style::Position::Fixed)
                .zIndex(DEBUG_Z_INDEX)
                .padding(style::Size::px(12.0))
                .right(style::Size::px(0))
                .top(style::Size::px(0))
            (
                text(U"Inspector").fontSize(style::Size::pt(24.0)),
                div()
                    .display(style::Display::Flex)
                    .flexDirection(FlexDirection::Col)
                (
                    text(U"Mouse").fontSize(style::Size::pt(12.0)),
                    div().display(style::Display::Flex).flexGap(style::Size::px(12.0))
                    (
                        mouseXText,
                        mouseYText
                    )
                )
            )
        }
    {
    }

    void Inspector::observe(const Event& event) {
        lastEventType = event.type;

        switch (event.type) {
            case EventType::MouseDown:
            case EventType::MouseUp:
            case EventType::MouseMove:
            case EventType::MouseEnter:
            case EventType::MouseLeave:
            case EventType::Click:
                lastMouse = std::get<MousePayload>(event.payload);
                break;
            case EventType::KeyDown:
            case EventType::KeyUp:
                lastKeyboard = std::get<KeyboardPayload>(event.payload);
                break;
            case EventType::ScrollWheel:
                lastScroll = std::get<ScrollPayload>(event.payload);
                break;
            case EventType::Focus:
            case EventType::Blur:
                break;
        }

        mouseXText.text(mouseXLabel());
        mouseYText.text(mouseYLabel());
        visualizerState.markDirty();
    }

    // store state
    // add an element with high z index, controlled by cursor, scroll for sorting by z-index or some other control?
    // 

    simd_float2 Inspector::getMouseState() {
        if (lastMouse.has_value()) {
            return simd_float2{lastMouse->position.x, lastMouse->position.y};
        }

        return simd_float2{0.0,0.0};
    }

    std::u32string Inspector::mouseXLabel() {
        auto [mouseX, mouseY] = getMouseState();
        auto mx = std::format("x: {}", float(mouseX));
        return std::u32string(mx.begin(), mx.end());
    }

    std::u32string Inspector::mouseYLabel() {
        auto [mouseX, mouseY] = getMouseState();
        auto my = std::format("y: {}", float(mouseY));
        return std::u32string(my.begin(), my.end());
    }

    NodeBuilder<Div<DivStorage>, DivProcessor<DivStorage, DivUniforms>> Inspector::visualizer() {
        return this->visualizerState;
    }
};
