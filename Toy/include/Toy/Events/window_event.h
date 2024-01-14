//
// Created by ZHIKANG on 2023/5/21.
//

#pragma once

#include <Toy/Events/event.h>

namespace toy
{
    class WindowCloseEvent
    {
    public:
        EVENT_CLASS_TYPE(WindowClose);
    };

    class WindowResizeEvent
    {
    public:
        explicit WindowResizeEvent(int32_t width, int32_t height)
        : window_width(width), window_height(height)
        {
        }

        EVENT_CLASS_TYPE(WindowResize);

        // The data specific the framebuffer size
        int32_t window_width;
        int32_t window_height;
    };

    class KeyPressedEvent
    {
    public:
        explicit KeyPressedEvent(const key_code key_code_in, int32_t mods, bool repeat_in = false)
        : key_code_save(key_code_in), mod_keys(mods), repeat(repeat_in)
        {

        }

        EVENT_CLASS_TYPE(KeyPressed);

        key_code key_code_save;
        int32_t mod_keys;
        bool repeat;
    };

    class KeyReleasedEvent
    {
    public:
        explicit KeyReleasedEvent(const key_code key_code_in, int32_t mods)
        : key_code_save(key_code_in), mod_keys(mods)
        {

        }

        EVENT_CLASS_TYPE(KeyReleased);

        key_code key_code_save;
        int32_t mod_keys;
    };

    class KeyTypedEvent
    {
    public:
        explicit KeyTypedEvent(const key_code key_code_in, int32_t mods)
        : key_code_save(key_code_in), mod_keys(mods)
        {

        }

        EVENT_CLASS_TYPE(KeyTyped)

        key_code key_code_save;
        int32_t mod_keys;
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
    };

    class MouseButtonPressedEvent
    {
    public:
        explicit MouseButtonPressedEvent(const mouse_code button, int32_t mods)
        : mouse_button(button), mod_keys(mods)
        {

        }

        EVENT_CLASS_TYPE(MouseButtonPressed)

        mouse_code mouse_button;
        int32_t mod_keys;
    };

    class MouseButtonReleasedEvent
    {
    public:
        explicit MouseButtonReleasedEvent(const mouse_code button, int32_t mods)
        : mouse_button(button), mod_keys(mods)
        {

        }

        EVENT_CLASS_TYPE(MouseButtonReleased)

        mouse_code mouse_button;
        int32_t mod_keys;
    };

    class DropEvent
    {
    public:
        explicit DropEvent(std::string_view filename)
        : drop_filename(filename)
        {

        }

        EVENT_CLASS_TYPE(Drop)

        std::string_view drop_filename;
    };
}





























