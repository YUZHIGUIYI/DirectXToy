//
// Created by ZZK on 2024/3/21.
//

#include <Sandbox/Docks/scene_dock.h>
#include <Toy/Core/subsystem.h>
#include <Toy/Core/input.h>
#include <Toy/Runtime/task_system.h>
#include <Toy/Runtime/renderer.h>
#include <Toy/Runtime/scene_graph.h>
#include <Sandbox/System/editing_system.h>
#include <Sandbox/System/picking_system.h>

#include <imgui_user/imgui_user.h>

namespace toy::editor
{
    static std::variant<std::true_type, std::false_type> bool_variant(bool condition)
    {
        if (condition) return std::true_type{};
        else return std::false_type{};
    }

    static void update_transform(TransformComponent &transform_comp, const DirectX::XMFLOAT3 &position, const DirectX::XMFLOAT4 &rotation, const DirectX::XMFLOAT3 &scale, ImGuizmo::OPERATION operation_type)
    {
        auto change_position = bool_variant(operation_type == ImGuizmo::OPERATION::TRANSLATE);
        auto change_rotation = bool_variant(operation_type == ImGuizmo::OPERATION::ROTATE);
        auto change_scale = bool_variant(operation_type == ImGuizmo::OPERATION::SCALE);
        std::visit([&transform_comp, &position, &rotation, &scale] (auto position_changed, auto rotation_changed, auto scale_changed) {
            if constexpr (position_changed)
            {
                transform_comp.transform.set_position(position);
            } else if constexpr (rotation_changed)
            {
                transform_comp.transform.set_rotation_in_quaternion(rotation);
            } else if constexpr (scale_changed)
            {
                transform_comp.transform.set_scale(scale);
            }
        }, change_position, change_rotation, change_scale);
    }

    SceneDock::SceneDock(std::string &&dock_name)
    : m_dock_name{ std::move(dock_name) }, m_viewport_setting{}
    {

    }

