//
// Created by ZZK on 2023/5/25.
//

#pragma once

#include <Toy/Core/key_code.h>
#include <Toy/Core/mouse_code.h>

namespace toy
{
    struct InputController
    {
    public:
        InputController();
        InputController(const InputController &) = delete;
        InputController &operator=(const InputController &) = delete;
        InputController(InputController &&) = delete;
        InputController &operator=(InputController &&) = delete;

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
        enum class InputState : uint16_t
        {
            None = 0b0,
            Pressed = 0b1,
            Released = 0b10,
            Repeat = 0b100
        };
        struct PrivateData
        {
            GLFWwindow* glfw_window = nullptr;
            DirectX::XMFLOAT2 last_cursor_pos = DirectX::XMFLOAT2{ 0.0f, 0.0f };
            std::map<key_code, InputState> key_states = {};
            std::map<mouse_code, InputState> mouse_states = {};
        };
        std::unique_ptr<PrivateData> m_private_data;
    };
}















