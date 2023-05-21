//
// Created by ZHIKANG on 2023/5/21.
//

#pragma once

#include <Toy/Core/key_code.h>
#include <Toy/Core/mouse_code.h>

namespace toy
{
    enum class event_type_e
    {
        None = 0,
        WindowClose = 1, WindowResize = 2, WindowFocus, WindowLostFocus, WindowMoved,
        KeyPressed, KeyReleased, KeyTyped,
        MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
    };

#define EVENT_CLASS_TYPE(type) static event_type_e get_static_type() { return event_type_e::type; } \
                                event_type_e get_event_type() const { return get_static_type(); } \
                                const char* get_name() const { return #type; }

    class window_close_event_c;
    class window_resize_event_c;
    class key_event_c;

    using event_t = std::variant<window_close_event_c, window_resize_event_c, key_event_c>;
}
