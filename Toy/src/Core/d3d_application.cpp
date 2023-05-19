//
// Created by ZZK on 2023/5/15.
//

#include <Toy/Core/d3d_application.h>

namespace toy
{
    d3d_application_c::d3d_application_c(GLFWwindow* window, const std::string &window_name,
                                            int32_t init_width, int32_t init_height)
    : class_glfw_window(window),
    class_main_wnd_title_(window_name),
    class_client_width_(init_width),
    class_client_height_(init_height),
    class_app_paused_(false),
    class_minimized_(false),
    class_maximized_(false),
    class_resizing_(false),
    class_enable_msaa_(true),
    class_msaa_quality_(0),
    class_d3d_device_(nullptr),
    class_d3d_immediate_context_(nullptr),
    class_swap_chain_(nullptr),
    class_depth_stencil_buffer_(nullptr),
    class_render_target_view_(nullptr),
    class_depth_stencil_view_(nullptr)
    {
        class_main_wnd_ = glfwGetWin32Window(class_glfw_window);
        class_screen_viewport_ = {};
    }

    d3d_application_c::~d3d_application_c()
    {
        if (class_d3d_immediate_context_)
            class_d3d_immediate_context_->ClearState();
    }

    void d3d_application_c::init()
    {
        init_d3d();
    }

    void d3d_application_c::on_resize()
    {
        std::cout << "Resize window\n";

        assert(class_d3d_device_);
        assert(class_d3d_immediate_context_);
        assert(class_swap_chain_);

        if (class_d3d_device1_ != nullptr)
        {
            assert(class_d3d_device1_);
            assert(class_d3d_immediate_context1_);
            assert(class_swap_chain1_);
        }

        // release resources pipeline
        class_render_target_view_.Reset();
        class_depth_stencil_view_.Reset();
        class_depth_stencil_buffer_.Reset();

        // reset swap chain and recreate render target view
        com_ptr<ID3D11Texture2D> back_buffer;
        class_swap_chain_->ResizeBuffers(1, (uint32_t)class_client_width_, (uint32_t)class_client_height_,
                                            DXGI_FORMAT_R8G8B8A8_UNORM, 0);
        class_swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(back_buffer.GetAddressOf()));
        class_d3d_device_->CreateRenderTargetView(back_buffer.Get(), nullptr, class_render_target_view_.GetAddressOf());

        // Set debug object name
        d3d11_set_debug_object_name(back_buffer.Get(), "BackBuffer[0]");
        back_buffer.Reset();

        // Create depth-stencil buffer and its view
        D3D11_TEXTURE2D_DESC depth_stencil_desc{};
        depth_stencil_desc.Width = static_cast<uint32_t>(class_client_width_);
        depth_stencil_desc.Height = static_cast<uint32_t>(class_client_height_);
        depth_stencil_desc.MipLevels = 1;
        depth_stencil_desc.ArraySize = 1;
        depth_stencil_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        if (class_enable_msaa_)
        {
            depth_stencil_desc.SampleDesc.Count = 4;
            depth_stencil_desc.SampleDesc.Quality = class_msaa_quality_ - 1;
        } else
        {
            depth_stencil_desc.SampleDesc.Count = 1;
            depth_stencil_desc.SampleDesc.Quality = 0;
        }

        depth_stencil_desc.Usage = D3D11_USAGE_DEFAULT;
        depth_stencil_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        depth_stencil_desc.CPUAccessFlags = 0;
        depth_stencil_desc.MiscFlags = 0;

        class_d3d_device_->CreateTexture2D(&depth_stencil_desc, nullptr, class_depth_stencil_buffer_.GetAddressOf());
        class_d3d_device_->CreateDepthStencilView(class_depth_stencil_buffer_.Get(), nullptr, class_depth_stencil_view_.GetAddressOf());

        // Bind render target view and depth-stencil view to pipeline
        class_d3d_immediate_context_->OMSetRenderTargets(1, class_render_target_view_.GetAddressOf(), class_depth_stencil_view_.Get());

        // Set viewport transform
        class_screen_viewport_.TopLeftX = 0.0;
        class_screen_viewport_.TopLeftY = 0.0;
        class_screen_viewport_.Width = static_cast<float>(class_client_width_);
        class_screen_viewport_.Height = static_cast<float>(class_client_height_);
        class_screen_viewport_.MinDepth = 0.0f;
        class_screen_viewport_.MaxDepth = 1.0f;

