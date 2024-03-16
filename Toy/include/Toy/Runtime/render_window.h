//
// Created by ZZK on 2024/3/15.
//

#pragma once

#include <Toy/Core/base.h>
#include <Toy/Events/event.h>
#include <Toy/Events/window_event.h>

namespace toy::runtime
{
    struct RenderWindow
    {
    public:
        explicit RenderWindow(int32_t width = 1600, int32_t height = 900);

        ~RenderWindow();

        RenderWindow(const RenderWindow &) = delete;
        RenderWindow &operator=(const RenderWindow &) = delete;
        RenderWindow(RenderWindow &&) = delete;
        RenderWindow &operator=(RenderWindow &&) = delete;

        void tick();

        [[nodiscard]] GLFWwindow *get_native_window() const;

        [[nodiscard]] bool poll_window_close() const;

        [[nodiscard]] bool poll_framebuffer_resize() const;

        [[nodiscard]] int32_t get_window_width() const;

        [[nodiscard]] int32_t get_window_height() const;

        std::vector<EngineEventVariant> poll_delegate_events();

    private:
        GLFWwindow* m_native_window = nullptr;
        int32_t m_window_width = 1600;
        int32_t m_window_height = 900;
        EngineEventVariant m_window_close_event = NoneEvent{};
        EngineEventVariant m_framebuffer_resize_event = NoneEvent{};
        std::vector<EngineEventVariant> m_delegate_events;
    };
}
