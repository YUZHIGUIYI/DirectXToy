//
// Created by ZZK on 2024/3/13.
//

#include <Toy/Runtime/renderer.h>
#include <Toy/Renderer/taa_settings.h>
#include <Toy/Renderer/cascaded_shadow_manager.h>
#include <Toy/Renderer/effects.h>
#include <Toy/Model/texture_manager.h>
#include <Toy/Model/model_manager.h>
#include <Toy/Renderer/render_states.h>
#include <Toy/Core/subsystem.h>
#include <Toy/Runtime/scene_graph.h>
#include <Toy/Runtime/task_system.h>
#include <Toy/ECS/components.h>

namespace toy::runtime
{
    static constexpr std::array<float, 4> s_clear_color = { 0.0f, 0.0f, 0.0f, 0.0f };

    Renderer::Renderer(GLFWwindow* native_window, int32_t width, int32_t height)
    : m_client_width(width), m_client_height(height)
    {
        m_main_wnd = glfwGetWin32Window(native_window);
        init();
    }

    Renderer::~Renderer()
    {
        if (!m_has_released)
        {
            release();
        }
    }

    void Renderer::reset_render_target()
    {
        ID3D11RenderTargetView* rtv =  get_back_buffer_rtv();
        m_d3d_immediate_context->OMSetRenderTargets(1, &rtv, nullptr);
        ++m_frame_count;
    }

    void Renderer::present()
    {
        m_swap_chain->Present(0, m_is_dxgi_flip_model ? DXGI_PRESENT_ALLOW_TEARING : 0);
    }

    void Renderer::reset_selected_entity(const EntityWrapper &entity_wrapper)
    {
        m_selected_entity = entity_wrapper;
    }

    void Renderer::release()
    {
        if (m_d3d_immediate_context)
        {
            m_d3d_immediate_context->ClearState();
            m_has_released = true;
        }
    }

    ID3D11ShaderResourceView *Renderer::get_cascade_shadow_shader_resource(uint32_t cascade_index)
    {
        if (cascade_index != m_cur_cascade_index)
        {
            cascade_index = std::clamp(cascade_index, static_cast<uint32_t>(0), static_cast<uint32_t>(CascadedShadowManager::get().cascade_levels));
            m_cur_cascade_index = cascade_index;
            cascade_shadow_pass(cascade_index);
        }
        return m_shadow_texture->get_shader_resource();
    }

    void Renderer::process_pending_events()
    {
        auto&& task_system = core::get_subsystem<TaskSystem>();
        if (task_system.empty()) return;
        // Note: current only handle WindowResizeEvent, DockResizeEvent, DropEvent
        EngineEventVariant delegate_event;
        while (task_system.try_get(delegate_event))
        {
            std::visit([this] (auto&& event) {
                using event_type = std::remove_cvref_t<decltype(event)>;
                if constexpr (std::is_same_v<event_type, WindowResizeEvent>)
                {
                    this->on_framebuffer_resize(event.window_width, event.window_height);
                } else if constexpr (std::is_same_v<event_type, DockResizeEvent>)
                {
                    this->on_render_target_resize(event.dock_width, event.dock_height);
                } else if constexpr (std::is_same_v<event_type, DropEvent>)
                {
                    this->on_file_drop(event.drop_filename);
                }
            }, delegate_event);
        }
    }

    void Renderer::tick()
    {
        auto&& scene_graph = core::get_subsystem<SceneGraph>();
        scene_graph.for_each<CameraComponent>([this] (CameraComponent &camera_component){
            auto&& camera = camera_component.camera;
            this->frustum_culling(*camera);
            this->shadow_pass(*camera);
            this->gbuffer_pass(*camera);
            this->lighting_and_taa_pass(*camera);
            this->skybox_pass(*camera);
        });
    }

    void Renderer::init()
    {
        // 1. Initialize renderer backend
        init_backend();

        // 2. Initialize asset manager
        init_asset_manager();

        // 3. Initialize render states and effects
        init_effects();

        // 3. Resize textures bound to render target views
        on_render_target_resize(m_dock_width, m_dock_height);
    }

