//
// Created by ZHIKANG on 2023/5/21.
//

#pragma once

#include <Toy/Events/event.h>

namespace toy
{
    class window_close_event_c
    {
    public:
        window_close_event_c() = default;

        EVENT_CLASS_TYPE(WindowClose);
    };

    class window_resize_event_c
    {
    public:
        window_resize_event_c(int32_t width, int32_t height)
        : window_width(width), window_height(height)
        {
        }

        EVENT_CLASS_TYPE(WindowResize);

        // The data specific the framebuffer size
        int32_t window_width;
        int32_t window_height;
    };

    class key_event_c
    {
    public:
        key_event_c(const key_code key_code_in)
        : key_code_save(key_code_in)
        {

        }

        key_code key_code_save;
    };

    class key_pressed_event_c : public key_event_c
    {
    public:
        key_pressed_event_c(const key_code key_code_in, bool repeat_in = false)
        : key_event_c(key_code_in), repeat(repeat_in)
        {

        }

        EVENT_CLASS_TYPE(KeyPressed);

        bool repeat;
    };

    class key_released_event_c : public key_event_c
    {
    public:
        key_released_event_c(const key_code key_code_in)
        : key_event_c(key_code_in)
        {

        }

        EVENT_CLASS_TYPE(KeyReleased);
    };

    class key_typed_event_c : public key_event_c
    {
    public:
        key_typed_event_c(const key_code key_code_in)
        : key_event_c(key_code_in)
        {

        }

        EVENT_CLASS_TYPE(KeyTyped)
    };
}
