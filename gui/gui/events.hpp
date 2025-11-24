//
//  events.hpp
//  gui
//
//  Created by Taanish Reja on 8/23/25.
//

#pragma once
#include <variant>

enum class EventType {
    KeyboardDown,
    Click,
};

struct MousePayload {
    float x;
    float y;
};

struct KeyboardPayload {
    char ch;
};

using EventPayload = std::variant<MousePayload, KeyboardPayload>;

struct Event {
    EventType type;
    EventPayload payload;
};

template<EventType E> struct event_payload;
template<> struct event_payload<EventType::KeyboardDown > { using type = KeyboardPayload; };
template<> struct event_payload<EventType::Click> { using type = MousePayload; };
template<EventType E> using event_payload_t = typename event_payload<E>::type;

