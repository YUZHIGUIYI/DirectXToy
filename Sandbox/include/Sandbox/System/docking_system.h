//
// Created by ZZK on 2024/3/13.
//

#pragma once

#include <Sandbox/Dock/dock.h>

namespace toy::editor
{
    struct DockingSystem
    {
    public:
        DockingSystem() = default;

        void register_dock(std::unique_ptr<Dock> &&dock);

        void tick(float delta_time);

    private:
        std::vector<std::unique_ptr<Dock>> m_ui_docks;
    };
}
