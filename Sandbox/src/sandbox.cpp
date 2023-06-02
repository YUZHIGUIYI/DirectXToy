//
// Created by ZZK on 2023/5/17.
//

#include <Sandbox/sandbox.h>

#include <DDSTextureLoader/DDSTextureLoader11.h>
#include <WICTextureLoader/WICTextureLoader11.h>

namespace toy
{

    sandbox_c::sandbox_c(GLFWwindow* window, const std::string &window_name, int32_t init_width, int32_t init_height)
    : d3d_application_c(window, window_name, init_width, init_height)
    {

    }

    sandbox_c::~sandbox_c()
    {

    }

    void sandbox_c::init()
    {
        d3d_application_c::init();

        // Initialize depth texture
        m_depth_texture = std::make_unique<Depth2D>(m_d3d_device.Get(), m_client_width, m_client_height);
        m_depth_texture->set_debug_object_name("DepthTexture");

        // Initialize texture manager and model manager
        model::TextureManagerHandle::get().init(m_d3d_device.Get());
        model::ModelManagerHandle::get().init(m_d3d_device.Get());

        // Initialize render states
        RenderStates::init(m_d3d_device.Get());
        // Initialize effect
        m_basic_effect.init(m_d3d_device.Get());
        // Initialize resource
        init_resource();

        // Change projection matrix when window resizes
        event_manager_c::subscribe(event_type_e::WindowResize, [this] (const event_t& event)
        {
            auto&& window_resize_event = std::get<window_resize_event_c>(event);
            // Check whether the window has been minimized
            if (window_resize_event.window_width == 0 || window_resize_event.window_height == 0)
            {
                return;
            }

            m_depth_texture.reset();
            m_depth_texture = std::make_unique<Depth2D>(m_d3d_device.Get(), window_resize_event.window_width, window_resize_event.window_height);
            m_depth_texture->set_debug_object_name("DepthTexture");

            if (m_camera != nullptr)
            {
                m_camera->set_frustum(DirectX::XM_PI / 3.0f, get_aspect_ratio(), 0.5f, 1600.0f);
                m_camera->set_viewport(0.0f, 0.0f, (float)m_client_width, (float)m_client_height);
                m_basic_effect.set_proj_matrix(m_camera->get_proj_xm());
            }
        });
    }

    void sandbox_c::update_scene(float dt)
    {
        auto cam_third = std::dynamic_pointer_cast<third_person_camera_c>(m_camera);

        ImGuiIO& io = ImGui::GetIO();
        float d1 = 0.0f, d2 = 0.0f;
        if (DX_INPUT::is_key_pressed(m_glfw_window, key::W))
        {
            d1 += dt;
        } else if (DX_INPUT::is_key_pressed(m_glfw_window, key::S))
        {
            d1 -= dt;
        } else if (DX_INPUT::is_key_pressed(m_glfw_window, key::A))
        {
            d2 -= dt;
        } else if (DX_INPUT::is_key_pressed(m_glfw_window, key::D))
        {
            d2 += dt;
        }

        // Rotate around specific object
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
        {
            cam_third->rotate_x(io.MouseDelta.y * 0.01f);
            cam_third->rotate_y(io.MouseDelta.x * 0.01f);
        }
        cam_third->approach(-io.MouseWheel * 1.0f);

        // Update view matrix and eye position that change every frame
        m_basic_effect.set_view_matrix(m_camera->get_view_xm());
        m_basic_effect.set_eye_pos(m_camera->get_position());
    }

    void sandbox_c::draw_scene()
    {
        // Create render target views of back buffers
        if (m_frame_count < m_back_buffer_count)
        {
            com_ptr<ID3D11Texture2D> back_buffer = nullptr;
            m_swap_chain->GetBuffer(0, IID_PPV_ARGS(back_buffer.GetAddressOf()));
            CD3D11_RENDER_TARGET_VIEW_DESC rtv_desc{ D3D11_RTV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB };
            m_d3d_device->CreateRenderTargetView(back_buffer.Get(), &rtv_desc, m_render_target_views[m_frame_count].ReleaseAndGetAddressOf());
        }

        static float color[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
        m_d3d_immediate_context->ClearRenderTargetView(get_back_buffer_rtv(), color);
        m_d3d_immediate_context->ClearDepthStencilView(m_depth_texture->get_depth_stencil(),
                                                                D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        ID3D11RenderTargetView* render_target_views[1] = { get_back_buffer_rtv() };
        m_d3d_immediate_context->OMSetRenderTargets(1, render_target_views, m_depth_texture->get_depth_stencil());
        auto viewport = m_camera->get_viewport();
        m_d3d_immediate_context->RSSetViewports(1, &viewport);

        m_basic_effect.set_default_render();
        m_ground.draw(m_d3d_immediate_context.Get(), m_basic_effect);
        m_house.draw(m_d3d_immediate_context.Get(), m_basic_effect);

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
        m_swap_chain->Present(0, m_is_dxgi_flip_model ? DXGI_PRESENT_ALLOW_TEARING : 0);
    }

    void sandbox_c::init_resource()
    {
        // Initialize ground
        model::Model* model_ = model::ModelManagerHandle::get().create_from_file("../data/models/ground_19.obj");
        m_ground.set_model(model_);
        model_->set_debug_object_name("ground_19");

        // Initialize house
        model_ = model::ModelManagerHandle::get().create_from_file("../data/models/house.obj");
        m_house.set_model(model_);
        model_->set_debug_object_name("house");

        // House bounding box
        DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(0.15f, 0.015f, 0.015f);
        DirectX::BoundingBox house_box = m_house.get_model()->bounding_box;
        house_box.Transform(house_box, scale);
        // Tie
        transform_c& house_transform = m_house.get_transform();
        house_transform.set_scale(0.015f, 0.015f, 0.015f);
        house_transform.set_position(0.0f, -(house_box.Center.y - house_box.Extents.y + 1.0f), 0.0f);

        // Initialize camera
        auto camera = std::make_shared<third_person_camera_c>();
        m_camera = camera;

        camera->set_viewport(0.0f, 0.0f, (float)m_client_width, (float)m_client_height);
        camera->set_target(DirectX::XMFLOAT3(0.0f, 0.5f, 0.0f));
        camera->set_distance(15.0f);
        camera->set_distance_min_max(6.0f, 100.0f);
        camera->set_rotation_x(DirectX::XM_PIDIV4);
        camera->set_frustum(DirectX::XM_PI / 3, get_aspect_ratio(), 1.0f, 1600.0f);

        m_basic_effect.set_world_matrix(DirectX::XMMatrixIdentity());
        m_basic_effect.set_view_matrix(camera->get_view_xm());
        m_basic_effect.set_proj_matrix(camera->get_proj_xm());
        m_basic_effect.set_eye_pos(camera->get_position());

        // Never change
        // Lights
        DirectionalLight dir_light{};
        dir_light.ambient = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
        dir_light.diffuse = DirectX::XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
        dir_light.specular = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
        dir_light.direction = DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f);
        m_basic_effect.set_dir_light(0, dir_light);

        PointLight point_light{};
        point_light.position = DirectX::XMFLOAT3(0.0f, 20.0f, 0.0f);
        point_light.ambient = DirectX::XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
        point_light.diffuse = DirectX::XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
        point_light.specular = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
        point_light.att = DirectX::XMFLOAT3(0.0f, 0.1f, 0.0f);
        point_light.range = 30.0f;
        m_basic_effect.set_point_light(0, point_light);
    }
}






































