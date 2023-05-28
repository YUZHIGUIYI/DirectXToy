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
        Material class_shadow_mat;
        Material class_wood_mat;

        render_object_c class_bolt_anim;
        render_object_c class_wood_crate;
        render_object_c class_floor;
        render_object_c class_mirror;
        std::vector<render_object_c> class_walls;

        std::vector<com_ptr<ID3D11ShaderResourceView>> class_bolt_srvs;     // Bolt animation textures

        std::shared_ptr<camera_c> class_camera;                             // Camera
        basic_effect_c class_basic_effect;                                  // Basic effect
    };
}



































