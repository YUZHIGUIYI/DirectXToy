//
// Created by ZZK on 2023/5/17.
//

#pragma once

#include <Toy/toy.h>

namespace toy
{
    struct vertex_data_s
    {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT4 color;
        inline static const std::array<D3D11_INPUT_ELEMENT_DESC, 2> input_layout{
            D3D11_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            D3D11_INPUT_ELEMENT_DESC{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };
    };

    struct mvp_s
    {
        DirectX::XMMATRIX model;
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX proj;
    };

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
        com_ptr<ID3D11InputLayout> class_vertex_layout;     // Vertex input layout
        com_ptr<ID3D11Buffer> class_vertex_buffer;          // Vertex buffer
        com_ptr<ID3D11Buffer> class_index_buffer;           // Index buffer
        com_ptr<ID3D11Buffer> class_constant_buffer;        // Constant buffer

        com_ptr<ID3D11VertexShader> class_vertex_shader;    // Vertex shader
        com_ptr<ID3D11PixelShader> class_pixel_shader;      // Pixel shader

        mvp_s class_mvp;                                   // Variable to modify constant buffer
    };
}



































