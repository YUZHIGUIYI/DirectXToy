//
// Created by ZZK on 2023/12/10.
//

#include <Sandbox/content_browser.h>

#include <imgui_internal.h>
#include <imgui_user/imgui_user.h>

namespace toy
{
    enum class IconType : uint16_t
    {
        Folder, Scene, Mesh, Material, Shader, Prefab, Sound, Text
    };

    enum class EntryAction
    {
        NONE, CLICKED, DOUBLE_CLICKED, RENAMED, DELETED
    };

    struct ShadowResourceViewInfo
    {
        ID3D11ShaderResourceView* shader_resource_view = nullptr;
        float width = 0.0f;
        float height = 0.0f;
    };

    static std::unordered_map<IconType, ShadowResourceViewInfo> icon_view_map = {};
    static const auto s_browser_root_path = std::filesystem::path{ "/Content" };
    static const auto s_root_path = std::filesystem::path{ DXTOY_HOME "data/textures" };

    static void store_shader_resource_view_info(IconType icon_type, ID3D11ShaderResourceView *shader_resource_view)
    {
        if (!shader_resource_view)
        {
            DX_CRITICAL("Invalid shader resource view input");
        }

        com_ptr<ID3D11Texture2D> temp_texture = nullptr;
        shader_resource_view->GetResource(reinterpret_cast<ID3D11Resource **>(temp_texture.GetAddressOf()));
        if (!temp_texture)
        {
            DX_CRITICAL("Current shader resource view failed to get its ID3D11Resource");
        }
        D3D11_TEXTURE2D_DESC temp_texture_desc = {};
        temp_texture->GetDesc(&temp_texture_desc);

        icon_view_map.try_emplace(icon_type, shader_resource_view, static_cast<float>(temp_texture_desc.Width), static_cast<float>(temp_texture_desc.Height));
    }

