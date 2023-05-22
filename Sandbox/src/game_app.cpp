//
// Created by ZZK on 2023/5/17.
//

#include <Sandbox/game_app.h>

namespace toy
{

    game_app_c::game_app_c(GLFWwindow* window, const std::string &window_name, int32_t init_width, int32_t init_height)
    : d3d_application_c(window, window_name, init_width, init_height), class_wireframe_mode(false)
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

    void game_app_c::update_scene(float dt)
    {
        static float phi = 0.0f;
        static float theta = 0.0f;
        static float tx = 0.0f;
        static float ty = 0.0f;
        static float scale = 1.0f;

        // ImGui window - properties
        if (ImGui::Begin("Type"))
        {
            static int32_t cur_mesh_item = 0;
            const std::array<const char *, 3> mesh_names{
                "Box", "Sphere", "Cone"
            };
            if (ImGui::Combo("Mesh", &cur_mesh_item, mesh_names.data(), mesh_names.size()))
            {
                geometry::MeshData<VertexPosColor, uint32_t> mesh_data;
                switch (cur_mesh_item)
                {
                    case 0 : mesh_data = geometry::create_box<VertexPosColor, uint32_t>(); break;
                    case 1 : mesh_data = geometry::create_sphere<VertexPosColor, uint32_t>(); break;
                    case 2 : mesh_data = geometry::create_cone<VertexPosColor, uint32_t>(); break;
                }
                reset_mesh(mesh_data);
            }
        }
        ImGui::End();

        if (ImGui::Begin("Properties"))
        {
            if (ImGui::Button("Reset Params"))
            {
                tx = ty = phi = theta = 0.0f;
                scale = 1.0f;
            }
            ImGui::SliderFloat("Scale", &scale, 0.2f, 2.0f);
            ImGui::SliderFloat("Phi", &phi, -DirectX::XM_PI, DirectX::XM_PI);
            ImGui::SliderFloat("Theta", &theta, -DirectX::XM_PI, DirectX::XM_PI);

            ImGui::Text("Position: (%.1f, %.1f, 0.0)", tx, ty);
        }

        // Prohibit control object while UI is active
        ImGuiIO& io = ImGui::GetIO();
        if (!ImGui::IsAnyItemActive())
        {
            // Mouse left button control moving
            if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
            {
                tx += io.MouseDelta.x * 0.01f;
                ty -= io.MouseDelta.y * 0.01f;
            }
            // Mouse right button control rotation
            else if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
            {
                phi -= io.MouseDelta.y * 0.02f;
                theta -= io.MouseDelta.x * 0.02f;
                phi = DirectX::XMScalarModAngle(phi);
                theta = DirectX::XMScalarModAngle(theta);
            }
            // Mouse wheel control scaling
            else if (io.MouseWheel != 0.0f)
            {
                scale += 0.02f * io.MouseWheel;
                scale = std::clamp(scale, 0.2f, 2.0f);
            }
        }
        ImGui::End();

        DirectX::XMMATRIX matrix_rotation = DirectX::XMMatrixRotationX(phi) * DirectX::XMMatrixRotationY(theta);

        class_mvp.model = DirectX::XMMatrixTranspose(
            DirectX::XMMatrixScalingFromVector(DirectX::XMVectorReplicate(scale)) *
            matrix_rotation *
            DirectX::XMMatrixTranslation(tx, ty, 0.0f)
        );
        class_mvp.proj = DirectX::XMMatrixTranspose(
            DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV2, get_aspect_ratio(), 1.0f, 1000.0f)
        );

        class_mvp.world_inv_transpose = DirectX::XMMatrixTranspose(inverse_transpose(matrix_rotation));

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
        // Draw object with solid mode
//        class_d3d_immediate_context_->RSSetState(nullptr);
        class_d3d_immediate_context_->DrawIndexed(class_index_count, 0, 0);

        // Draw object with wireframe mode
//        class_d3d_immediate_context_->RSSetState(class_rs_wireframe.Get());
//        class_d3d_immediate_context_->DrawIndexed(class_index_count, 0, 0);

