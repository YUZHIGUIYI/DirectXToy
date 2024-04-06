//
// Created by ZZK on 2024/4/6.
//

#pragma once

#include <Sandbox/Docks/dock.h>

namespace toy::editor
{
    struct InspectorDock final : Dock
    {
    public:
        explicit InspectorDock(std::string &&dock_name);

        ~InspectorDock() override = default;

        void on_render(float delta_time) override;

    private:
        void draw_entity(EntityWrapper &entity_wrapper);

    private:
        std::string m_dock_name;
    };
}
