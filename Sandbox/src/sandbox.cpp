//
// Created by ZZK on 2023/5/17.
//

#include <Sandbox/sandbox.h>
#include <wincodec.h>
#include <IconsFontAwesome6.h>

#include <ScreenGrab/ScreenGrab11.h>

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
        // Initialize scene rendering texture
        m_lit_texture = std::make_unique<Texture2D>(m_d3d_device.Get(), m_client_width, m_client_height,
                    DXGI_FORMAT_R8G8B8A8_UNORM,1, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS);
        m_lit_texture->set_debug_object_name("LitTexture");
        // Initialize temporary texture
        m_temp_texture = std::make_unique<Texture2D>(m_d3d_device.Get(), m_client_width, m_client_height,
                    DXGI_FORMAT_R8G8B8A8_UNORM, 1, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS);
        m_temp_texture->set_debug_object_name("TempTexture");

        // Initialize texture manager and model manager
        model::TextureManagerHandle::get().init(m_d3d_device.Get());
        model::ModelManagerHandle::get().init(m_d3d_device.Get());

        // Initialize render states
        RenderStates::init(m_d3d_device.Get());
        // Initialize effect
        m_basic_effect.init(m_d3d_device.Get());
        m_skybox_effect.init(m_d3d_device.Get());
        m_post_process_effect.init(m_d3d_device.Get());
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

            m_lit_texture.reset();
            m_lit_texture = std::make_unique<Texture2D>(m_d3d_device.Get(), m_client_width, m_client_height,
                            DXGI_FORMAT_R8G8B8A8_UNORM,1, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS);
            m_lit_texture->set_debug_object_name("LitTexture");

            m_temp_texture.reset();
            m_temp_texture = std::make_unique<Texture2D>(m_d3d_device.Get(), window_resize_event.window_width, window_resize_event.window_height,
                            DXGI_FORMAT_R8G8B8A8_UNORM, 1, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS);
            m_temp_texture->set_debug_object_name("TempTexture");

            if (m_camera != nullptr)
            {
                m_camera->set_frustum(DirectX::XM_PI / 3.0f, get_aspect_ratio(), 0.5f, 1600.0f);
                m_camera->set_viewport(0.0f, 0.0f, float(m_client_width), float(m_client_height));
                m_basic_effect.set_proj_matrix(m_camera->get_proj_xm());
                m_skybox_effect.set_proj_matrix(m_camera->get_proj_xm());
            }
        });
    }

    void sandbox_c::update_scene(float dt)
    {
        ImVec2 mouse_pos = ImGui::GetMousePos();
        ImGuiIO& io = ImGui::GetIO();
        static int32_t mouse_status = 0;
        if (DX_INPUT::is_mouse_button_pressed(m_glfw_window, mouse::ButtonRight))
        {
            if (mouse_pos.x >= m_debug_texture_xy.x && mouse_pos.x < m_debug_texture_xy.x + m_debug_texture_wh.x &&
                mouse_pos.y >= m_debug_texture_xy.y && mouse_pos.y < m_debug_texture_xy.y + m_debug_texture_wh.y)
            {
                mouse_status = 1;
            } else
            {
                mouse_status = 0;
            }
        }

        if (mouse_status == 1)
        {
            float yaw = 0.0f, pitch = 0.0f;
            if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
            {

                yaw += io.MouseDelta.x * 0.015f;
                pitch += io.MouseDelta.y * 0.015f;
            }
            m_debug_camera->rotate_y(yaw);
            m_debug_camera->pitch(pitch);
        } else
        {
            m_camera_controller.update(dt, m_glfw_window);
        }

        // Update view matrix and eye position that change every frame
        m_basic_effect.set_view_matrix(m_camera->get_view_xm());
        m_basic_effect.set_eye_pos(m_camera->get_position());
        m_skybox_effect.set_view_matrix(m_camera->get_view_xm());

        if (ImGui::Begin("Dynamic Cube Mapping"))
        {
            static auto sphere_item = static_cast<int32_t>(m_sphere_mode);
            static const char* sphere_modes[3] = {
                "None",
                "Reflection",
                "Refraction"
            };
            if (ImGui::Combo( "Sphere Mode", &sphere_item, sphere_modes, ARRAYSIZE(sphere_modes)))
            {
                m_sphere_mode = static_cast<SphereMode>(sphere_item);
            }
            if (sphere_item == 2)
            {
                if (ImGui::SliderFloat("Eta", &m_eta, 0.2f, 1.0f))
                {
                    m_basic_effect.set_refraction_eta(m_eta);
                }
            }
        }
        ImGui::End();

        if (ImGui::Begin("Gaussian-Sobel Filter"))
        {
            static int32_t mode = m_blur_mode;
            static const char* blur_modes[2] = {
                "Sobel Mode",
                "Gaussian Mode"
            };
            if (ImGui::Combo("Mode", &mode, blur_modes, ARRAYSIZE(blur_modes)))
            {
                m_blur_mode = mode;
            }
            if (m_blur_mode)
            {
                if (ImGui::SliderInt(ICON_FA_SLIDERS " Blur Radius", &m_blur_radius, 1, 15))
                {
                    m_post_process_effect.set_blur_kernel_size(m_blur_radius * 2 + 1);
                }
                if (ImGui::SliderFloat(ICON_FA_SLIDERS " Blur Sigma", &m_blur_sigma, 1.0f, 20.0f))
                {
                    m_post_process_effect.set_blur_sigma(m_blur_sigma);
                }
                ImGui::SliderInt(ICON_FA_SLIDERS " Blur Times", &m_blur_times, 0, 5);
            }
        }
        ImGui::End();

        if (ImGui::Begin("Screenshot"))
        {
            if (ImGui::Button(ICON_FA_TABLET_SCREEN_BUTTON))
            {
                m_screenshot_started = true;
            }
        }
        ImGui::End();

        // Set sphere animations
        m_sphere_rad += 2.0f * dt;
        for (size_t i = 0; i < 4; ++i)
        {
            auto&& transform = m_spheres[i].get_transform();
            auto pos = transform.get_position();
            pos.y = 0.5f * std::sin(m_sphere_rad);
            transform.set_position(pos);
        }
        m_spheres[4].get_transform().rotate_around(DirectX::XMFLOAT3{}, DirectX::XMFLOAT3{0.0f, 1.0f, 0.0f}, 2.0f * dt);
    }

    void sandbox_c::draw_scene()
    {
        using namespace DirectX;
        // Create render target views of back buffers
        if (m_frame_count < m_back_buffer_count)
        {
            com_ptr<ID3D11Texture2D> back_buffer = nullptr;
            m_swap_chain->GetBuffer(0, IID_PPV_ARGS(back_buffer.GetAddressOf()));
            CD3D11_RENDER_TARGET_VIEW_DESC rtv_desc{ D3D11_RTV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB };
            m_d3d_device->CreateRenderTargetView(back_buffer.Get(), &rtv_desc, m_render_target_views[m_frame_count].ReleaseAndGetAddressOf());
        }

        // Generate dynamic sky box
        static const std::array<XMFLOAT3, 6> ups = {
            XMFLOAT3{ 0.0f, 1.0f, 0.0f },	 // +X
            XMFLOAT3{ 0.0f, 1.0f, 0.0f },   // -X
            XMFLOAT3{ 0.0f, 0.0f, -1.0f },  // +Y
            XMFLOAT3{ 0.0f, 0.0f, 1.0f },   // -Y
            XMFLOAT3{ 0.0f, 1.0f, 0.0f },   // +Z
            XMFLOAT3{ 0.0f, 1.0f, 0.0f }    // -Z
        };

        static const std::array<XMFLOAT3, 6> looks = {
            XMFLOAT3{ 1.0f, 0.0f, 0.0f },	// +X
            XMFLOAT3{ -1.0f, 0.0f, 0.0f }, // -X
            XMFLOAT3{ 0.0f, 1.0f, 0.0f },	// +Y
            XMFLOAT3{ 0.0f, -1.0f, 0.0f }, // -Y
            XMFLOAT3{ 0.0f, 0.0f, 1.0f },	// +Z
            XMFLOAT3{ 0.0f, 0.0f, -1.0f }, // -Z
        };

        BoundingFrustum frustum{};
        BoundingFrustum::CreateFromMatrix(frustum, m_camera->get_proj_xm());
        frustum.Transform(frustum, m_camera->get_local_to_world_xm());

        // Center sphere
        m_center_sphere.frustum_culling(frustum);
        m_basic_effect.set_eye_pos(m_center_sphere.get_transform().get_position());
        if (m_center_sphere.in_frustum())
        {
            // Draw every surface, take sphere as center
            for (size_t i = 0; i < looks.size(); ++i)
            {
                m_cube_camera->look_to(m_center_sphere.get_transform().get_position(), looks[i], ups[i]);
                // Do not draw center sphere
                draw(false, *m_cube_camera, m_dynamic_texture_cube->get_render_target(i), m_dynamic_cube_depth_texture->get_depth_stencil());
            }
        }

        // Draw scene to scene rendering buffer
        draw(true, *m_camera, m_lit_texture->get_render_target(), m_depth_texture->get_depth_stencil());
        // Note: clear render target view and depth stencil view
        m_d3d_immediate_context->OMSetRenderTargets(0, nullptr, nullptr);

        // Filter operation
        //// Gaussian filter
        if (m_blur_mode == 1)
        {
            for (int32_t i = 0; i < m_blur_times; ++i)
            {
                m_post_process_effect.compute_gaussian_blur_x(m_d3d_immediate_context.Get(),
                                    m_lit_texture->get_shader_resource(),
                                    m_temp_texture->get_unordered_access(),
                                    m_client_width, m_client_height);
                m_post_process_effect.compute_gaussian_blur_y(m_d3d_immediate_context.Get(),
                                    m_temp_texture->get_shader_resource(),
                                    m_lit_texture->get_unordered_access(),
                                    m_client_width, m_client_height);
            }
            // RGB to sRGB
            m_post_process_effect.render_composite(m_d3d_immediate_context.Get(),
                                    m_lit_texture->get_shader_resource(), nullptr,
                                    get_back_buffer_rtv(),
                                    m_camera->get_viewport());
        }
        //// Sobel filter
        else
        {
            m_post_process_effect.compute_sobel(m_d3d_immediate_context.Get(),
                                                m_lit_texture->get_shader_resource(),
                                                m_temp_texture->get_unordered_access(),
                                                m_client_width, m_client_height);
            m_post_process_effect.render_composite(m_d3d_immediate_context.Get(),
                                                m_lit_texture->get_shader_resource(),
                                                m_temp_texture->get_shader_resource(),
                                                get_back_buffer_rtv(),
                                                m_camera->get_viewport());
        }

        // Note: bind to back buffer rtv to present
        auto rtv = get_back_buffer_rtv();
        m_d3d_immediate_context->OMSetRenderTargets(1, &rtv, nullptr);

        // Draw skybox
        static bool debug_cube = false;
        if (ImGui::Begin("Dynamic Cube Mapping"))
        {
            ImGui::Checkbox("Debug Cube", &debug_cube);
        }
        ImGui::End();

        m_debug_texture_xy = {};
        m_debug_texture_wh = {};
        if (debug_cube)
        {
            if (ImGui::Begin(ICON_FA_SCISSORS " Debug"))
            {
                D3D11_VIEWPORT viewport = m_debug_camera->get_viewport();
                ID3D11RenderTargetView* rtvs[1] = { m_debug_dynamic_cube_texture->get_render_target() };
                m_d3d_immediate_context->RSSetViewports(1, &viewport);
                m_d3d_immediate_context->OMSetRenderTargets(1, rtvs, nullptr);
                m_skybox_effect.set_default_render();
                m_skybox_effect.set_view_matrix(m_debug_camera->get_view_xm());
                m_skybox_effect.set_proj_matrix(m_debug_camera->get_proj_xm());
                m_debug_skybox.draw(m_d3d_immediate_context.Get(), m_skybox_effect);
                // Note: clear after drawing
                ID3D11ShaderResourceView* null_srvs[3]{};
                m_d3d_immediate_context->PSSetShaderResources(0, 3, null_srvs);
                // Note: restore
                viewport = m_camera->get_viewport();
                rtvs[0] = get_back_buffer_rtv();
                m_d3d_immediate_context->RSSetViewports(1, &viewport);
                m_d3d_immediate_context->OMSetRenderTargets(1, rtvs, nullptr);

                ImVec2 win_size = ImGui::GetWindowSize();
                float smaller = (std::min)(win_size.x - 20.0f, win_size.y - 36.0f);
                ImGui::Image(m_debug_dynamic_cube_texture->get_shader_resource(), ImVec2{ smaller, smaller });
                m_debug_texture_xy = ImGui::GetItemRectMin();
                m_debug_texture_wh = { smaller, smaller };
            }
            ImGui::End();
        }

        // Screenshot
        if (m_screenshot_started)
        {
            com_ptr<ID3D11Texture2D> back_buffer = nullptr;
            get_back_buffer_rtv()->GetResource(reinterpret_cast<ID3D11Resource**>(back_buffer.GetAddressOf()));
            // Output screenshot
            DirectX::SaveWICTextureToFile(m_d3d_immediate_context.Get(), back_buffer.Get(), GUID_ContainerFormatJpeg,
                                            L"./output.jpg", &GUID_WICPixelFormat24bppBGR);
            m_screenshot_started = false;
        }

        // Note: present function is called in application
    }

    void sandbox_c::init_resource()
    {
        // Initialize skybox associated
        com_ptr<ID3D11Texture2D> tex = nullptr;
        D3D11_TEXTURE2D_DESC tex_desc{};
        std::string file_name_str{};
        std::vector<ID3D11ShaderResourceView*> cube_textures;
        std::unique_ptr<TextureCube> tex_cube;
        // Daylight
        {
            file_name_str = "../data/textures/daylight0.png";
            cube_textures.reserve(6);
            for (size_t i = 0; i < 6; ++i)
            {
                auto pos = file_name_str.find_last_of('.');
                file_name_str[pos - 1] = '0' + char(i);
                cube_textures.push_back(model::TextureManagerHandle::get().create_from_file(file_name_str, false, 1));
            }
            cube_textures[0]->GetResource(reinterpret_cast<ID3D11Resource**>(tex.ReleaseAndGetAddressOf()));
            tex->GetDesc(&tex_desc);
            tex_cube = std::make_unique<TextureCube>(m_d3d_device.Get(), tex_desc.Width, tex_desc.Height, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
            tex_cube->set_debug_object_name("Daylight");
            for (uint32_t i = 0; i < 6; ++i)
            {
                cube_textures[i]->GetResource(reinterpret_cast<ID3D11Resource**>(tex.ReleaseAndGetAddressOf()));
                m_d3d_immediate_context->CopySubresourceRegion(tex_cube->get_texture(), D3D11CalcSubresource(0, i, 1),
                                                                0, 0, 0, tex.Get(), 0, nullptr);
            }
            model::TextureManagerHandle::get().add_texture("Daylight", tex_cube->get_shader_resource());
        }

        // Dynamic skybox
        m_dynamic_texture_cube = std::make_unique<TextureCube>(m_d3d_device.Get(), 256, 256, DXGI_FORMAT_R8G8B8A8_UNORM);
        m_dynamic_cube_depth_texture = std::make_unique<Depth2D>(m_d3d_device.Get(), 256, 256);
        m_debug_dynamic_cube_texture = std::make_unique<Texture2D>(m_d3d_device.Get(), 256, 256, DXGI_FORMAT_R8G8B8A8_UNORM);
        model::TextureManagerHandle::get().add_texture("DynamicCube", m_dynamic_texture_cube->get_shader_resource());

        m_dynamic_texture_cube->set_debug_object_name("DynamicTextureCube");
        m_dynamic_cube_depth_texture->set_debug_object_name("DynamicCubeDepthTexture");
        m_debug_dynamic_cube_texture->set_debug_object_name("DebugDynamicCube");

        // Initialize render objects
        // Sphere
        using namespace DirectX;
        {
            model::Model* model = model::ModelManagerHandle::get().create_from_geometry("Sphere", geometry::create_sphere());
            model->set_debug_object_name("Sphere");
            model::TextureManagerHandle::get().create_from_file("../data/textures/stone.dds");
            model->materials[0].set<std::string>("$Diffuse", "../data/textures/stone.dds");
            model->materials[0].set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            model->materials[0].set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f));
            model->materials[0].set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f));
            model->materials[0].set<float>("$SpecularPower", 16.0f);
            model->materials[0].set<XMFLOAT4>("$ReflectColor", XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f));

            std::vector<transform_c> sphere_transforms = {
                transform_c(XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT3(), XMFLOAT3(4.5f, 0.0f, 4.5f)),
                transform_c(XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT3(), XMFLOAT3(-4.5f, 0.0f, 4.5f)),
                transform_c(XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT3(), XMFLOAT3(-4.5f, 0.0f, -4.5f)),
                transform_c(XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT3(), XMFLOAT3(4.5f, 0.0f, -4.5f)),
                transform_c(XMFLOAT3(0.5f, 0.5f, 0.5f), XMFLOAT3(), XMFLOAT3(2.5f, 0.0f, 0.0f)),
            };

            for (size_t i = 0; i < sphere_transforms.size(); ++i)
            {
                m_spheres[i].get_transform() = sphere_transforms[i];
                m_spheres[i].set_model(model);
            }
            m_center_sphere.set_model(model);
        }
        // Ground
        {
            model::Model* model = model::ModelManagerHandle::get().create_from_geometry("Ground",
                geometry::create_plane(XMFLOAT2(10.0f, 10.0f), XMFLOAT2(5.0f, 5.0f)));
            model->set_debug_object_name("Ground");
            model::TextureManagerHandle::get().create_from_file("../data/textures/floor.dds");
            model->materials[0].set<std::string>("$Diffuse", "../data/textures/floor.dds");
            model->materials[0].set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            model->materials[0].set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f));
            model->materials[0].set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            model->materials[0].set<float>("$SpecularPower", 16.0f);
            model->materials[0].set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
            m_ground.set_model(model);
            m_ground.get_transform().set_position(0.0f, -3.0f, 0.0f);
        }
        // Cylinder
        {
            model::Model* model = model::ModelManagerHandle::get().create_from_geometry("Cylinder",
                geometry::create_cylinder(0.5f, 2.0f));
            model->set_debug_object_name("Cylinder");
            model::TextureManagerHandle::get().create_from_file("../data/textures/bricks.dds");
            model->materials[0].set<std::string>("$Diffuse", "../data/textures/bricks.dds");
            model->materials[0].set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            model->materials[0].set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f));
            model->materials[0].set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
            model->materials[0].set<float>("$SpecularPower", 16.0f);
            model->materials[0].set<XMFLOAT4>("$ReflectColor", XMFLOAT4());

            // Fix position
            std::vector<transform_c> cylinder_transforms = {
                transform_c(XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(), XMFLOAT3(0.0f, -1.99f, 0.0f)),
                transform_c(XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(), XMFLOAT3(4.5f, -1.99f, 4.5f)),
                transform_c(XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(), XMFLOAT3(-4.5f, -1.99f, 4.5f)),
                transform_c(XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(), XMFLOAT3(-4.5f, -1.99f, -4.5f)),
                transform_c(XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(), XMFLOAT3(4.5f, -1.99f, -4.5f)),
            };

            for (size_t i = 0; i < cylinder_transforms.size(); ++i)
            {
                m_cylinders[i].set_model(model);
                m_cylinders[i].get_transform() = cylinder_transforms[i];
            }
        }
        // Skybox cube
        {
            model::Model* model = model::ModelManagerHandle::get().create_from_geometry("Skybox",
                geometry::create_box());
            model->set_debug_object_name("Skybox");
            model->materials[0].set<std::string>("$Skybox", "Daylight");
            m_skybox.set_model(model);
        }
        // Debug cube
        {
            model::Model* model = model::ModelManagerHandle::get().create_from_geometry("DebugSkybox",
                geometry::create_box());
            model->set_debug_object_name("DebugSkybox");
            model->materials[0].set<std::string>("$Skybox", "DynamicCube");
            m_debug_skybox.set_model(model);
        }

        // Initialize camera
        m_camera = std::make_shared<first_person_camera_c>();
        m_camera_controller.init(m_camera.get());
        m_camera->set_viewport(0.0f, 0.0f, (float)m_client_width, (float)m_client_height);
        m_camera->set_frustum(XM_PI / 3, get_aspect_ratio(), 1.0f, 1600.0f);
        m_camera->look_to(XMFLOAT3(0.0f, 0.0f, -10.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));

        m_cube_camera = std::make_shared<first_person_camera_c>();
        m_cube_camera->set_frustum(XM_PIDIV2, 1.0f, 0.1f, 1000.0f);
        m_cube_camera->set_viewport(0.0f, 0.0f, 256.0f, 256.0f);

        m_debug_camera = std::make_shared<first_person_camera_c>();
        m_debug_camera->set_frustum(XM_PIDIV2, 1.0f, 0.1f, 1000.0f);
        m_debug_camera->set_viewport(0.0f, 0.0f, 256.0f, 256.0f);

        // Initialize values that never change
        std::vector<DirectionalLight> dir_lights(4);
        dir_lights[0].ambient = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);
        dir_lights[0].diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
        dir_lights[0].specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
        dir_lights[0].direction = XMFLOAT3(-0.577f, -0.577f, 0.577f);
        dir_lights[1] = dir_lights[0];
        dir_lights[1].direction = XMFLOAT3(0.577f, -0.577f, 0.577f);
        dir_lights[2] = dir_lights[0];
        dir_lights[2].direction = XMFLOAT3(0.577f, -0.577f, -0.577f);
        dir_lights[3] = dir_lights[0];
        dir_lights[3].direction = XMFLOAT3(-0.577f, -0.577f, -0.577f);
        for (int i = 0; i < dir_lights.size(); ++i)
        {
            m_basic_effect.set_dir_light(i, dir_lights[i]);
        }

        // Initialize post process effect
        m_post_process_effect.set_blur_kernel_size(m_blur_radius * 2 + 1);
        m_post_process_effect.set_blur_sigma(m_blur_sigma);
    }

    void sandbox_c::draw(bool draw_center_sphere, const toy::camera_c &camera,
                                ID3D11RenderTargetView *rtv, ID3D11DepthStencilView *dsv)
    {
        using namespace DirectX;
        static float color[4] = { 0.1f, 0.1f, 0.1f, 1.0f };

        m_d3d_immediate_context->ClearRenderTargetView(rtv, color);
        m_d3d_immediate_context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        m_d3d_immediate_context->OMSetRenderTargets(1, &rtv, dsv);

        BoundingFrustum frustum{};
        BoundingFrustum::CreateFromMatrix(frustum, camera.get_proj_xm());
        frustum.Transform(frustum, camera.get_local_to_world_xm());
        D3D11_VIEWPORT viewport = camera.get_viewport();
        m_d3d_immediate_context->RSSetViewports(1, &viewport);

        // Draw objects
        m_basic_effect.set_view_matrix(camera.get_view_xm());
        m_basic_effect.set_proj_matrix(camera.get_proj_xm());
        m_basic_effect.set_eye_pos(camera.get_position());
        m_basic_effect.set_default_render();

        // Only center sphere has reflection or refraction property
        if (draw_center_sphere)
        {
            switch (m_sphere_mode)
            {
                case SphereMode::None:
                {
                    m_basic_effect.set_reflection_enabled(false);
                    m_basic_effect.set_refraction_enabled(false);
                    break;
                }
                case SphereMode::Reflection:
                {
                    m_basic_effect.set_reflection_enabled(true);
                    m_basic_effect.set_refraction_enabled(false);
                    break;
                }
                case SphereMode::Refraction:
                {
                    m_basic_effect.set_reflection_enabled(false);
                    m_basic_effect.set_refraction_enabled(true);
                    break;
                }
            }
            m_basic_effect.set_texture_cube(m_dynamic_texture_cube->get_shader_resource());
            m_center_sphere.draw(m_d3d_immediate_context.Get(), m_basic_effect);
            m_basic_effect.set_texture_cube(nullptr);
        }

        // Draw ground
        m_basic_effect.set_reflection_enabled(false);
        m_basic_effect.set_refraction_enabled(false);

        m_ground.draw(m_d3d_immediate_context.Get(), m_basic_effect);

        // Draw five cylinders
        for (auto&& cylinder : m_cylinders)
        {
            cylinder.frustum_culling(frustum);
            cylinder.draw(m_d3d_immediate_context.Get(), m_basic_effect);
        }

        // Draw five spheres
        for (auto&& sphere : m_spheres)
        {
            sphere.frustum_culling(frustum);
            sphere.draw(m_d3d_immediate_context.Get(), m_basic_effect);
        }

        // Draw skybox
        m_skybox_effect.set_view_matrix(camera.get_view_xm());
        m_skybox_effect.set_proj_matrix(camera.get_proj_xm());
        m_skybox_effect.set_default_render();
        m_skybox.draw(m_d3d_immediate_context.Get(), m_skybox_effect);
    }
}






































