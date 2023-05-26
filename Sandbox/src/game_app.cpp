//
// Created by ZZK on 2023/5/17.
//

#include <Sandbox/game_app.h>

#include <DDSTextureLoader/DDSTextureLoader11.h>
#include <WICTextureLoader/WICTextureLoader11.h>

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

        RenderStates::init(class_d3d_device_.Get());
        init_effect();
        init_resource();

        // Change projection matrix when window resizes
        event_manager_c::subscribe(event_type_e::WindowResize, [this] (const event_t& event)
        {
            if (class_camera != nullptr && class_constant_buffers[3] != nullptr)
            {
                class_camera->set_frustum(DirectX::XM_PI / 3.0f, get_aspect_ratio(), 0.5f, 1600.0f);
                class_camera->set_viewport(0.0f, 0.0f, (float)class_client_width_, (float)class_client_height_);

                class_cb_on_resize.proj = DirectX::XMMatrixTranspose(class_camera->get_proj_xm());

                D3D11_MAPPED_SUBRESOURCE mapped_data{};
                class_d3d_immediate_context_->Map(class_constant_buffers[3].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_data);
                memcpy_s(mapped_data.pData, sizeof(CBChangeOnResize), &class_cb_on_resize, sizeof(CBChangeOnResize));
                class_d3d_immediate_context_->Unmap(class_constant_buffers[3].Get(), 0);
            }
        });
    }

    void game_app_c::update_scene(float dt)
    {
        auto cam_third = std::dynamic_pointer_cast<third_person_camera_c>(class_camera);
        ImGuiIO& io = ImGui::GetIO();
        float d1 = 0.0f, d2 = 0.0f;

        if (DX_INPUT::is_key_pressed(class_glfw_window, key::W))
        {
            d1 += dt;
        } else if (DX_INPUT::is_key_pressed(class_glfw_window, key::S))
        {
            d1 -= dt;
        } else if (DX_INPUT::is_key_pressed(class_glfw_window, key::A))
        {
            d2 -= dt;
        } else if (DX_INPUT::is_key_pressed(class_glfw_window, key::D))
        {
            d2 += dt;
        }

//        cam_first->walk(d1 * 6.0f);
//        cam_first->strafe(d2 * 6.0f);
//        cam_first->move_forward(d1 * 6.0f);
//        cam_first->strafe(d2 * 6.0f);

        DirectX::XMFLOAT3 target = class_wire_fence.get_transform().get_position();
        cam_third->set_target(target);
        // Rotate around specific object
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
        {
            cam_third->rotate_x(io.MouseDelta.y * 0.01f);
            cam_third->rotate_y(io.MouseDelta.x * 0.01f);
        }
        cam_third->approach(-io.MouseWheel * 1.0f);

        // Update view matrix and constant buffer that changes every frame
        DirectX::XMStoreFloat4(&class_cb_every_frame.eye_pos, class_camera->get_position_xm());
        class_cb_every_frame.view = DirectX::XMMatrixTranspose(class_camera->get_view_xm());

        D3D11_MAPPED_SUBRESOURCE mapped_data{};
        class_d3d_immediate_context_->Map(class_constant_buffers[2].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_data);
        memcpy_s(mapped_data.pData, sizeof(CBChangeEveryFrame), &class_cb_every_frame, sizeof(CBChangeEveryFrame));
        class_d3d_immediate_context_->Unmap(class_constant_buffers[2].Get(), 0);
    }

    void game_app_c::draw_scene()
    {
        assert(class_d3d_immediate_context_);
        assert(class_swap_chain_);

        static float color[4] { 0.1f, 0.1f, 0.1f, 1.0f };
        class_d3d_immediate_context_->ClearRenderTargetView(class_render_target_view_.Get(), color);
        class_d3d_immediate_context_->ClearDepthStencilView(class_depth_stencil_view_.Get(),
                                                                D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        // 1. Write a stencil value 1 to the stencil buffer for the specular reflection area
        // Back cull mode
        class_d3d_immediate_context_->RSSetState(nullptr);
        class_d3d_immediate_context_->OMSetDepthStencilState(RenderStates::ds_write_stencil.Get(), 1);
        class_d3d_immediate_context_->OMSetBlendState(RenderStates::bs_no_color_write.Get(), nullptr, 0xFFFFFFFF);

        class_mirror.draw(class_d3d_immediate_context_.Get());

        // 2. Draw opaque reflection object
        // Enable reflection
        class_cb_states.is_reflection = true;
        D3D11_MAPPED_SUBRESOURCE mapped_data{};
        class_d3d_immediate_context_->Map(class_constant_buffers[1].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_data);
        memcpy_s(mapped_data.pData, sizeof(CBDrawStates), &class_cb_states, sizeof(CBDrawStates));
        class_d3d_immediate_context_->Unmap(class_constant_buffers[1].Get(), 0);
        // Draw opaque objects, close-wise cull
        // Draw for area with stencil value 1
        class_d3d_immediate_context_->RSSetState(RenderStates::rs_cull_clock_wise.Get());
        class_d3d_immediate_context_->OMSetDepthStencilState(RenderStates::ds_draw_with_stencil.Get(), 1);
        class_d3d_immediate_context_->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);

        class_walls[2].draw(class_d3d_immediate_context_.Get());
        class_walls[3].draw(class_d3d_immediate_context_.Get());
        class_walls[4].draw(class_d3d_immediate_context_.Get());
        class_floor.draw(class_d3d_immediate_context_.Get());

        // 3. Draw transparent reflection object
        // Disable close-wise cull
        // Draw for area with stencil value 1
        // Transparent blend
        class_d3d_immediate_context_->RSSetState(RenderStates::rs_no_cull.Get());
        class_d3d_immediate_context_->OMSetDepthStencilState(RenderStates::ds_draw_with_stencil.Get(), 1);
        class_d3d_immediate_context_->OMSetBlendState(RenderStates::bs_transparent.Get(), nullptr, 0xFFFFFFFF);

        class_wire_fence.draw(class_d3d_immediate_context_.Get());
        class_water.draw(class_d3d_immediate_context_.Get());
//        class_mirror.draw(class_d3d_immediate_context_.Get());    // No transparent, must modify alpha

        // Disable reflection
        class_cb_states.is_reflection = false;
        class_d3d_immediate_context_->Map(class_constant_buffers[1].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_data);
        memcpy_s(mapped_data.pData, sizeof(CBDrawStates), &class_cb_states, sizeof(CBDrawStates));
        class_d3d_immediate_context_->Unmap(class_constant_buffers[1].Get(), 0);

        // 4. Draw normal opaque objects
        class_d3d_immediate_context_->RSSetState(nullptr);
        class_d3d_immediate_context_->OMSetDepthStencilState(nullptr, 0);
        class_d3d_immediate_context_->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);

        for (auto&& wall : class_walls)
        {
            wall.draw(class_d3d_immediate_context_.Get());
        }
        class_floor.draw(class_d3d_immediate_context_.Get());

        // 5. Draw normal transparent objects
        // Disable close-wise
        // Enable transparent blend
        class_d3d_immediate_context_->RSSetState(RenderStates::rs_no_cull.Get());
        class_d3d_immediate_context_->OMSetDepthStencilState(nullptr, 0);
        class_d3d_immediate_context_->OMSetBlendState(RenderStates::bs_transparent.Get(), nullptr, 0xFFFFFFFF);

        class_wire_fence.draw(class_d3d_immediate_context_.Get());
        class_water.draw(class_d3d_immediate_context_.Get());

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
        auto&& input_layout = VertexPosNormalTex ::get_input_layout();
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
        // Set constant buffer description
        D3D11_BUFFER_DESC cbd{};
        cbd.Usage = D3D11_USAGE_DYNAMIC;
        cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        // Create constant buffer for VS and PS
        cbd.ByteWidth = sizeof(CBChangeEveryDraw);
        class_d3d_device_->CreateBuffer(&cbd, nullptr, class_constant_buffers[0].GetAddressOf());
        cbd.ByteWidth = sizeof(CBDrawStates);
        class_d3d_device_->CreateBuffer(&cbd, nullptr, class_constant_buffers[1].GetAddressOf());
        cbd.ByteWidth = sizeof(CBChangeEveryFrame);
        class_d3d_device_->CreateBuffer(&cbd, nullptr, class_constant_buffers[2].GetAddressOf());
        cbd.ByteWidth = sizeof(CBChangeOnResize);
        class_d3d_device_->CreateBuffer(&cbd, nullptr, class_constant_buffers[3].GetAddressOf());
        cbd.ByteWidth = sizeof(CBChangeRarely);
        class_d3d_device_->CreateBuffer(&cbd, nullptr, class_constant_buffers[4].GetAddressOf());

        // Initialize objects
        com_ptr<ID3D11ShaderResourceView> texture = nullptr;
        DirectX::CreateDDSTextureFromFile(class_d3d_device_.Get(), L"../data/textures/WireFence.dds", nullptr, texture.GetAddressOf());
        class_wire_fence.set_buffer(class_d3d_device_.Get(), geometry::create_box<VertexPosNormalTex, uint32_t>());
        class_wire_fence.set_texture(texture.Get());
        class_wire_fence.get_transform().set_position(0.0f, 0.01f, 7.5f);

        DirectX::CreateDDSTextureFromFile(class_d3d_device_.Get(), L"../data/textures/floor.dds", nullptr, texture.ReleaseAndGetAddressOf());
        class_floor.set_buffer(class_d3d_device_.Get(),
                                geometry::create_plane<VertexPosNormalTex, uint32_t>(DirectX::XMFLOAT2(20.0f, 20.0f), DirectX::XMFLOAT2(5.0f, 5.0f)));
        class_floor.set_texture(texture.Get());
        class_floor.get_transform().set_position(0.0f, -1.0f, 0.0f);

        class_walls.resize(5);
        DirectX::CreateDDSTextureFromFile(class_d3d_device_.Get(), L"../data/textures/brick.dds", nullptr, texture.ReleaseAndGetAddressOf());
        // Control five walls generation, the middle of 0 and 1 is used to place mirror
        //     ____     ____
        //    /| 0 |   | 1 |\
        //   /4|___|___|___|2\
        //  /_/_ _ _ _ _ _ _\_\
        // | /       3       \ |
        // |/_________________\|
        //
        for (uint32_t i = 0; i < 5; ++i)
        {
            class_walls[i].set_texture(texture.Get());
        }
        class_walls[0].set_buffer(class_d3d_device_.Get(),
                                    geometry::create_plane<VertexPosNormalTex, uint32_t>(DirectX::XMFLOAT2(6.0f, 8.0f), DirectX::XMFLOAT2(1.5f, 2.0f)));
        class_walls[1].set_buffer(class_d3d_device_.Get(),
                                    geometry::create_plane<VertexPosNormalTex, uint32_t>(DirectX::XMFLOAT2(6.0f, 8.0f), DirectX::XMFLOAT2(1.5f, 2.0f)));
        class_walls[2].set_buffer(class_d3d_device_.Get(),
                                    geometry::create_plane<VertexPosNormalTex, uint32_t>(DirectX::XMFLOAT2(20.0f, 8.0f), DirectX::XMFLOAT2(5.0f, 2.0f)));
        class_walls[3].set_buffer(class_d3d_device_.Get(),
                                    geometry::create_plane<VertexPosNormalTex, uint32_t>(DirectX::XMFLOAT2(20.0f, 8.0f), DirectX::XMFLOAT2(5.0f, 2.0f)));
        class_walls[4].set_buffer(class_d3d_device_.Get(),
                                    geometry::create_plane<VertexPosNormalTex, uint32_t>(DirectX::XMFLOAT2(20.0f, 8.0f), DirectX::XMFLOAT2(5.0f, 2.0f)));

        class_walls[0].get_transform().set_rotation(-DirectX::XM_PIDIV2, 0.0f, 0.0f);
        class_walls[0].get_transform().set_position(-7.0f, 3.0f, 10.0f);
        class_walls[1].get_transform().set_rotation(-DirectX::XM_PIDIV2, 0.0f, 0.0f);
        class_walls[1].get_transform().set_position(7.0f, 3.0f, 10.0f);
        class_walls[2].get_transform().set_rotation(-DirectX::XM_PIDIV2, DirectX::XM_PIDIV2, 0.0f);
        class_walls[2].get_transform().set_position(10.0f, 3.0f, 0.0f);
        class_walls[3].get_transform().set_rotation(-DirectX::XM_PIDIV2, DirectX::XM_PI, 0.0f);
        class_walls[3].get_transform().set_position(0.0f, 3.0f, -10.0f);
        class_walls[4].get_transform().set_rotation(-DirectX::XM_PIDIV2, -DirectX::XM_PIDIV2, 0.0f);
        class_walls[4].get_transform().set_position(-10.0f, 3.0f, 0.0f);

        DirectX::CreateDDSTextureFromFile(class_d3d_device_.Get(), L"../data/textures/water.dds", nullptr, texture.ReleaseAndGetAddressOf());
        class_water.set_buffer(class_d3d_device_.Get(),
                                geometry::create_plane<VertexPosNormalTex, uint32_t>(DirectX::XMFLOAT2(20.0f, 20.0f), DirectX::XMFLOAT2(10.0f, 10.0f)));
        class_water.set_texture(texture.Get());

        DirectX::CreateDDSTextureFromFile(class_d3d_device_.Get(), L"../data/textures/ice.dds", nullptr, texture.ReleaseAndGetAddressOf());
        class_mirror.set_buffer(class_d3d_device_.Get(),
                            geometry::create_plane<VertexPosNormalTex, uint32_t>(DirectX::XMFLOAT2(8.0f, 8.0f), DirectX::XMFLOAT2(1.0f, 1.0f)));
        class_mirror.set_texture(texture.Get());
        class_mirror.get_transform().set_rotation(-DirectX::XM_PIDIV2, 0.0f, 0.0f);
        class_mirror.get_transform().set_position(0.0f, 3.0f, 10.0f);

        // Create constant buffer that changes every frame
        auto camera = std::make_shared<third_person_camera_c>();
        camera->set_viewport(0.0f, 0.0f, (float)class_client_width_, (float)class_client_height_);
        camera->set_target(class_wire_fence.get_transform().get_position());
        camera->set_distance(8.0f);
        camera->set_distance_min_max(3.0f, 20.0f);
        camera->set_rotation_x(DirectX::XM_PIDIV4);
        class_camera = camera;

        class_cb_every_frame.view = DirectX::XMMatrixTranspose(class_camera->get_view_xm());
        DirectX::XMStoreFloat4(&class_cb_every_frame.eye_pos, class_camera->get_position_xm());

        // Create constant buffer that changes when window resizes
        class_camera->set_frustum(DirectX::XM_PI / 3.0f, get_aspect_ratio(), 0.5f, 1600.0f);
        class_cb_on_resize.proj = DirectX::XMMatrixTranspose(class_camera->get_proj_xm());

        // Create constant buffer that rarely change
        class_cb_rarely.reflection = DirectX::XMMatrixTranspose(DirectX::XMMatrixReflect(DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 10.0f)));

        // Update constant buffer resource
        D3D11_MAPPED_SUBRESOURCE mapped_data{};
        class_d3d_immediate_context_->Map(class_constant_buffers[3].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_data);
        memcpy_s(mapped_data.pData, sizeof(CBChangeOnResize), &class_cb_on_resize, sizeof(CBChangeOnResize));
        class_d3d_immediate_context_->Unmap(class_constant_buffers[3].Get(), 0);

        class_d3d_immediate_context_->Map(class_constant_buffers[4].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_data);
        memcpy_s(mapped_data.pData, sizeof(CBChangeRarely), &class_cb_rarely, sizeof(CBChangeRarely));
        class_d3d_immediate_context_->Unmap(class_constant_buffers[4].Get(), 0);

        // Set pipeline state
        //// Vertex layout and primitive type
        class_d3d_immediate_context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        class_d3d_immediate_context_->IASetInputLayout(class_vertex_layout.Get());
        //// Bind constant buffer and sampler and  texture and shaders to pipeline
        class_d3d_immediate_context_->VSSetShader(class_vertex_shader.Get(), nullptr, 0);

        class_d3d_immediate_context_->VSSetConstantBuffers(0, 1, class_constant_buffers[0].GetAddressOf());
        class_d3d_immediate_context_->VSSetConstantBuffers(1, 1, class_constant_buffers[1].GetAddressOf());
        class_d3d_immediate_context_->VSSetConstantBuffers(2, 1, class_constant_buffers[2].GetAddressOf());
        class_d3d_immediate_context_->VSSetConstantBuffers(3, 1, class_constant_buffers[3].GetAddressOf());
        class_d3d_immediate_context_->VSSetConstantBuffers(4, 1, class_constant_buffers[4].GetAddressOf());

        class_d3d_immediate_context_->PSSetConstantBuffers(0, 1, class_constant_buffers[0].GetAddressOf());
        class_d3d_immediate_context_->PSSetConstantBuffers(1, 1, class_constant_buffers[1].GetAddressOf());
        class_d3d_immediate_context_->PSSetConstantBuffers(2, 1, class_constant_buffers[2].GetAddressOf());
        class_d3d_immediate_context_->PSSetConstantBuffers(4, 1, class_constant_buffers[4].GetAddressOf());

        class_d3d_immediate_context_->PSSetShader(class_pixel_shader.Get(), nullptr, 0);
        class_d3d_immediate_context_->PSSetSamplers(0, 1, RenderStates::ss_linear_wrap.GetAddressOf());

        // TODO: set debug object name
    }
}






































