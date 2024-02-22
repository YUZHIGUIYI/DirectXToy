//
// Created by ZZK on 2023/12/4.
//

#include <Sandbox/gizmos.h>

namespace toy
{
    struct Snap
    {
        std::array<float, 3> translation_snap = { 1.0f, 1.0f, 1.0f };
        float rotation_degree_snap = 15.0f;
        float scale_snap = 0.1f;
    };

    static Snap s_snap_data = {};

    static std::variant<std::true_type, std::false_type> bool_variant(bool cond)
    {
        if (cond) return std::true_type{};
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

    Gizmos::Gizmos(GLFWwindow *glfw_window)
    : m_glfw_window(glfw_window)
    {

    }

    void Gizmos::on_gizmos_render(Entity &selected_entity, const ViewerSpecification &viewer_specification, std::shared_ptr<Camera> camera)
    {
        using namespace DirectX;

        static ImGuizmo::OPERATION gizmo_type = ImGuizmo::OPERATION::TRANSLATE;
        auto&& input_controller = InputController::get();
        if (!ImGuizmo::IsUsing())
        {
            if (input_controller.is_key_pressed_with_mod(key::W, key::LeftShift))
            {
                gizmo_type = ImGuizmo::OPERATION::TRANSLATE;
            } else if (input_controller.is_key_pressed_with_mod(key::R, key::LeftShift))
            {
                gizmo_type = ImGuizmo::OPERATION::SCALE;
            } else if (input_controller.is_key_pressed_with_mod(key::E, key::LeftShift))
            {
                gizmo_type = ImGuizmo::OPERATION::ROTATE;
            }
        }
        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist();
        ImGuizmo::SetRect(viewer_specification.lower_bound.x, viewer_specification.lower_bound.y,
                        viewer_specification.upper_bound.x - viewer_specification.lower_bound.x,
                        viewer_specification.upper_bound.y - viewer_specification.lower_bound.y);

        auto camera_proj = camera->get_proj_xm();
        auto camera_view = camera->get_view_xm();

        auto& transform_component = selected_entity.get_component<TransformComponent>();
        auto transform_matrix = transform_component.transform.get_local_to_world_matrix_xm();

        float* snap_values = nullptr;
        if (gizmo_type == ImGuizmo::OPERATION::TRANSLATE)
        {
            snap_values = s_snap_data.translation_snap.data();
        } else if (gizmo_type == ImGuizmo::OPERATION::ROTATE)
        {
            snap_values = &s_snap_data.rotation_degree_snap;
        } else if (gizmo_type == ImGuizmo::OPERATION::SCALE)
        {
            snap_values = &s_snap_data.scale_snap;
        }
        ImGuizmo::Manipulate((float *)&camera_view, (float *)&camera_proj, (ImGuizmo::OPERATION)gizmo_type, ImGuizmo::LOCAL, (float *)&transform_matrix,
                                nullptr, snap_values);

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

            update_transform(transform_component, out_position, out_rotation, out_scale, gizmo_type);
        }
    }
}