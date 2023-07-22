//
// Created by ZZK on 2023/6/25.
//

#pragma once

#include <Toy/toy.h>
#include <Sandbox/render_object.h>
#include <Sandbox/scene_hierarchy_panel.h>

namespace toy
{
    struct ViewerSpecification
    {
        int32_t width = 640;
        int32_t height = 360;

        DirectX::XMFLOAT2 lower_bound{};
        DirectX::XMFLOAT2 upper_bound{};

        [[nodiscard]] float get_aspect_ratio() const
        {
            return static_cast<float>(width) / static_cast<float>(height);
        }
    };

    enum class LightCullTechnique
    {
        CULL_FORWARD_NONE = 0,                  // Forward rendering, no light-cull
        CULL_FORWARD_PREZ_NONE,                 // Forward rendering, pre-write depth, no light-cull
        CULL_FORWARD_COMPUTE_SHADER_TILE,       // Forward rendering, pre-write depth, tile-based light-cull
        CULL_DEFERRED_NONE,                     // Deferred rendering
        CULL_DEFERRED_COMPUTE_SHADER_TILE       // Tile-based deferred rendering
    };

    class Viewer final : public ILayer
    {
    public:
        Viewer(std::string_view viewer_name);
        ~Viewer() override = default;

        void on_attach(d3d_application_c* app) override;

        void on_detach() override;

        void on_resize() override;

        void on_ui_menu() override;

        void on_ui_render() override;

        void on_render(float dt) override;

        void on_file_drop(std::string_view filename) override;

    private:
        bool is_viewer_size_changed();

        void init_resource();
        void init_light_params();

        DirectX::XMFLOAT3 hue_to_rgb(float hue);

        void resize_lights(uint32_t active_lights);
        void update_lights(float dt);

        void XM_CALLCONV setup_lights(DirectX::XMMATRIX view);

        void resize_buffers(uint32_t width, uint32_t height, uint32_t msaa_samples);

        void render_forward(bool do_pre_z);
        void render_gbuffer();
        void render_skybox();

        void on_gizmo_render(Entity& selected_entity);

        void on_update(float dt);

    private:
        d3d_application_c* m_d3d_app = nullptr;
        std::string_view m_viewer_name;

        // Viewer specification
        ViewerSpecification m_viewer_spec{};

        // Settings
        LightCullTechnique m_light_cull_technique = LightCullTechnique::CULL_DEFERRED_COMPUTE_SHADER_TILE;
        float m_light_height_scale = 0.25f;
        uint32_t m_msaa_samples = 1;

        // Resources
        std::unique_ptr<Texture2DMS> m_lit_buffer;                                  // Scene rendering buffer
        std::unique_ptr<StructureBuffer<DirectX::XMUINT2>> m_flat_lit_buffer;       // Scene rendering buffer - TBDR
        std::unique_ptr<Depth2DMS> m_depth_buffer;                                  // Depth buffer
        com_ptr<ID3D11DepthStencilView> m_depth_buffer_read_only_dsv;               // Read-only depth stencil view
        std::vector<std::unique_ptr<Texture2DMS>> m_gbuffers;                       // G-Buffers
        std::unique_ptr<Texture2D> m_debug_normal_gbuffer;                          // Debug normal buffer
        std::unique_ptr<Texture2D> m_debug_posz_grad_gbuffer;                       // Debug z-grad buffer
        std::unique_ptr<Texture2D> m_debug_albedo_gbuffer;                          // Debug color buffer

        std::unique_ptr<Texture2D> m_viewer_buffer;                                 // Viewer image buffer

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

        // Effects
        ForwardEffect m_forward_effect;                                             // Forward rendering effect
        DeferredEffect m_deferred_effect;                                           // Deferred rendering effect
        SkyboxEffect m_skybox_effect;                                               // Skybox rendering effect

        // Camera
        std::shared_ptr<camera_c> m_camera;                                         // Camera
        FirstPersonCameraController m_camera_controller;                            // Camera controller

        // Check whether load-file operation is currently executing
        std::atomic<bool> m_busy = false;

        // Settings
        bool m_animate_lights = false;
        bool m_lighting_only = false;
        bool m_face_normals = false;
        bool m_visualize_light_count = false;
        bool m_visualize_shading_freq = false;
        bool m_clear_gbuffers = false;
        bool m_msaa_samples_changed = false;

        // Viewer status
        bool m_viewer_focused = false;
        bool m_viewer_hovered = false;

        // TODO: Scene control via ECS
        std::shared_ptr<Scene> m_editor_scene = nullptr;
        Entity m_selected_entity{};
        SceneHierarchyPanel m_scene_hierarchy_panel{};
    };
}










