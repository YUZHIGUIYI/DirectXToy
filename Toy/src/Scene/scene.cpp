//
// Created by ZZK on 2023/7/1.
//

#include <Toy/Scene/scene.h>
#include <Toy/Scene/entity.h>

#include <Toy/Scene/components.h>

namespace toy
{
    Entity Scene::create_entity(std::string_view entity_name)
    {
        Entity entity{ registry_handle.create(), this };
        auto& tag = entity.add_component<TagComponent>();
        tag.tag = entity_name.empty() ? "Entity" : entity_name;
        return entity;
    }

    void Scene::destroy_entity(const toy::Entity &entity)
    {
        registry_handle.destroy(entity.entity_handle);
    }

    void Scene::frustum_culling(const DirectX::BoundingFrustum &frustum_in_world)
    {
        auto view = registry_handle.view<TransformComponent, StaticMeshComponent>();
        for (auto entity : view)
        {
            const auto [transform_component, static_mesh_component] = view.get<TransformComponent, StaticMeshComponent>(entity);
            // TODO: frustum culling for non-skybox mesh
            if (!static_mesh_component.is_skybox)
            {
                static_mesh_component.frustum_culling(transform_component.transform, frustum_in_world);
            }
        }
    }

    void Scene::render_scene(ID3D11DeviceContext* device_context, IEffect& effect, bool render_skybox)
    {
        auto view = registry_handle.view<TransformComponent, StaticMeshComponent>();
        for (auto entity : view)
        {
            const auto [transform_component, static_mesh_component] = view.get<TransformComponent, StaticMeshComponent>(entity);
            // TODO: conditional rendering
            if (render_skybox && static_mesh_component.is_skybox)
            {
                static_mesh_component.render(device_context, effect, transform_component.transform);
            } else if (!render_skybox && !static_mesh_component.is_skybox)
            {
                static_mesh_component.render(device_context, effect, transform_component.transform);
            }
        }
    }
}





















