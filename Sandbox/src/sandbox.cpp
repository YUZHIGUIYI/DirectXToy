//
// Created by ZZK on 2023/5/17.
//

#include <Sandbox/sandbox.h>
#include <wincodec.h>
#include <IconsFontAwesome6.h>

#include <ScreenGrab/ScreenGrab11.h>

#include <random>

namespace toy
{
    using namespace DirectX;

    const float s_color[4] = { 0.1f, 0.1f, 0.1f, 1.0f };

    sandbox_c::sandbox_c(GLFWwindow* window, int32_t init_width, int32_t init_height)
    : d3d_application_c(window, init_width, init_height)
    {
        m_vpp_size = { 640, 320, 640, 320 };
        m_vpa_size = m_vpp_size;
        m_vpn_size = m_vpp_size;
        m_vpz_size = m_vpp_size;
    }

    sandbox_c::~sandbox_c()
    {

    }

    void sandbox_c::init()
    {
        d3d_application_c::init();

        // Initialize texture manager and model manager
        model::TextureManagerHandle::get().init(m_d3d_device.Get());
        model::ModelManagerHandle::get().init(m_d3d_device.Get());

        // Initialize render states
        RenderStates::init(m_d3d_device.Get());

        // Initialize effects
        m_forward_effect.init(m_d3d_device.Get());
        m_deferred_effect.init(m_d3d_device.Get());
        m_skybox_effect.init(m_d3d_device.Get());

        // Initialize resource
        init_resource();

        // Initialize buffers
        resize_buffers(m_vpp_size.cur_width, m_vpp_size.cur_height, m_msaa_samples);

        resize_viewport_buffer(m_debug_final_scene, m_vpp_size, DXGI_FORMAT_R8G8B8A8_UNORM);
        resize_viewport_buffer(m_debug_albedo_gbuffer, m_vpa_size, DXGI_FORMAT_R8G8B8A8_UNORM);
        resize_viewport_buffer(m_debug_normal_gbuffer, m_vpn_size, DXGI_FORMAT_R8G8B8A8_UNORM);
        resize_viewport_buffer(m_debug_posz_grad_gbuffer, m_vpz_size, DXGI_FORMAT_R16G16_FLOAT);

        // Change camera when window resizes
        event_manager_c::subscribe(event_type_e::WindowResize, [this] (const event_t& event)
        {
            auto&& window_resize_event = std::get<window_resize_event_c>(event);

            // Check whether the window has been minimized
            auto width = window_resize_event.window_width;
            auto height = window_resize_event.window_height;
            if (width == 0 || height == 0)
            {
                return;
            }
//
//            if (m_camera != nullptr)
//            {
//                auto aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
//                m_camera->set_frustum(XM_PI / 3.0f, aspect_ratio, 0.5f, 300.0f);
//                m_camera->set_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
//                m_forward_effect.set_proj_matrix(m_camera->get_proj_xm(true));
//                m_deferred_effect.set_proj_matrix(m_camera->get_proj_xm(true));
//                m_forward_effect.set_camera_near_far(0.5f, 300.0f);
//                m_deferred_effect.set_camera_near_far(0.5f, 300.0f);
//            }
//
            resize_buffers(width, height, m_msaa_samples);
        });
    }