        // Render ImGui
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // Update and render additional platform windows
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        // Present
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
        auto&& input_layout = VertexPosColor::get_input_layout();
        class_d3d_device_->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, class_vertex_shader.GetAddressOf());
        class_d3d_device_->CreateInputLayout(input_layout.data(), input_layout.size(),
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
        // Initialize mesh data
        auto mesh_data = geometry::create_box<VertexPosColor, uint32_t>();
        reset_mesh(mesh_data);

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
        class_mvp.world_inv_transpose = DirectX::XMMatrixIdentity();

        //// Create rasterization state
        D3D11_RASTERIZER_DESC rasterizer_desc{};
        rasterizer_desc.FillMode = D3D11_FILL_WIREFRAME;
        rasterizer_desc.CullMode = D3D11_CULL_NONE;
        rasterizer_desc.FrontCounterClockwise = false;
        rasterizer_desc.DepthClipEnable = true;
        class_d3d_device_->CreateRasterizerState(&rasterizer_desc, class_rs_wireframe.GetAddressOf());

        // Set pipeline state
        //// Vertex layout and primitive type
        class_d3d_immediate_context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        class_d3d_immediate_context_->IASetInputLayout(class_vertex_layout.Get());
        //// Bind constant buffer and shaders to pipeline
        class_d3d_immediate_context_->VSSetShader(class_vertex_shader.Get(), nullptr, 0);
        class_d3d_immediate_context_->VSSetConstantBuffers(0, 1, class_constant_buffer.GetAddressOf());
        class_d3d_immediate_context_->PSSetShader(class_pixel_shader.Get(), nullptr, 0);

        // Set debug objects
//        d3d11_set_debug_object_name(class_vertex_layout.Get(), "VertexPosColorLayout");
//        d3d11_set_debug_object_name(class_vertex_buffer.Get(), "VertexBuffer");
//        d3d11_set_debug_object_name(class_vertex_shader.Get(), "Trangle_VS");
//        d3d11_set_debug_object_name(class_pixel_shader.Get(), "Trangle_PS");
    }

    void game_app_c::reset_mesh(const geometry::MeshData<VertexPosColor> &mesh_data)
    {
        // Release resources
        class_vertex_buffer.Reset();
        class_index_buffer.Reset();

        // Set vertex buffer description
        D3D11_BUFFER_DESC vertex_buffer_desc{};
        vertex_buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
        vertex_buffer_desc.ByteWidth = static_cast<uint32_t>(mesh_data.vertices.size() * sizeof(VertexPosColor));
        vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertex_buffer_desc.CPUAccessFlags = 0;
        // Create vertex buffer
        D3D11_SUBRESOURCE_DATA init_data{};
        init_data.pSysMem = mesh_data.vertices.data();
        class_d3d_device_->CreateBuffer(&vertex_buffer_desc, &init_data, class_vertex_buffer.GetAddressOf());
        // Set vertex buffer in input assembly stage
        uint32_t stride = sizeof(VertexPosColor);
        uint32_t offset = 0;
        class_d3d_immediate_context_->IASetVertexBuffers(0, 1, class_vertex_buffer.GetAddressOf(), &stride, &offset);

        // Set index buffer
        class_index_count = mesh_data.indices.size();
        D3D11_BUFFER_DESC index_buffer_desc{};
        index_buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
        index_buffer_desc.ByteWidth = static_cast<uint32_t>(class_index_count * sizeof(uint32_t));
        index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        index_buffer_desc.CPUAccessFlags = 0;
        // Create index buffer
        init_data.pSysMem = mesh_data.indices.data();
        class_d3d_device_->CreateBuffer(&index_buffer_desc, &init_data, class_index_buffer.GetAddressOf());
        // Set index buffer in input assembly stage
        class_d3d_immediate_context_->IASetIndexBuffer(class_index_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

        // Set debug objects
    }
}






































