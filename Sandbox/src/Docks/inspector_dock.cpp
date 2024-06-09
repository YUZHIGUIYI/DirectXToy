//
// Created by ZZK on 2024/4/6.
//

#include <Sandbox/Docks/inspector_dock.h>

#include <imgui_internal.h>

#include <Toy/Core/subsystem.h>
#include <Toy/Runtime/scene_graph.h>
#include <Toy/Runtime/input_controller.h>
#include <Toy/Model/texture_manager.h>
#include <Toy/Model/material.h>
#include <Sandbox/Runtime/editing_system.h>

namespace toy::editor
{
    static std::array<std::string_view, 4> s_vec4_button_label  = { "X", "Y", "Z", "W" };
    static std::array<std::string_view, 4> s_vec4_drag_label  = { "##X", "##Y", "##Z", "##W" };
    static std::array<std::string_view, 4> s_color_button_label = { "R", "G", "B", "A" };
    static std::array<std::string_view, 4> s_color_drag_label = { "##R", "##G", "##B", "##A" };
    static std::string_view s_metalness_button_label = "M";
    static std::string_view s_roughness_button_label = "R";

    static constexpr ImVec2 s_image_size = { 150.0f, 150.0f };
    static constexpr float s_column_width = 114.0f;

    struct ButtonStyle
    {
        ImVec4 button_color = {};
        ImVec4 button_hovered_color = {};
        ImVec4 button_active_color = {};
    };

