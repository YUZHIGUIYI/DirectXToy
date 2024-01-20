//
// Created by ZHIKANG on 2023/5/21.
//

#pragma once

#include <Toy/Core/key_code.h>
#include <Toy/Core/mouse_code.h>

namespace toy
{
    enum class EventType
    {
        None = 0,
        WindowClose = 1, WindowResize = 2,
        KeyTyped,
        MouseMoved, MouseScrolled, MouseButton,
        Drop
    };

#define EVENT_CLASS_TYPE(type) static EventType get_static_type() { return EventType::type; } \
                                EventType get_event_type() const { return get_static_type(); } \
                                const char* get_name() const { return #type; }

    class WindowCloseEvent;
    class WindowResizeEvent;
    class KeyTypedEvent;
    class MouseMovedEvent;
    class MouseScrolledEvent;
    class MouseButtonEvent;
    class DropEvent;

    using EngineEventVariant = std::variant<WindowCloseEvent, WindowResizeEvent, KeyTypedEvent,
                                MouseMovedEvent, MouseScrolledEvent, MouseButtonEvent,
                                DropEvent>;
}
