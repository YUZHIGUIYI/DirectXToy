//
// Created by ZHIKANG on 2023/6/3.
//

#pragma once

#include <Toy/Scene/camera.h>

namespace toy
{
    class CameraController
    {
    public:
        CameraController() = default;

        CameraController(const CameraController &) = delete;
        CameraController& operator=(const CameraController &) = delete;

        virtual ~CameraController() = default;
        virtual void update(float delta_time) = 0;
    };

    class FirstPersonCameraController final : public CameraController
    {
    public:
        FirstPersonCameraController() = default;
        ~FirstPersonCameraController() override = default;

        void update(float delta_time) override;

        void init(FirstPersonCamera* camera, GLFWwindow *glfw_window);
        void set_mouse_sensitivity(float x, float y);
        void set_move_speed(float speed);

    private:
        FirstPersonCamera* m_camera = nullptr;
        GLFWwindow* m_glfw_window = nullptr;

        float m_move_speed = 5.0f;
        float m_mouse_sensitivity_x = 0.005f;
        float m_mouse_sensitivity_y = 0.005f;

        float m_current_yaw = 0.0f;
        float m_current_pitch = 0.0f;

        DirectX::XMFLOAT3 m_move_direction = {};
        float m_move_velocity = 0.0f;
        float m_velocity_drag = 0.0f;
        float m_total_drag_time_to_zero = 0.25f;
        float m_drag_timer = 0.0f;
    };
}










