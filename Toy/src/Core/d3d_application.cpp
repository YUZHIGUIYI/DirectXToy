//
// Created by ZZK on 2023/5/15.
//

#include <Toy/Core/d3d_application.h>
#include <Toy/ImGui/imgui_pass.h>

namespace toy
{
    d3d_application_c::d3d_application_c(GLFWwindow* window, int32_t init_width, int32_t init_height)
    : m_glfw_window(window),
    m_client_width(init_width),
    m_client_height(init_height),
    m_app_stopped(false),
    m_window_minimized(false),
    m_window_maximized(false),
    m_window_resized(false)
    {
        m_main_wnd = glfwGetWin32Window(m_glfw_window);
    }

    d3d_application_c::~d3d_application_c()
    {
        ImGuiPass::release();
        if (m_d3d_immediate_context)
        {
            m_d3d_immediate_context->ClearState();
        }
    }

    void d3d_application_c::init()
    {
        DX_CORE_INFO("Initialize DXToy engine");

        init_d3d();

        // Initialize ImGui pass
        ImGuiPass::init(m_glfw_window, m_d3d_device.Get(), m_d3d_immediate_context.Get());

        // Initialize event manager, subscribe window-resize event
        glfwSetWindowUserPointer(m_glfw_window, this);
        event_manager_c::init(m_glfw_window);
        event_manager_c::subscribe(event_type_e::WindowClose, [this] (const event_t& event)
        {
            on_close(event);
        });
        event_manager_c::subscribe(event_type_e::WindowResize, [this] (const event_t& event)
        {
            on_resize(event);
        });
        glfwSetDropCallback(m_glfw_window, [](GLFWwindow* window, int32_t path_count, const char** paths)
        {
            auto* app = static_cast<d3d_application_c *>(glfwGetWindowUserPointer(window));
            for (int32_t i = 0; i < path_count; i++)
            {
                app->on_file_drop(paths[i]);
            }
        });
    }

    void d3d_application_c::on_resize(const event_t &event)
    {
        DX_CORE_INFO("Resize window");

        auto&& window_resize_event = std::get<window_resize_event_c>(event);

        if (window_resize_event.window_width == 0 || window_resize_event.window_height == 0)
        {
            m_window_minimized = true;
            DX_CORE_INFO("Window minimizing");
            return;
        } else
        {
            m_window_minimized = false;
        }

        m_client_width = window_resize_event.window_width;
        m_client_height = window_resize_event.window_height;

        assert(m_d3d_device);
        assert(m_d3d_immediate_context);
        assert(m_swap_chain);

        // Recreate swap chain and render target views
        for (uint32_t i = 0; i < m_back_buffer_count; ++i)
        {
            m_render_target_views[i].Reset();
        }
        m_swap_chain->ResizeBuffers(m_back_buffer_count, m_client_width, m_client_height, DXGI_FORMAT_R8G8B8A8_UNORM,
                                            m_is_dxgi_flip_model ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0);
        m_frame_count = 0;

        // Create render target views of back buffers
        for (size_t frame_index = 0; frame_index < m_back_buffer_count; ++frame_index)
        {
            com_ptr<ID3D11Texture2D> back_buffer = nullptr;
            m_swap_chain->GetBuffer(0, IID_PPV_ARGS(back_buffer.GetAddressOf()));
            CD3D11_RENDER_TARGET_VIEW_DESC rtv_desc{ D3D11_RTV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB };
            m_d3d_device->CreateRenderTargetView(back_buffer.Get(), &rtv_desc, m_render_target_views[frame_index].ReleaseAndGetAddressOf());
        }
    }

    void d3d_application_c::on_close(const event_t &event)
    {
        m_app_stopped = true;
    }

    void d3d_application_c::tick()
    {
        float last_time = 0.0f;
        while (!m_app_stopped)
        {
            event_manager_c::update();

            if (!m_window_minimized)
            {
                auto current_time = static_cast<float>(glfwGetTime());

                ImGuiPass::begin();
                // Menu
                if (ImGui::BeginMainMenuBar())
                {
                    for (auto& layer : m_layer_stack)
                    {
                        layer->on_ui_menu();
                    }
                    ImGui::EndMainMenuBar();
                }

                // Setting up the viewport and getting information
                for (auto& layer : m_layer_stack)
                {
                    layer->on_resize();
                }

                // UI rendering
                for (auto& layer : m_layer_stack)
                {
                    layer->on_ui_render();
                }

                for (auto& layer : m_layer_stack)
                {
                    layer->on_render(current_time - last_time);
                }

//                update_scene(current_time - last_time);
//                draw_scene();

                ImGuiPass::end();
                present();

                ++m_frame_count;
                last_time = current_time;
            }
        }

        using namespace std::chrono_literals;
        std::this_thread::sleep_for(100ms);
    }

    // Present back buffer view
    void d3d_application_c::present()
    {
        // Present
        m_swap_chain->Present(0, m_is_dxgi_flip_model ? DXGI_PRESENT_ALLOW_TEARING : 0);
    }

    // Add layer into engine
    void d3d_application_c::add_layer(const std::shared_ptr<ILayer> &layer)
    {
        m_layer_stack.emplace_back(layer);
        layer->on_attach(this);
    }

    // File drop operation
    void d3d_application_c::on_file_drop(std::string_view filename)
    {
        for (auto& layer : m_layer_stack)
        {
            layer->on_file_drop(filename);
        }
    }

    // Create directX 3d device and device context
    void d3d_application_c::init_d3d()
    {
        HRESULT hr = S_OK;

        uint32_t create_device_flags = 0;
#if defined(_DEBUG)
        create_device_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
        // Driver type array
        std::array<D3D_DRIVER_TYPE, 3> driver_types{
            D3D_DRIVER_TYPE_HARDWARE,
            D3D_DRIVER_TYPE_WARP,
            D3D_DRIVER_TYPE_REFERENCE
        };

        // Feature level array
        std::array<D3D_FEATURE_LEVEL, 2> feature_levels{
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
                hr = D3D11CreateDevice(nullptr, drive_type, nullptr, create_device_flags, &feature_levels[1], (uint32_t)(feature_levels.size() - 1),
                                        D3D11_SDK_VERSION, m_d3d_device.GetAddressOf(), &feature_level, m_d3d_immediate_context.GetAddressOf());
            }
            if (SUCCEEDED(hr))
                break;
        }

        if (FAILED(hr))
        {
            DX_CORE_CRITICAL("Fail to create D3D11Device");
        }

        // Check whether it support D3D_FEATURE_LEVEL_11_1 or D3D_FEATURE_LEVEL_11_0
        if (feature_level != D3D_FEATURE_LEVEL_11_1 && feature_level != D3D_FEATURE_LEVEL_11_0)
        {
            DX_CORE_CRITICAL("D3D11 feature level 11 is unsupported");
        }

        // Check MSAA quality supported
        uint32_t quality = 0;
        m_d3d_device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &quality);
        assert(quality > 0);

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

        // After window has been resized, call this function
        window_resize_event_c event{ m_client_width, m_client_height };
        on_resize(event);
    }
}
































