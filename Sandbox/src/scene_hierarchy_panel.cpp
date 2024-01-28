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

    void SceneHierarchyPanel::set_selected_entity(const toy::Entity &selected_entity)
    {
        m_selected_entity = selected_entity;
    }

    void SceneHierarchyPanel::on_ui_render()
    {
        ImGui::Begin("Scene Hierarchy");
        if (m_scene_context)
        {
            for (auto entity_handle : m_scene_context->general_static_mesh_entities)
            {
                Entity entity{ entity_handle, m_scene_context.get() };
                draw_entity_node(entity);
            }
        }
        ImGui::End();

        ImGui::Begin("Properties");
        if (m_selected_entity.is_valid())
        {
            draw_components(m_selected_entity);
        }
        ImGui::End();

    }

    void SceneHierarchyPanel::draw_entity_node(toy::Entity& entity)
    {
        auto&& tag_component = entity.get_component<TagComponent>();
        auto flags = ((m_selected_entity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
        if (ImGui::CollapsingHeader(tag_component.tag.c_str(), flags))
        {
            ImGui::BulletText("%s", tag_component.tag.c_str());
        }
        if (ImGui::IsItemClicked())
        {
            m_selected_entity = entity;
        }
    }

    void SceneHierarchyPanel::draw_components(toy::Entity& entity)
    {
        if (!entity.has_component<StaticMeshComponent>())
        {
            return;
        }

        auto& static_mesh_component = entity.get_component<StaticMeshComponent>();
        auto& transform_component = entity.get_component<TransformComponent>();
        if (!static_mesh_component.is_skybox)
        {
            auto& entity_tag = entity.get_component<TagComponent>().tag;

            static std::array<char, 256> tag_buffer;
            tag_buffer.fill(0);
            size_t minimum_tag_size = std::min(entity_tag.size(), tag_buffer.size());
            std::memcpy(tag_buffer.data(), entity_tag.data(), minimum_tag_size);
            if (ImGui::InputText("##Tag", tag_buffer.data(), sizeof(tag_buffer), ImGuiInputTextFlags_EnterReturnsTrue))
            {
                entity_tag.assign(tag_buffer.data(), tag_buffer.size());
            }

            auto position = transform_component.transform.position;
            auto rotation = transform_component.transform.get_euler_angles();
            auto scale = transform_component.transform.scale;
            rotation = math::float3_degrees(rotation);
            draw_vec3_control("Translation", position);
            draw_vec3_control("Rotation", rotation);
            draw_vec3_control("Scale", scale, 1.0f);
            transform_component.transform.set_position(position);
            transform_component.transform.set_rotation_in_degree(rotation);
            transform_component.transform.set_scale(scale);
        }
    }
}













