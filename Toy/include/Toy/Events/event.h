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
        KeyPressed, KeyReleased,
        MouseMoved, MouseScrolled, MouseButtonPressed, MouseButtonReleased,
        Drop,
        KeyTyped
    };

#define EVENT_CLASS_TYPE(type) static EventType get_static_type() { return EventType::type; } \
                                EventType get_event_type() const { return get_static_type(); } \
                                const char* get_name() const { return #type; }

    class WindowCloseEvent;
    class WindowResizeEvent;
    class KeyPressedEvent;
    class KeyReleasedEvent;
    class MouseMovedEvent;
    class MouseScrolledEvent;
    class MouseButtonPressedEvent;
    class MouseButtonReleasedEvent;
    class DropEvent;

    using EngineEventVariant = std::variant<WindowCloseEvent, WindowResizeEvent, KeyPressedEvent, KeyReleasedEvent,
                                MouseMovedEvent, MouseScrolledEvent, MouseButtonPressedEvent, MouseButtonReleasedEvent,
                                DropEvent>;
}
