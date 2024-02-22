//
// Created by ZHIKANG on 2023/6/3.
//

#include <Toy/Scene/camera_controller.h>
#include <Toy/Core/input.h>

namespace toy
{
    void FirstPersonCameraController::init(toy::FirstPersonCamera *camera, GLFWwindow *glfw_window)
    {
        m_camera = camera;
        m_glfw_window = glfw_window;
    }

    void FirstPersonCameraController::update(float delta_time)
    {
        using namespace DirectX;
        auto&& input_controller = InputController::get();
        ImGuiIO& io = ImGui::GetIO();

        static constexpr float multiplier = 5.0f;
        if (input_controller.is_mouse_button_pressed(mouse::ButtonMiddle))
        {
            float delta_move_x = io.MouseDelta.x;
            float delta_move_y = io.MouseDelta.y;
            if (input_controller.is_key_pressed(key::LeftShift))
            {
                delta_move_x *= multiplier;
                delta_move_y *= multiplier;
            }
            if (delta_move_x != 0.0f)
            {
                m_camera->move_local(DirectX::XMFLOAT3{ -1.0f * delta_move_x * m_mouse_sensitivity_x, 0.0f, 0.0f });
            }
            if (delta_move_y != 0.0f)
            {
                m_camera->move_local(DirectX::XMFLOAT3{ 0.0f, delta_move_y * m_mouse_sensitivity_y, 0.0f });
            }
        }

        if (input_controller.is_mouse_button_pressed(mouse::ButtonRight))
        {
            if (input_controller.is_key_pressed(key::W))
            {
                m_camera->move_local(DirectX::XMFLOAT3{ 0.0f, 0.0f, m_move_speed * delta_time });
            }
            if (input_controller.is_key_pressed(key::S))
            {
                m_camera->move_local(DirectX::XMFLOAT3{ 0.0f, 0.0f, -m_move_speed * delta_time });
            }
            if (input_controller.is_key_pressed(key::A))
            {
                m_camera->move_local(DirectX::XMFLOAT3{ -m_move_speed * delta_time, 0.0f, 0.0f });
            }
            if (input_controller.is_key_pressed(key::D))
            {
                m_camera->move_local(DirectX::XMFLOAT3{ m_move_speed * delta_time, 0.0f, 0.0f });
            }

            float yaw = io.MouseDelta.x * m_mouse_sensitivity_x;
            float pitch = io.MouseDelta.y * m_mouse_sensitivity_y;
            m_camera->rotate_y(yaw);
            m_camera->pitch(pitch);
        }
    }

    void FirstPersonCameraController::set_mouse_sensitivity(float x, float y)
    {
        m_mouse_sensitivity_x = x;
        m_mouse_sensitivity_y = y;
    }

    void FirstPersonCameraController::set_move_speed(float speed)
    {
        m_move_speed = speed;
    }
}