    static constexpr std::array<ButtonStyle, 4> s_button_style = {
        ButtonStyle{ ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f }, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f }, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f } },
        ButtonStyle{ ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f }, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f }, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f } },
        ButtonStyle{ ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f }, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f }, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f } },
        ButtonStyle{ ImVec4{ 0.1f, 0.25f, 0.25f, 1.0f }, ImVec4{ 0.2f, 0.25f, 0.25f, 1.0f }, ImVec4{ 0.3f, 0.25f, 0.25f, 1.0f } },
    };

    template <typename T>
    concept DirectXMConcept = requires (T data) {
        data.x;
        data.y;
    };

    template <DirectXMConcept DirectXM, size_t N>
    static constexpr auto xm_to_array(const DirectXM &xm_data) -> std::array<std::decay_t<decltype(std::declval<DirectXM>().x)>, N>
    {
        using member_data_type = std::decay_t<decltype(std::declval<DirectXM>().x)>;
        std::array<member_data_type, N> array_data = {};
        auto member_data = reinterpret_cast<const member_data_type *>(&xm_data);
        for (size_t i = 0; i < N; ++i)
        {
            array_data[i] = *member_data;
            ++member_data;
        }
        return array_data;
    }

    template <DirectXMConcept DirectXM>
    static void draw_item(std::string_view item_label, std::span<std::string_view> component_button_label, std::span<std::string_view> component_drag_label, DirectXM &xm_data,
                            float reset_value = 0.0f, float min_value = -10000.0f, float max_value = 10000.0f)
    {
        using member_data_type = std::decay_t<decltype(std::declval<DirectXM>().x)>;

        ImGuiIO& io = ImGui::GetIO();
        auto component_size = component_button_label.size();
        auto bold_font = io.Fonts->Fonts[0];
        auto array_data = reinterpret_cast<member_data_type *>(&xm_data);

        float line_height = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
        ImVec2 button_size{ line_height + 2.0f, line_height };

        ImGui::PushID(item_label.data());

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, s_column_width);
        ImGui::Text("%s", item_label.data());
        ImGui::NextColumn();

        ImGui::PushMultiItemsWidths(static_cast<int32_t>(component_size), ImGui::CalcItemWidth());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2{ 0, 0 });

        for (size_t component_index = 0; component_index < component_size; ++component_index)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, s_button_style[component_index].button_color);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, s_button_style[component_index].button_hovered_color);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, s_button_style[component_index].button_active_color);
            ImGui::PushFont(bold_font);
            if (ImGui::Button(component_button_label[component_index].data(), button_size))
            {
                *array_data = static_cast<member_data_type>(reset_value);
            }
            ImGui::PopFont();
            ImGui::PopStyleColor(3);

            ImGui::SameLine();
            if constexpr (std::is_same_v<member_data_type, float>)
            {
                ImGui::DragFloat(component_drag_label[component_index].data(), array_data, 0.1f, min_value, max_value, "%.2f");
            } else if constexpr (std::is_same_v<member_data_type, int>)
            {
                ImGui::DragInt(component_drag_label[component_index].data(), array_data, 0.1f, static_cast<int>(min_value), static_cast<int>(max_value));
            }
            ImGui::PopItemWidth();

            if (component_index == component_size - 1) continue;

            ImGui::SameLine();
            ++array_data;
        }

        ImGui::PopStyleVar();
        ImGui::Columns(1);

        ImGui::PopID();
    }

    template <typename T>
    requires std::is_same_v<float, T> || std::is_same_v<int, T>
    static void draw_item(std::string_view item_label, std::string_view button_label, std::string_view drag_label, T &data,
                            float reset_value = 0.0f, float min_value = -10000.0f, float max_value = 10000.0f)
    {
        ImGuiIO& io = ImGui::GetIO();
        auto bold_font = io.Fonts->Fonts[0];
        float line_height = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
        ImVec2 button_size{ line_height + 2.0f, line_height };

        ImGui::PushID(item_label.data());

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, s_column_width);
        ImGui::Text("%s", item_label.data());
        ImGui::NextColumn();

        ImGui::PushMultiItemsWidths(1, ImGui::CalcItemWidth());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2{ 0, 0 });

        ImGui::PushStyleColor(ImGuiCol_Button, s_button_style[0].button_color);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, s_button_style[0].button_hovered_color);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, s_button_style[0].button_active_color);
        ImGui::PushFont(bold_font);
        if (ImGui::Button(button_label.data(), button_size))
        {
            data = static_cast<T>(reset_value);
        }
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        if constexpr (std::is_same_v<T, float>)
        {
            ImGui::DragFloat(drag_label.data(), &data, 0.1f, min_value, max_value, "%.2f");
        } else if constexpr (std::is_same_v<T, int>)
        {
            ImGui::DragInt(drag_label.data(), &data, 0.1f, static_cast<int>(min_value), static_cast<int>(max_value));
        }
        ImGui::PopItemWidth();

        ImGui::PopStyleVar();
        ImGui::Columns(1);

        ImGui::PopID();
    }

    static void draw_image(std::string_view item_label, ID3D11ShaderResourceView *shader_resource_view)
    {

        ImGui::PushID(item_label.data());

        const ImVec2 text_size = ImGui::CalcTextSize(item_label.data(), nullptr, false, s_column_width);
        float cursor_pos_y = ImGui::GetCursorPosY();
        float new_cursor_pos_y = std::abs(s_image_size.y - text_size.y) / 2.0f + cursor_pos_y;
        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, s_column_width);
        ImGui::SetCursorPosY(new_cursor_pos_y);
        ImGui::Text("%s", item_label.data());
        ImGui::NextColumn();

        ImGui::PushMultiItemsWidths(1, ImGui::CalcItemWidth());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2{ 0, 0 });

        ImGui::Image(shader_resource_view, s_image_size);

        ImGui::PopItemWidth();

        ImGui::PopStyleVar();
        ImGui::Columns(1);

        ImGui::PopID();
    }

    InspectorDock::InspectorDock(std::string &&dock_name)
    : m_dock_name{ std::move(dock_name) }
    {

    }

    void InspectorDock::on_render(float delta_time)
    {
        auto&& editing_system = core::get_subsystem<EditingSystem>();
        auto&& selected_entity = editing_system.get_selected_entity();

        ImGui::Begin(m_dock_name.c_str());
        if (selected_entity.is_valid())
        {
            draw_entity(selected_entity);
        }
        ImGui::End();
    }

    void InspectorDock::draw_entity(EntityWrapper &entity_wrapper)
    {
        auto& static_mesh_component = entity_wrapper.get_component<StaticMeshComponent>();
        auto& transform_component = entity_wrapper.get_component<TransformComponent>();
        auto& entity_tag = entity_wrapper.get_component<TagComponent>().tag;

        // Tag component
        static std::array<char, 64> tag_buffer;
        tag_buffer.fill(0);
        size_t minimum_tag_size = std::min(entity_tag.size(), tag_buffer.size());
        std::memcpy(tag_buffer.data(), entity_tag.data(), minimum_tag_size);
        if (ImGui::InputText("##Tag", tag_buffer.data(), sizeof(tag_buffer), ImGuiInputTextFlags_EnterReturnsTrue))
        {
            entity_tag.assign(tag_buffer.data(), tag_buffer.size());
        }
        ImGui::Separator();

        // Transform component
        auto position = transform_component.transform.position;
        auto rotation = transform_component.transform.get_euler_angles();
        auto scale = transform_component.transform.scale;
        rotation = math::float3_degrees(rotation);

        ImGui::PushID(&transform_component);
        ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
        if (ImGui::CollapsingHeader("Transform"))
        {
            ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 8.0f);
            ImGui::TreePush("Transform");

            draw_item("Translation", std::span{ s_vec4_button_label.data(), 3 }, std::span{ s_vec4_drag_label.data(), 3 }, position);
            draw_item("Rotation", std::span{ s_vec4_button_label.data(), 3 }, std::span{ s_vec4_drag_label.data(), 3 }, rotation);
            draw_item("Scale", std::span{ s_vec4_button_label.data(), 3 }, std::span{ s_vec4_drag_label.data(), 3 }, scale, 1.0f);

            ImGui::TreePop();
            ImGui::PopStyleVar();
        }
        ImGui::PopID();

        transform_component.transform.set_position(position);
        transform_component.transform.set_rotation_in_degree(rotation);
        transform_component.transform.set_scale(scale);

        // Model component
        ImGui::Separator();
        ImGui::PushID(&static_mesh_component);
        ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
        if (ImGui::CollapsingHeader("Model"))
        {
            ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 8.0f);
            ImGui::TreePush("Model");

            auto&& material = static_mesh_component.model_asset->materials[0];
            auto&& texture_manager = model::TextureManager::get();
            constexpr std::string_view diffuse_color_name = material_semantics_name(model::MaterialSemantics::DiffuseColor);
            constexpr std::string_view metalness_name = material_semantics_name(model::MaterialSemantics::Metalness);
            constexpr std::string_view roughness_name = material_semantics_name(model::MaterialSemantics::Roughness);
            constexpr std::string_view diffuse_map_name = material_semantics_name(model::MaterialSemantics::DiffuseMap);
            constexpr std::string_view normal_map_name = material_semantics_name(model::MaterialSemantics::NormalMap);
            constexpr std::string_view metalness_map_name = material_semantics_name(model::MaterialSemantics::MetalnessMap);
            constexpr std::string_view roughness_map_name = material_semantics_name(model::MaterialSemantics::RoughnessMap);
            if (material.has_property(diffuse_color_name))
            {
                auto&& diffuse_color = material.get<DirectX::XMFLOAT4>(diffuse_color_name);
                draw_item("Diffuse", std::span{ s_color_button_label.data(), s_color_button_label.size() },
                            std::span{ s_color_drag_label.data(), s_color_drag_label.size() }, diffuse_color);
            }
            if (material.has_property(metalness_name))
            {
                auto&& metalness_value = material.get<float>(metalness_name);
                draw_item("Metalness", s_metalness_button_label, s_vec4_drag_label.front(), metalness_value);
            }
            if (material.has_property(roughness_name))
            {
                auto&& roughness_value = material.get<float>(roughness_name);
                draw_item("Roughness", s_roughness_button_label, s_vec4_drag_label.front(), roughness_value);
            }
            if (material.has_property(diffuse_map_name))
            {
                auto&& diffuse_map_path = material.get<std::string>(diffuse_map_name);
                draw_image("DiffuseMap", texture_manager.get_texture(diffuse_map_path));
            }
            if (material.has_property(normal_map_name))
            {
                auto&& normal_map_path = material.get<std::string>(normal_map_name);
                draw_image("NormalMap", texture_manager.get_texture(normal_map_path));
            }
            if (material.has_property(metalness_map_name))
            {
                auto&& metalness_map_path = material.get<std::string>(metalness_map_name);
                draw_image("MetalnessMap", texture_manager.get_texture(metalness_map_path));
            }
            if (material.has_property(roughness_map_name))
            {
                auto&& roughness_map_path = material.get<std::string>(roughness_map_name);
                draw_image("RoughnessMap", texture_manager.get_texture(roughness_map_path));
            }

            ImGui::TreePop();
            ImGui::PopStyleVar();
        }
        ImGui::PopID();
    }
}