//
// Created by ZZK on 2024/3/17.
//

#pragma once

#include <Toy/Core/base.h>
#include <Toy/ECS/entity_wrapper.h>
#include <Toy/Renderer/effect_interface.h>

namespace toy
{
    class Camera;
}
namespace toy::runtime
{
    struct SceneGraph
    {
    public:
        SceneGraph() = default;

        ~SceneGraph() = default;

        SceneGraph(const SceneGraph &) = delete;
        SceneGraph &operator=(const SceneGraph &) = delete;
        SceneGraph(SceneGraph &&) = delete;
        SceneGraph &operator=(SceneGraph &&) = delete;

        EntityWrapper create_entity(std::string_view entity_name);
        void destroy_entity(EntityWrapper &entity_wrapper);

        template <typename ... Components>
        void for_each(std::function<void(Components& ...)> &&func);

        void frustum_culling(const DirectX::BoundingFrustum &frustum_in_world);

        void render_skybox(ID3D11DeviceContext *device_context, IEffect &effect);

        void render_static_mesh_shadow(ID3D11DeviceContext *device_context, IEffect &effect);

        void render_static_mesh(ID3D11DeviceContext *device_context, IEffect &effect);

        bool pick_entity(EntityWrapper &selected_entity, const Camera &camera, float mouse_pos_x, float mouse_pos_y);

        EntityWrapper get_entity(uint32_t entity_id);

        [[nodiscard]] const std::vector<entt::entity> &get_static_mesh_entities() const;

        [[nodiscard]] const DirectX::BoundingBox &get_scene_bounding_box() const;

    private:
        entt::registry registry_handle = {};
        std::vector<entt::entity> static_mesh_entities;
        std::vector<entt::entity> entities_in_frustum;
        entt::entity skybox_entity = entt::null;
        DirectX::BoundingBox scene_bounding_box = {};
    };

    template <typename ... Components>
    void SceneGraph::for_each(std::function<void(Components& ...)> &&func)
    {
        auto view = registry_handle.view<Components ...>();
        for (auto entity : view)
        {
            func(view.template get<Components>(entity)...);
        }
    }
}