        class_d3d_immediate_context_->RSSetViewports(1, &class_screen_viewport_);
    }

    void d3d_application_c::tick()
    {
        while (!glfwWindowShouldClose(class_glfw_window))
        {
            update_scene(1.0f);
            draw_scene();
            glfwPollEvents();
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
                                    D3D11_SDK_VERSION, class_d3d_device_.GetAddressOf(), &feature_level, class_d3d_immediate_context_.GetAddressOf());
            // Can not use D3D_FEATURE_LEVEL_11_1, try D3D_FEATURE_LEVEL_11_0
            if (hr == E_INVALIDARG)
            {
                hr = D3D11CreateDevice(nullptr, drive_type, nullptr, create_device_flags, &feature_levels[1], (uint32_t)(feature_levels.size() - 1),
                                        D3D11_SDK_VERSION, class_d3d_device_.GetAddressOf(), &feature_level, class_d3d_immediate_context_.GetAddressOf());
            }
            if (SUCCEEDED(hr))
                break;
        }

        if (FAILED(hr))
        {
            throw std::runtime_error("D3D11CreateDevice failed");
        }

        // Check whether it support D3D_FEATURE_LEVEL_11_1 or D3D_FEATURE_LEVEL_11_0
        if (feature_level != D3D_FEATURE_LEVEL_11_1 && feature_level != D3D_FEATURE_LEVEL_11_0)
        {
            throw std::runtime_error("Direct 3D feature level 11 unsupported");
        }

        // Check MSAA quality supported
        class_d3d_device_->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &class_msaa_quality_);
        assert(class_msaa_quality_ > 0);

        com_ptr<IDXGIDevice> dxgi_device = nullptr;
        com_ptr<IDXGIAdapter> dxgi_adapter = nullptr;
        com_ptr<IDXGIFactory1> dxgi_factory1 = nullptr;     // D3D11.0(include DXGI1.1) interface
        com_ptr<IDXGIFactory2> dxgi_factory2 = nullptr;     // D3D11.1(include DXGI1.2) interface

        // Get DXGI factory to create D3D device
        class_d3d_device_.As(&dxgi_device);
        dxgi_device->GetAdapter(dxgi_adapter.GetAddressOf());
        dxgi_adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(dxgi_factory1.GetAddressOf()));

        // Check whether it has included IDXGIFactory2 interface
        hr = dxgi_factory1.As(&dxgi_factory2);
        // If it has included, support D3D11.1
        if (dxgi_factory2 != nullptr)
        {
            class_d3d_device_.As(&class_d3d_device1_);
            class_d3d_immediate_context_.As(&class_d3d_immediate_context1_);

            // Describe swap chain
            DXGI_SWAP_CHAIN_DESC1 sd1{};
            sd1.Width = static_cast<uint32_t>(class_client_width_);
            sd1.Height = static_cast<uint32_t>(class_client_height_);
            sd1.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            if (class_enable_msaa_)
            {
                sd1.SampleDesc.Count = 4;
                sd1.SampleDesc.Quality = class_msaa_quality_ - 1;
            } else
            {
                sd1.SampleDesc.Count = 1;
                sd1.SampleDesc.Quality = 0;
            }
            sd1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            sd1.BufferCount = 1;
            sd1.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
            sd1.Flags = 0;

            DXGI_SWAP_CHAIN_FULLSCREEN_DESC fd{};
            fd.RefreshRate.Numerator = 60;
            fd.RefreshRate.Denominator = 1;
            fd.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
            fd.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
            fd.Windowed = TRUE;

            // Create swap chain for current window
            dxgi_factory2->CreateSwapChainForHwnd(class_d3d_device_.Get(), class_main_wnd_, &sd1, &fd,
                                                    nullptr, class_swap_chain1_.GetAddressOf());
            class_swap_chain1_.As(&class_swap_chain_);
        } else
        {
            // Describe swap chain
            DXGI_SWAP_CHAIN_DESC sd{};
            sd.BufferDesc.Width = static_cast<uint32_t>(class_client_width_);
            sd.BufferDesc.Height = static_cast<uint32_t>(class_client_height_);
            sd.BufferDesc.RefreshRate.Numerator = 60;
            sd.BufferDesc.RefreshRate.Denominator = 1;
            sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
            sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
            if (class_enable_msaa_)
            {
                sd.SampleDesc.Count = 4;
                sd.SampleDesc.Quality = class_msaa_quality_ - 1;
            } else
            {
                sd.SampleDesc.Count = 1;
                sd.SampleDesc.Quality = 0;
            }
            sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            sd.BufferCount = 1;
            sd.OutputWindow = class_main_wnd_;
            sd.Windowed = TRUE;
            sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
            sd.Flags = 0;
            dxgi_factory1->CreateSwapChain(class_d3d_device_.Get(), &sd, class_swap_chain_.GetAddressOf());
        }

        // Set debug object name
        d3d11_set_debug_object_name(class_d3d_immediate_context_.Get(), "ImmediateContext");
        dxgi_set_debug_object_name(class_swap_chain_.Get(), "SwapChain");

        // After window has been resized, call this function
        on_resize();
    }

}
































