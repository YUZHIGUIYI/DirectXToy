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
        [[nodiscard]] HWND get_main_wnd() const { return class_main_wnd_; }
        [[nodiscard]] float get_aspect_ratio() const { return static_cast<float>(class_client_width_) / static_cast<float>(class_client_height_); }

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

    protected:
        GLFWwindow* class_glfw_window;                               // GLFW window
        HWND class_main_wnd_;                                        // Main window handle

        template<typename T>
        using com_ptr = Microsoft::WRL::ComPtr<T>;

        // Direct3D 11
        com_ptr<ID3D11Device> class_d3d_device_;                     // D3D11 Device
        com_ptr<ID3D11DeviceContext> class_d3d_immediate_context_;   // D3D11 Device Context
        com_ptr<IDXGISwapChain> class_swap_chain_;                   // D3D11 Swap Chain;
        // Direct3D 11.1
        com_ptr<ID3D11Device1> class_d3d_device1_;                   // D3D11.1 Device
        com_ptr<ID3D11DeviceContext1> class_d3d_immediate_context1_; // D3D11.1 Device Context
        com_ptr<IDXGISwapChain1> class_swap_chain1_;                 // D3D11.1 Swap Chain
        // Common Resources
        com_ptr<ID3D11Texture2D> class_depth_stencil_buffer_;        // Depth-stencil Buffer
        com_ptr<ID3D11RenderTargetView> class_render_target_view_;   // Render Target View
        com_ptr<ID3D11DepthStencilView> class_depth_stencil_view_;   // Depth-stencil view
        D3D11_VIEWPORT class_screen_viewport_;                       // Viewport

        // For Derived Class
        std::string class_main_wnd_title_;                           // Main Window Title
        int32_t class_client_width_;                                 // Viewport Width
        int32_t class_client_height_;                                // Viewport Height

        uint32_t class_msaa_quality_;                                // MSAA quality
        bool class_app_paused_;                                      // Application pause
        bool class_minimized_;                                       // Application minimized
        bool class_maximized_;                                       // Application maximized
        bool class_resizing_;                                        // Application resize
        bool class_enable_msaa_;                                     // Enable MSAA
    };
}























































