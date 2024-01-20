//
// Created by ZZK on 2023/5/25.
//

#include <Toy/Core/input.h>

namespace toy
{
    enum class InputState : uint16_t
    {
        None = 0,
        Pressed = 1,
        Released = 2,
        Repeat = 4
    };

    struct InputStateWrapper
    {
        InputState input_state = InputState::None;
        operator uint16_t() const
        {
            return static_cast<uint16_t>(input_state);
        }
    };

    struct InputController::PrivateData
    {
        GLFWwindow* glfw_window = nullptr;
        DirectX::XMFLOAT2 last_cursor_pos = DirectX::XMFLOAT2{ 0.0f, 0.0f };
        std::map<key_code, uint16_t> key_states = {};
        std::map<mouse_code, uint16_t> mouse_states = {};
    };

    InputController::InputController()
    : m_private_data(std::make_unique<PrivateData>())
    {
        reset_state();
    }

    void InputController::register_event(GLFWwindow *glfw_window)
    {
        m_private_data->glfw_window = glfw_window;
    }

    void InputController::reset_state()
    {
        m_private_data->key_states[key::R] = 0;
        m_private_data->key_states[key::E] = 0;
        m_private_data->key_states[key::Q] = 0;
        m_private_data->key_states[key::W] = 0;
        m_private_data->key_states[key::A] = 0;
        m_private_data->key_states[key::S] = 0;
        m_private_data->key_states[key::D] = 0;
        m_private_data->key_states[key::LeftShift] = 0;
        m_private_data->key_states[key::LeftControl] = 0;
        m_private_data->key_states[key::LeftAlt] = 0;
        m_private_data->key_states[key::RightShift] = 0;
        m_private_data->key_states[key::RightControl] = 0;
        m_private_data->key_states[key::RightAlt] = 0;

        m_private_data->mouse_states[mouse::ButtonLeft] = 0;
        m_private_data->mouse_states[mouse::ButtonRight] = 0;
        m_private_data->mouse_states[mouse::ButtonMiddle] = 0;
    }

    void InputController::update_state()
    {
        // Check pressed or repeat
        bool shift_key = glfwGetKey(m_private_data->glfw_window, key::LeftShift) || glfwGetKey(m_private_data->glfw_window, key::RightShift);
        bool ctrl_key = glfwGetKey(m_private_data->glfw_window, key::LeftControl) || glfwGetKey(m_private_data->glfw_window, key::RightControl);
        bool alt_key = glfwGetKey(m_private_data->glfw_window, key::LeftAlt) || glfwGetKey(m_private_data->glfw_window, key::RightAlt);
        bool w_key = glfwGetKey(m_private_data->glfw_window, key::W) || glfwGetKey(m_private_data->glfw_window, key::Up);
        bool a_key = glfwGetKey(m_private_data->glfw_window, key::A) || glfwGetKey(m_private_data->glfw_window, key::Left);
        bool s_key = glfwGetKey(m_private_data->glfw_window, key::S) || glfwGetKey(m_private_data->glfw_window, key::Down);
        bool d_key = glfwGetKey(m_private_data->glfw_window, key::D) || glfwGetKey(m_private_data->glfw_window, key::Right);
        bool q_key = glfwGetKey(m_private_data->glfw_window, key::Q);
        bool e_key = glfwGetKey(m_private_data->glfw_window, key::E);
        bool r_key = glfwGetKey(m_private_data->glfw_window, key::R);
        bool left_button = glfwGetMouseButton(m_private_data->glfw_window, mouse::ButtonLeft);
        bool right_button = glfwGetMouseButton(m_private_data->glfw_window, mouse::ButtonRight);
        bool middle_button = glfwGetMouseButton(m_private_data->glfw_window, mouse::ButtonMiddle);

        m_private_data->key_states[key::LeftShift] = shift_key ? InputStateWrapper{ InputState::Pressed } : InputStateWrapper{};
        m_private_data->key_states[key::LeftControl] = ctrl_key ? InputStateWrapper{ InputState::Pressed } : InputStateWrapper{};
        m_private_data->key_states[key::LeftAlt] = alt_key ? InputStateWrapper{ InputState::Pressed } : InputStateWrapper{};
        m_private_data->key_states[key::W] = w_key ? InputStateWrapper{ InputState::Pressed } : InputStateWrapper{};
        m_private_data->key_states[key::A] = a_key ? InputStateWrapper{ InputState::Pressed } : InputStateWrapper{};
        m_private_data->key_states[key::S] = s_key ? InputStateWrapper{ InputState::Pressed } : InputStateWrapper{};
        m_private_data->key_states[key::D] = d_key ? InputStateWrapper{ InputState::Pressed } : InputStateWrapper{};
        m_private_data->key_states[key::Q] = q_key ? InputStateWrapper{ InputState::Pressed } : InputStateWrapper{};
        m_private_data->key_states[key::E] = e_key ? InputStateWrapper{ InputState::Pressed } : InputStateWrapper{};
        m_private_data->key_states[key::R] = r_key ? InputStateWrapper{ InputState::Pressed } : InputStateWrapper{};

        m_private_data->mouse_states[mouse::ButtonLeft] = left_button ? InputStateWrapper{ InputState::Pressed } : InputStateWrapper{};
        m_private_data->mouse_states[mouse::ButtonRight] = right_button ? InputStateWrapper{ InputState::Pressed } : InputStateWrapper{};
        m_private_data->mouse_states[mouse::ButtonMiddle] = middle_button ? InputStateWrapper{ InputState::Pressed } : InputStateWrapper{};
    }

