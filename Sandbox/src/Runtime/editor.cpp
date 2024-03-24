//
// Created by ZZK on 2024/3/13.
//

#include <Sandbox/Runtime/editor.h>
#include <Toy/Core/subsystem.h>
#include <Toy/Runtime/events.h>
#include <Toy/Runtime/render_window.h>
#include <Toy/Runtime/task_system.h>
#include <Sandbox/System/gui_system.h>
#include <Sandbox/System/docking_system.h>
#include <Sandbox/System/editing_system.h>
#include <Sandbox/System/picking_system.h>
#include <Sandbox/Docks/scene_dock.h>
#include <Sandbox/Docks/hierarchy_dock.h>
#include <Sandbox/Docks/content_browser_dock.h>
#include <Sandbox/Docks/admin_console_dock.h>

#include <Sandbox/file_dialog.h>

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
        auto&& input_controller = core::get_subsystem<InputController>();
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
        auto on_create_project = [this] () {
            this->m_show_start_page = false;
        };

        ImGui::PushFont(nullptr);
        ImGui::TextUnformatted("Recent projects");
        ImGui::Separator();

        ImGui::BeginGroup();
        {
            if (ImGui::Button("New project", ImVec2{ ImGui::GetContentRegionAvail().x, 0.0f }))
            {
                on_create_project();
            }
            if (ImGui::Button("Open other", ImVec2{ ImGui::GetContentRegionAvail().x, 0.0f }))
            {
                on_create_project();
            }
        }
        ImGui::EndGroup();
        ImGui::PopFont();
    }

    void EditorApplication::on_docks_render(float delta_time)
    {
        on_menu_render();

        auto&& docking_system = core::get_subsystem<DockingSystem>();
        docking_system.tick(delta_time);
    }
}