    void Renderer::init_backend()
    {
        // Initialize DirectX11 device and context
        HRESULT hr = S_OK;

        uint32_t create_device_flags = 0;
#if defined(_DEBUG)
        create_device_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
        // Driver type array
        const std::array<D3D_DRIVER_TYPE, 3> driver_types{
                D3D_DRIVER_TYPE_HARDWARE,
                D3D_DRIVER_TYPE_WARP,
                D3D_DRIVER_TYPE_REFERENCE
        };

        // Feature level array
        const std::array<D3D_FEATURE_LEVEL, 2> feature_levels{
                D3D_FEATURE_LEVEL_11_1,
                D3D_FEATURE_LEVEL_11_0,
        };

        D3D_FEATURE_LEVEL feature_level;
        for (auto drive_type : driver_types)
        {
            hr = D3D11CreateDevice(nullptr, drive_type, nullptr, create_device_flags, feature_levels.data(), (uint32_t)feature_levels.size(),
                                    D3D11_SDK_VERSION, m_d3d_device.GetAddressOf(), &feature_level, m_d3d_immediate_context.GetAddressOf());
            // Can not use D3D_FEATURE_LEVEL_11_1, try D3D_FEATURE_LEVEL_11_0
            if (hr == E_INVALIDARG)
            {
                hr = D3D11CreateDevice(nullptr, drive_type, nullptr, create_device_flags, feature_levels.data() + 1, (uint32_t)(feature_levels.size() - 1),
                                        D3D11_SDK_VERSION, m_d3d_device.GetAddressOf(), &feature_level, m_d3d_immediate_context.GetAddressOf());
            }
            if (SUCCEEDED(hr)) break;
        }

        if (FAILED(hr))
        {
            DX_CORE_CRITICAL("Failed to create D3D11Device");
        }

        // Check whether it support D3D_FEATURE_LEVEL_11_1 or D3D_FEATURE_LEVEL_11_0
        if (feature_level != D3D_FEATURE_LEVEL_11_1 && feature_level != D3D_FEATURE_LEVEL_11_0)
        {
            DX_CORE_CRITICAL("D3D11 feature level 11 is unsupported");
        }

        // Check MSAA quality supported
        uint32_t quality = 0;
        m_d3d_device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &quality);
        DX_CORE_ASSERT(quality > 0, "MSAA quality must be over 0");

        com_ptr<IDXGIDevice> dxgi_device = nullptr;
        com_ptr<IDXGIAdapter> dxgi_adapter = nullptr;
        com_ptr<IDXGIFactory1> dxgi_factory1 = nullptr;     // D3D11.0(include DXGI1.1) interface
        com_ptr<IDXGIFactory2> dxgi_factory2 = nullptr;     // D3D11.1(include DXGI1.2) interface

