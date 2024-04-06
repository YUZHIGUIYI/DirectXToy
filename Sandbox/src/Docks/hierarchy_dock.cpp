//
// Created by ZZK on 2024/3/16.
//

#include <Sandbox/Docks/hierarchy_dock.h>
#include <imgui_internal.h>

#include <Toy/Core/subsystem.h>
#include <Toy/Runtime/scene_graph.h>
#include <Toy/Runtime/input_controller.h>
#include <Sandbox/Runtime/editing_system.h>

namespace toy::editor
{
    HierarchyDock::HierarchyDock(std::string&& dock_name)
    : m_dock_name{ std::move( dock_name ) }
    {

    }

    void HierarchyDock::on_render(float delta_time)
    {
        auto&& scene_graph = core::get_subsystem<runtime::SceneGraph>();
        auto&& editing_system = core::get_subsystem<EditingSystem>();
        auto&& static_mesh_entities = scene_graph.get_static_mesh_entities();
        auto&& selected_entity = editing_system.get_selected_entity();
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;

        ImGui::Begin(m_dock_name.c_str());
        if (ImGui::BeginChild("HierarchyContent", ImGui::GetContentRegionAvail(), false, window_flags))
        {
            for (auto entity_id : static_mesh_entities)
            {
                auto entity_wrapper = scene_graph.get_entity(static_cast<uint32_t>(entity_id));
                draw_entity_node(entity_wrapper, selected_entity);
            }
        }
        ImGui::EndChild();
        ImGui::End();
    }

    void HierarchyDock::draw_entity_node(EntityWrapper& entity_wrapper, EntityWrapper &selected_entity_wrapper)
    {
        auto&& editing_system = core::get_subsystem<EditingSystem>();
        auto&& input_controller = core::get_subsystem<runtime::InputController>();
        auto&& tag_buffer = entity_wrapper.get_component<TagComponent>().tag;

        ImGui::PushID(static_cast<int32_t>(entity_wrapper.entity_inst));
        ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_Leaf;
        ImVec2 pos = ImGui::GetCursorScreenPos();
        bool is_selected = false;
        if (selected_entity_wrapper.is_valid())
        {
            is_selected = (entity_wrapper == selected_entity_wrapper);
        }
        if (is_selected)
        {
            node_flags |= ImGuiTreeNodeFlags_Selected;
        }

        if (ImGui::IsWindowFocused())
        {
            if (is_selected && !ImGui::IsAnyItemActive())
            {
                if (input_controller.is_key_pressed(key::F2))
                {
                    m_edit_label = true;
                    ImGui::SetKeyboardFocusHere();
                }
            }
        }

        ImGui::AlignTextToFramePadding();
        bool is_opened = ImGui::TreeNodeEx(tag_buffer.c_str(), node_flags);
        if (m_edit_label && is_selected)
        {
            static std::array<char, 64> input_buffer;
            input_buffer.fill(0);
            size_t minimum_buffer_size = std::min(input_buffer.size(), tag_buffer.size());
            std::memcpy(input_buffer.data(), tag_buffer.c_str(), minimum_buffer_size);

            ImGui::SetCursorScreenPos(pos);
            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);

            ImGui::PushID(static_cast<int32_t>(entity_wrapper.entity_inst));
            if (ImGui::InputText("", input_buffer.data(), input_buffer.size(),
                                    ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
            {
                tag_buffer.assign(input_buffer.data(), input_buffer.size());
                m_edit_label = false;
            }

            ImGui::PopItemWidth();

            if (!ImGui::IsItemActive() && (ImGui::IsMouseClicked(0) || ImGui::IsMouseDragging(0)))
            {
                m_edit_label = false;
            }
            ImGui::PopID();
        }

        if (ImGui::IsItemHovered() && !ImGui::IsMouseDragging(0))
        {
            if (ImGui::IsMouseReleased(0))
            {
                if (!is_selected) m_edit_label = false;
                editing_system.select(entity_wrapper);
            }
            if (ImGui::IsMouseDoubleClicked(0))
            {
                m_edit_label = is_selected;
            }
        }

        if (ImGui::IsMouseDoubleClicked(0) && !ImGui::IsItemClicked())
        {
            editing_system.unselect();
        }

        if (is_opened)
        {
            ImGui::TreePop();
        }

        ImGui::PopID();
    }
}






