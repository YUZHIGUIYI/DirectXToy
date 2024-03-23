//
// Created by ZZK on 2024/3/22.
//

#include <Sandbox/System/editing_system.h>
#include <Toy/Core/subsystem.h>
#include <Toy/runtime/scene_graph.h>

namespace toy::editor
{
    EditingSystem::EditingSystem()
    {
        // Initialize editor camera
        using namespace DirectX;
        auto&& scene_graph = core::get_subsystem<runtime::SceneGraph>();
        auto editor_camera = std::make_shared<FirstPersonCamera>();
        float width = 1600.0f;
        float height = 900.0f;
        editor_camera->set_viewport(0.0f, 0.0f, width, height);
        editor_camera->set_frustum(XM_PI / 3.0f, width / height, 0.5f, 360.0f);
        editor_camera->look_at(XMFLOAT3{-60.0f, 10.0f, 2.5f }, XMFLOAT3{ 0.0f, 0.0f, 0.0f }, XMFLOAT3{ 0.0f, 1.0f, 0.0f });

        m_editor_camera_entity = scene_graph.create_entity("EditorCamera");
        auto&& camera_component = m_editor_camera_entity.add_component<CameraComponent>();
        camera_component.camera = std::move(editor_camera);
        camera_component.camera_type = CameraType::FirstPersonCamera;
    }

    void EditingSystem::select(uint32_t entity_id)
    {
        auto&& scene_graph = core::get_subsystem<runtime::SceneGraph>();
        m_selected_entity = scene_graph.get_entity(entity_id);
    }

    void EditingSystem::unselect()
    {
        m_selected_entity = {};
    }

    EntityWrapper &EditingSystem::get_editor_camera_entity()
    {
        return m_editor_camera_entity;
    }

    EntityWrapper &EditingSystem::get_selected_entity()
    {
        return m_selected_entity;
    }

    GizmoSnap &EditingSystem::get_gizmo_snap()
    {
        return m_gizmo_snap;
    }

    ImGuizmo::OPERATION &EditingSystem::get_gizmo_operation()
    {
        return m_gizmo_operation;
    }

    float &EditingSystem::get_mouse_speed()
    {
        return m_mouse_speed;
    }

    float &EditingSystem::get_mouse_sensitivity_x()
    {
        return m_mouse_sensitivity_x;
    }

    float &EditingSystem::get_mouse_sensitivity_y()
    {
        return m_mouse_sensitivity_y;
    }
}