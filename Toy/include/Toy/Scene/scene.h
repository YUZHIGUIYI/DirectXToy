//
// Created by ZZK on 2023/7/1.
//

#pragma once

#include <Toy/Renderer/effect_interface.h>
#include <entt/entt.hpp>

namespace toy
{
    struct Entity;

    class camera_c;

    struct Scene : public disable_copyable_c
    {
    public:
        entt::registry registry_handle{};
        std::vector<entt::entity> general_static_mesh_entities;

    public:
        Scene() = default;

        Entity get_skybox_entity();

        Entity create_entity(std::string_view entity_name);
        void destroy_entity(const Entity &entity);

        void update_cascaded_shadow(std::function<void(const DirectX::BoundingBox &bounding_box)> &&update_func);

        void frustum_culling(const DirectX::BoundingFrustum &frustum_in_world);

        void render_skybox(ID3D11DeviceContext *device_context, IEffect &effect);

        void render_static_mesh_shadow(ID3D11DeviceContext *device_context, IEffect &effect);

        void render_static_mesh(ID3D11DeviceContext *device_context, IEffect &effect);

        bool pick_entity(Entity &selected_entity, const camera_c &camera, float mouse_pos_x, float mouse_pos_y);

    private:
        void adjust_illuminant();

    private:
        std::vector<entt::entity> entities_in_viewer;
        std::vector<entt::entity> illuminant_entities_in_viewer;
        entt::entity skybox_entity = entt::null;
    };
}
















































