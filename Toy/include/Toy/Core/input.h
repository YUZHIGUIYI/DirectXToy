//
// Created by ZZK on 2023/5/25.
//

#pragma once

#include <Toy/Core/key_code.h>
#include <Toy/Core/mouse_code.h>

namespace toy
{
    class InputController
    {
    private:
        InputController();

    public:
        InputController(const InputController &) = delete;
        InputController &operator=(const InputController &) = delete;

        // Tick
        void update_state();

        // Register
        void register_event(GLFWwindow *glfw_window);

        [[nodiscard]] bool is_key_pressed(key_code key_in) const;

        // Require Shift, Control or Alt
        [[nodiscard]] bool is_key_pressed_with_mod(key_code key_in, key_code key_mod) const;

        [[nodiscard]] bool is_key_pressed_with_mouse(key_code key_in, mouse_code button) const;

        [[nodiscard]] bool is_mouse_button_pressed(mouse_code button) const;

        DirectX::XMFLOAT2 get_cursor_position();

        static InputController &get();

    private:
        void reset_state();

    private:
        struct PrivateData;
        std::unique_ptr<PrivateData> m_private_data;
    };
}















