//
// Created by ZZK on 2024/3/16.
//

#pragma once

#include <Sandbox/Docks/dock.h>

namespace toy::editor
{
    struct HierarchyDock final : Dock
    {
    public:
        explicit HierarchyDock(std::string&& dock_name);

        ~HierarchyDock() override = default;

        void on_render(float delta_time) override;

    private:
        void draw_entity_node(EntityWrapper& entity_wrapper, EntityWrapper &selected_entity_wrapper);

    private:
        std::string m_dock_name;
        bool m_edit_label = false;
    };
}
