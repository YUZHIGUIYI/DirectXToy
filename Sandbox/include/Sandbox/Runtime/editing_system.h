//
// Created by ZZK on 2024/3/22.
//

#pragma once

#include <Sandbox/Core/common.h>

namespace toy::editor
{
    struct EditingSystem
    {
    public:
        EditingSystem();

        void select(uint32_t entity_id);

        void unselect();

        EntityWrapper &get_editor_camera_entity();

        EntityWrapper &get_selected_entity();

        GizmoSnap &get_gizmo_snap();

        ImGuizmo::OPERATION &get_gizmo_operation();

        float &get_mouse_speed();

        float &get_mouse_sensitivity_x();

        float &get_mouse_sensitivity_y();

    private:
        EntityWrapper m_selected_entity = {};
        EntityWrapper m_editor_camera_entity = {};

        GizmoSnap m_gizmo_snap = {};
        ImGuizmo::OPERATION m_gizmo_operation = ImGuizmo::OPERATION::TRANSLATE;

        float m_mouse_speed = 15.0f;
        float m_mouse_sensitivity_x = 0.008f;
        float m_mouse_sensitivity_y = 0.008f;
    };
}
