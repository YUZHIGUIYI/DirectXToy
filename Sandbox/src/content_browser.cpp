//
// Created by ZZK on 2023/12/10.
//

#include <Sandbox/content_browser.h>

#include <imgui_internal.h>

namespace toy
{
    enum class IconType : uint16_t
    {
        Folder, Scene, Mesh, Material, Shader, Prefab, Sound
    };

    enum class EntryAction
    {
        NONE, CLICKED, DOUBLE_CLICKED, RENAMED, DELETED
    };

    static std::unordered_map<IconType, ID3D11ShaderResourceView *> icon_view_map = {};

    static bool draw_entry(IconType icon_type, std::string_view icon_name)
    {
        EntryAction entry_action = EntryAction::NONE;
        auto icon_view = icon_view_map[icon_type];

        ImGui::PushID(icon_name.data());

        ImVec2 item_size = { 60.0f, 60.0f };
        ImVec2 texture_size = item_size;
        ImVec2 uv0 = { 0.0f, 0.0f };
        ImVec2 uv1 = { 1.0f, 1.0f };

        auto color_style = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ color_style.x, color_style.y, color_style.z, 0.44f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ color_style.x, color_style.y, color_style.z, 0.86f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ color_style.x, color_style.y, color_style.z, 1.0f });

        const int32_t padding = 15;
        auto cursor_position = ImGui::GetCursorScreenPos();
        cursor_position.y += item_size.y + padding * 2.0f;
        if (ImGui::ImageButton(icon_view, texture_size, uv0, uv1, padding))
        {
            entry_action = EntryAction::CLICKED;
        }
//        ImGui::ImageButtonWithAspectAndTextDOWN(icon_view, std::string{ icon_name }, texture_size, item_size, uv0, uv1, padding);

        ImGui::PopStyleColor(3);

        ImGui::PopID();

        return true;
    }

    ContentBrowser::ContentBrowser()
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

        icon_view_map.try_emplace(IconType::Folder, folder_srv);
        icon_view_map.try_emplace(IconType::Scene, scene_srv);
        icon_view_map.try_emplace(IconType::Mesh, mesh_srv);
        icon_view_map.try_emplace(IconType::Material, material_srv);
        icon_view_map.try_emplace(IconType::Shader, shader_srv);
        icon_view_map.try_emplace(IconType::Prefab, prefab_srv);
        icon_view_map.try_emplace(IconType::Sound, sound_srv);

        root_path = std::filesystem::path{ DXTOY_HOME "data/textures" };
    }

    void ContentBrowser::on_browser_render()
    {
        int32_t id = 0;
        const auto temp_root_path = std::filesystem::path{ "/data" };
        const auto hierarchy = std::filesystem::directory_iterator{ root_path };
        std::vector<std::filesystem::directory_entry> directory_entries;
        for (auto&& directory_entry : hierarchy)
        {
            directory_entries.push_back(directory_entry);
        }

//        if (root_path != temp_root_path || !std::filesystem::exists(cache_path))
//        {
//            root_path = temp_root_path;
//            cache_iter = std::filesystem::directory_iterator{ root_path };
//        }

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
//        for (auto&& directory_entry : hierarchy)
//        {
//            if (!directory_entry.is_directory()) continue;
//            ImGui::SameLine();
//            ImGui::AlignTextToFramePadding();
//            ImGui::TextUnformatted("/");
//            ImGui::SameLine();
//
//            ImGui::PushID(id++);
//            ImGui::Button(directory_entry.path().filename().string().c_str());
//            ImGui::PopID();
//        }
        ImGui::Separator();

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
        std::filesystem::path current_path = cache_path.root_path();
        if (ImGui::BeginChild("Asset content", ImGui::GetContentRegionAvail(), false, flags))
        {
            auto process_cache_entry = [&] (const std::filesystem::directory_entry &cache_entry)
            {
                auto&& absolute_path = cache_entry.path();
                auto file_extension = absolute_path.extension().string();
                auto relative_path = absolute_path.relative_path();
                auto filename = absolute_path.filename().string();

                if (cache_entry.is_directory())
                {
                    draw_entry(IconType::Folder, filename);
                } else if (file_extension == ".dds")
                {
                    draw_entry(IconType::Shader, filename);
                } else if (file_extension == ".png")
                {
                    draw_entry(IconType::Scene, filename);
                } else
                {
                    draw_entry(IconType::Sound, filename);
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

    void ContentBrowser::set_cache_path(const std::filesystem::path &path)
    {
        if (cache_path == path) return;
        cache_path = path;
        cache_iter = std::filesystem::directory_iterator{ path };
    }
}










































