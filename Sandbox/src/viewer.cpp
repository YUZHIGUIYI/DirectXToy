//
// Created by ZZK on 2023/6/25.
//

#include <Sandbox/viewer.h>
#include <Sandbox/file_dialog.h>

#include <IconsFontAwesome6.h>

#include <random>

namespace toy
{
    // Color to clear rtv
    const float s_color[4] = { 0.1f, 0.1f, 0.1f, 1.0f };

    static void show_busy_window(std::string_view busy_reason_text)
    {
        if (busy_reason_text.empty())
        {
            return;
        }

        // Display a modal window when loading assets or other long operation on separated thread
        ImGui::OpenPopup("Busy Info");

        // Position in the center of the main window when appearing
        const ImVec2 win_size{ 300.0f, 75.0f };
        ImGui::SetNextWindowSize(win_size);
        const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2{ 0.5f, 0.5f });

        // Window without any decoration
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 15.0f);
        if (ImGui::BeginPopupModal("Busy Info", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration))
        {
            // Center text in window
            const ImVec2 available = ImGui::GetContentRegionAvail();
            const ImVec2 text_size = ImGui::CalcTextSize(busy_reason_text.data(), nullptr, false, available.x);

            ImVec2 pos{ (available.x - text_size.x) * 0.5f, (available.y - text_size.y) * 0.2f };

            ImGui::SetCursorPosX(pos.x);
            ImGui::Text("%s", busy_reason_text.data());

            // Animation
            ImGui::SetCursorPosX(available.x * 0.5f);
            ImGui::Text("%c", "|/-\\"[static_cast<int>(ImGui::GetTime() / 0.25f) & 3]);
            ImGui::EndPopup();
        }
        ImGui::PopStyleVar();
    }

    Viewer::Viewer(std::string_view viewer_name)
    : m_viewer_name(viewer_name)
    {

    }

    bool Viewer::is_viewer_size_changed()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin(m_viewer_name.data());
        ImVec2 viewport_size         = ImGui::GetContentRegionAvail();
        bool viewport_size_changed = (static_cast<int32_t>(viewport_size.x) != m_viewer_spec.width
                                        || static_cast<int32_t>(viewport_size.y) != m_viewer_spec.height);
        ImGui::End();
        ImGui::PopStyleVar();

        if (viewport_size_changed)
        {
            m_viewer_spec.width = static_cast<int32_t>(viewport_size.x);
            m_viewer_spec.height = static_cast<int32_t>(viewport_size.y);
        }

        return viewport_size_changed;
    }

    void Viewer::on_attach(toy::d3d_application_c *app)
    {
        m_d3d_app = app;

        // Initialize texture manager and model manager
        auto d3d_device = m_d3d_app->get_device();
        model::TextureManagerHandle::get().init(d3d_device);
        model::ModelManagerHandle::get().init(d3d_device);

        // Initialize render states
        RenderStates::init(d3d_device);

        // Initialize effects
        m_forward_effect.init(d3d_device);
        m_deferred_effect.init(d3d_device);
        m_skybox_effect.init(d3d_device);

        // Initialize resource
        init_resource();

        // Initialize buffers
        resize_buffers(m_viewer_spec.width, m_viewer_spec.height, m_msaa_samples);
    }

    void Viewer::on_detach()
    {

    }

    void Viewer::on_resize()
    {
        using namespace DirectX;

        if (!is_viewer_size_changed())
        {
            return;
        }

        // Recreate resources
        auto width = m_viewer_spec.width;
        auto height = m_viewer_spec.height;
        auto aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
        m_camera->set_frustum(XM_PI / 3.0f, aspect_ratio, 0.5f, 300.0f);
        m_camera->set_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
        m_forward_effect.set_proj_matrix(m_camera->get_proj_xm(true));
        m_deferred_effect.set_proj_matrix(m_camera->get_proj_xm(true));
        m_forward_effect.set_camera_near_far(0.5f, 300.0f);
        m_deferred_effect.set_camera_near_far(0.5f, 300.0f);

        resize_buffers(width, height, m_msaa_samples);
    }

    void Viewer::on_ui_menu()
    {
        bool load_file = false;

        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Load", "Ctrl+O"))
            {
                load_file = true;
            }
            ImGui::Separator();
            ImGui::EndMenu();
        }

        if (m_busy) return;
        auto glfw_window = m_d3d_app->get_glfw_window();
        if (DX_INPUT::is_key_pressed(glfw_window, key::O) && DX_INPUT::is_key_pressed(glfw_window, key::LeftControl))
        {
            load_file = true;
        }

        if (load_file)
        {
            auto filename = FileDialog::window_open_file_dialog(glfw_window, "Load glTF | HDR",
                                                                "glTF(.gltf, .glb), HDR(.hdr)|*.gltf;*.glb;*.hdr");
            on_file_drop(filename);
        }
    }

    void Viewer::on_file_drop(std::string_view filename)
    {
        if (m_busy) return;

        m_busy.store(true);
        std::thread([this, filename]
        {
            const std::string extension = std::filesystem::path(filename).extension().string();
            if (extension == ".gltf" || extension == ".glb")
            {
                // TODO: create scene;
            } else if (extension == ".hdr")
            {
                // TODO: create hdr
            }

            DX_INFO("Load asset file successfully");
            m_busy.store(false);
        }).detach();
    }

    void Viewer::on_ui_render()
    {
        // Rendering settings
#pragma region IMGUI
        bool need_gpu_timer_reset = false;
        if (ImGui::Begin("Tile-Based Deferred Rendering"))
        {
            static const char* msaa_modes[] = {
                "None",
                "2x MSAA",
                "4x MSAA",
                "8x MSAA"
            };
            static int curr_msaa_item = 0;
            if (ImGui::Combo("MSAA", &curr_msaa_item, msaa_modes, ARRAYSIZE(msaa_modes)))
            {
                switch (curr_msaa_item)
                {
                    case 0: m_msaa_samples = 1; break;
                    case 1: m_msaa_samples = 2; break;
                    case 2: m_msaa_samples = 4; break;
                    case 3: m_msaa_samples = 8; break;
                }
                resize_buffers(static_cast<uint32_t>(m_viewer_spec.width), static_cast<uint32_t>(m_viewer_spec.height), m_msaa_samples);
                m_forward_effect.set_msaa_samples(m_msaa_samples);
                m_skybox_effect.set_msaa_samples(m_msaa_samples);
                m_deferred_effect.set_msaa_samples(m_msaa_samples);
                need_gpu_timer_reset = true;
            }

            static const char* light_culliing_modes[] = {
                "Forward: No Culling",
                "Forward: Pre-Z No Culling",
                "Forward+: Compute Shader Tile",
                "Deferred: No Culling",
                "Deferred: Compute Shader Tile"
            };
            static int curr_light_culling_item = static_cast<int>(m_light_cull_technique);
            if (ImGui::Combo("Light Culling", &curr_light_culling_item, light_culliing_modes, ARRAYSIZE(light_culliing_modes)))
            {
                m_light_cull_technique = static_cast<LightCullTechnique>(curr_light_culling_item);
                need_gpu_timer_reset = true;
            }

            if (ImGui::Checkbox("Animate Lights", &m_animate_lights))
            {
                need_gpu_timer_reset = true;
            }

            if (ImGui::Checkbox("Lighting Only", &m_lighting_only))
            {
                m_forward_effect.set_lighting_only(m_lighting_only);
                m_deferred_effect.set_lighting_only(m_lighting_only);
                need_gpu_timer_reset = true;
            }

            if (ImGui::Checkbox("Face Normals", &m_face_normals))
            {
                m_forward_effect.set_face_normals(m_face_normals);
                m_deferred_effect.set_face_normals(m_face_normals);
                need_gpu_timer_reset = true;
            }

            if (ImGui::Checkbox("Visualize Light Count", &m_visualize_light_count))
            {
                m_forward_effect.set_visualize_light_count(m_visualize_light_count);
                m_deferred_effect.set_visualize_light_count(m_visualize_light_count);
                need_gpu_timer_reset = true;
            }

            if (m_light_cull_technique >= LightCullTechnique::CULL_DEFERRED_NONE)
            {
                ImGui::Checkbox("Clear G-Buffers", &m_clear_gbuffers);
                if (m_msaa_samples > 1 && ImGui::Checkbox("Visualize Shading Freq", &m_visualize_shading_freq))
                {
                    m_deferred_effect.set_visualize_shading_freq(m_visualize_shading_freq);
                    need_gpu_timer_reset = true;
                }
            }

            ImGui::Text(ICON_FA_WEIGHT_SCALE  " Light Height Scale");
            ImGui::PushID(0);
            if (ImGui::SliderFloat("", &m_light_height_scale, 0.25f, 1.0f))
            {
                update_lights(0.0f);
                need_gpu_timer_reset = true;
            }
            ImGui::PopID();

            static int light_level = static_cast<int>(log2f(static_cast<float>(m_active_lights)));
            ImGui::Text(ICON_FA_LIGHTBULB " Lights: %d", m_active_lights);
            ImGui::PushID(1);
            if (ImGui::SliderInt("", &light_level, 0, (int)roundf(log2f(MAX_LIGHTS)), ""))
            {
                m_active_lights = (1 << light_level);
                resize_lights(m_active_lights);
                update_lights(0.0f);
                need_gpu_timer_reset = true;
            }
            ImGui::PopID();
        }
        ImGui::End();

#pragma endregion

        // Rendering viewport
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });
        ImGui::Begin(m_viewer_name.data());

        auto viewer_min_region = ImGui::GetWindowContentRegionMin();
        auto viewer_max_region = ImGui::GetWindowContentRegionMax();
        auto viewer_offset = ImGui::GetWindowPos();
        m_viewer_spec.lower_bound = DirectX::XMFLOAT2{ viewer_offset.x + viewer_min_region.x, viewer_offset.y + viewer_min_region.y };
        m_viewer_spec.upper_bound = DirectX::XMFLOAT2{ viewer_offset.x + viewer_max_region.x, viewer_offset.y + viewer_max_region.y };

        m_viewer_focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootWindow);
        m_viewer_hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootWindow);

        ImGui::Image(m_viewer_buffer->get_shader_resource(), ImGui::GetContentRegionAvail());

        ImGui::End();
        ImGui::PopStyleVar();

        // Rendering scene hierarchy panel
        m_scene_hierarchy_panel.on_ui_render();
    }

    void Viewer::on_render(float dt)
    {
        using namespace DirectX;

        GLFWwindow* glfw_window = m_d3d_app->get_glfw_window();
        // Update camera if viewer is focused and hovered
        if (m_viewer_focused && m_viewer_hovered)
        {
            m_camera_controller.update(dt, glfw_window);
        }

        m_forward_effect.set_view_matrix(m_camera->get_view_xm());
        m_deferred_effect.set_view_matrix(m_camera->get_view_xm());
        setup_lights(m_camera->get_view_xm());

        // Update collision
        BoundingFrustum frustum{};
        BoundingFrustum::CreateFromMatrix(frustum, m_camera->get_proj_xm());
        frustum.Transform(frustum, m_camera->get_local_to_world_xm());
//        m_sponza.frustum_culling(frustum);

        // TODO: entity
        m_editor_scene->frustum_culling(frustum);
        // TODO: end

        // Update lights
        if (m_animate_lights)
        {
            update_lights(dt);
        }

        // Rendering scene
        ID3D11Device* d3d_device = m_d3d_app->get_device();
        ID3D11DeviceContext* d3d_device_context = m_d3d_app->get_device_context();

        // Scene rendering part
        if (m_light_cull_technique == LightCullTechnique::CULL_FORWARD_NONE)
        {
            render_forward(false);
        } else if (m_light_cull_technique <= LightCullTechnique::CULL_FORWARD_COMPUTE_SHADER_TILE)
        {
            render_forward(true);
        } else
        {
            render_gbuffer();

            if (m_light_cull_technique == LightCullTechnique::CULL_DEFERRED_NONE)
            {
                m_deferred_effect.compute_lighting_default(d3d_device_context, m_lit_buffer->get_render_target(),
                                                            m_depth_buffer_read_only_dsv.Get(), m_light_buffer->get_shader_resource(),
                                                            m_gbuffer_srvs.data(), m_camera->get_viewport());
            } else if (m_light_cull_technique == LightCullTechnique::CULL_DEFERRED_COMPUTE_SHADER_TILE)
            {
                m_deferred_effect.compute_tiled_light_culling(d3d_device_context,
                                                            m_flat_lit_buffer->get_unordered_access(),
                                                            m_light_buffer->get_shader_resource(),
                                                            m_gbuffer_srvs.data());
            }
        }

        render_skybox();

        // Deferred rendering debug
        if (m_light_cull_technique >= LightCullTechnique::CULL_DEFERRED_NONE)
        {
            float aspect_ratio = m_viewer_spec.get_aspect_ratio();
            if (ImGui::Begin("Normal"))
            {
                m_deferred_effect.debug_normal_gbuffer(d3d_device_context, m_debug_normal_gbuffer->get_render_target(),
                                                        m_gbuffer_srvs[0], m_camera->get_viewport());
                ImVec2 winSize = ImGui::GetWindowSize();
                float smaller = (std::min)((winSize.x - 20.0f) / aspect_ratio, winSize.y - 20.0f);
                ImGui::Image(m_debug_normal_gbuffer->get_shader_resource(), ImVec2(smaller * aspect_ratio, smaller));
            }
            ImGui::End();

            if (ImGui::Begin("Albedo"))
            {
                d3d_device_context->ResolveSubresource(m_debug_albedo_gbuffer->get_texture(), 0, m_gbuffers[1]->get_texture(), 0, DXGI_FORMAT_R8G8B8A8_UNORM);
                ImVec2 winSize = ImGui::GetWindowSize();
                float smaller = (std::min)((winSize.x - 20) / aspect_ratio, winSize.y - 20);
                ImGui::Image(m_debug_albedo_gbuffer->get_shader_resource(), ImVec2(smaller * aspect_ratio, smaller));
            }
            ImGui::End();

            if (ImGui::Begin("PosZGrad"))
            {

                m_deferred_effect.debug_pos_z_grad_gbuffer(d3d_device_context, m_debug_posz_grad_gbuffer->get_render_target(),
                                                            m_gbuffer_srvs[2], m_camera->get_viewport());
                ImVec2 winSize = ImGui::GetWindowSize();
                float smaller = (std::min)((winSize.x - 20) / aspect_ratio, winSize.y - 20);
                ImGui::Image(m_debug_posz_grad_gbuffer->get_shader_resource(), ImVec2(smaller * aspect_ratio, smaller));
            }
            ImGui::End();
        }

        ID3D11RenderTargetView* rtvs[1] = { m_d3d_app->get_back_buffer_rtv() };
        d3d_device_context->OMSetRenderTargets(1, rtvs, nullptr);
    }

    void Viewer::init_resource()
    {
        // Initialize camera and controller
        using namespace DirectX;
        auto camera = std::make_shared<first_person_camera_c>();
        m_camera = camera;

        m_camera->set_viewport(0.0f, 0.0f, static_cast<float>(m_viewer_spec.width), static_cast<float>(m_viewer_spec.height));
        m_camera->set_frustum(XM_PI / 3.0f, m_viewer_spec.get_aspect_ratio(), 0.5f, 300.0f);
        camera->look_at(XMFLOAT3(-60.0f, 10.0f, 2.5f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));

        m_camera_controller.init(camera.get());
        m_camera_controller.set_move_speed(10.0f);

        // Initialize effects
        m_forward_effect.set_view_matrix(camera->get_view_xm());
        m_deferred_effect.set_view_matrix(camera->get_view_xm());
        //// reverse z
        m_forward_effect.set_proj_matrix(camera->get_proj_xm(true));
        m_deferred_effect.set_proj_matrix(camera->get_proj_xm(true));
        m_forward_effect.set_camera_near_far(0.5f, 300.0f);
        m_deferred_effect.set_camera_near_far(0.5f, 300.0f);
        m_forward_effect.set_msaa_samples(1);
        m_deferred_effect.set_msaa_samples(1);
        m_skybox_effect.set_msaa_samples(1);

        // Skybox texture
        model::TextureManagerHandle::get().create_from_file("../data/textures/Clouds.dds");

        // Initialize models
        model::ModelManagerHandle::get().create_from_file("../data/models/Sponza/sponza.gltf");
        model::ModelManagerHandle::get().create_from_geometry("skyboxCube", geometry::create_box());

        // Initialize lighting
        init_light_params();
        resize_lights(m_active_lights);
        update_lights(0.0f);

        // TODO: entity
        m_editor_scene = std::make_shared<Scene>();
        auto sponza_entity = m_editor_scene->create_entity("Sponza");
        auto& sponza_transform = sponza_entity.add_component<TransformComponent>();
        sponza_transform.transform.set_scale(0.05f, 0.05f, 0.05f);
        auto& sponza_mesh = sponza_entity.add_component<StaticMeshComponent>();
        sponza_mesh.model_asset = model::ModelManagerHandle::get().get_model("../data/models/Sponza/sponza.gltf");

        auto skybox_entity = m_editor_scene->create_entity("Skybox");
        skybox_entity.add_component<TransformComponent>();
        auto& skybox_mesh = skybox_entity.add_component<StaticMeshComponent>();
        skybox_mesh.model_asset = model::ModelManagerHandle::get().get_model("skyboxCube");
        skybox_mesh.is_skybox = true;
        skybox_mesh.model_asset->materials[0].set<std::string>("$Skybox", "../data/textures/Clouds.dds");

        m_scene_hierarchy_panel.set_context(m_editor_scene);
    }

    void Viewer::init_light_params()
    {
        using namespace DirectX;

        m_point_light_params.resize(MAX_LIGHTS);
        m_point_light_init_data.resize(MAX_LIGHTS);
        m_point_light_pos_worlds.resize(MAX_LIGHTS);

        // Using fixed random number seed to maintain consistency
        std::mt19937 rng(1337);
        constexpr float maxRadius = 100.0f;
        constexpr float attenuationStartFactor = 0.8f;
        std::uniform_real<float> radiusNormDist(0.0f, 1.0f);
        std::uniform_real<float> angleDist(0.0f, 2.0f * XM_PI);
        std::uniform_real<float> heightDist(0.0f, 75.0f);
        std::uniform_real<float> animationSpeedDist(2.0f, 20.0f);
        std::uniform_int<int> animationDirection(0, 1);
        std::uniform_real<float> hueDist(0.0f, 1.0f);
        std::uniform_real<float> intensityDist(0.2f, 0.8f);
        std::uniform_real<float> attenuationDist(2.0f, 20.0f);

        for (uint32_t i = 0; i < MAX_LIGHTS; ++i)
        {
            PointLightApp& params = m_point_light_params[i];
            PointLightInitData& data = m_point_light_init_data[i];

            data.radius = std::sqrt(radiusNormDist(rng)) * maxRadius;
            data.angle = angleDist(rng);
            data.height = heightDist(rng);
            // Normalized velocity
            data.animation_speed = (animationDirection(rng) * 2.0f - 1.0f) * animationSpeedDist(rng) / data.radius;

            // HSL->RGB
            params.color = hue_to_rgb(hueDist(rng));
            XMStoreFloat3(&params.color, XMLoadFloat3(&params.color) * intensityDist(rng));
            params.attenuation_end = attenuationDist(rng);
            params.attenuation_begin = attenuationStartFactor * params.attenuation_end;
        }
    }

    DirectX::XMFLOAT3 Viewer::hue_to_rgb(float hue)
    {
        using namespace DirectX;

        float intPart = 0.0f;
        float fracPart = std::modf(hue * 6.0f, &intPart);
        int region = static_cast<int>(intPart);

        switch (region)
        {
            case 0: return XMFLOAT3{1.0f, fracPart, 0.0f };
            case 1: return XMFLOAT3{1.0f - fracPart, 1.0f, 0.0f };
            case 2: return XMFLOAT3{0.0f, 1.0f, fracPart };
            case 3: return XMFLOAT3{0.0f, 1.0f - fracPart, 1.0f };
            case 4: return XMFLOAT3{fracPart, 0.0f, 1.0f };
            case 5: return XMFLOAT3{1.0f, 0.0f, 1.0f - fracPart };
        }
        return XMFLOAT3{0.0f, 0.0f, 0.0f };
    }

    void Viewer::resize_lights(uint32_t active_lights)
    {
        ID3D11Device* d3d_device = m_d3d_app->get_device();
        m_active_lights = active_lights;
        m_light_buffer = std::make_unique<StructureBuffer<PointLightApp>>(d3d_device, active_lights, D3D11_BIND_SHADER_RESOURCE, false, true);
    }

    void Viewer::update_lights(float dt)
    {
        using namespace DirectX;

        static float totalTime = 0.0f;
        totalTime += dt;
        for (uint32_t i = 0; i < m_active_lights; ++i)
        {
            const auto& data = m_point_light_init_data[i];
            float angle = data.angle + totalTime * data.animation_speed;
            m_point_light_pos_worlds[i] = XMFLOAT3(
                data.radius * std::cos(angle),
                data.height * m_light_height_scale,
                data.radius * std::sin(angle)
            );
        }
    }

    void XM_CALLCONV Viewer::setup_lights(DirectX::XMMATRIX view)
    {
        using namespace DirectX;

        ID3D11DeviceContext* d3d_device_context = m_d3d_app->get_device_context();
        XMVector3TransformCoordStream(&m_point_light_params[0].pos_v, sizeof(PointLightApp),
        &m_point_light_pos_worlds[0], sizeof(XMFLOAT3), m_active_lights, view);

        PointLightApp* pData = m_light_buffer->map_discard(d3d_device_context);
        memcpy_s(pData, sizeof(PointLightApp) * m_active_lights,
        m_point_light_params.data(), sizeof(PointLightApp) * m_active_lights);
        m_light_buffer->unmap(d3d_device_context);
    }

    void Viewer::resize_buffers(uint32_t width, uint32_t height, uint32_t msaa_samples)
    {
        ID3D11Device* d3d_device = m_d3d_app->get_device();
        // Initialize resources for forward rendering
        uint32_t tileWidth = (width + COMPUTE_SHADER_TILE_GROUP_DIM - 1) / COMPUTE_SHADER_TILE_GROUP_DIM;
        uint32_t tileHeight = (height + COMPUTE_SHADER_TILE_GROUP_DIM - 1) / COMPUTE_SHADER_TILE_GROUP_DIM;
        m_tile_buffer = std::make_unique<StructureBuffer<TileInfo>>(d3d_device,
                                                                    width * height, D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE);

        // Initialize resources for deferred rendering
        DXGI_SAMPLE_DESC sampleDesc{};
        sampleDesc.Count = msaa_samples;
        sampleDesc.Quality = 0;
        m_lit_buffer = std::make_unique<Texture2DMS>(d3d_device, width, height,
                                                    DXGI_FORMAT_R16G16B16A16_FLOAT, sampleDesc,
                                                    D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);

        m_flat_lit_buffer = std::make_unique<StructureBuffer<DirectX::XMUINT2>>(
                d3d_device, width * height * msaa_samples,
                D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE);

        m_depth_buffer = std::make_unique<Depth2DMS>(d3d_device, width, height, sampleDesc,
                                                    // Provide template for MSAA
                                                    m_msaa_samples > 1 ? DepthStencilBitsFlag::Depth_32Bits_Stencil_8Bits_Unused_24Bits : DepthStencilBitsFlag::Depth_32Bits,
                                                    D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE);

        // Create read-only depth/stencil view
        {
            D3D11_DEPTH_STENCIL_VIEW_DESC desc{};
            m_depth_buffer->get_depth_stencil()->GetDesc(&desc);
            desc.Flags = D3D11_DSV_READ_ONLY_DEPTH;
            d3d_device->CreateDepthStencilView(m_depth_buffer->get_texture(), &desc, m_depth_buffer_read_only_dsv.ReleaseAndGetAddressOf());
        }

        // G-Buffer
        // MRT requires all G-Buffers use the same MSAA level
        m_gbuffers.clear();
        // normal_specular
        m_gbuffers.push_back(std::make_unique<Texture2DMS>(d3d_device, width, height, DXGI_FORMAT_R16G16B16A16_FLOAT, sampleDesc,
                                                            D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE));
        // albedo
        m_gbuffers.push_back(std::make_unique<Texture2DMS>(d3d_device, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, sampleDesc,
                                                            D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE));
        // posZ grad
        m_gbuffers.push_back(std::make_unique<Texture2DMS>(d3d_device, width, height, DXGI_FORMAT_R16G16_FLOAT, sampleDesc,
                                                            D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE));

        // Set G-Buffer resource list
        m_gbuffer_rtvs.resize(m_gbuffers.size(), 0);
        m_gbuffer_srvs.resize(4, 0);
        for (std::size_t i = 0; i < m_gbuffers.size(); ++i)
        {
            m_gbuffer_rtvs[i] = m_gbuffers[i]->get_render_target();
            m_gbuffer_srvs[i] = m_gbuffers[i]->get_shader_resource();
        }
        // Depth buffer srv for reading
        m_gbuffer_srvs.back() = m_depth_buffer->get_shader_resource();

        // Debug buffers
        m_debug_albedo_gbuffer = std::make_unique<Texture2D>(d3d_device, width, height, DXGI_FORMAT_R8G8B8A8_UNORM);
        m_debug_normal_gbuffer = std::make_unique<Texture2D>(d3d_device, width, height, DXGI_FORMAT_R8G8B8A8_UNORM);
        m_debug_posz_grad_gbuffer = std::make_unique<Texture2D>(d3d_device, width, height, DXGI_FORMAT_R16G16_FLOAT);

        // Viewer buffer
        m_viewer_buffer = std::make_unique<Texture2D>(d3d_device, width, height, DXGI_FORMAT_R8G8B8A8_UNORM);
    }

    void Viewer::render_forward(bool do_pre_z)
    {
        ID3D11DeviceContext* d3d_device_context = m_d3d_app->get_device_context();

        d3d_device_context->ClearRenderTargetView(m_lit_buffer->get_render_target(), s_color);
        // Reverse z-buffer, the far plane is 0.0
        d3d_device_context->ClearDepthStencilView(m_depth_buffer->get_depth_stencil(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0);

        D3D11_VIEWPORT viewport = m_camera->get_viewport();
        d3d_device_context->RSSetViewports(1, &viewport);

        // PreZ Pass
        if (do_pre_z)
        {
            d3d_device_context->OMSetRenderTargets(0, nullptr, m_depth_buffer->get_depth_stencil());
            m_forward_effect.set_pre_z_pass_render();

            // TODO: entity
            m_editor_scene->render_scene(d3d_device_context, m_forward_effect);
            // TODO: end

            d3d_device_context->OMSetRenderTargets(0, nullptr, nullptr);
        }

        // Lighting culling
        if (m_light_cull_technique == LightCullTechnique::CULL_FORWARD_COMPUTE_SHADER_TILE)
        {
            m_forward_effect.compute_tiled_light_culling(d3d_device_context,
                                                        m_tile_buffer->get_unordered_access(),
                                                        m_light_buffer->get_shader_resource(),
                                                        m_gbuffer_srvs[3]);
        }

        // Draw
        {
            ID3D11RenderTargetView* rtvs[1] = { m_lit_buffer->get_render_target() };
            d3d_device_context->OMSetRenderTargets(1, rtvs, m_depth_buffer->get_depth_stencil());

            if (m_light_cull_technique == LightCullTechnique::CULL_FORWARD_COMPUTE_SHADER_TILE)
            {
                m_forward_effect.set_tile_buffer(m_tile_buffer->get_shader_resource());
                m_forward_effect.set_tiled_light_culling_render();
            } else
            {
                m_forward_effect.set_default_render();
            }

            m_forward_effect.set_light_buffer(m_light_buffer->get_shader_resource());

            // TODO: entity
            m_editor_scene->render_scene(d3d_device_context, m_forward_effect);
            // TODO: end

            // Clear binding
            m_forward_effect.set_tile_buffer(nullptr);
            m_forward_effect.apply(d3d_device_context);
            d3d_device_context->OMSetRenderTargets(0, nullptr, nullptr);
        }
    }

    void Viewer::render_gbuffer()
    {
        ID3D11DeviceContext* d3d_device_context = m_d3d_app->get_device_context();

        // Only need to clear depth buffer, since we use skybox sampling to replace no-written pixel(far plane)
        // Use depth buffer to reconstruct position, only position in frustum can be draw
        // Reverse z-buffer, far plane is 0.0
        if (m_clear_gbuffers)
        {
            for (auto rtv : m_gbuffer_rtvs)
            {
                d3d_device_context->ClearRenderTargetView(rtv, s_color);
            }
        }
        d3d_device_context->ClearDepthStencilView(m_depth_buffer->get_depth_stencil(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0);

        D3D11_VIEWPORT viewport = m_camera->get_viewport();
        d3d_device_context->RSSetViewports(1, &viewport);

        {
            m_deferred_effect.set_gbuffer_render();
            d3d_device_context->OMSetRenderTargets(uint32_t(m_gbuffers.size()), m_gbuffer_rtvs.data(), m_depth_buffer->get_depth_stencil());

            // TODO: entity
            m_editor_scene->render_scene(d3d_device_context, m_deferred_effect);
            // TODO: end

            d3d_device_context->OMSetRenderTargets(0, nullptr, nullptr);
        }
    }

    void Viewer::render_skybox()
    {
        ID3D11DeviceContext* d3d_device_context = m_d3d_app->get_device_context();

        d3d_device_context->ClearRenderTargetView(m_viewer_buffer->get_render_target(), s_color);
        d3d_device_context->ClearRenderTargetView(m_d3d_app->get_back_buffer_rtv(), s_color);

        D3D11_VIEWPORT skybox_viewport = m_camera->get_viewport();
        skybox_viewport.MinDepth = 1.0f;
        skybox_viewport.MaxDepth = 1.0f;
        d3d_device_context->RSSetViewports(1, &skybox_viewport);

        m_skybox_effect.set_default_render();

        m_skybox_effect.set_view_matrix(m_camera->get_view_xm());
        // Reverse z
        m_skybox_effect.set_proj_matrix(m_camera->get_proj_xm(true));

        // Binding buffers
        if (m_light_cull_technique == LightCullTechnique::CULL_DEFERRED_COMPUTE_SHADER_TILE)
        {
            m_skybox_effect.set_flat_lit_texture(m_flat_lit_buffer->get_shader_resource(), static_cast<uint32_t>(m_viewer_spec.width), static_cast<uint32_t>(m_viewer_spec.height));
        } else
        {
            m_skybox_effect.set_lit_texture(m_lit_buffer->get_shader_resource());
        }

        m_skybox_effect.set_depth_texture(m_depth_buffer->get_shader_resource());
        m_skybox_effect.apply(d3d_device_context);

        // Full screen draw, do not need depth buffer, do not need to clear back buffer - if you use back buffer as rtv
        // Otherwise you must clear back buffer first
        ID3D11RenderTargetView* rtvs[1] = { m_viewer_buffer->get_render_target() };
        d3d_device_context->OMSetRenderTargets(1, rtvs, nullptr);

        // TODO: entity
        m_editor_scene->render_scene(d3d_device_context, m_skybox_effect, true);
        // TODO: end

        // Clear status
        d3d_device_context->OMSetRenderTargets(0, nullptr, nullptr);
        m_skybox_effect.set_lit_texture(nullptr);
        m_skybox_effect.set_flat_lit_texture(nullptr, 0, 0);
        m_skybox_effect.set_depth_texture(nullptr);
        m_skybox_effect.apply(d3d_device_context);
    }
}


