        // Get DXGI factory to create D3D device
        m_d3d_device.As(&dxgi_device);
        dxgi_device->GetAdapter(dxgi_adapter.GetAddressOf());
        dxgi_adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(dxgi_factory1.GetAddressOf()));

        // Check whether it has included IDXGIFactory2 interface
        hr = dxgi_factory1.As(&dxgi_factory2);
        // If it has included, support D3D11.1
        if (dxgi_factory2 != nullptr)
        {
            m_d3d_device.As(&m_d3d_device1);
            m_d3d_immediate_context.As(&m_d3d_immediate_context1);

            // Describe swap chain
            DXGI_SWAP_CHAIN_DESC1 sd1{};
            sd1.Width = static_cast<uint32_t>(m_client_width);
            sd1.Height = static_cast<uint32_t>(m_client_height);
            sd1.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

            sd1.SampleDesc.Count = 1;
            sd1.SampleDesc.Quality = 0;

            sd1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
#if _WIN32_WINNT >= _WIN32_WINNT_WIN10
            m_back_buffer_count = 2;
            sd1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
            sd1.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
            m_is_dxgi_flip_model = true;
#else
            m_back_buffer_count = 1;
            sd1.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
            sd1.Flags = 0;
#endif
            sd1.BufferCount = m_back_buffer_count;

            DXGI_SWAP_CHAIN_FULLSCREEN_DESC fd{};
            fd.RefreshRate.Numerator = 60;
            fd.RefreshRate.Denominator = 1;
            fd.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
            fd.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
            fd.Windowed = TRUE;

            // Create swap chain for current window
            dxgi_factory2->CreateSwapChainForHwnd(m_d3d_device.Get(), m_main_wnd, &sd1, &fd,
                                                    nullptr, m_swap_chain1.GetAddressOf());
            m_swap_chain1.As(&m_swap_chain);
        } else
        {
            // Describe swap chain
            DXGI_SWAP_CHAIN_DESC sd{};
            sd.BufferDesc.Width = static_cast<uint32_t>(m_client_width);
            sd.BufferDesc.Height = static_cast<uint32_t>(m_client_height);
            sd.BufferDesc.RefreshRate.Numerator = 60;
            sd.BufferDesc.RefreshRate.Denominator = 1;
            sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
            sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
            sd.SampleDesc.Count = 1;
            sd.SampleDesc.Quality = 0;
            sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            sd.BufferCount = 1;
            sd.OutputWindow = m_main_wnd;
            sd.Windowed = TRUE;
            sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
            sd.Flags = 0;
            dxgi_factory1->CreateSwapChain(m_d3d_device.Get(), &sd, m_swap_chain.GetAddressOf());
        }

        // Since window has been resized, invoke this function
        on_framebuffer_resize(m_client_width, m_client_height);
    }

    void Renderer::init_asset_manager()
    {
        // Initialize texture manager and model manager
        model::TextureManager::get().init(m_d3d_device.Get());
        model::ModelManager::get().init(m_d3d_device.Get());
    }

    void Renderer::init_effects()
    {
        // Initialize render states
        RenderStates::init(m_d3d_device.Get());

        // Initialize effects
        ShadowEffect::get().init(m_d3d_device.Get());
        DeferredPBREffect::get().init(m_d3d_device.Get());
        SimpleSkyboxEffect::get().init(m_d3d_device.Get());
        PreProcessEffect::get().init(m_d3d_device.Get());
        TAAEffect::get().init(m_d3d_device.Get());
        GizmosWireEffect::get().init(m_d3d_device.Get());

        // Initialize shadow manager
        CascadedShadowManager::get().init(m_d3d_device.Get());

        // Initialize fixed states of effects
        auto&& cascade_shadow_manager = CascadedShadowManager::get();
        auto&& deferred_pbr_effect = DeferredPBREffect::get();
        //// 1. Deferred pbr effect
        deferred_pbr_effect.set_pcf_kernel_size(cascade_shadow_manager.blur_kernel_size);
        deferred_pbr_effect.set_pcf_depth_bias(cascade_shadow_manager.pcf_depth_bias);
        deferred_pbr_effect.set_shadow_type(static_cast<uint8_t>(cascade_shadow_manager.shadow_type));
        deferred_pbr_effect.set_shadow_size(cascade_shadow_manager.shadow_size);
        deferred_pbr_effect.set_cascade_blend_area(cascade_shadow_manager.blend_between_cascades_range);
        deferred_pbr_effect.set_cascade_levels(cascade_shadow_manager.cascade_levels);
        deferred_pbr_effect.set_cascade_interval_selection_enabled(static_cast<bool>(cascade_shadow_manager.selected_cascade_selection));
        deferred_pbr_effect.set_light_bleeding_reduction(cascade_shadow_manager.light_bleeding_reduction);
        deferred_pbr_effect.set_magic_power(cascade_shadow_manager.magic_power);
        deferred_pbr_effect.set_positive_exponent(cascade_shadow_manager.positive_exponent);
        deferred_pbr_effect.set_negative_exponent(cascade_shadow_manager.negative_exponent);
        deferred_pbr_effect.set_16_bit_format_shadow(false);
        //// 2. TAA effect
        //// 3. Shadow effect
        ShadowEffect::get().set_blur_kernel_size(cascade_shadow_manager.blur_kernel_size);
        ShadowEffect::get().set_blur_sigma(cascade_shadow_manager.gaussian_blur_sigma);
        ShadowEffect::get().set_16bit_format_shadow(false);
    }

    void Renderer::on_framebuffer_resize(int32_t width, int32_t height)
    {
        if (width == 0 || height == 0)
        {
            m_window_minimized = true;
            DX_CORE_INFO("Minimize renderer window");
            return;
        } else
        {
            m_window_minimized = false;
            DX_CORE_INFO("Resize renderer window");
        }

        m_client_width = width;
        m_client_height = height;

        DX_CORE_ASSERT(m_d3d_device, "DirectX11 device has been expired");
        DX_CORE_ASSERT(m_d3d_immediate_context, "DirectX11 context has been expired");
        DX_CORE_ASSERT(m_swap_chain, "Swap chain has been expired");

        // Recreate swap chain and render target views
        for (uint32_t i = 0; i < m_back_buffer_count; ++i)
        {
            m_render_target_views[i].Reset();
        }
        m_swap_chain->ResizeBuffers(m_back_buffer_count, m_client_width, m_client_height, DXGI_FORMAT_R8G8B8A8_UNORM,
                                    m_is_dxgi_flip_model ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0);
        m_frame_count = 0;

        // Create render target views of back buffers
        for (uint32_t frame_index = 0; frame_index < m_back_buffer_count; ++frame_index)
        {
            com_ptr<ID3D11Texture2D> back_buffer = nullptr;
            m_swap_chain->GetBuffer(0, IID_PPV_ARGS(back_buffer.GetAddressOf()));
            CD3D11_RENDER_TARGET_VIEW_DESC rtv_desc{ D3D11_RTV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB };
            m_d3d_device->CreateRenderTargetView(back_buffer.Get(), &rtv_desc, m_render_target_views[frame_index].ReleaseAndGetAddressOf());
        }
    }

    void Renderer::on_file_drop(std::string_view filepath)
    {
        auto&& scene_graph = core::get_subsystem<SceneGraph>();
        const std::string extension = std::filesystem::path(filepath).extension().string();
        auto filename = std::filesystem::path(filepath).filename();
        if (extension == ".gltf" || extension == ".glb" || extension == ".fbx")
        {
            auto new_entity = scene_graph.create_entity(filename.string());
            model::ModelManager::get().create_from_file(filepath, static_cast<uint32_t>(new_entity.entity_inst));
            auto& new_transform = new_entity.add_component<TransformComponent>();
            new_transform.transform.set_scale(0.5f, 0.5f, 0.5f);
            auto& new_mesh = new_entity.add_component<StaticMeshComponent>();
            new_mesh.model_asset = model::ModelManager::get().get_model(filepath);
        } else if (extension == ".hdr")
        {
            auto d3d_device = m_d3d_device.Get();
            auto d3d_device_context = m_d3d_immediate_context.Get();
            PreProcessEffect::get().compute_cubemap(d3d_device, d3d_device_context, filepath);
            PreProcessEffect::get().compute_sp_env_map(d3d_device, d3d_device_context);
            PreProcessEffect::get().compute_irradiance_map(d3d_device, d3d_device_context);
            PreProcessEffect::get().compute_brdf_lut(d3d_device, d3d_device_context);
        }
    }

    void Renderer::on_render_target_resize(int32_t width, int32_t height)
    {
        DX_CORE_INFO("Resize render target: {} + {}", width, height);
        // Reset dock size for render targets
        m_dock_width = width;
        m_dock_height = height;

        auto d3d_device = m_d3d_device.Get();

        // Initialize depth resource for GBuffer
        m_depth_texture = std::make_unique<Depth2D>(d3d_device, width, height, DepthStencilBitsFlag::Depth_32Bits);

        // Initialize GBuffer
        //// Albedo and metalness
        m_gbuffer.albedo_metalness_buffer = std::make_unique<Texture2D>(d3d_device, width, height, DXGI_FORMAT_R8G8B8A8_UNORM,
                                                                        1, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
        //// Normal and roughness
        m_gbuffer.normal_roughness_buffer = std::make_unique<Texture2D>(d3d_device, width, height, DXGI_FORMAT_R16G16B16A16_FLOAT,
                                                                        1, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
        //// World position
        m_gbuffer.world_position_buffer = std::make_unique<Texture2D>(d3d_device, width, height, DXGI_FORMAT_R16G16B16A16_FLOAT,
                                                                        1, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
        //// Motion vector
        m_gbuffer.motion_vector_buffer = std::make_unique<Texture2D>(d3d_device, width, height, DXGI_FORMAT_R16G16_UNORM,
                                                                        1, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
        //// Entity id
        m_gbuffer.entity_id_buffer = std::make_unique<Texture2D>(d3d_device, width, height, DXGI_FORMAT_R32_UINT,
                                                                    1, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);

        // Viewer texture
        m_view_texture = std::make_unique<Texture2D>(d3d_device, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 1);

        // TAA input and output
        m_history_texture = std::make_unique<Texture2D>(d3d_device, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 1);
        m_lighting_pass_texture = std::make_unique<Texture2D>(d3d_device, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 1);
        m_taa_texture = std::make_unique<Texture2D>(d3d_device, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 1);

        // Shadow texture for debugging
        m_shadow_texture = std::make_unique<Texture2D>(d3d_device, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 1);
    }

    void Renderer::frustum_culling(const toy::Camera &camera)
    {
        using namespace DirectX;

        auto&& scene_graph = core::get_subsystem<SceneGraph>();
        BoundingFrustum frustum = {};
        BoundingFrustum::CreateFromMatrix(frustum, camera.get_proj_xm());
        frustum.Transform(frustum, camera.get_local_to_world_xm());
        scene_graph.frustum_culling(frustum);
    }

    void Renderer::shadow_pass(const Camera &camera)
    {
        using namespace DirectX;
        auto&& scene_graph = core::get_subsystem<SceneGraph>();
        auto&& shadow_effect = ShadowEffect::get();
        auto&& cascade_shadow_manager = CascadedShadowManager::get();
        auto&& scene_bounding_box = scene_graph.get_scene_bounding_box();
        auto viewport = cascade_shadow_manager.get_shadow_viewport();

        // Update light view matrix and set in effect
        scene_graph.for_each<DirectionalLightComponent>([&camera, &scene_bounding_box, &cascade_shadow_manager, &shadow_effect] (DirectionalLightComponent &directional_light_component) {
            directional_light_component.transform.set_position(directional_light_component.position);
            directional_light_component.transform.look_at(directional_light_component.target, DirectX::XMFLOAT3{ 0.0f, 1.0f, 0.0f });
            auto light_view_matrix = directional_light_component.transform.get_world_to_local_matrix_xm();

            cascade_shadow_manager.update_frame(camera, scene_bounding_box, light_view_matrix);

            shadow_effect.set_view_matrix(light_view_matrix);
        });

        m_d3d_immediate_context->RSSetViewports(1, &viewport);

        for (size_t cascade_index = 0; cascade_index < cascade_shadow_manager.cascade_levels; ++cascade_index)
        {
            switch (cascade_shadow_manager.shadow_type)
            {
                case ShadowType::ShadowType_CSM: shadow_effect.set_default_render(); break;
                case ShadowType::ShadowType_VSM:
                case ShadowType::ShadowType_ESM:
                case ShadowType::ShadowType_EVSM2:
                case ShadowType::ShadowType_EVSM4: shadow_effect.set_depth_only_render(); break;
            }

            ID3D11RenderTargetView* depth_rtv = cascade_shadow_manager.get_cascade_render_target_view(cascade_index);
            ID3D11DepthStencilView* depth_dsv = cascade_shadow_manager.get_depth_buffer_dsv();
            m_d3d_immediate_context->ClearRenderTargetView(depth_rtv, s_clear_color.data());
            m_d3d_immediate_context->ClearDepthStencilView(depth_dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);
            if (cascade_shadow_manager.shadow_type != ShadowType::ShadowType_CSM) depth_rtv = nullptr;
            m_d3d_immediate_context->OMSetRenderTargets(1, &depth_rtv, depth_dsv);

            XMMATRIX shadow_proj = cascade_shadow_manager.get_shadow_project_xm(cascade_index);
            shadow_effect.set_proj_matrix(shadow_proj);

            scene_graph.render_static_mesh_shadow(m_d3d_immediate_context.Get(), shadow_effect);

            m_d3d_immediate_context->OMSetRenderTargets(0, nullptr, nullptr);

            if (cascade_shadow_manager.shadow_type == ShadowType::ShadowType_VSM || cascade_shadow_manager.shadow_type >= ShadowType::ShadowType_EVSM2)
            {
                if (cascade_shadow_manager.shadow_type == ShadowType::ShadowType_VSM)
                {
                    shadow_effect.render_variance_shadow(m_d3d_immediate_context.Get(), cascade_shadow_manager.get_depth_buffer_srv(),
                                                            cascade_shadow_manager.get_cascade_render_target_view(cascade_index), cascade_shadow_manager.get_shadow_viewport());
                } else
                {
                    shadow_effect.render_exponential_variance_shadow(m_d3d_immediate_context.Get(), cascade_shadow_manager.get_depth_buffer_srv(),
                                                                        cascade_shadow_manager.get_cascade_render_target_view(cascade_index), cascade_shadow_manager.get_shadow_viewport(),
                                                                        cascade_shadow_manager.positive_exponent,
                                                                        cascade_shadow_manager.shadow_type == ShadowType::ShadowType_EVSM4 ? &cascade_shadow_manager.negative_exponent : nullptr);
                }
                if (cascade_shadow_manager.blur_kernel_size > 1)
                {
                    shadow_effect.gaussian_blur_x(m_d3d_immediate_context.Get(), cascade_shadow_manager.get_cascade_output(cascade_index),
                                                    cascade_shadow_manager.get_temp_texture_rtv(), cascade_shadow_manager.get_shadow_viewport());
                    shadow_effect.gaussian_blur_y(m_d3d_immediate_context.Get(), cascade_shadow_manager.get_temp_texture_output(),
                                                    cascade_shadow_manager.get_cascade_render_target_view(cascade_index), cascade_shadow_manager.get_shadow_viewport());
                }
            } else if (cascade_shadow_manager.shadow_type == ShadowType::ShadowType_ESM)
            {
                if (cascade_shadow_manager.blur_kernel_size > 1)
                {
                    shadow_effect.render_exponential_shadow(m_d3d_immediate_context.Get(), cascade_shadow_manager.get_depth_buffer_srv(),
                                                            cascade_shadow_manager.get_temp_texture_rtv(), cascade_shadow_manager.get_shadow_viewport(), cascade_shadow_manager.magic_power);
                    shadow_effect.log_gaussian_blur(m_d3d_immediate_context.Get(), cascade_shadow_manager.get_temp_texture_output(),
                                                    cascade_shadow_manager.get_cascade_render_target_view(cascade_index), cascade_shadow_manager.get_shadow_viewport());
                } else
                {
                    shadow_effect.render_exponential_shadow(m_d3d_immediate_context.Get(), cascade_shadow_manager.get_depth_buffer_srv(),
                                                            cascade_shadow_manager.get_cascade_render_target_view(cascade_index), cascade_shadow_manager.get_shadow_viewport(), cascade_shadow_manager.magic_power);
                }
            }
        }

        if ((cascade_shadow_manager.shadow_type == ShadowType::ShadowType_VSM || cascade_shadow_manager.shadow_type >= ShadowType::ShadowType_EVSM2) && cascade_shadow_manager.generate_mips)
        {
            m_d3d_immediate_context->GenerateMips(cascade_shadow_manager.get_cascades_output());
        }
    }

    void Renderer::gbuffer_pass(const Camera &camera)
    {
        auto&& scene_graph = core::get_subsystem<SceneGraph>();
        D3D11_VIEWPORT viewport = camera.get_viewport();
        std::array<ID3D11RenderTargetView *, 5> gbuffer_rtvs{
            m_gbuffer.albedo_metalness_buffer->get_render_target(),
            m_gbuffer.normal_roughness_buffer->get_render_target(),
            m_gbuffer.world_position_buffer->get_render_target(),
            m_gbuffer.motion_vector_buffer->get_render_target(),
            m_gbuffer.entity_id_buffer->get_render_target()
        };

        m_d3d_immediate_context->ClearRenderTargetView(m_gbuffer.albedo_metalness_buffer->get_render_target(), s_clear_color.data());
        m_d3d_immediate_context->ClearRenderTargetView(m_gbuffer.normal_roughness_buffer->get_render_target(), s_clear_color.data());
        m_d3d_immediate_context->ClearRenderTargetView(m_gbuffer.world_position_buffer->get_render_target(), s_clear_color.data());
        m_d3d_immediate_context->ClearRenderTargetView(m_gbuffer.motion_vector_buffer->get_render_target(), s_clear_color.data());
        m_d3d_immediate_context->ClearRenderTargetView(m_gbuffer.entity_id_buffer->get_render_target(), s_clear_color.data());
        m_d3d_immediate_context->ClearDepthStencilView(m_depth_texture->get_depth_stencil(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0);
        m_d3d_immediate_context->RSSetViewports(1, &viewport);
        DeferredPBREffect::get().set_gbuffer_render();
        m_d3d_immediate_context->OMSetRenderTargets(static_cast<uint32_t>(gbuffer_rtvs.size()), gbuffer_rtvs.data(), m_depth_texture->get_depth_stencil());
        scene_graph.render_static_mesh(m_d3d_immediate_context.Get(), DeferredPBREffect::get());
        m_d3d_immediate_context->OMSetRenderTargets(0, nullptr, nullptr);
    }

    void Renderer::lighting_and_taa_pass(const Camera &camera)
    {
        static bool first_frame = true;
        static uint32_t taa_frame_counter = 0;
        D3D11_VIEWPORT viewport = camera.get_viewport();

        set_shadow_paras();

        m_d3d_immediate_context->ClearRenderTargetView(m_lighting_pass_texture->get_render_target(), s_clear_color.data());
        m_d3d_immediate_context->ClearRenderTargetView(m_taa_texture->get_render_target(), s_clear_color.data());
        DeferredPBREffect::get().set_proj_matrix(camera.get_proj_xm(true));
        DeferredPBREffect::get().set_view_matrix(camera.get_view_xm());
        DeferredPBREffect::get().set_camera_position(camera.get_position());
        DeferredPBREffect::get().set_camera_near_far(camera.get_near_z(), camera.get_far_z());
        DeferredPBREffect::get().set_viewer_size(m_dock_width, m_dock_height);
        DeferredPBREffect::get().set_lighting_pass_render();
        DeferredPBREffect::get().deferred_lighting_pass(m_d3d_immediate_context.Get(), m_lighting_pass_texture->get_render_target(),
                                                        m_gbuffer, camera.get_viewport());

        if (m_selected_entity.is_valid())
        {
            auto&& gizmos_wire_effect = GizmosWireEffect::get();
            auto& transform_component = m_selected_entity.get_component<TransformComponent>();
            auto world_matrix = transform_component.transform.get_local_to_world_matrix_xm();
            auto& static_mesh_component = m_selected_entity.get_component<StaticMeshComponent>();
            gizmos_wire_effect.set_world_matrix(world_matrix);
            gizmos_wire_effect.set_view_matrix(camera.get_view_xm());
            gizmos_wire_effect.set_proj_matrix(camera.get_proj_xm(true));
            gizmos_wire_effect.set_vertex_buffer(m_d3d_immediate_context.Get(), static_mesh_component.get_local_bounding_box());
            gizmos_wire_effect.render(m_d3d_immediate_context.Get(), m_lighting_pass_texture->get_render_target(), m_depth_texture->get_depth_stencil(), viewport);
        }

        if (first_frame)
        {
            m_d3d_immediate_context->CopyResource(m_taa_texture->get_texture(), m_lighting_pass_texture->get_texture());
            first_frame = false;
        } else
        {
            TAAEffect::get().set_viewer_size(m_dock_width, m_dock_height);
            TAAEffect::get().set_camera_near_far(camera.get_near_z(), camera.get_far_z());
            TAAEffect::get().render(m_d3d_immediate_context.Get(), m_history_texture->get_shader_resource(), m_lighting_pass_texture->get_shader_resource(),
                                    m_gbuffer.motion_vector_buffer->get_shader_resource(), m_depth_texture->get_shader_resource(),
                                    m_taa_texture->get_render_target(), viewport);
            taa_frame_counter = (taa_frame_counter + 1) % taa::s_taa_sample;
        }

        m_d3d_immediate_context->CopyResource(m_history_texture->get_texture(), m_taa_texture->get_texture());
    }

    void Renderer::skybox_pass(const Camera &camera)
    {
        auto&& scene_graph = core::get_subsystem<SceneGraph>();
        auto render_target_view = m_view_texture->get_render_target();
        D3D11_VIEWPORT viewport = camera.get_viewport();
        viewport.MinDepth = 1.0f;
        viewport.MaxDepth = 1.0f;

        m_d3d_immediate_context->ClearRenderTargetView(render_target_view, s_clear_color.data());
        m_d3d_immediate_context->RSSetViewports(1, &viewport);

        SimpleSkyboxEffect::get().set_skybox_render();
        SimpleSkyboxEffect::get().set_view_matrix(camera.get_view_xm());
        SimpleSkyboxEffect::get().set_proj_matrix(camera.get_proj_xm(true));
        SimpleSkyboxEffect::get().set_depth_texture(m_depth_texture->get_shader_resource());
        SimpleSkyboxEffect::get().set_scene_texture(m_taa_texture->get_shader_resource());
        SimpleSkyboxEffect::get().apply(m_d3d_immediate_context.Get());

        m_d3d_immediate_context->OMSetRenderTargets(1, &render_target_view, nullptr);
        scene_graph.render_skybox(m_d3d_immediate_context.Get(), SimpleSkyboxEffect::get());
        SimpleSkyboxEffect::get().set_depth_texture(nullptr);
        SimpleSkyboxEffect::get().set_scene_texture(nullptr);
        m_d3d_immediate_context->OMSetRenderTargets(0, nullptr, nullptr);
    }

    void Renderer::set_shadow_paras()
    {
        using namespace DirectX;

        auto&& scene_graph = core::get_subsystem<runtime::SceneGraph>();

        static const XMMATRIX s_transform = {
            0.5f, 0.0f, 0.0f, 0.0f,
            0.0f, -0.5f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.5f, 0.5f, 0.0f, 1.0f
        };

        auto&& deferred_pbr_effect = DeferredPBREffect::get();
        auto&& cascade_shadow_manager = CascadedShadowManager::get();

        std::array<XMFLOAT4, 8> scales = {};
        std::array<XMFLOAT4, 8> offsets = {};
        // From NDC [-1, 1]^2 to texture coordinates [0, 1]^2
        for (size_t cascade_index = 0; cascade_index < cascade_shadow_manager.cascade_levels; ++cascade_index)
        {
            XMMATRIX shadow_texture = cascade_shadow_manager.get_shadow_project_xm(cascade_index) * s_transform;
            scales[cascade_index].x  = XMVectorGetX(shadow_texture.r[0]);
            scales[cascade_index].y = XMVectorGetY(shadow_texture.r[1]);
            scales[cascade_index].z = XMVectorGetZ(shadow_texture.r[2]);
            scales[cascade_index].w = 1.0f;
            XMStoreFloat3((XMFLOAT3 *)(offsets.data() + cascade_index), shadow_texture.r[3]);
        }

        deferred_pbr_effect.set_cascade_frustums_eye_space_depths(cascade_shadow_manager.get_cascade_partitions());
        deferred_pbr_effect.set_cascade_offsets(std::span{ offsets.data(), offsets.size() });
        deferred_pbr_effect.set_cascade_scales(std::span{ scales.data(), scales.size() });
        deferred_pbr_effect.set_shadow_texture_array(cascade_shadow_manager.get_cascades_output());

        // Update view matrix and light direction
        scene_graph.for_each<DirectionalLightComponent>([&deferred_pbr_effect] (DirectionalLightComponent& directional_light_component) {
            auto light_intensity = directional_light_component.intensity;
            auto light_color = directional_light_component.color;
            auto light_radiance = DirectX::XMFLOAT3{ light_intensity * light_color.x, light_intensity * light_color.y, light_intensity * light_color.z };
            deferred_pbr_effect.set_shadow_view_matrix(directional_light_component.transform.get_world_to_local_matrix_xm());
            deferred_pbr_effect.set_light_direction(directional_light_component.transform.get_forward_axis());
            deferred_pbr_effect.set_light_radiance(light_radiance);
        });
    }

    void Renderer::cascade_shadow_pass(uint32_t cascade_index)
    {
        // Cascade shadow debugging
        auto&& cascade_shadow_manager = CascadedShadowManager::get();
        m_d3d_immediate_context->ClearRenderTargetView(m_shadow_texture->get_render_target(), s_clear_color.data());
        ShadowEffect::get().render_depth_to_texture(m_d3d_immediate_context.Get(), cascade_shadow_manager.get_cascade_output(cascade_index),
                                                    m_shadow_texture->get_render_target(), cascade_shadow_manager.shadow_viewport);
    }
}