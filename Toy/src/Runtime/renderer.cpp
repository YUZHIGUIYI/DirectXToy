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

namespace toy::runtime
{
    Renderer::Renderer(int32_t width, int32_t height)
    : m_client_width(width), m_client_height(height)
    {
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

    void Renderer::release()
    {
        if (m_d3d_immediate_context)
        {
            m_d3d_immediate_context->ClearState();
            m_has_released = true;
        }
    }

    void Renderer::process_pending_events(const std::vector<EngineEventVariant> &pending_events)
    {
        // Note: current only handle WindowResizeEvent, DockResizeEvent, DropEvent
        for (auto&& delegate_event : pending_events)
        {
            std::visit([this] (auto&& event) {
                using event_type = std::decay_t<decltype(event)>;
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
        // TODO
    }

    void Renderer::init()
    {
        // 1. Initialize renderer backend
        init_backend();

        // 2. Initialize resources
        init_resources();
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

        // Set debug object name
#if defined(GRAPHICS_DEBUGGER_OBJECT_NAME)
        set_debug_object_name(m_d3d_immediate_context.Get(), "ImmediateContext");
        set_debug_object_name(m_swap_chain.Get(), "SwapChain");
#endif

        // Since window has been resized, invoke this function
        on_framebuffer_resize(m_client_width, m_client_height);
    }

    void Renderer::init_resources()
    {
        // TODO: initialize effects
        // Initialize texture manager and model manager
        model::TextureManager::get().init(m_d3d_device.Get());
        model::ModelManager::get().init(m_d3d_device.Get());

        // Initialize render states
        RenderStates::init(m_d3d_device.Get());

        // Initialize effects
        ShadowEffect::get().init(m_d3d_device.Get());
        DeferredPBREffect::get().init(m_d3d_device.Get());
        SimpleSkyboxEffect::get().init(m_d3d_device.Get());
        PreProcessEffect::get().init(m_d3d_device.Get());
        TAAEffect::get().init(m_d3d_device.Get());
        GizmosWireEffect::get().init(m_d3d_device.Get());

        // Initialize shadow
        CascadedShadowManager::get().init(m_d3d_device.Get());

        // Initialize camera and controller and other effects
        using namespace DirectX;
        using namespace toy::model;

        auto camera = std::make_shared<FirstPersonCamera>();
        m_camera = camera;
        m_camera->set_viewport(0.0f, 0.0f, static_cast<float>(m_dock_width), static_cast<float>(m_dock_height));
        m_camera->set_frustum(XM_PI / 3.0f, static_cast<float>(m_dock_width) / static_cast<float>(m_dock_height), 0.5f, 360.0f);
        camera->look_at(XMFLOAT3{-60.0f, 10.0f, 2.5f }, XMFLOAT3{ 0.0f, 0.0f, 0.0f }, XMFLOAT3{ 0.0f, 1.0f, 0.0f });



        auto directional_light = std::make_shared<FirstPersonCamera>();
        m_directional_light = directional_light;
        directional_light->set_viewport(0.0f, 0.0f, static_cast<float>(m_dock_width), static_cast<float>(m_dock_height));
        directional_light->set_frustum(XM_PI / 3.0f, 1.0f, 0.1f, 1000.0f);
        directional_light->look_at(XMFLOAT3{ -15.0f, 55.0f, -10.0f }, XMFLOAT3{ 0.0f, 0.0f, 0.0f }, XMFLOAT3{ 0.0f, 1.0f, 0.0f });
        m_camera_controller.init(camera.get());

        // Note: Initialize effects
        auto&& cascade_shadow_manager = CascadedShadowManager::get();
        auto&& deferred_pbr_effect = DeferredPBREffect::get();
        //// 1. Deferred pbr effect
        deferred_pbr_effect.set_view_matrix(camera->get_view_xm());
        deferred_pbr_effect.set_proj_matrix(camera->get_proj_xm(true));
        deferred_pbr_effect.set_camera_near_far(m_camera->get_near_z(), m_camera->get_far_z());

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
        TAAEffect::get().set_viewer_size(m_dock_width, m_dock_height);
        TAAEffect::get().set_camera_near_far(m_camera->get_near_z(), m_camera->get_far_z());
        //// 3. Shadow effect
        ShadowEffect::get().set_view_matrix(m_directional_light->get_view_xm());
        ShadowEffect::get().set_blur_kernel_size(cascade_shadow_manager.blur_kernel_size);
        ShadowEffect::get().set_blur_sigma(cascade_shadow_manager.gaussian_blur_sigma);
        ShadowEffect::get().set_16bit_format_shadow(false);
    }

    void Renderer::on_framebuffer_resize(int32_t width, int32_t height)
    {
        if (width == 0 || height == 0)
        {
            m_window_minimized = true;
            DX_CORE_INFO("Renderer window minimizing");
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
        const std::string extension = std::filesystem::path(filepath).extension().string();
        auto name = std::filesystem::path(filepath).filename();
        if (extension == ".gltf" || extension == ".glb" || extension == ".fbx")
        {
            auto new_entity = m_editor_scene->create_entity(name.string());
            model::ModelManager::get().create_from_file(filename, static_cast<uint32_t>(new_entity.entity_handle));

            auto& new_transform = new_entity.add_component<TransformComponent>();
            new_transform.transform.set_scale(0.1f, 0.1f, 0.1f);
            auto& new_mesh = new_entity.add_component<StaticMeshComponent>();
            new_mesh.model_asset = model::ModelManager::get().get_model(filename);

        } else if (extension == ".hdr")
        {
            auto d3d_device = m_d3d_app->get_device();
            auto d3d_device_context = m_d3d_app->get_device_context();
            PreProcessEffect::get().compute_cubemap(d3d_device, d3d_device_context, filename);
            PreProcessEffect::get().compute_sp_env_map(d3d_device, d3d_device_context);
            PreProcessEffect::get().compute_irradiance_map(d3d_device, d3d_device_context);
            PreProcessEffect::get().compute_brdf_lut(d3d_device, d3d_device_context);
        }
    }

    void Renderer::on_render_target_resize(int32_t width, int32_t height)
    {
        // TODO: reset buffers
        m_dock_width = width;
        m_dock_height = height;

        auto d3d_device = m_d3d_device.Get();

        // Initialize depth resource for GBuffer
        m_depth_texture = std::make_unique<Depth2D>(d3d_device, width, height, DepthStencilBitsFlag::Depth_32Bits);

        // GBuffer
        // Albedo and metalness
        m_gbuffer.albedo_metalness_buffer = std::make_unique<Texture2D>(d3d_device, width, height, DXGI_FORMAT_R8G8B8A8_UNORM,
                                                                        1, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
        // Normal and roughness
        m_gbuffer.normal_roughness_buffer = std::make_unique<Texture2D>(d3d_device, width, height, DXGI_FORMAT_R16G16B16A16_FLOAT,
                                                                        1, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
        // World position
        m_gbuffer.world_position_buffer = std::make_unique<Texture2D>(d3d_device, width, height, DXGI_FORMAT_R16G16B16A16_FLOAT,
                                                                        1, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
        // Motion vector
        m_gbuffer.motion_vector_buffer = std::make_unique<Texture2D>(d3d_device, width, height, DXGI_FORMAT_R16G16_UNORM,
                                                                        1, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
        // Entity id
        m_gbuffer.entity_id_buffer = std::make_unique<Texture2D>(d3d_device, width, height, DXGI_FORMAT_R32_UINT,
                                                                    1, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);

        // Viewer texture
        m_view_texture = std::make_unique<Texture2D>(d3d_device, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 1);

        // TAA input and output
        m_history_texture = std::make_unique<Texture2D>(d3d_device, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 1);
        m_lighting_pass_texture = std::make_unique<Texture2D>(d3d_device, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 1);
        m_taa_texture = std::make_unique<Texture2D>(d3d_device, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 1);

        // Shadow effect debug
        m_shadow_texture = std::make_unique<Texture2D>(d3d_device, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 1);
    }

    void Renderer::shadow_pass()
    {

    }

    void Renderer::gbuffer_pass()
    {

    }

    void Renderer::lighting_and_taa_pass()
    {

    }

    void Renderer::skybox_pass()
    {

    }
}