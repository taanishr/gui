//
//  events.hpp
//  gui
//
//  Created by Taanish Reja on 8/23/25.
//

#pragma once

enum class EventType {
    KeyboardDown,
    Click,
};

struct Event {
    EventType type;
    std::any payload;
};

struct MousePayload {
    float x;
    float y;
};

struct KeyboardPayload {
    char ch;
};

using EventHandler = std::function<void(Event&)>;

