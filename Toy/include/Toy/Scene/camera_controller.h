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

        void init(FirstPersonCamera* camera, GLFWwindow *glfw_window = nullptr);
        void set_mouse_sensitivity(float x, float y);
        void set_move_speed(float speed);

    private:
        FirstPersonCamera* m_camera = nullptr;
        GLFWwindow* m_glfw_window = nullptr;

        float m_move_speed = 15.0f;
        float m_mouse_sensitivity_x = 0.008f;
        float m_mouse_sensitivity_y = 0.008f;
    };
}










