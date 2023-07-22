//
// Created by ZZK on 2023/7/1.
//

#pragma once

#include <Toy/Renderer/effect_interface.h>
#include <Toy/Scene/camera.h>

#include <entt/entt.hpp>

namespace toy
{
    struct Entity;

    struct Scene : public disable_copyable_c
    {
        entt::registry registry_handle{};
        std::vector<entt::entity> general_static_mesh_entities;

        Scene() = default;

        Entity create_entity(std::string_view entity_name);
        void destroy_entity(const Entity& entity);

        void frustum_culling(const DirectX::BoundingFrustum& frustum_in_world);

        void render_scene(ID3D11DeviceContext* device_context, IEffect& effect, bool render_skybox = false);

        bool pick_entity(Entity& selected_entity, const camera_c& camera, float mouse_pos_x, float mouse_pos_y);

    private:
        std::vector<entt::entity> entities_in_viewer;
        entt::entity skybox_entity = entt::null;
    };
}
















































