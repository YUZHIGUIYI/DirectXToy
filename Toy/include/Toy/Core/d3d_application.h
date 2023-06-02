//
// Created by ZZK on 2023/5/15.
//

#pragma once

#include <Toy/Core/base.h>
#include <Toy/Events/event_manager.h>

namespace toy
{
    class d3d_application_c : public disable_copyable_c
    {
    public:
        d3d_application_c(GLFWwindow* window, const std::string& window_name, int32_t init_width, int32_t init_height);
        virtual ~d3d_application_c();

    public:
        [[nodiscard]] HWND get_main_wnd() const { return m_main_wnd; }
        [[nodiscard]] float get_aspect_ratio() const { return static_cast<float>(m_client_width) / static_cast<float>(m_client_height); }

    public:
        // Client need to override
        virtual void init();                                         // Init window and direct3D
        virtual void update_scene(float dt) = 0;                     // Update per-frame
        virtual void draw_scene() = 0;                               // Draw per-frame

        void on_resize(const event_t& event);                        // Call when resize window
        void on_close(const event_t& event);                         // Call when close window
        void tick();                                                 // Run application, and game-loop

    protected:
        void init_d3d();                                             // Initialize Direct3D
        void init_imgui();                                           // Initialize ImGui

        ID3D11RenderTargetView* get_back_buffer_rtv() { return m_render_target_views[m_frame_count % m_back_buffer_count].Get(); }

    protected:
        GLFWwindow* m_glfw_window;                                  // GLFW window
        HWND m_main_wnd;                                            // Main window handle

        // Direct3D 11
        com_ptr<ID3D11Device> m_d3d_device;                         // D3D11 Device
        com_ptr<ID3D11DeviceContext> m_d3d_immediate_context;       // D3D11 Device Context
        com_ptr<IDXGISwapChain> m_swap_chain;                       // D3D11 Swap Chain;
        // Direct3D 11.1
        com_ptr<ID3D11Device1> m_d3d_device1;                       // D3D11.1 Device
        com_ptr<ID3D11DeviceContext1> m_d3d_immediate_context1;     // D3D11.1 Device Context
        com_ptr<IDXGISwapChain1> m_swap_chain1;                     // D3D11.1 Swap Chain

        // Back buffers
        com_ptr<ID3D11RenderTargetView> m_render_target_views[2];   // Render target views
        uint32_t m_back_buffer_count = 0;                           // Back buffer count
        uint32_t m_frame_count = 0;                                 // Current frame

        // For Derived Class
        std::string m_main_wnd_title;                               // Main Window Title
        int32_t m_client_width;                                     // Viewport Width
        int32_t m_client_height;                                    // Viewport Height

        bool m_is_dxgi_flip_model = false;                          // Use DXGI flip model
        bool m_app_stopped;                                         // Application pause
        bool m_window_minimized;                                    // Application minimized
        bool m_window_maximized;                                    // Application maximized
        bool m_window_resized;                                      // Application resize
    };
}























































