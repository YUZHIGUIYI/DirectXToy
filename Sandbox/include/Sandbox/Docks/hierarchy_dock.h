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

        void set_context(const std::shared_ptr<Scene> &scene_graph_context);

        void set_entity(const Entity& selected_entity);

        void on_render() override;

    private:
        void draw_entity_node(Entity& entity);

        void draw_components(Entity& entity);

    private:
        std::string m_dock_name;
        std::shared_ptr<Scene> m_scene_graph;
        Entity m_selected_entity;
    };
}
