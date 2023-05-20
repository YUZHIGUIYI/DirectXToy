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
        static float phi = 0.0f;
        static float theta = 0.0f;
        phi += 0.3f * dt;
        theta += 0.37f * dt;

        class_mvp.model = DirectX::XMMatrixTranspose(
            DirectX::XMMatrixRotationX(phi) * DirectX::XMMatrixRotationY(theta)
        );
        class_mvp.proj = DirectX::XMMatrixTranspose(
            DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV2, get_aspect_ratio(), 1.0f, 1000.0f)
        );

        // Update constant buffer
        D3D11_MAPPED_SUBRESOURCE mapped_data{};
        class_d3d_immediate_context_->Map(class_constant_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_data);
        memcpy_s(mapped_data.pData, sizeof(mvp_s), &class_mvp, sizeof(mvp_s));
        class_d3d_immediate_context_->Unmap(class_constant_buffer.Get(), 0);
    }

    void game_app_c::draw_scene()
    {
        assert(class_d3d_immediate_context_);
        assert(class_swap_chain_);

        static float color[4] { 0.1f, 0.1f, 0.1f, 1.0f };
        class_d3d_immediate_context_->ClearRenderTargetView(class_render_target_view_.Get(), color);
        class_d3d_immediate_context_->ClearDepthStencilView(class_depth_stencil_view_.Get(),
                                                                D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        // Draw cube
        class_d3d_immediate_context_->DrawIndexed(36, 0, 0);
        class_swap_chain_->Present(0, 0);
    }

    void game_app_c::init_effect()
    {
        com_ptr<ID3DBlob> blob = nullptr;

        // Create vertex shader and vertex layout
        if (create_shader_from_file(L"../data/shaders/base_vs.cso", L"../data/shaders/base_vs.hlsl", "VS",
                                    "vs_5_0", blob.ReleaseAndGetAddressOf()) != S_OK)
        {
            DX_CRITICAL("Can not compile vertex shader file");
        }
        class_d3d_device_->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, class_vertex_shader.GetAddressOf());
        class_d3d_device_->CreateInputLayout(vertex_data_s::input_layout.data(), vertex_data_s::input_layout.size(),
                                                blob->GetBufferPointer(), blob->GetBufferSize(), class_vertex_layout.GetAddressOf());

        if (create_shader_from_file(L"../data/shaders/base_ps.cso", L"../data/shaders/base_ps.hlsl", "PS",
                                    "ps_5_0", blob.ReleaseAndGetAddressOf()) != S_OK)
        {
            DX_CRITICAL("Can not compile pixel shader file");
        }
        class_d3d_device_->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, class_pixel_shader.GetAddressOf());
    }

    void game_app_c::init_resource()
    {
        // Set cube vertices
        std::vector<vertex_data_s> vertices{
            { DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) },
            { DirectX::XMFLOAT3(-1.0f, 1.0f, -1.0f),  DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
            { DirectX::XMFLOAT3(1.0f, 1.0f, -1.0f),   DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
            { DirectX::XMFLOAT3(1.0f, -1.0f, -1.0f),  DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
            { DirectX::XMFLOAT3(-1.0f, -1.0f, 1.0f),  DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
            { DirectX::XMFLOAT3(-1.0f, 1.0f, 1.0f),   DirectX::XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },
            { DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f),    DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
            { DirectX::XMFLOAT3(1.0f, -1.0f, 1.0f),   DirectX::XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) }
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

        // Set index data
        std::vector<uint32_t> indices{
            // front
            0, 1, 2,
            2, 3, 0,
            // left
            4, 5, 1,
            1, 0, 4,
            // top
            1, 5, 6,
            6, 2, 1,
            // back
            7, 6, 5,
            5, 4, 7,
            // right
            3, 2, 6,
            6, 7, 3,
            // bottom
            4, 0, 3,
            3, 7, 4
        };

        // Set index buffer description
        D3D11_BUFFER_DESC ibd{};
        ibd.Usage = D3D11_USAGE_IMMUTABLE;
        ibd.ByteWidth = indices.size() * sizeof(indices[0]);
        ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
        ibd.CPUAccessFlags = 0;
        // Create index buffer
        init_data.pSysMem = indices.data();
        class_d3d_device_->CreateBuffer(&ibd, &init_data, class_index_buffer.GetAddressOf());

        // Set constant buffer description
        D3D11_BUFFER_DESC cbd{};
        cbd.Usage = D3D11_USAGE_DYNAMIC;
        cbd.ByteWidth = sizeof(mvp_s);
        cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        // Create constant buffer without initial data
        class_d3d_device_->CreateBuffer(&cbd, nullptr, class_constant_buffer.GetAddressOf());

        // Initialize mvp data
        class_mvp.model = DirectX::XMMatrixIdentity();
        class_mvp.view = DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtLH(
            DirectX::XMVectorSet(0.0f, 0.0f, -5.0f, 0.0f),
            DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),
            DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
        ));
        class_mvp.proj = DirectX::XMMatrixTranspose(
            DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV2, get_aspect_ratio(), 1.0f, 1000.0f)
        );


        // Set pipeline state
        //// Input assembly state - vertex buffer setting
        uint32_t stride = sizeof(vertex_data_s);    // Stride of vertex data
        uint32_t offset = 0;                        // Initial offset
        class_d3d_immediate_context_->IASetVertexBuffers(0, 1, class_vertex_buffer.GetAddressOf(), &stride, &offset);
        //// Input assembly state -  index buffer setting
        class_d3d_immediate_context_->IASetIndexBuffer(class_index_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
        //// Vertex layout and primitive type
        class_d3d_immediate_context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        class_d3d_immediate_context_->IASetInputLayout(class_vertex_layout.Get());
        // Bind constant buffer and shaders to pipeline
        class_d3d_immediate_context_->VSSetShader(class_vertex_shader.Get(), nullptr, 0);
        class_d3d_immediate_context_->VSSetConstantBuffers(0, 1, class_constant_buffer.GetAddressOf());
        class_d3d_immediate_context_->PSSetShader(class_pixel_shader.Get(), nullptr, 0);

        // Set debug objects
//        d3d11_set_debug_object_name(class_vertex_layout.Get(), "VertexPosColorLayout");
//        d3d11_set_debug_object_name(class_vertex_buffer.Get(), "VertexBuffer");
//        d3d11_set_debug_object_name(class_vertex_shader.Get(), "Trangle_VS");
//        d3d11_set_debug_object_name(class_pixel_shader.Get(), "Trangle_PS");
    }
}






































