//
// Created by ZZK on 2024/3/13.
//

#pragma once

#include <Toy/Core/base.h>
#include <Toy/Events/event.h>
#include <Toy/Events/window_event.h>
#include <Toy/Renderer/texture_2d.h>
#include <Toy/Renderer/gbuffer_definition.h>
#include <Toy/Scene/camera.h>
#include <Toy/Scene/entity_wrapper.h>

namespace toy::runtime
{
    struct Renderer
    {
    public:
        explicit Renderer(GLFWwindow* native_window, int32_t width, int32_t height);

        ~Renderer();

        Renderer(const Renderer &) = delete;
        Renderer& operator=(const Renderer &) = delete;
        Renderer(Renderer &&) = delete;
        Renderer& operator=(Renderer &&) = delete;

        void tick();

        void process_pending_events();

        // Invoke before frame end
        void reset_render_target();

        void present();

        // Invoke if selected entity changed
        void reset_selected_entity(const EntityWrapper &entity_wrapper);

        void release();

        ID3D11ShaderResourceView *get_cascade_shadow_shader_resource(uint32_t cascade_index);

    public:
        [[nodiscard]] bool is_renderer_minimized() const { return m_window_minimized; }
        [[nodiscard]] HWND get_main_wnd() const { return m_main_wnd; }
        [[nodiscard]] float get_aspect_ratio() const { return static_cast<float>(m_client_width) / static_cast<float>(m_client_height); }

        [[nodiscard]] ID3D11Device* get_device() const { return m_d3d_device.Get(); }
        [[nodiscard]] ID3D11DeviceContext* get_device_context() const { return m_d3d_immediate_context.Get(); }

        [[nodiscard]] ID3D11RenderTargetView* get_back_buffer_rtv() const { return m_render_target_views[m_frame_count % m_back_buffer_count].Get(); }

        [[nodiscard]] ID3D11ShaderResourceView* get_shadow_debug_srv() const { return m_shadow_texture->get_shader_resource(); }

        [[nodiscard]] ID3D11ShaderResourceView* get_view_srv() const { return m_view_texture->get_shader_resource(); }

        [[nodiscard]] const GBufferDefinition& get_gbuffer_definition() const { return m_gbuffer; }

    private:
        void init();

        void init_backend();

        void init_resources();

        void on_framebuffer_resize(int32_t width, int32_t height);

        void on_render_target_resize(int32_t width, int32_t height);

        void on_file_drop(std::string_view filepath);

    private:
        void frustum_culling(const Camera &camera);

        void shadow_pass(const Camera &camera);

        void gbuffer_pass(const Camera &camera);

        void lighting_and_taa_pass(const Camera &camera);

        void skybox_pass(const Camera &camera);

        void set_shadow_paras();

        void cascade_shadow_pass(uint32_t cascade_index);

    private:
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

        // Resources
        std::unique_ptr<Texture2D> m_shadow_texture = nullptr;
        std::unique_ptr<Depth2D> m_depth_texture = nullptr;
        std::unique_ptr<Texture2D> m_history_texture = nullptr;
        std::unique_ptr<Texture2D> m_lighting_pass_texture = nullptr;
        std::unique_ptr<Texture2D> m_taa_texture = nullptr;
        std::unique_ptr<Texture2D> m_view_texture = nullptr;
        GBufferDefinition m_gbuffer;

        // Directional light
        std::shared_ptr<Camera> m_directional_light = nullptr;

        // Selected entity
        EntityWrapper m_selected_entity = {};

        // Frame buffer size
        int32_t m_client_width = 1920;                                          // Frame buffer Width
        int32_t m_client_height = 1080;                                         // Frame buffer Height

        // Render target and shader resource view size
        int32_t m_dock_width = 1600;
        int32_t m_dock_height = 900;

        // Cascade index
        uint32_t m_cur_cascade_index = 1;

        bool m_is_dxgi_flip_model = false;                                      // Use DXGI flip model
        bool m_window_minimized = false;                                        // Renderer minimized
        bool m_has_released = false;                                            // Whether renderer has been reset
    };
}
