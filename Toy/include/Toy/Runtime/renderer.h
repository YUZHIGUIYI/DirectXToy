//
// Created by ZZK on 2024/3/13.
//

#pragma once

#include <Toy/Core/base.h>

namespace toy::runtime
{
    struct Renderer
    {
    public:
        Renderer();

        ~Renderer();

        Renderer(const Renderer &) = delete;
        Renderer& operator=(const Renderer &) = delete;
        Renderer(Renderer &&) = delete;
        Renderer& operator=(Renderer &&) = delete;

        void reset_render_target();

        void present();

        void release();

    public:
        [[nodiscard]] HWND get_main_wnd() const { return m_main_wnd; }
        [[nodiscard]] float get_aspect_ratio() const { return static_cast<float>(m_client_width) / static_cast<float>(m_client_height); }
        [[nodiscard]] GLFWwindow* get_glfw_window() const { return m_glfw_window; }

        [[nodiscard]] ID3D11Device* get_device() const { return m_d3d_device.Get(); }
        [[nodiscard]] ID3D11DeviceContext* get_device_context() const { return m_d3d_immediate_context.Get(); }

        [[nodiscard]] ID3D11RenderTargetView* get_back_buffer_rtv() const { return m_render_target_views[m_frame_count % m_back_buffer_count].Get(); }

    private:
        void init_backend();

        void init_effects();

        void on_resize();

    private:
        GLFWwindow* m_glfw_window = nullptr;                                    // GLFW window
        HWND m_main_wnd;                                                        // Main window handle

        // Direct3D 11
        com_ptr<ID3D11Device> m_d3d_device = nullptr;                           // D3D11 Device
        com_ptr<ID3D11DeviceContext> m_d3d_immediate_context = nullptr;         // D3D11 Device Context
        com_ptr<IDXGISwapChain> m_swap_chain = nullptr;                         // D3D11 Swap Chain;
        // Direct3D 11.1
        com_ptr<ID3D11Device1> m_d3d_device1 = nullptr;                         // D3D11.1 Device
        com_ptr<ID3D11DeviceContext1> m_d3d_immediate_context1 = nullptr;       // D3D11.1 Device Context
        com_ptr<IDXGISwapChain1> m_swap_chain1 = nullptr;                       // D3D11.1 Swap Chain

        // Back buffers
        com_ptr<ID3D11RenderTargetView> m_render_target_views[2];               // Render target views
        uint32_t m_back_buffer_count = 0;                                       // Back buffer count
        uint32_t m_frame_count = 0;                                             // Current frame

        // Frame buffer size
        int32_t m_client_width = 1920;                                          // Frame buffer Width
        int32_t m_client_height = 1080;                                         // Frame buffer Height

        bool m_is_dxgi_flip_model = false;                                      // Use DXGI flip model
        bool m_window_minimized = false;                                        // Renderer minimized
        bool m_has_released = false;                                            // Whether renderer has been reset
    };
}
