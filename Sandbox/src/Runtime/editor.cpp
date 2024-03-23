//
// Created by ZZK on 2024/3/13.
//

#include <Sandbox/Runtime/editor.h>
#include <Toy/Core/subsystem.h>
#include <Sandbox/System/gui_system.h>
#include <Sandbox/System/docking_system.h>
#include <Sandbox/Docks/scene_dock.h>
#include <Sandbox/Docks/hierarchy_dock.h>
#include <Sandbox/Docks/content_browser_dock.h>
#include <Sandbox/Docks/admin_console_dock.h>

namespace toy::editor
{
    void EditorApplication::start()
    {
        runtime::Application::start();

        core::add_subsystem<GuiSystem>();
        auto&& docking_system = core::add_subsystem<DockingSystem>();

        docking_system.register_dock(std::make_unique<SceneDock>("Viewport"));
        docking_system.register_dock(std::make_unique<HierarchyDock>("Hierarchy"));
        docking_system.register_dock(std::make_unique<ContentBrowserDock>("Content browser"));
        docking_system.register_dock(std::make_unique<AdminConsoleDock>("Console"));
    }

    void EditorApplication::setup()
    {
        runtime::Application::setup();
    }

    void EditorApplication::stop()
    {
        runtime::Application::stop();
    }
}