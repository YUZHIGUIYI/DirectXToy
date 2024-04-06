//
// Created by ZHIKANG on 2023/5/21.
//

#pragma once

#include <Toy/Events/event.h>

namespace toy
{
    class NoneEvent
    {
    public:
        explicit NoneEvent(EventPriority priority = EventPriority::Sixth)
        : event_priority(priority)
        {

        }

        EVENT_CLASS_TYPE(None);

        EventPriority event_priority;
    };

    class WindowCloseEvent
    {
    public:
        explicit WindowCloseEvent(EventPriority priority = EventPriority::First)
        : event_priority(priority)
        {

        }

        EVENT_CLASS_TYPE(WindowClose);

        EventPriority event_priority;
    };

    class WindowResizeEvent
    {
    public:
        explicit WindowResizeEvent(int32_t width, int32_t height, EventPriority priority = EventPriority::Second)
        : window_width(width), window_height(height), event_priority(priority)
        {

        }

        EVENT_CLASS_TYPE(WindowResize);

        // The data specific the framebuffer size
        int32_t window_width;
        int32_t window_height;
        EventPriority event_priority;
    };

    class DockResizeEvent
    {
    public:
        explicit DockResizeEvent(int32_t width, int32_t height, EventPriority priority = EventPriority::Third)
        : dock_width(width), dock_height(height), event_priority(priority)
        {

        }

        EVENT_CLASS_TYPE(DockResize);

        // The data specific the dock ui size
        int32_t dock_width;
        int32_t dock_height;
        EventPriority event_priority;
    };

    class KeyTypedEvent
    {
    public:
        explicit KeyTypedEvent(const key_code key_code_in, int32_t mods, bool is_pressed, bool is_repeat = false)
        : key_code_save(key_code_in), mod_keys(mods), pressed(is_pressed), repeat(is_repeat)
        {

        }

        EVENT_CLASS_TYPE(KeyTyped);

        key_code key_code_save;
        int32_t mod_keys;
        EventPriority event_priority = EventPriority::Fifth;
        bool pressed;
        bool repeat;
    };

    class MouseMovedEvent
    {
    public:
        explicit MouseMovedEvent(const float x, const float y)
        : mouse_x_pos(x), mouse_y_pos(y)
        {

        }

        EVENT_CLASS_TYPE(MouseMoved)

        float mouse_x_pos;
        float mouse_y_pos;
        EventPriority event_priority = EventPriority::Fifth;
    };

    class MouseScrolledEvent
    {
    public:
        explicit MouseScrolledEvent(const float x_offset, const float y_offset)
        : mouse_x_offset(x_offset), mouse_y_offset(y_offset)
        {

        }

        EVENT_CLASS_TYPE(MouseScrolled)

        float mouse_x_offset;
        float mouse_y_offset;
        EventPriority event_priority = EventPriority::Fifth;
    };

    class MouseButtonEvent
    {
    public:
        explicit MouseButtonEvent(const mouse_code button, int32_t mods)
        : mouse_button(button), mod_keys(mods)
        {

        }

        EVENT_CLASS_TYPE(MouseButton)

        mouse_code mouse_button;
        int32_t mod_keys;
        EventPriority event_priority = EventPriority::Fifth;
    };

    class DropEvent
    {
    public:
        explicit DropEvent(std::string_view filename, EventPriority priority = EventPriority::Fourth)
        : drop_filename(filename), event_priority(priority)
        {

        }

        EVENT_CLASS_TYPE(Drop)

        std::string drop_filename;
        EventPriority event_priority;
    };
}





























