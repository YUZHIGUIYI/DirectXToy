//
// Created by ZZK on 2023/7/1.
//

#pragma once

#include <Toy/Renderer/effect_interface.h>
#include <entt/entt.hpp>

namespace toy
{
    struct Entity;

    class Camera;

    struct Scene : public disable_copyable_c
    {
    public:
        entt::registry registry_handle{};
        std::vector<entt::entity> general_static_mesh_entities;

    public:
        Scene();

        Entity create_entity(std::string_view entity_name);
        void destroy_entity(const Entity &entity);

        void update_cascaded_shadow(std::function<void(const DirectX::BoundingBox &bounding_box)> &&update_func);

        void frustum_culling(const DirectX::BoundingFrustum &frustum_in_world);

        void render_skybox(ID3D11DeviceContext *device_context, IEffect &effect);

        void render_static_mesh_shadow(ID3D11DeviceContext *device_context, IEffect &effect);

        void render_static_mesh(ID3D11DeviceContext *device_context, IEffect &effect);

        bool pick_entity(Entity &selected_entity, const Camera &camera, float mouse_pos_x, float mouse_pos_y);

        Entity get_entity(uint32_t entity_id);

        Entity get_skybox_entity();

    private:
        void adjust_illuminant();

    private:
        std::vector<entt::entity> entities_in_viewer;
        std::vector<entt::entity> illuminant_entities_in_viewer;
        entt::entity skybox_entity = entt::null;
        DirectX::BoundingBox scene_bounding_box = {};
    };
}
















































