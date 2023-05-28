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

        // Initialize render states
        RenderStates::init(class_d3d_device_.Get());
        // Initialize basic effect
        class_basic_effect.init(class_d3d_device_.Get());
        // Initialize resource
        init_resource();

        // Change projection matrix when window resizes
        event_manager_c::subscribe(event_type_e::WindowResize, [this] (const event_t& event)
        {
            if (class_camera != nullptr)
            {
                class_camera->set_frustum(DirectX::XM_PI / 3.0f, get_aspect_ratio(), 0.5f, 1600.0f);
                class_camera->set_viewport(0.0f, 0.0f, (float)class_client_width_, (float)class_client_height_);
                class_basic_effect.set_proj_matrix(class_camera->get_proj_xm());
            }
        });
    }

    void sandbox_c::update_scene(float dt)
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

        DirectX::XMFLOAT3 target = class_bolt_anim.get_transform().get_position();
        cam_third->set_target(target);
        // Rotate around specific object
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
        {
            cam_third->rotate_x(io.MouseDelta.y * 0.01f);
            cam_third->rotate_y(io.MouseDelta.x * 0.01f);
        }
        cam_third->approach(-io.MouseWheel * 1.0f);

        // Update view matrix and eye position that change every frame
        class_basic_effect.set_view_matrix(class_camera->get_view_xm());
        class_basic_effect.set_eye_pos(class_camera->get_position());

        // Update bolt animation
        static uint32_t cur_bolt_frame = 0;
        static float frame_time = 0.0f;
        static float frame_in_s = 1.0f / 60.0f;
        class_bolt_anim.set_texture(class_bolt_srvs[cur_bolt_frame].Get());
        if (frame_time > frame_in_s)
        {
            cur_bolt_frame = (cur_bolt_frame + 1) % 60;
            frame_time -= frame_in_s;
        }
        frame_time += dt;
    }

    void sandbox_c::draw_scene()
    {
        assert(class_d3d_immediate_context_);
        assert(class_swap_chain_);

        static float color[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
        class_d3d_immediate_context_->ClearRenderTargetView(class_render_target_view_.Get(), color);
        class_d3d_immediate_context_->ClearDepthStencilView(class_depth_stencil_view_.Get(),
                                                                D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

        // 1. Write a stencil value 1 to the stencil buffer for the specular reflection area
        class_basic_effect.set_write_stencil_only_render(class_d3d_immediate_context_.Get(), 1);
        class_mirror.draw(class_d3d_immediate_context_.Get(), class_basic_effect);

        // 2. Draw opaque reflection objects
        class_basic_effect.set_reflection_state(true);      // Enable reflection
        class_basic_effect.set_default_with_stencil_render(class_d3d_immediate_context_.Get(), 1);

        class_walls[2].draw(class_d3d_immediate_context_.Get(), class_basic_effect);
        class_walls[3].draw(class_d3d_immediate_context_.Get(), class_basic_effect);
        class_walls[4].draw(class_d3d_immediate_context_.Get(), class_basic_effect);
        class_floor.draw(class_d3d_immediate_context_.Get(), class_basic_effect);
        class_wood_crate.draw(class_d3d_immediate_context_.Get(), class_basic_effect);

        // 3. Draw shadow of opaque reflection object
        class_wood_crate.set_material(class_shadow_mat);
        class_basic_effect.set_shadow_state(true);      // Enable shadow
        class_basic_effect.set_no_double_blend_render(class_d3d_immediate_context_.Get(), 1);

        class_wood_crate.draw(class_d3d_immediate_context_.Get(), class_basic_effect);

        // Restore state of opaque reflection object
        class_basic_effect.set_shadow_state(false);
        class_wood_crate.set_material(class_wood_mat);

        // 4. Draw blending reflection bolt animation and mirror
        class_basic_effect.set_no_depth_write_with_stencil_render(class_d3d_immediate_context_.Get(), 1);
        class_bolt_anim.draw(class_d3d_immediate_context_.Get(), class_basic_effect);

        class_basic_effect.set_reflection_state(false);     // Disable reflection
        class_basic_effect.set_alpha_blend_with_stencil_render(class_d3d_immediate_context_.Get(), 1);
        class_mirror.draw(class_d3d_immediate_context_.Get(), class_basic_effect);

        // 5. Draw normal opaque objects
        class_basic_effect.set_default_render(class_d3d_immediate_context_.Get());

        for (auto&& wall : class_walls)
        {
            wall.draw(class_d3d_immediate_context_.Get(), class_basic_effect);
        }
        class_floor.draw(class_d3d_immediate_context_.Get(), class_basic_effect);
        class_wood_crate.draw(class_d3d_immediate_context_.Get(), class_basic_effect);

        // 6. Draw shadow of normal opaque object
        class_wood_crate.set_material(class_shadow_mat);
        class_basic_effect.set_shadow_state(true);      // Enable shadow
        class_basic_effect.set_no_double_blend_render(class_d3d_immediate_context_.Get(), 0);

        class_wood_crate.draw(class_d3d_immediate_context_.Get(), class_basic_effect);

        class_basic_effect.set_shadow_state(false);
        class_wood_crate.set_material(class_wood_mat);

        // 7. Draw blending bolt animation
        class_basic_effect.set_no_depth_write_render(class_d3d_immediate_context_.Get());
        class_bolt_anim.draw(class_d3d_immediate_context_.Get(), class_basic_effect);

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

    void sandbox_c::init_resource()
    {
        // Initialize material
        Material material{
            DirectX::XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f),
            DirectX::XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f),
            DirectX::XMFLOAT4(0.1f, 0.1f, 0.1f, 16.0f)
        };

        class_wood_mat = material;
        class_shadow_mat = Material{
            DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f),
            DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.5f),
            DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 16.0f)
        };

        // Initialize bolt textures and bolt
        class_bolt_srvs.assign(60, nullptr);
        for (uint32_t i = 1; i <= 60; ++i)
        {
            std::wstring file_str{};
            if (i < 10)
            {
                file_str = L"../data/textures/BoltAnim/Bolt00" + std::to_wstring(i) + L".bmp";
            } else
            {
                file_str = L"../data/textures/BoltAnim/Bolt0" + std::to_wstring(i) + L".bmp";
            }
            DirectX::CreateWICTextureFromFile(class_d3d_device_.Get(), file_str.c_str(), nullptr,
                                                class_bolt_srvs[static_cast<size_t>(i - 1)].GetAddressOf());
        }
        class_bolt_anim.set_buffer(class_d3d_device_.Get(), geometry::create_sphere<VertexPosNormalTex, uint32_t>(4.0f));
        class_bolt_anim.set_material(material);
        class_bolt_anim.get_transform().set_position(0.0f, 2.01f, 0.0f);

        // Initialize objects
        com_ptr<ID3D11ShaderResourceView> texture = nullptr;
        DirectX::CreateDDSTextureFromFile(class_d3d_device_.Get(), L"../data/textures/WoodCrate.dds", nullptr, texture.GetAddressOf());
        class_wood_crate.set_buffer(class_d3d_device_.Get(), geometry::create_box<VertexPosNormalTex, uint32_t>());
        class_wood_crate.set_texture(texture.Get());
        class_wood_crate.set_material(material);
        class_wood_crate.get_transform().set_position(0.0f, 0.01f, 0.0f);

        DirectX::CreateDDSTextureFromFile(class_d3d_device_.Get(), L"../data/textures/floor.dds", nullptr, texture.ReleaseAndGetAddressOf());
        class_floor.set_buffer(class_d3d_device_.Get(),
                                geometry::create_plane<VertexPosNormalTex, uint32_t>(DirectX::XMFLOAT2(20.0f, 20.0f), DirectX::XMFLOAT2(5.0f, 5.0f)));
        class_floor.set_texture(texture.Get());
        class_floor.set_material(material);
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
            class_walls[i].set_material(material);
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

        material.ambient = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
        material.diffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
        material.specular = DirectX::XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);
        DirectX::CreateDDSTextureFromFile(class_d3d_device_.Get(), L"../data/textures/ice.dds", nullptr, texture.ReleaseAndGetAddressOf());
        class_mirror.set_buffer(class_d3d_device_.Get(),
                            geometry::create_plane<VertexPosNormalTex, uint32_t>(DirectX::XMFLOAT2(8.0f, 8.0f), DirectX::XMFLOAT2(1.0f, 1.0f)));
        class_mirror.set_material(material);
        class_mirror.set_texture(texture.Get());
        class_mirror.get_transform().set_rotation(-DirectX::XM_PIDIV2, 0.0f, 0.0f);
        class_mirror.get_transform().set_position(0.0f, 3.0f, 10.0f);

        // Create constant buffer that changes every frame
        auto camera = std::make_shared<third_person_camera_c>();
        camera->set_viewport(0.0f, 0.0f, (float)class_client_width_, (float)class_client_height_);
        camera->set_target(class_bolt_anim.get_transform().get_position());
        camera->set_distance(5.0f);
        camera->set_distance_min_max(2.0f, 14.0f);
        camera->set_rotation_x(DirectX::XM_PIDIV2);
        class_camera = camera;

        class_basic_effect.set_view_matrix(class_camera->get_view_xm());
        class_basic_effect.set_eye_pos(class_camera->get_position());

        // Create constant buffer that changes when window resizes
        class_camera->set_frustum(DirectX::XM_PI / 3.0f, get_aspect_ratio(), 0.5f, 1600.0f);
        class_basic_effect.set_proj_matrix(class_camera->get_proj_xm());

        // Create constant buffer that rarely change
        class_basic_effect.set_reflection_matrix(DirectX::XMMatrixReflect(DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 10.0f)));
        //// Shadow matrix
        class_basic_effect.set_shadow_matrix(DirectX::XMMatrixShadow(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.99f),
                                                DirectX::XMVectorSet(0.0f, 10.0f, -10.0f, 1.0f)));
        class_basic_effect.set_ref_shadow_matrix(DirectX::XMMatrixShadow(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.99f),
                                                DirectX::XMVectorSet(0.0f, 10.0f, 30.0f, 1.0f)));

        // Create illuminant
        DirectionalLight dir_light{
            DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f),
            DirectX::XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f),
            DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f),
            DirectX::XMFLOAT3(0.0f, -1.0f, 0.0f),
            0.0f
        };
        class_basic_effect.set_dir_light(0, dir_light);
        PointLight point_light{
            DirectX::XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f),
            DirectX::XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f),
            DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f),
            DirectX::XMFLOAT3(0.0f, 10.0f, -10.0f),
            25.0f,
            DirectX::XMFLOAT3(0.0f, 0.1f, 0.0f),
            0.0f
        };
        class_basic_effect.set_point_light(0, point_light);

        // TODO: set debug object name
    }
}






































