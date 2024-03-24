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
        for (auto& dock : m_ui_docks)
        {
            dock->on_resize();
            dock->on_render(delta_time);
        }
    }
}
