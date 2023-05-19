//
// Created by ZZK on 2023/5/17.
//

#include <Sandbox/game_app.h>

namespace toy
{

    game_app_c::game_app_c(GLFWwindow* window, const std::string &window_name, int32_t init_width, int32_t init_height)
    : d3d_application_c(window, window_name, init_width, init_height)
    {

    }

    game_app_c::~game_app_c()
    {

    }

    void game_app_c::init()
    {
        d3d_application_c::init();

        init_effect();
        init_resource();
    }

    void game_app_c::on_resize()
    {
        d3d_application_c::on_resize();
    }

    void game_app_c::update_scene(float dt)
    {

    }

    void game_app_c::draw_scene()
    {
        assert(class_d3d_immediate_context_);
        assert(class_swap_chain_);

        static float color[4] { 0.1f, 0.1f, 0.1f, 1.0f };
        class_d3d_immediate_context_->ClearRenderTargetView(class_render_target_view_.Get(), color);
        class_d3d_immediate_context_->ClearDepthStencilView(class_depth_stencil_view_.Get(),
                                                                D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        // Draw triangle
        class_d3d_immediate_context_->Draw(3, 0);
        class_swap_chain_->Present(0, 0);
    }

    void game_app_c::init_effect()
    {
        com_ptr<ID3DBlob> blob = nullptr;

        // Create vertex shader and vertex layout
        if (create_shader_from_file(L"../data/shaders/triangle_vs.cso", L"../data/shaders/triangle_vs.hlsl", "VS",
                                    "vs_5_0", blob.ReleaseAndGetAddressOf()) != S_OK)
        {
            throw std::runtime_error("Can not compile vertex shader file");
        }
        class_d3d_device_->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, class_vertex_shader.GetAddressOf());
        class_d3d_device_->CreateInputLayout(vertex_data_s::input_layout.data(), vertex_data_s::input_layout.size(),
                                                blob->GetBufferPointer(), blob->GetBufferSize(), class_vertex_layout.GetAddressOf());

        if (create_shader_from_file(L"../data/shaders/triangle_ps.cso", L"../data/shaders/triangle_ps.hlsl", "PS",
                                    "ps_5_0", blob.ReleaseAndGetAddressOf()) != S_OK)
        {
            throw std::runtime_error("Can not compile pixel shader file");
        }
        class_d3d_device_->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, class_pixel_shader.GetAddressOf());
    }

    void game_app_c::init_resource()
    {
        // Set triangle vertices
        std::vector<vertex_data_s> vertices{
            { DirectX::XMFLOAT3{0.0f, 0.5f, 0.5f}, DirectX::XMFLOAT4{0.0f, 1.0f, 0.0f, 1.0f} },
            { DirectX::XMFLOAT3{0.5f, -0.5f, 0.5f}, DirectX::XMFLOAT4{0.0f, 0.0f, 1.0f, 1.0f} },
            { DirectX::XMFLOAT3{-0.5f, -0.5f, 0.5f}, DirectX::XMFLOAT4{1.0f, 0.0f, 0.0f, 1.0f} }
        };

        // Set vertex buffer description
        D3D11_BUFFER_DESC vbd{};
        vbd.Usage = D3D11_USAGE_IMMUTABLE;
        vbd.ByteWidth = vertices.size() * sizeof(vertices[0]);
        vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vbd.CPUAccessFlags = 0;
        // Create vertex buffer
        D3D11_SUBRESOURCE_DATA init_data{};
        init_data.pSysMem = vertices.data();
        class_d3d_device_->CreateBuffer(&vbd, &init_data, class_vertex_buffer.GetAddressOf());

        // Set pipeline
        //// Input assembly state - vertex buffer setting
        uint32_t stride = sizeof(vertex_data_s);    // Stride of vertex data
        uint32_t offset = 0;                        // Initial offset
        class_d3d_immediate_context_->IASetVertexBuffers(0, 1, class_vertex_buffer.GetAddressOf(), &stride, &offset);
        //// Vertex layout and primitive type
        class_d3d_immediate_context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        class_d3d_immediate_context_->IASetInputLayout(class_vertex_layout.Get());
        // Bind shaders to pipeline
        class_d3d_immediate_context_->VSSetShader(class_vertex_shader.Get(), nullptr, 0);
        class_d3d_immediate_context_->PSSetShader(class_pixel_shader.Get(), nullptr, 0);

        // Set debug objects
//        d3d11_set_debug_object_name(class_vertex_layout.Get(), "VertexPosColorLayout");
//        d3d11_set_debug_object_name(class_vertex_buffer.Get(), "VertexBuffer");
//        d3d11_set_debug_object_name(class_vertex_shader.Get(), "Trangle_VS");
//        d3d11_set_debug_object_name(class_pixel_shader.Get(), "Trangle_PS");
    }
}






