    bool InputController::is_key_pressed(toy::key_code key_in) const
    {
        if (m_private_data->key_states.contains(key_in))
        {
            return m_private_data->key_states[key_in] & InputStateWrapper{ InputState::Pressed };
        } else
        {
            auto state = glfwGetKey(m_private_data->glfw_window, key_in);
            return state == GLFW_PRESS || state == GLFW_REPEAT;
        }
    }

    bool InputController::is_key_pressed_with_mod(toy::key_code key_in, toy::key_code key_mod) const
    {
        if (!m_private_data->key_states.contains(key_in) || !m_private_data->key_states.contains(key_mod))
        {
            // Pressed or repeat is OK
            bool first_state = glfwGetKey(m_private_data->glfw_window, key_in);
            bool second_state = glfwGetKey(m_private_data->glfw_window, key_mod);
            return first_state && second_state;
        } else
        {
            return m_private_data->key_states[key_in] & m_private_data->key_states[key_mod];
        }
    }

    bool InputController::is_key_pressed_with_mouse(toy::key_code key_in, toy::mouse_code button) const
    {
        if (!m_private_data->key_states.contains(key_in) || !m_private_data->mouse_states.contains(button))
        {
            // Pressed or repeat is OK
            bool key_state = glfwGetKey(m_private_data->glfw_window, key_in);
            bool mouse_state = glfwGetMouseButton(m_private_data->glfw_window, button);
            return key_state && mouse_state;
        } else
        {
            return m_private_data->key_states[key_in] & m_private_data->mouse_states[button];
        }
    }

    bool InputController::is_mouse_button_pressed(toy::mouse_code button) const
    {
        return m_private_data->mouse_states[button] & InputStateWrapper{ InputState::Pressed };
    }

    DirectX::XMFLOAT2 InputController::get_cursor_position()
    {
        double x_pos = 0.0, y_pos = 0.0;
        glfwGetCursorPos(m_private_data->glfw_window, &x_pos, &y_pos);
        m_private_data->last_cursor_pos = DirectX::XMFLOAT2{ static_cast<float>(x_pos), static_cast<float>(y_pos) };
        return m_private_data->last_cursor_pos;
    }

    InputController &InputController::get()
    {
        static InputController input_controller = {};
        return input_controller;
    }
}


























