//
// Created by ZZK on 2023/10/1.
//

#pragma once

#include <Toy/toy.h>
#include <Sandbox/render_object.h>
#include <Sandbox/scene_hierarchy_panel.h>

namespace toy::viewer
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

    class PBRViewer final : public ILayer
    {
    public:
        explicit PBRViewer(std::string_view viewer_name);
        ~PBRViewer() override;

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

        void resize_buffers(uint32_t width, uint32_t height);

        void render_gbuffer();

        void taa_pass();

        void render_skybox();

        void on_gizmo_render(Entity& selected_entity);

        void on_update(float dt);

    private:
        d3d_application_c* m_d3d_app = nullptr;
        std::string_view m_viewer_name;

        // Viewer specification
        ViewerSpecification m_viewer_spec{};

        // Resources
        std::unique_ptr<Depth2D> m_depth_buffer;                                    // Depth buffer
        std::unique_ptr<Texture2D> m_history_buffer;                                // History frame buffer
        std::unique_ptr<Texture2D> m_cur_buffer;                                    // Current frame buffer
        com_ptr<ID3D11DepthStencilView> m_depth_buffer_read_only_dsv;               // Read-only depth stencil view
        std::vector<std::unique_ptr<Texture2D>> m_gbuffers;                         // G-Buffers

        std::unique_ptr<Texture2D> m_viewer_buffer;                                 // Viewer image buffer

        // Disposable
        std::vector<ID3D11RenderTargetView *> m_gbuffer_rtvs;
        std::vector<ID3D11ShaderResourceView *> m_gbuffer_srvs;

        // Camera
        std::shared_ptr<camera_c> m_camera;                                         // Camera
        FirstPersonCameraController m_camera_controller;                            // Camera controller

        // Check whether load-file operation is currently executing
        std::atomic<bool> m_busy = false;

        // Viewer status
        bool m_viewer_focused = false;
        bool m_viewer_hovered = false;

        // Scene control via ECS
        std::shared_ptr<Scene> m_editor_scene = nullptr;
        Entity m_selected_entity{};
        SceneHierarchyPanel m_scene_hierarchy_panel{};
    };
}