    static bool draw_entry(IconType icon_type, const float size, const std::filesystem::path &icon_path, std::function<void(const std::filesystem::path &path)> &&on_clicked = {})
    {
        EntryAction entry_action = EntryAction::NONE;
        auto&& icon_info = icon_view_map[icon_type];
        auto icon_name = icon_path.filename().string();

        ImGui::PushID(icon_name.c_str());

        ImVec2 item_size = { size, size };
        ImVec2 texture_size = { icon_info.width, icon_info.height };
        ImVec2 uv0 = { 0.0f, 0.0f };
        ImVec2 uv1 = { 1.0f, 1.0f };

        auto color_style = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ color_style.x, color_style.y, color_style.z, 0.44f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ color_style.x, color_style.y, color_style.z, 0.86f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ color_style.x, color_style.y, color_style.z, 1.0f });

        const int32_t padding = 15;
        auto cursor_position = ImGui::GetCursorScreenPos();
        cursor_position.y += item_size.y + padding * 2.0f;
        if (ImGui::ImageButtonWithAspectAndTextDOWN(icon_info.shader_resource_view, icon_name, texture_size, item_size, uv0, uv1, padding))
        {
            entry_action = EntryAction::CLICKED;
        }

        ImGui::PopStyleColor(3);

        if (ImGui::IsItemHovered())
        {
            if (ImGui::IsMouseDoubleClicked(0) && icon_type == IconType::Folder)
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                entry_action = EntryAction::DOUBLE_CLICKED;
            }
            auto imgui_context = ImGui::GetCurrentContext();
            if (!imgui_context->DragDropActive)
            {
                ImGui::BeginTooltip();
                ImGui::TextUnformatted(icon_name.data());
                ImGui::EndTooltip();
            }
        }

        if (ImGui::BeginPopupContextItem("ENTRY_CONTEXT_MENU"))
        {
            if (ImGui::MenuItem("RENAME", "F2"))
            {
                entry_action = EntryAction::RENAMED;
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("DELETE", "DEL"))
            {
                entry_action = EntryAction::DELETED;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        // Handle entry action
        switch (entry_action)
        {
            case EntryAction::CLICKED:
            {
                break;
            }
            case EntryAction::DOUBLE_CLICKED:
            {
                on_clicked(icon_path);
                break;
            }
            case EntryAction::RENAMED:
            {
                break;
            }
            case EntryAction::DELETED:
            {
                break;
            }
            default:
                break;
        }

        ImGui::PopID();

        return true;
    }

    ContentBrowser::ContentBrowser()
    : browser_cache_path(s_root_path)
    {
        using namespace toy::model;

        auto&& texture_manager = TextureManager::get();
        auto folder_srv = texture_manager.create_from_file(DXTOY_HOME "data/icons/folder.png");
        auto scene_srv = texture_manager.create_from_file(DXTOY_HOME "data/icons/scene.png");
        auto mesh_srv = texture_manager.create_from_file(DXTOY_HOME "data/icons/mesh.png");
        auto material_srv = texture_manager.create_from_file(DXTOY_HOME "data/icons/material.png");
        auto shader_srv = texture_manager.create_from_file(DXTOY_HOME "data/icons/shader.png");
        auto prefab_srv = texture_manager.create_from_file(DXTOY_HOME "data/icons/prefab.png");
        auto sound_srv = texture_manager.create_from_file(DXTOY_HOME "data/icons/sound.png");
        auto text_srv = texture_manager.create_from_file(DXTOY_HOME "data/icons/copy.png");

        store_shader_resource_view_info(IconType::Folder, folder_srv);
        store_shader_resource_view_info(IconType::Scene, scene_srv);
        store_shader_resource_view_info(IconType::Mesh, mesh_srv);
        store_shader_resource_view_info(IconType::Material, material_srv);
        store_shader_resource_view_info(IconType::Shader, shader_srv);
        store_shader_resource_view_info(IconType::Prefab, prefab_srv);
        store_shader_resource_view_info(IconType::Sound, sound_srv);
        store_shader_resource_view_info(IconType::Text, text_srv);
    }

    void ContentBrowser::on_browser_render()
    {
        // Prohibit crossing root path
        if (browser_cache_path == s_browser_root_path)
        {
            browser_cache_path = s_root_path.root_path();
        }

        const auto  browser_temp_path = browser_cache_path;
        const auto hierarchy = std::filesystem::directory_iterator{ browser_temp_path };
        int32_t id = 0;

        // Store all director entries of current path
        std::vector<std::filesystem::directory_entry> directory_entries;
        for (auto&& directory_entry : hierarchy)
        {
            directory_entries.push_back(directory_entry);
        }

        // Split paths and store
        std::vector<std::filesystem::path> split_paths;
        auto split_path = browser_temp_path;
        while (split_path.has_parent_path() && split_path.has_filename())
        {
            split_paths.emplace_back(split_path);
            split_path = split_path.parent_path();
        }
        split_paths.emplace_back(s_browser_root_path);

        ImGui::Begin("Content browser");

        if (ImGui::Button("Import..."))
        {
            // TODO
        }
        ImGui::SameLine();
        ImGui::PushItemWidth(80.0f);
        ImGui::SliderFloat("##scale", &icon_scale, 0.5f, 1.0f);
        const float size = ImGui::GetFrameHeight() * 5.0f * icon_scale;
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::TextUnformatted("Scale icons");
            ImGui::EndTooltip();
        }
        ImGui::PopItemWidth();
        for (auto it = split_paths.crbegin(); it != split_paths.crend(); ++it)
        {
            ImGui::SameLine();
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("/");
            ImGui::SameLine();

            ImGui::PushID(id++);
            bool clicked = ImGui::Button(it->filename().string().c_str());
            ImGui::PopID();

            if (clicked)
            {
                set_browser_cache_path(*it);
                break;
            }
        }
        ImGui::Separator();

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
        if (ImGui::BeginChild("Asset content", ImGui::GetContentRegionAvail(), false, flags))
        {
            auto process_cache_entry = [this, size] (const std::filesystem::directory_entry &cache_entry)
            {
                auto&& absolute_path = cache_entry.path();
                auto file_extension = absolute_path.extension().string();

                if (cache_entry.is_directory())
                {
                    draw_entry(IconType::Folder, size, absolute_path, [this] (const std::filesystem::path &path) { this->set_browser_cache_path(path); });
                } else if (file_extension == ".hlsl" || file_extension == ".glsl")
                {
                    draw_entry(IconType::Shader, size, absolute_path);
                } else if (file_extension == ".png" || file_extension == ".jpg" || file_extension == ".dds")
                {
                    draw_entry(IconType::Scene, size, absolute_path);
                } else if (file_extension == ".txt" || file_extension == ".ini" || file_extension == ".log")
                {
                    draw_entry(IconType::Text, size, absolute_path);
                } else
                {
                    draw_entry(IconType::Sound, size, absolute_path);
                }
            };

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 10.0f, 10.0f });
            const auto& style = ImGui::GetStyle();
            auto avail = ImGui::GetContentRegionAvail().x;
            auto item_size = size + style.ItemSpacing.x;
            auto items_per_line_exact = avail / item_size;
            auto items_per_line_floor = std::floor(items_per_line_exact);
            auto entry_count = directory_entries.size();
            auto items_per_line = std::min(static_cast<size_t>(items_per_line_floor), entry_count);
            auto extra = ((items_per_line_exact - items_per_line_floor) * item_size) / std::max(1.0f, items_per_line_floor - 1.0f);
            auto lines = std::max<int32_t>(0, int32_t(std::ceil(float(entry_count) / float(items_per_line))));
            ImGuiListClipper clipper;
            clipper.Begin(lines);
            while (clipper.Step())
            {
                for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
                {
                    auto start = static_cast<size_t>(i) * items_per_line;
                    auto end = start + std::min(entry_count - start, items_per_line);
                    for (size_t j = start; j < end; ++j)
                    {
                        ImGui::PushID(int32_t(j));
                        process_cache_entry(directory_entries[j]);
                        ImGui::PopID();

                        if (j != (end - 1))
                        {
                            ImGui::SameLine(0.0f, style.ItemSpacing.x + extra);
                        }
                    }
                }
            }
            ImGui::PopStyleVar();
        }
        ImGui::EndChild();

        ImGui::End();
    }

    void ContentBrowser::set_browser_cache_path(const std::filesystem::path &path)
    {
        if (browser_cache_path != path)
        {
            browser_cache_path = path;
        }
    }
}










































