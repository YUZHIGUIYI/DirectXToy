//
// Created by ZZK on 2023/5/17.
//

#pragma once

#include <Sandbox/render_object.h>

namespace toy
{
    // For ImGui viewport
    struct ViewportSize
    {
        int32_t pre_width;
        int32_t pre_height;
        int32_t cur_width;
        int32_t cur_height;

        bool is_viewport_size_changed()
        {
            bool has_changed = (pre_width != cur_width) || (pre_height != cur_height);
            if (has_changed)
            {
                pre_width = cur_width;
                pre_height = cur_height;
            }
            return has_changed;
        }

        float get_aspect_ratio() const
        {
            return static_cast<float>(cur_width) / static_cast<float>(cur_height);
        }
    };

    class sandbox_c : public d3d_application_c
    {
    public:
        enum class LightCullTechnique
        {
            CULL_FORWARD_NONE = 0,                  // Forward rendering, no light-cull
            CULL_FORWARD_PREZ_NONE,                 // Forward rendering, pre-write depth, no light-cull
            CULL_FORWARD_COMPUTE_SHADER_TILE,       // Forward rendering, pre-write depth, tile-based light-cull
            CULL_DEFERRED_NONE,                     // Deferred rendering
            CULL_DEFERRED_COMPUTE_SHADER_TILE       // Tile-based deferred rendering
        };

    public:
        sandbox_c(GLFWwindow* window, const std::string& window_name, int32_t init_width, int32_t init_height);
        ~sandbox_c() override;

        void init() override;
        void update_scene(float dt) override;
        void draw_scene() override;

    private:
        void init_resource();
        void init_light_params();

        DirectX::XMFLOAT3 hue_to_rgb(float hue);

        void resize_lights(uint32_t active_lights);
        void update_lights(float dt);

        void XM_CALLCONV setup_lights(DirectX::XMMATRIX view);

        void resize_buffers(uint32_t width, uint32_t height, uint32_t msaa_samples);

        void resize_viewport_buffer(std::unique_ptr<Texture2D>& vp_buffer, const ViewportSize vp_size, DXGI_FORMAT format);

        void render_forward(bool do_pre_z);
        void render_gbuffer();
        void render_skybox();

    private:
        // Settings
        LightCullTechnique m_light_cull_technique = LightCullTechnique::CULL_DEFERRED_COMPUTE_SHADER_TILE;
        bool m_animate_lights = false;
        bool m_lighting_only = false;
        bool m_face_normals = false;
        bool m_visualize_light_count = false;
        bool m_visualize_shading_freq = false;
        bool m_clear_gbuffers = false;
        float m_light_height_scale = 0.25f;

        uint32_t m_msaa_samples = 1;
        bool m_msaa_samples_changed = false;

        // Resources
        std::unique_ptr<Texture2DMS> m_lit_buffer;                                  // Scene rendering buffer
        std::unique_ptr<StructureBuffer<DirectX::XMUINT2>> m_flat_lit_buffer;       // Scene rendering buffer - TBDR
        std::unique_ptr<Depth2DMS> m_depth_buffer;                                  // Depth buffer
        com_ptr<ID3D11DepthStencilView> m_depth_buffer_read_only_dsv;               // Read-only depth stencil view
        std::vector<std::unique_ptr<Texture2DMS>> m_gbuffers;                       // G-Buffers
        std::unique_ptr<Texture2D> m_debug_normal_gbuffer;                          // Debug normal buffer
        std::unique_ptr<Texture2D> m_debug_posz_grad_gbuffer;                       // Debug z-grad buffer
        std::unique_ptr<Texture2D> m_debug_albedo_gbuffer;                          // Debug color buffer

        std::unique_ptr<Texture2D> m_debug_final_scene;                             // Debug deferred rendering scene

        // Disposable
        std::vector<ID3D11RenderTargetView *> m_gbuffer_rtvs;
        std::vector<ID3D11ShaderResourceView *> m_gbuffer_srvs;

        // Lighting
        uint32_t m_active_lights = (MAX_LIGHTS >> 3);
        std::vector<PointLightInitData> m_point_light_init_data;
        std::vector<PointLightApp> m_point_light_params;
        std::vector<DirectX::XMFLOAT3> m_point_light_pos_worlds;
        std::unique_ptr<StructureBuffer<PointLightApp>> m_light_buffer;             // Point light buffer
        std::unique_ptr<StructureBuffer<TileInfo>> m_tile_buffer;                   // Point light index buffer

        // Model
        RenderObject m_sponza;                                                      // Scene model
        RenderObject m_skybox;                                                      // Skybox model

        // Effects
        ForwardEffect m_forward_effect;                                             // Forward rendering effect
        DeferredEffect m_deferred_effect;                                           // Deferred rendering effect
        SkyboxEffect m_skybox_effect;                                               // Skybox rendering effect

        // Camera
        std::shared_ptr<camera_c> m_camera;                                         // Camera
        FirstPersonCameraController m_camera_controller;                            // Camera controller

        // Viewport size
        ViewportSize m_vpp_size;                                                    // Final rendering scene viewport size
        ViewportSize m_vpa_size;                                                    // Albedo rendering scene viewport size
        ViewportSize m_vpn_size;                                                    // Normal rendering scene viewport size
        ViewportSize m_vpz_size;                                                    // Z-grad rendering scene viewport size
    };
}



































