//
// Created by ZZK on 2023/12/4.
//

#include <Sandbox/gizmos.h>

namespace toy
{
    static std::variant<std::true_type, std::false_type> bool_variant(bool cond)
    {
        if (cond) return std::true_type{};
        else return std::false_type{};
    }

    static void update_transform(TransformComponent &transform_comp, std::array<float, 3> &translation, std::array<float, 3> &rotation, std::array<float, 3> &scale, ImGuizmo::OPERATION operation_type)
    {
        auto change_translation = bool_variant(operation_type == ImGuizmo::OPERATION::TRANSLATE);
        auto change_rotation = bool_variant(operation_type == ImGuizmo::OPERATION::ROTATE);
        auto change_scale = bool_variant(operation_type == ImGuizmo::OPERATION::SCALE);
        std::visit([&transform_comp, &translation, &rotation, &scale] (auto translation_changed, auto rotation_changed, auto scale_changed) {
            if constexpr (translation_changed)
            {
                transform_comp.transform.set_position(translation[0], translation[1], translation[2]);
            } else if constexpr (rotation_changed)
            {
                for (auto& each_rotation : rotation)
                {
                    each_rotation = DirectX::XMConvertToRadians(each_rotation);
                }
                transform_comp.transform.set_rotation(rotation[0], rotation[1], rotation[2]);
            } else if constexpr (scale_changed)
            {
                for (auto& each_scale : scale)
                {
                    each_scale = std::clamp(each_scale, 0.1f, 100.0f);
                }
                transform_comp.transform.set_scale(scale[0], scale[1], scale[2]);
            }
        }, change_translation, change_rotation, change_scale);
    }

    Gizmos::Gizmos(toy::d3d_application_c *d3d_application)
    : d3d_app(d3d_application)
    {

    }

    void Gizmos::on_gizmos_render(Entity &selected_entity, const ViewerSpecification &viewer_specification, std::shared_ptr<camera_c> camera)
    {
        static ImGuizmo::OPERATION gizmo_type = ImGuizmo::OPERATION::TRANSLATE;
        bool shift_pressed = DX_INPUT::is_key_pressed(d3d_app->get_glfw_window(), key::LeftShift);
        if (shift_pressed && !ImGuizmo::IsUsing())
        {
            if (DX_INPUT::is_key_pressed(d3d_app->get_glfw_window(), key::F1))
            {
                gizmo_type = ImGuizmo::OPERATION::TRANSLATE;
            } else if (DX_INPUT::is_key_pressed(d3d_app->get_glfw_window(), key::F2))
            {
                gizmo_type = ImGuizmo::OPERATION::SCALE;
            } else if (DX_INPUT::is_key_pressed(d3d_app->get_glfw_window(), key::F3))
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

        bool keep_snap = DX_INPUT::is_key_pressed(d3d_app->get_glfw_window(), key::LeftControl);
        float snap_value = 5.0f;
        std::array<float, 3> snap_values = { snap_value, snap_value, snap_value };
        ImGuizmo::Manipulate((float *)&camera_view, (float *)&camera_proj, (ImGuizmo::OPERATION)gizmo_type, ImGuizmo::LOCAL, (float *)&transform_matrix,
                                nullptr, keep_snap ? snap_values.data() : nullptr);

        if (ImGuizmo::IsUsing())
        {
            std::array<float, 3> translation_vector = {};
            std::array<float, 3> rotation_vector = {};
            std::array<float, 3> scale_vector = {};
            ImGuizmo::DecomposeMatrixToComponents((float *)&transform_matrix, translation_vector.data(), rotation_vector.data(), scale_vector.data());
            update_transform(transform_component, translation_vector, rotation_vector, scale_vector, gizmo_type);
        }
    }
}