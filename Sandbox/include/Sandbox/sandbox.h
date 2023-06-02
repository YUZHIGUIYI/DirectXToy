//
// Created by ZZK on 2023/5/17.
//

#pragma once

#include <Sandbox/render_object.h>

namespace toy
{
    class sandbox_c : public d3d_application_c
    {
    public:
        sandbox_c(GLFWwindow* window, const std::string& window_name, int32_t init_width, int32_t init_height);
        ~sandbox_c() override;

        void init() override;
        void update_scene(float dt) override;
        void draw_scene() override;

    private:
        void init_resource();

    private:
        BasicEffect m_basic_effect;                         // Render object effect manage

        std::unique_ptr<Depth2D> m_depth_texture;           // Depth buffer

        RenderObject m_house;                               // House
        RenderObject m_ground;                              // Ground

        std::shared_ptr<third_person_camera_c> m_camera;    // Camera
    };
}



































