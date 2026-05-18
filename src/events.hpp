//
//  events.hpp
//  gui
//
//  Created by Taanish Reja on 8/23/25.
//

#pragma once
#include <variant>
#include "metal_imports.hpp"
#include "new_arch.hpp"

// enum class EventType {
//     KeyboardDown,
//     Click,
// };

// struct MousePayload {
//     float x;
//     float y;
// };

// struct KeyboardPayload {
//     char ch;
// };

// using EventPayload = std::variant<MousePayload, KeyboardPayload>;

// struct Event {
//     EventType type;
//     EventPayload payload;
// };

// template<EventType E> struct event_payload;
// template<> struct event_payload<EventType::KeyboardDown > { using type = KeyboardPayload; };
// template<> struct event_payload<EventType::Click> { using type = MousePayload; };
// template<EventType E> using event_payload_t = typename event_payload<E>::type;

namespace runtime {

enum class EventType {
    MouseDown,
    MouseUp,
    MouseMove,
    MouseEnter,
    MouseLeave,
    Click,
    KeyDown,
    KeyUp,
    Focus,
    Blur,
    ScrollWheel
};

// struct Event {
//     EventType type;
    
//     // Mouse events
//     simd_float2 mousePosition{0, 0};
//     int mouseButton = 0;  // 0 = left, 1 = right, 2 = middle
    
//     // Keyboard events
//     int keyCode = 0;
//     bool shiftPressed = false;
//     bool ctrlPressed = false;
//     bool altPressed = false;
    
//     // Propagation control
//     bool propagationStopped = false;
    
//     void stopPropagation() { propagationStopped = true; }
// };

enum class MouseButton {
    None,
    Left,
    Middle,
    Right
};

struct Modifiers {
    bool shift = false;
    bool ctrl = false;
    bool alt = false;
    bool meta = false;
};

struct MousePayload {
    simd_float2 position;
    MouseButton button;
    Modifiers modifiers;
};

struct KeyboardPayload {
    int keyCode;
    Modifiers modifiers;
};

struct ScrollPayload {
    simd_float2 position;
    float dx;
    float dy;
    Modifiers modifiers;
};

struct FocusPayload {};

using EventPayload = std::variant<MousePayload, KeyboardPayload, ScrollPayload, FocusPayload>;

template<EventType E> struct event_payload;
template<> struct event_payload<EventType::MouseDown> { using type = MousePayload; };
template<> struct event_payload<EventType::MouseUp> { using type = MousePayload; };
template<> struct event_payload<EventType::MouseMove> { using type = MousePayload; };
template<> struct event_payload<EventType::MouseEnter> { using type = MousePayload; };
template<> struct event_payload<EventType::MouseLeave> { using type = MousePayload; };
template<> struct event_payload<EventType::Click> { using type = MousePayload; };
template<> struct event_payload<EventType::KeyDown> { using type = KeyboardPayload; };
template<> struct event_payload<EventType::KeyUp> { using type = KeyboardPayload; };
template<> struct event_payload<EventType::Focus> { using type = FocusPayload; };
template<> struct event_payload<EventType::Blur> { using type = FocusPayload; };
template<> struct event_payload<EventType::ScrollWheel> { using type = ScrollPayload; };
template<EventType E> using event_payload_t = typename event_payload<E>::type;

struct Event {
    EventType type;
    EventPayload payload;
    bool propagationStopped = false;
    
    void stopPropagation() { propagationStopped = true; }
    
    template<EventType E>
    auto& get() {
        return std::get<event_payload_t<E>>(payload);
    }
    
    template<EventType E>
    const auto& get() const {
        return std::get<event_payload_t<E>>(payload);
    }
};

template <typename U>
struct HitTestContext {
    const layout::Finalized<U>& finalized;
    const layout::LayoutResult& layout;
};

}