    void sandbox_c::update_scene(float dt)
    {
        // Check viewport size change
        if (m_vpp_size.is_viewport_size_changed())
        {
            auto width = m_vpp_size.cur_width;
            auto height = m_vpp_size.cur_height;
            auto aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
            m_camera->set_frustum(XM_PI / 3.0f, aspect_ratio, 0.5f, 300.0f);
            m_camera->set_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
            m_forward_effect.set_proj_matrix(m_camera->get_proj_xm(true));
            m_deferred_effect.set_proj_matrix(m_camera->get_proj_xm(true));
            m_forward_effect.set_camera_near_far(0.5f, 300.0f);
            m_deferred_effect.set_camera_near_far(0.5f, 300.0f);

            resize_buffers(width, height, m_msaa_samples);

            resize_viewport_buffer(m_debug_final_scene, m_vpp_size, DXGI_FORMAT_R8G8B8A8_UNORM);
        }
        if (m_vpa_size.is_viewport_size_changed())
        {
            resize_viewport_buffer(m_debug_albedo_gbuffer, m_vpa_size, DXGI_FORMAT_R8G8B8A8_UNORM);
        }
        if (m_vpn_size.is_viewport_size_changed())
        {
            resize_viewport_buffer(m_debug_normal_gbuffer, m_vpn_size, DXGI_FORMAT_R8G8B8A8_UNORM);
        }
        if (m_vpz_size.is_viewport_size_changed())
        {
            resize_viewport_buffer(m_debug_posz_grad_gbuffer, m_vpz_size, DXGI_FORMAT_R16G16_FLOAT);
        }

        // Update camera
        m_camera_controller.update(dt, m_glfw_window);

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
                resize_buffers(m_vpp_size.cur_width, m_vpp_size.cur_height, m_msaa_samples);
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
            if (m_animate_lights)
            {
                update_lights(dt);
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

        m_forward_effect.set_view_matrix(m_camera->get_view_xm());
        m_deferred_effect.set_view_matrix(m_camera->get_view_xm());
        setup_lights(m_camera->get_view_xm());

        // Update collision
        BoundingFrustum frustum{};
        BoundingFrustum::CreateFromMatrix(frustum, m_camera->get_proj_xm());
        frustum.Transform(frustum, m_camera->get_local_to_world_xm());
        m_sponza.frustum_culling(frustum);
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
                m_deferred_effect.compute_lighting_default(m_d3d_immediate_context.Get(), m_lit_buffer->get_render_target(),
                                                        m_depth_buffer_read_only_dsv.Get(), m_light_buffer->get_shader_resource(),
                                                        m_gbuffer_srvs.data(), m_camera->get_viewport());
            } else if (m_light_cull_technique == LightCullTechnique::CULL_DEFERRED_COMPUTE_SHADER_TILE)
            {
                m_deferred_effect.compute_tiled_light_culling(m_d3d_immediate_context.Get(),
                                                            m_flat_lit_buffer->get_unordered_access(),
                                                            m_light_buffer->get_shader_resource(),
                                                            m_gbuffer_srvs.data());
            }
        }

        render_skybox();

