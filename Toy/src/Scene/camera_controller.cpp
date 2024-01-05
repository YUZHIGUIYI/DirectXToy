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
        ImGuiIO& io = ImGui::GetIO();

        float yaw = 0.0f, pitch = 0.0f;
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
        {
            yaw += io.MouseDelta.x * m_mouse_sensitivity_x;
            pitch += io.MouseDelta.y * m_mouse_sensitivity_y;
        }

        int32_t forward = (
            (DX_INPUT::is_key_pressed(m_glfw_window, key::W) ?  1 : 0) +
            (DX_INPUT::is_key_pressed(m_glfw_window, key::S) ? -1 : 0));
        int32_t strafe = (
            (DX_INPUT::is_key_pressed(m_glfw_window, key::A) ? -1 : 0) +
            (DX_INPUT::is_key_pressed(m_glfw_window, key::D) ?  1 : 0));
        if (forward || strafe)
        {
            // Note: only state "using namespace DirectX" in advance, can you use operator*
            XMVECTOR dir = m_camera->get_look_axis_xm() * static_cast<float>(forward) + m_camera->get_right_axis_xm() * static_cast<float>(strafe);
            XMStoreFloat3(&m_move_direction, dir);
            m_move_velocity = m_move_speed;
            m_drag_timer = m_total_drag_time_to_zero;
            m_velocity_drag = m_move_speed / m_drag_timer;
        } else
        {
            if (m_drag_timer > 0.0f)
            {
                m_drag_timer -= delta_time;
                m_move_velocity -= m_velocity_drag * delta_time;
            } else
            {
                m_move_velocity = 0.0f;
            }
        }

        m_camera->rotate_y(yaw);
        m_camera->pitch(pitch);

        m_camera->translate(m_move_direction, m_move_velocity * delta_time);
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