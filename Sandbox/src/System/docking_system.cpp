//
// Created by ZZK on 2024/3/13.
//

#include <Sandbox/System/docking_system.h>

namespace toy::editor
{
    void DockingSystem::register_dock(std::unique_ptr<Dock> &&dock)
    {
        m_ui_docks.emplace_back(std::move(dock));
    }

    void DockingSystem::tick(float delta_time)
    {
        // Main menu
        if (ImGui::BeginMainMenuBar())
        {
            for (auto& dock : m_ui_docks)
            {
                dock->on_ui_menu();
            }
            ImGui::EndMainMenuBar();
        }

        // Setting up the viewport and getting information
        for (auto& dock : m_ui_docks)
        {
            dock->on_resize();
        }

        for (auto& dock : m_ui_docks)
        {
            dock->on_render();
        }
    }
}
