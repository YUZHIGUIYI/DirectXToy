//
// Created by ZZK on 2023/7/2.
//

#pragma once

#include <Toy/toy.h>

namespace toy
{
    struct SceneHierarchyPanel
    {
    public:
        SceneHierarchyPanel() = default;
        SceneHierarchyPanel(const std::shared_ptr<Scene>& scene_context);

        void set_context(const std::shared_ptr<Scene>& scene_context);

        void set_selected_entity(const Entity& selected_entity);

        void on_ui_render();

    private:
        void draw_entity_node(Entity& entity);

        void draw_components(Entity& entity);

    private:
        std::shared_ptr<Scene> m_scene_context = nullptr;
        Entity m_selected_entity{};
    };









}






