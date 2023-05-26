//
// Created by ZZK on 2023/5/17.
//

#pragma once

#include <Sandbox/game_object.h>

namespace toy
{
    class game_app_c : public d3d_application_c
    {
    public:
        game_app_c(GLFWwindow* window, const std::string& window_name, int32_t init_width, int32_t init_height);
        ~game_app_c() override;

        void init() override;
        void update_scene(float dt) override;
        void draw_scene() override;

    private:
        void init_effect();
        void init_resource();

    private:
        com_ptr<ID3D11InputLayout> class_vertex_layout;         // Vertex input layout
        com_ptr<ID3D11Buffer> class_constant_buffers[5];        // Constant buffers

        com_ptr<ID3D11VertexShader> class_vertex_shader;        // Vertex shader
        com_ptr<ID3D11PixelShader> class_pixel_shader;          // Pixel shader

        std::shared_ptr<camera_c> class_camera;                 // Camera

        CBChangeEveryFrame class_cb_every_frame;                // Constant buffer that changes every frame
        CBDrawStates class_cb_states;                           // Constant buffer that control draw states
        CBChangeOnResize class_cb_on_resize;                    // Constant buffer that changes when window resizes
        CBChangeRarely class_cb_rarely;                         // Constant buffer that stores variables that rarely change

        game_object_c class_wire_fence;
        game_object_c class_floor;
        game_object_c class_water;
        game_object_c class_mirror;
        std::vector<game_object_c> class_walls;
    };
}



































