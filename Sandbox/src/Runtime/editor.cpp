//
// Created by ZZK on 2024/3/13.
//

#include <Sandbox/Runtime/editor.h>
#include <Toy/Core/subsystem.h>
#include <Toy/Runtime/events.h>
#include <Toy/Runtime/render_window.h>
#include <Toy/Runtime/task_system.h>
#include <Sandbox/Runtime/gui_system.h>
#include <Sandbox/Runtime/docking_system.h>
#include <Sandbox/Runtime/editing_system.h>
#include <Sandbox/Runtime/picking_system.h>
#include <Sandbox/Docks/scene_dock.h>
#include <Sandbox/Docks/hierarchy_dock.h>
#include <Sandbox/Docks/content_browser_dock.h>
#include <Sandbox/Docks/admin_console_dock.h>
#include <Sandbox/Docks/inspector_dock.h>

#include <Sandbox/Core/file_dialog.h>

namespace toy::editor
{
    void EditorApplication::start()
    {
        runtime::Application::start();

        core::add_subsystem<GuiSystem>();
        core::add_subsystem<EditingSystem>();
        core::add_subsystem<PickingSystem>();
        auto&& docking_system = core::add_subsystem<DockingSystem>();
        docking_system.register_dock(std::make_unique<SceneDock>("Viewport"));
        docking_system.register_dock(std::make_unique<HierarchyDock>("Hierarchy"));
        docking_system.register_dock(std::make_unique<InspectorDock>("Inspector"));
        docking_system.register_dock(std::make_unique<ContentBrowserDock>("Content browser"));
        docking_system.register_dock(std::make_unique<AdminConsoleDock>("Console"));

        runtime::on_frame_render = [this] (float delta_time) {
            this->on_frame_render(delta_time);
        };
    }

    void EditorApplication::setup()
    {
        runtime::Application::setup();
    }

    void EditorApplication::stop()
    {
        runtime::Application::stop();
    }

    void EditorApplication::on_frame_render(float delta_time)
    {
        if (m_show_start_page)
        {
            on_start_page_render();
        } else
        {
            on_docks_render(delta_time);
        }
    }

    void EditorApplication::on_menu_render()
    {
        auto&& input_controller = core::get_subsystem<runtime::InputController>();
        auto&& render_window = core::get_subsystem<runtime::RenderWindow>();
        auto&& task_system = core::get_subsystem<runtime::TaskSystem>();
        bool load_file = false;
        bool exit = false;
        if (input_controller.is_key_pressed_with_mod(key::Q, key::LeftControl))
        {
            exit = true;
        }

        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Load", "Ctrl+O"))
                {
                    load_file = true;
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit", "Ctrl+Q"))
                {
                    exit = true;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        if (exit)
        {
            quit();
            return;
        }

        if (load_file)
        {
            auto filepath = FileDialog::window_open_file_dialog(render_window.get_native_window(), "Load glTF | HDR | FBX",
                                                                "glTF(.gltf, .glb), HDR(.hdr), FBX(.fbx)|*.gltf;*.hdr;*.fbx");
            task_system.push(DropEvent{ filepath });
        }
    }

    void EditorApplication::on_start_page_render()
    {
        static constexpr std::string_view s_start_page_text = "Start page";

        auto on_create_project = [this] () {
            auto&& picking_system = core::get_subsystem<PickingSystem>();
            picking_system.reset_state(false);
            this->m_show_start_page = false;
        };

        ImGui::OpenPopup(s_start_page_text.data());

        const ImVec2 win_size{ 600.0f, 105.0f };
        ImGui::SetNextWindowSize(win_size);
        const ImVec2 win_center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(win_center, ImGuiCond_Always, ImVec2{ 0.5f, 0.5f });

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 15.0f);
        if (ImGui::BeginPopupModal(s_start_page_text.data(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove))
        {
            const ImVec2 available_region = ImGui::GetContentRegionAvail();
            const ImVec2 text_size = ImGui::CalcTextSize(s_start_page_text.data(), nullptr, false, available_region.x);
            ImVec2 text_position;
            text_position.x = (available_region.x - text_size.x) * 0.5f;
            text_position.y = (available_region.y - text_size.y) * 0.5f;

            ImGui::SetCursorPosX(text_position.x);
            ImGui::Text("%s", s_start_page_text.data());

            ImGui::SetCursorPosY(text_size.y * 2.0f);
            if (ImGui::Button("New project", ImVec2{ ImGui::GetContentRegionAvail().x, 0.0f }))
            {
                on_create_project();
            }

            if (ImGui::Button("Open project", ImVec2{ ImGui::GetContentRegionAvail().x, 0.0f }))
            {
                on_create_project();
            }
            ImGui::EndPopup();
        }
        ImGui::PopStyleVar();
    }

    void EditorApplication::on_docks_render(float delta_time)
    {
        on_menu_render();

        auto&& docking_system = core::get_subsystem<DockingSystem>();
        docking_system.tick(delta_time);
    }
}