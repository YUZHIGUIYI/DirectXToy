//
// Created by ZZK on 2024/3/16.
//

#pragma once

#include <Sandbox/Core/common.h>
#include <Sandbox/Docks/dock.h>
#include <Toy/Scene/entity_wrapper.h>

namespace toy::editor
{
    struct SceneDock final : Dock
    {
    public:
        explicit SceneDock(std::string &&dock_name);

        ~SceneDock() override = default;

        void on_resize() override;

        void on_render(float delta_time) override;

    private:
        bool is_viewport_resize();

        void on_gizmos_render(EntityWrapper &selected_entity, GizmoSnap &gizmo_snap, ImGuizmo::OPERATION &gizmo_operation, const std::shared_ptr<Camera> &editor_camera);

        void on_camera_movement(std::shared_ptr<FirstPersonCamera> &editor_camera, float delta_time, float mouse_speed, float mouse_sensitivity_x, float mouse_sensitivity_y);

    private:
        std::string m_dock_name;
        ViewportSetting m_viewport_setting;
    };
}
