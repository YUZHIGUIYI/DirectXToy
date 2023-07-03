//
// Created by ZZK on 2023/7/2.
//

#include <Sandbox/scene_hierarchy_panel.h>

#include <imgui_internal.h>

#include <IconsFontAwesome6.h>

namespace toy
{
    static void draw_vec3_control(std::string_view label, DirectX::XMFLOAT3& vec3, float reset_value = 0.0f, float column_width = 108.0f)
    {
        ImGuiIO& io = ImGui::GetIO();
        auto bold_font = io.Fonts->Fonts[0];

        ImGui::PushID(label.data());

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, column_width);
        ImGui::Text("%s", label.data());
        ImGui::NextColumn();

        ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2{ 0, 0 });

        float line_height = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
        ImVec2 button_size{ line_height + 3.0f, line_height };

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
        ImGui::PushFont(bold_font);
        if (ImGui::Button(ICON_FA_X, button_size))
        {
            vec3.x = reset_value;
        }
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##X", &vec3.x, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        ImGui::PushFont(bold_font);
        if (ImGui::Button(ICON_FA_Y, button_size))
        {
            vec3.y = reset_value;
        }
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##Y", &vec3.y, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
        ImGui::PushFont(bold_font);
        if (ImGui::Button(ICON_FA_Z, button_size))
        {
            vec3.z = reset_value;
        }
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##Z", &vec3.z, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();

        ImGui::PopStyleVar();

        ImGui::Columns(1);

        ImGui::PopID();
    }

    SceneHierarchyPanel::SceneHierarchyPanel(const std::shared_ptr<Scene> &scene_context)
    {
        set_context(scene_context);
    }

    void SceneHierarchyPanel::set_context(const std::shared_ptr<Scene> &scene_context)
    {
        m_scene_context = scene_context;
    }

    void SceneHierarchyPanel::on_ui_render()
    {
        ImGui::Begin("Scene Hierarchy");

        if (m_scene_context)
        {
            auto entity_iterator = m_scene_context->registry_handle.storage<entt::entity>().each();
            for (const auto& it : entity_iterator)
            {
                auto entity_handle = std::get<0>(it);
                Entity entity{ entity_handle, m_scene_context.get() };
                draw_components(entity);
            }
        }

        ImGui::End();
    }

    void SceneHierarchyPanel::draw_components(toy::Entity entity)
    {
        if (!entity.has_component<StaticMeshComponent>())
        {
            return;
        }

        auto& static_mesh_component = entity.get_component<StaticMeshComponent>();
        auto& transform_component = entity.get_component<TransformComponent>();
        if (!static_mesh_component.is_skybox)
        {
            auto& tag = entity.get_component<TagComponent>().tag;

            char buffer[256];
            std::memset(buffer, 0, sizeof(buffer));
            strncpy_s(buffer, sizeof(buffer), tag.data(), 256);
            if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
            {
                // TODO
            }

            draw_vec3_control("Translation", transform_component.transform.get_position());
            draw_vec3_control("Rotation", transform_component.transform.get_rotation());
            draw_vec3_control("Scale", transform_component.transform.get_scale(), 1.0f);
        }
    }
}