    bool SceneDock::is_viewport_resize()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });
        ImGui::Begin(m_dock_name.c_str());

        auto viewport_size     = ImGui::GetContentRegionAvail();
        auto viewer_min_region = ImGui::GetWindowContentRegionMin();
        auto viewer_max_region = ImGui::GetWindowContentRegionMax();
        auto viewer_offset     = ImGui::GetWindowPos();

        bool viewport_size_changed = (static_cast<int32_t>(viewport_size.x) != m_viewport_setting.width
                                        || static_cast<int32_t>(viewport_size.y) != m_viewport_setting.height);
        ImGui::End();
        ImGui::PopStyleVar();

        if (viewport_size_changed)
        {
            m_viewport_setting.width = static_cast<int32_t>(viewport_size.x);
            m_viewport_setting.height = static_cast<int32_t>(viewport_size.y);

            m_viewport_setting.lower_bound = DirectX::XMFLOAT2{ viewer_offset.x + viewer_min_region.x, viewer_offset.y + viewer_min_region.y };
            m_viewport_setting.upper_bound = DirectX::XMFLOAT2{ viewer_offset.x + viewer_max_region.x, viewer_offset.y + viewer_max_region.y };

            auto&& picking_system = core::get_subsystem<PickingSystem>();
            picking_system.reset_viewport_setting(m_viewport_setting);
        }

        return viewport_size_changed;
    }

    void SceneDock::on_resize()
    {
        if (is_viewport_resize())
        {
            auto&& task_system = core::get_subsystem<runtime::TaskSystem>();
            task_system.push(DockResizeEvent{ m_viewport_setting.width, m_viewport_setting.height });
        }
    }

    void SceneDock::on_render(float delta_time)
    {
        auto&& renderer = core::get_subsystem<runtime::Renderer>();
        auto&& editing_system = core::get_subsystem<EditingSystem>();
        auto&& selected_entity = editing_system.get_selected_entity();
        auto&& editor_camera_entity = editing_system.get_editor_camera_entity();
        auto&& editor_camera = editor_camera_entity.get_component<CameraComponent>().camera;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });
        ImGui::Begin(m_dock_name.c_str());

        bool dock_focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootWindow);
        bool dock_hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootWindow);

        ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyle().Colors[ImGuiCol_Button]);
        ImGui::RenderFrameEx(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), true, 0.0f, 2.0f);
        ImGui::PopStyleColor();

        ImGui::Image(renderer.get_view_srv(), ImGui::GetContentRegionAvail());

        // Render gizmo
        if (dock_focused && selected_entity.is_valid())
        {
            auto&& gizmo_snap = editing_system.get_gizmo_snap();
            auto&& gizmo_operation = editing_system.get_gizmo_operation();
            on_gizmos_render(selected_entity, gizmo_snap, gizmo_operation, editor_camera);
        }

        // Handle camera movement
        if (dock_focused && dock_hovered)
        {
            float mouse_speed = editing_system.get_mouse_speed();
            float mouse_sensitivity_x = editing_system.get_mouse_sensitivity_x();
            float mouse_sensitivity_y = editing_system.get_mouse_sensitivity_y();
            auto first_person_camera = std::static_pointer_cast<FirstPersonCamera>(editor_camera);
            on_camera_movement(first_person_camera, delta_time, mouse_speed, mouse_sensitivity_x, mouse_sensitivity_y);
        }

        ImGui::End();
        ImGui::PopStyleVar();
    }

    void SceneDock::on_gizmos_render(EntityWrapper &selected_entity, GizmoSnap &gizmo_snap,
                                        ImGuizmo::OPERATION &gizmo_operation, const std::shared_ptr<Camera> &editor_camera)
    {
        using namespace DirectX;

        auto&& input_controller = core::get_subsystem<InputController>();
        if (!ImGuizmo::IsUsing())
        {
            if (input_controller.is_key_pressed_with_mod(key::W, key::LeftShift))
            {
                gizmo_operation = ImGuizmo::OPERATION::TRANSLATE;
            } else if (input_controller.is_key_pressed_with_mod(key::R, key::LeftShift))
            {
                gizmo_operation = ImGuizmo::OPERATION::SCALE;
            } else if (input_controller.is_key_pressed_with_mod(key::E, key::LeftShift))
            {
                gizmo_operation = ImGuizmo::OPERATION::ROTATE;
            }
        }

        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist();
        ImGuizmo::SetRect(m_viewport_setting.lower_bound.x, m_viewport_setting.lower_bound.y,
                            m_viewport_setting.upper_bound.x - m_viewport_setting.lower_bound.x,
                            m_viewport_setting.upper_bound.y - m_viewport_setting.lower_bound.y);

        auto camera_proj = editor_camera->get_proj_xm();
        auto camera_view = editor_camera->get_view_xm();

        auto& transform_component = selected_entity.get_component<TransformComponent>();
        auto transform_matrix = transform_component.transform.get_local_to_world_matrix_xm();

        float* snap_values = nullptr;
        if (gizmo_operation == ImGuizmo::OPERATION::TRANSLATE)
        {
            snap_values = gizmo_snap.translation_snap.data();
        } else if (gizmo_operation == ImGuizmo::OPERATION::ROTATE)
        {
            snap_values = &gizmo_snap.rotation_degree_snap;
        } else if (gizmo_operation == ImGuizmo::OPERATION::SCALE)
        {
            snap_values = &gizmo_snap.scale_snap;
        }
        // Manipulate entity
        ImGuizmo::Manipulate((float *)&camera_view, (float *)&camera_proj, gizmo_operation, ImGuizmo::LOCAL, (float *)&transform_matrix,
                                nullptr, snap_values);

        // Apply entity transform
        if (ImGuizmo::IsUsing())
        {
            XMVECTOR translation_vec;
            XMVECTOR rotation_vec;
            XMVECTOR scale_vec;
            XMFLOAT3 out_position = {};
            XMFLOAT4 out_rotation = {};
            XMFLOAT3 out_scale = {};
            XMMatrixDecompose(&scale_vec, &rotation_vec, &translation_vec, transform_matrix);
            XMStoreFloat3(&out_position, translation_vec);
            XMStoreFloat4(&out_rotation, rotation_vec);
            XMStoreFloat3(&out_scale, scale_vec);
            update_transform(transform_component, out_position, out_rotation, out_scale, gizmo_operation);
        }
    }

    void SceneDock::on_camera_movement(std::shared_ptr<FirstPersonCamera> &editor_camera, float delta_time, float mouse_speed,
                                        float mouse_sensitivity_x, float mouse_sensitivity_y)
    {
        using namespace DirectX;
        auto&& input_controller = core::get_subsystem<InputController>();

        ImGuiIO& io = ImGui::GetIO();

        static constexpr float multiplier = 5.0f;
        if (input_controller.is_mouse_button_pressed(mouse::ButtonMiddle))
        {
            float delta_move_x = io.MouseDelta.x;
            float delta_move_y = io.MouseDelta.y;
            if (input_controller.is_key_pressed(key::LeftShift))
            {
                delta_move_x *= multiplier;
                delta_move_y *= multiplier;
            }
            if (delta_move_x != 0.0f)
            {
                editor_camera->move_local(DirectX::XMFLOAT3{ -1.0f * delta_move_x * mouse_sensitivity_x, 0.0f, 0.0f });
            }
            if (delta_move_y != 0.0f)
            {
                editor_camera->move_local(DirectX::XMFLOAT3{ 0.0f, delta_move_y * mouse_sensitivity_y, 0.0f });
            }
        }

        if (input_controller.is_mouse_button_pressed(mouse::ButtonRight))
        {
            if (input_controller.is_key_pressed(key::W))
            {
                editor_camera->move_local(DirectX::XMFLOAT3{ 0.0f, 0.0f, mouse_speed * delta_time });
            }
            if (input_controller.is_key_pressed(key::S))
            {
                editor_camera->move_local(DirectX::XMFLOAT3{ 0.0f, 0.0f, -mouse_speed * delta_time });
            }
            if (input_controller.is_key_pressed(key::A))
            {
                editor_camera->move_local(DirectX::XMFLOAT3{ -mouse_speed * delta_time, 0.0f, 0.0f });
            }
            if (input_controller.is_key_pressed(key::D))
            {
                editor_camera->move_local(DirectX::XMFLOAT3{ mouse_speed * delta_time, 0.0f, 0.0f });
            }

            float yaw = io.MouseDelta.x * mouse_sensitivity_x;
            float pitch = io.MouseDelta.y * mouse_sensitivity_y;
            editor_camera->rotate_y(yaw);
            editor_camera->pitch(pitch);
        }
    }
}