        // ImGui part
        if (m_light_cull_technique >= LightCullTechnique::CULL_DEFERRED_NONE)
        {
            if (ImGui::Begin("Normal"))
            {
                m_deferred_effect.debug_normal_gbuffer(m_d3d_immediate_context.Get(), m_debug_normal_gbuffer->get_render_target(),
                                                    m_gbuffer_srvs[0], m_camera->get_viewport());
                ImVec2 winSize = ImGui::GetWindowSize();
                float smaller = (std::min)((winSize.x - 20) / get_aspect_ratio(), winSize.y - 20);
                ImGui::Image(m_debug_normal_gbuffer->get_shader_resource(), ImVec2(smaller * get_aspect_ratio(), smaller));
            }
            ImGui::End();
//            m_deferred_effect.debug_normal_gbuffer(m_d3d_immediate_context.Get(), m_debug_normal_gbuffer->get_render_target(),
//                                                    m_gbuffer_srvs[0], m_camera->get_viewport());
//            Viewport::set_viewport("Normal", m_debug_normal_gbuffer->get_shader_resource(), m_vpn_size.cur_width, m_vpn_size.cur_height);

            if (ImGui::Begin("Albedo"))
            {
                m_d3d_immediate_context->ResolveSubresource(m_debug_albedo_gbuffer->get_texture(), 0, m_gbuffers[1]->get_texture(), 0, DXGI_FORMAT_R8G8B8A8_UNORM);
                ImVec2 winSize = ImGui::GetWindowSize();
                float smaller = (std::min)((winSize.x - 20) / get_aspect_ratio(), winSize.y - 20);
                ImGui::Image(m_debug_albedo_gbuffer->get_shader_resource(), ImVec2(smaller * get_aspect_ratio(), smaller));
            }
            ImGui::End();
//            m_d3d_immediate_context->ResolveSubresource(m_debug_albedo_gbuffer->get_texture(), 0, m_gbuffers[1]->get_texture(), 0, DXGI_FORMAT_R8G8B8A8_UNORM);
//            Viewport::set_viewport("Albedo", m_debug_albedo_gbuffer->get_shader_resource(), m_vpa_size.cur_width, m_vpa_size.cur_height);

            if (ImGui::Begin("PosZGrad"))
            {

                m_deferred_effect.debug_pos_z_grad_gbuffer(m_d3d_immediate_context.Get(), m_debug_posz_grad_gbuffer->get_render_target(),
                                                        m_gbuffer_srvs[2], m_camera->get_viewport());
                ImVec2 winSize = ImGui::GetWindowSize();
                float smaller = (std::min)((winSize.x - 20) / get_aspect_ratio(), winSize.y - 20);
                ImGui::Image(m_debug_posz_grad_gbuffer->get_shader_resource(), ImVec2(smaller * get_aspect_ratio(), smaller));
            }
            ImGui::End();
//            m_deferred_effect.debug_pos_z_grad_gbuffer(m_d3d_immediate_context.Get(), m_debug_posz_grad_gbuffer->get_render_target(),
//                                                        m_gbuffer_srvs[2], m_camera->get_viewport());
//            Viewport::set_viewport("PosZGrad", m_debug_posz_grad_gbuffer->get_shader_resource(), m_vpz_size.cur_width, m_vpz_size.cur_height);
        }
        Viewport::set_viewport("Viewport", m_debug_final_scene->get_shader_resource(), m_vpp_size.cur_width, m_vpp_size.cur_height);

        ID3D11RenderTargetView* rtvs[1] = { get_back_buffer_rtv() };
        m_d3d_immediate_context->OMSetRenderTargets(1, rtvs, nullptr);

