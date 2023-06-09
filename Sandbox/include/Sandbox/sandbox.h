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
        enum class SphereMode { None, Reflection, Refraction };

    public:
        sandbox_c(GLFWwindow* window, const std::string& window_name, int32_t init_width, int32_t init_height);
        ~sandbox_c() override;

        void init() override;
        void update_scene(float dt) override;
        void draw_scene() override;

    private:
        void init_resource();

        void draw(bool draw_center_sphere, const camera_c& camera, ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* dsv);

    private:
        BasicEffect m_basic_effect;                                 // Render object effect manage
        SkyboxEffect m_skybox_effect;                               // Skybox effect manage
        PostProcessEffect m_post_process_effect;                    // PostProcess effect manage

        std::unique_ptr<Depth2D> m_depth_texture;                   // Depth buffer
        std::unique_ptr<TextureCube> m_dynamic_texture_cube;        // Dynamic skybox
        std::unique_ptr<Depth2D> m_dynamic_cube_depth_texture;      // Dynamic skybox depth buffer
        std::unique_ptr<Texture2D> m_debug_dynamic_cube_texture;    // Debug dynamic skybox

        std::unique_ptr<Texture2D> m_lit_texture;                   // Scene rendering buffer
        std::unique_ptr<Texture2D> m_temp_texture;                  // Temporary buffer for blurring operation

        RenderObject m_spheres[5];                                  // Spheres
        RenderObject m_center_sphere;                               // Center sphere
        RenderObject m_ground;                                      // Ground
        RenderObject m_cylinders[5];                                // Cylinders
        RenderObject m_skybox;                                      // Skybox
        RenderObject m_debug_skybox;                                // Debug skybox

        std::shared_ptr<first_person_camera_c> m_camera;            // Main camera
        std::shared_ptr<first_person_camera_c> m_cube_camera;       // Dynamic skybox camera
        std::shared_ptr<first_person_camera_c> m_debug_camera;      // Debug dynamic skybox camera
        FirstPersonCameraController m_camera_controller;            // Camera controller

        ImVec2 m_debug_texture_xy;                                  // Debug texture position
        ImVec2 m_debug_texture_wh;                                  // Debug texture size

        SphereMode m_sphere_mode = SphereMode::None;                // Reflection or refraction or null

        float m_sphere_rad = 0.0f;                                  // Sphere rotate radian
        float m_eta = 1.0f/ 1.5f;                                   // Refractive index of air medium

        int32_t m_blur_mode = 1;                                    // 0 represents Sobel-mode, 1 represents Blur-mode
        float m_blur_sigma = 2.5f;                                  // Sigma value used for ambiguity
        int32_t m_blur_radius = 5;                                  // Radius used for ambiguity
        int32_t m_blur_times = 1;                                   // Ambiguity degree

        bool m_screenshot_started = false;                          // Capture the current screen content
    };
}



