        // Note: present function is called in application
    }

    void sandbox_c::init_resource()
    {
        // Initialize camera and controller
        auto camera = std::make_shared<first_person_camera_c>();
        m_camera = camera;

        m_camera->set_viewport(0.0f, 0.0f, static_cast<float>(m_vpp_size.cur_width), static_cast<float>(m_vpp_size.cur_height));
        m_camera->set_frustum(XM_PI / 3.0f, m_vpp_size.get_aspect_ratio(), 0.5f, 300.0f);
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
        m_sponza.set_model(model::ModelManagerHandle::get().create_from_file("../data/models/Sponza/sponza.gltf"));
        m_sponza.get_transform().set_scale(0.05f, 0.05f, 0.05f);
        model::ModelManagerHandle::get().create_from_geometry("skyboxCube", geometry::create_box());
        auto model = model::ModelManagerHandle::get().get_model("skyboxCube");
        model->materials[0].set<std::string>("$Skybox", "../data/textures/Clouds.dds");
        m_skybox.set_model(model);

        // Initialize lighting
        init_light_params();
        resize_lights(m_active_lights);
        update_lights(0.0f);
    }

    void sandbox_c::init_light_params()
    {
        m_point_light_params.resize(MAX_LIGHTS);
        m_point_light_init_data.resize(MAX_LIGHTS);
        m_point_light_pos_worlds.resize(MAX_LIGHTS);

        // 使用固定的随机数种子保持一致性
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
            // 归一化速度
            data.animation_speed = (animationDirection(rng) * 2.0f - 1.0f) * animationSpeedDist(rng) / data.radius;

            // HSL->RGB
            params.color = hue_to_rgb(hueDist(rng));
            XMStoreFloat3(&params.color, XMLoadFloat3(&params.color) * intensityDist(rng));
            params.attenuation_end = attenuationDist(rng);
            params.attenuation_begin = attenuationStartFactor * params.attenuation_end;
        }
    }

    DirectX::XMFLOAT3 sandbox_c::hue_to_rgb(float hue)
    {
        float intPart;
        float fracPart = std::modf(hue * 6.0f, &intPart);
        int region = static_cast<int>(intPart);

        switch (region)
        {
            case 0: return XMFLOAT3(1.0f, fracPart, 0.0f);
            case 1: return XMFLOAT3(1.0f - fracPart, 1.0f, 0.0f);
            case 2: return XMFLOAT3(0.0f, 1.0f, fracPart);
            case 3: return XMFLOAT3(0.0f, 1.0f - fracPart, 1.0f);
            case 4: return XMFLOAT3(fracPart, 0.0f, 1.0f);
            case 5: return XMFLOAT3(1.0f, 0.0f, 1.0f - fracPart);
        }
        return XMFLOAT3{0.0f, 0.0f, 0.0f };
    }

    void sandbox_c::resize_lights(uint32_t active_lights)
    {
        m_active_lights = active_lights;
        m_light_buffer = std::make_unique<StructureBuffer<PointLightApp>>(m_d3d_device.Get(), active_lights, D3D11_BIND_SHADER_RESOURCE, false, true);

        // TODO: set debug object name
    }

    void sandbox_c::update_lights(float dt)
    {
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

    void XM_CALLCONV sandbox_c::setup_lights(DirectX::XMMATRIX view)
    {
        XMVector3TransformCoordStream(&m_point_light_params[0].pos_v, sizeof(PointLightApp),
        &m_point_light_pos_worlds[0], sizeof(XMFLOAT3), m_active_lights, view);

        PointLightApp* pData = m_light_buffer->map_discard(m_d3d_immediate_context.Get());
        memcpy_s(pData, sizeof(PointLightApp) * m_active_lights,
            m_point_light_params.data(), sizeof(PointLightApp) * m_active_lights);
        m_light_buffer->unmap(m_d3d_immediate_context.Get());
    }

    void sandbox_c::resize_buffers(uint32_t width, uint32_t height, uint32_t msaa_samples)
    {
        // Initialize resources for forward rendering
        uint32_t tileWidth = (width + COMPUTE_SHADER_TILE_GROUP_DIM - 1) / COMPUTE_SHADER_TILE_GROUP_DIM;
        uint32_t tileHeight = (height + COMPUTE_SHADER_TILE_GROUP_DIM - 1) / COMPUTE_SHADER_TILE_GROUP_DIM;
        m_tile_buffer = std::make_unique<StructureBuffer<TileInfo>>(m_d3d_device.Get(),
                width * height, D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE);

        // Initialize resources for deferred rendering
        DXGI_SAMPLE_DESC sampleDesc{};
        sampleDesc.Count = msaa_samples;
        sampleDesc.Quality = 0;
        m_lit_buffer = std::make_unique<Texture2DMS>(m_d3d_device.Get(), width, height,
                                                DXGI_FORMAT_R16G16B16A16_FLOAT, sampleDesc,
                                                D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);

        m_flat_lit_buffer = std::make_unique<StructureBuffer<DirectX::XMUINT2>>(
                        m_d3d_device.Get(), width * height * msaa_samples,
                        D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE);

        m_depth_buffer = std::make_unique<Depth2DMS>(m_d3d_device.Get(), width, height, sampleDesc,
                                                    // Provide template for MSAA
                                                    m_msaa_samples > 1 ? DepthStencilBitsFlag::Depth_32Bits_Stencil_8Bits_Unused_24Bits : DepthStencilBitsFlag::Depth_32Bits,
                                                    D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE);

        // Create read-only depth/stencil view
        {
            D3D11_DEPTH_STENCIL_VIEW_DESC desc{};
            m_depth_buffer->get_depth_stencil()->GetDesc(&desc);
            desc.Flags = D3D11_DSV_READ_ONLY_DEPTH;
            m_d3d_device->CreateDepthStencilView(m_depth_buffer->get_texture(), &desc, m_depth_buffer_read_only_dsv.ReleaseAndGetAddressOf());
        }

        // G-Buffer
        // MRT requires all G-Buffers use the same MSAA level
        m_gbuffers.clear();
        // normal_specular
        m_gbuffers.push_back(std::make_unique<Texture2DMS>(m_d3d_device.Get(), width, height, DXGI_FORMAT_R16G16B16A16_FLOAT, sampleDesc,
                                                            D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE));
        // albedo
        m_gbuffers.push_back(std::make_unique<Texture2DMS>(m_d3d_device.Get(), width, height, DXGI_FORMAT_R8G8B8A8_UNORM, sampleDesc,
                                                            D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE));
        // posZ grad
        m_gbuffers.push_back(std::make_unique<Texture2DMS>(m_d3d_device.Get(), width, height, DXGI_FORMAT_R16G16_FLOAT, sampleDesc,
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

        // Debug buffers - has been discarded
//        m_debug_albedo_gbuffer = std::make_unique<Texture2D>(m_d3d_device.Get(), width, height, DXGI_FORMAT_R8G8B8A8_UNORM);
//        m_debug_normal_gbuffer = std::make_unique<Texture2D>(m_d3d_device.Get(), width, height, DXGI_FORMAT_R8G8B8A8_UNORM);
//        m_debug_posz_grad_gbuffer = std::make_unique<Texture2D>(m_d3d_device.Get(), width, height, DXGI_FORMAT_R16G16_FLOAT);

        // TODO: set debug object name
    }

    void sandbox_c::resize_viewport_buffer(std::unique_ptr<Texture2D>& vp_buffer, const ViewportSize vp_size, DXGI_FORMAT format)
    {
        vp_buffer = std::make_unique<Texture2D>(m_d3d_device.Get(), uint32_t(vp_size.cur_width), uint32_t(vp_size.cur_height), format);
        DX_INFO("Resize debug viewport");
    }

    void sandbox_c::render_forward(bool do_pre_z)
    {
        m_d3d_immediate_context->ClearRenderTargetView(m_lit_buffer->get_render_target(), s_color);
        // Reverse z-buffer, the far plane is 0.0
        m_d3d_immediate_context->ClearDepthStencilView(m_depth_buffer->get_depth_stencil(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0);

        D3D11_VIEWPORT viewport = m_camera->get_viewport();
        m_d3d_immediate_context->RSSetViewports(1, &viewport);

        // PreZ Pass
        if (do_pre_z)
        {
            m_d3d_immediate_context->OMSetRenderTargets(0, nullptr, m_depth_buffer->get_depth_stencil());
            m_forward_effect.set_pre_z_pass_render();
            m_sponza.draw(m_d3d_immediate_context.Get(), m_forward_effect);
            m_d3d_immediate_context->OMSetRenderTargets(0, nullptr, nullptr);
        }

        // Lighting culling
        if (m_light_cull_technique == LightCullTechnique::CULL_FORWARD_COMPUTE_SHADER_TILE)
        {
            m_forward_effect.compute_tiled_light_culling(m_d3d_immediate_context.Get(),
                                                        m_tile_buffer->get_unordered_access(),
                                                        m_light_buffer->get_shader_resource(),
                                                        m_gbuffer_srvs[3]);
        }

        // Draw
        {
            ID3D11RenderTargetView* rtvs[1] = { m_lit_buffer->get_render_target() };
            m_d3d_immediate_context->OMSetRenderTargets(1, rtvs, m_depth_buffer->get_depth_stencil());

            if (m_light_cull_technique == LightCullTechnique::CULL_FORWARD_COMPUTE_SHADER_TILE)
            {
                m_forward_effect.set_tile_buffer(m_tile_buffer->get_shader_resource());
                m_forward_effect.set_tiled_light_culling_render();
            } else
            {
                m_forward_effect.set_default_render();
            }

            m_forward_effect.set_light_buffer(m_light_buffer->get_shader_resource());
            m_sponza.draw(m_d3d_immediate_context.Get(), m_forward_effect);

            // Clear binding
            m_forward_effect.set_tile_buffer(nullptr);
            m_forward_effect.apply(m_d3d_immediate_context.Get());
            m_d3d_immediate_context->OMSetRenderTargets(0, nullptr, nullptr);
        }
    }

    void sandbox_c::render_gbuffer()
    {
        // Only need to clear depth buffer, since we use skybox sampling to replace no-written pixel(far plane)
        // Use depth buffer to reconstruct position, only position in frustum can be draw
        // Reverse z-buffer, far plane is 0.0
        if (m_clear_gbuffers)
        {
            for (auto rtv : m_gbuffer_rtvs)
            {
                m_d3d_immediate_context->ClearRenderTargetView(rtv, s_color);
            }
        }
        m_d3d_immediate_context->ClearDepthStencilView(m_depth_buffer->get_depth_stencil(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0);

        D3D11_VIEWPORT viewport = m_camera->get_viewport();
        m_d3d_immediate_context->RSSetViewports(1, &viewport);

        {
            m_deferred_effect.set_gbuffer_render();
            m_d3d_immediate_context->OMSetRenderTargets(uint32_t(m_gbuffers.size()), m_gbuffer_rtvs.data(), m_depth_buffer->get_depth_stencil());
            m_sponza.draw(m_d3d_immediate_context.Get(), m_deferred_effect);

            m_d3d_immediate_context->OMSetRenderTargets(0, nullptr, nullptr);
        }
    }

    void sandbox_c::render_skybox()
    {
        m_d3d_immediate_context->ClearRenderTargetView(m_debug_final_scene->get_render_target(), s_color);
        m_d3d_immediate_context->ClearRenderTargetView(get_back_buffer_rtv(), s_color);

        D3D11_VIEWPORT skybox_viewport = m_camera->get_viewport();
        skybox_viewport.MinDepth = 1.0f;
        skybox_viewport.MaxDepth = 1.0f;
        m_d3d_immediate_context->RSSetViewports(1, &skybox_viewport);

        m_skybox_effect.set_default_render();

        m_skybox_effect.set_view_matrix(m_camera->get_view_xm());
        // Reverse z
        m_skybox_effect.set_proj_matrix(m_camera->get_proj_xm(true));

        // Binding buffers
        if (m_light_cull_technique == LightCullTechnique::CULL_DEFERRED_COMPUTE_SHADER_TILE)
        {
            m_skybox_effect.set_flat_lit_texture(m_flat_lit_buffer->get_shader_resource(), m_vpp_size.cur_width, m_vpp_size.cur_height);
        } else
        {
            m_skybox_effect.set_lit_texture(m_lit_buffer->get_shader_resource());
        }

        m_skybox_effect.set_depth_texture(m_depth_buffer->get_shader_resource());
        m_skybox_effect.apply(m_d3d_immediate_context.Get());

        // Full screen draw, do not need depth buffer, do not need to clear back buffer - if you use back buffer as rtv
        // Otherwise you must clear back buffer first
        ID3D11RenderTargetView* rtvs[1] = { m_debug_final_scene->get_render_target() };
        m_d3d_immediate_context->OMSetRenderTargets(1, rtvs, nullptr);
        m_skybox.draw(m_d3d_immediate_context.Get(), m_skybox_effect);

        // Clear status
        m_d3d_immediate_context->OMSetRenderTargets(0, nullptr, nullptr);
        m_skybox_effect.set_lit_texture(nullptr);
        m_skybox_effect.set_flat_lit_texture(nullptr, 0, 0);
        m_skybox_effect.set_depth_texture(nullptr);
        m_skybox_effect.apply(m_d3d_immediate_context.Get());
    }
}






































