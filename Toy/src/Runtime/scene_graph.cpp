//
// Created by ZZK on 2024/3/17.
//

#include <Toy/Runtime/scene_graph.h>
#include <Toy/Model/model_manager.h>
#include <Toy/ECS/collision.h>
#include <Toy/ECS/camera.h>
#include <Toy/ECS/components.h>

namespace toy::runtime
{
    EntityWrapper SceneGraph::create_entity(std::string_view entity_name)
    {
        EntityWrapper entity_wrapper{ &registry_handle, registry_handle.create() };
        auto& tag = entity_wrapper.add_component<TagComponent>();
        tag.tag = entity_name.empty() ? "Entity" : entity_name;
        return entity_wrapper;
    }

    void SceneGraph::destroy_entity(EntityWrapper &entity_wrapper)
    {
        registry_handle.destroy(entity_wrapper.entity_inst);
        entity_wrapper.registry_handle = nullptr;
    }

    void SceneGraph::frustum_culling(const DirectX::BoundingFrustum &frustum_in_world)
    {
        using namespace DirectX;
        static_mesh_entities.clear();
        entities_in_frustum.clear();

        uint32_t static_mesh_entity_index = 0;
        auto view = registry_handle.view<TransformComponent, StaticMeshComponent>();
        for (auto entity : view)
        {
            const auto [transform_component, static_mesh_component] = view.get<TransformComponent, StaticMeshComponent>(entity);

            if (auto in_frustum = static_mesh_component.frustum_culling(transform_component.transform, frustum_in_world))
            {
                entities_in_frustum.push_back(entity);
            }
            static_mesh_entities.push_back(entity);

            // Calculate scene bounding box for calculating near and far plane in lighting space
            if (static_mesh_entity_index == 0)
            {
                scene_bounding_box = static_mesh_component.get_bounding_box(transform_component.transform);
            } else
            {
                BoundingBox bounding_box = static_mesh_component.get_bounding_box(transform_component.transform);
                BoundingBox::CreateMerged(scene_bounding_box, scene_bounding_box, bounding_box);
            }
            ++static_mesh_entity_index;
        }
    }

    void SceneGraph::render_static_mesh_shadow(ID3D11DeviceContext *device_context, IEffect &effect)
    {
        // Exclude illuminant
        auto view = registry_handle.view<TransformComponent, StaticMeshComponent>();
        for (auto entity : view)
        {
            const auto [transform_component, static_mesh_component] = view.get<TransformComponent, StaticMeshComponent>(entity);
            static_mesh_component.render(device_context, effect, transform_component.transform);
        }
    }

    void SceneGraph::render_static_mesh(ID3D11DeviceContext *device_context, IEffect &effect)
    {
        // Static mesh in viewer
        auto view = registry_handle.view<TransformComponent, StaticMeshComponent>();
        for (auto entity : entities_in_frustum)
        {
            const auto [transform_component, static_mesh_component] = view.get<TransformComponent, StaticMeshComponent>(entity);
            static_mesh_component.render(device_context, effect, transform_component.transform);
        }
    }

    bool SceneGraph::pick_entity(EntityWrapper &selected_entity, const Camera &camera,
                                    float mouse_pos_x, float mouse_pos_y)
    {
        Ray ray = Ray::screen_to_ray(camera, mouse_pos_x, mouse_pos_y);

        auto view = registry_handle.view<TransformComponent, StaticMeshComponent>();
        bool pick_anything = false;
        for (auto entity : entities_in_frustum)
        {
            const auto [transform_component, static_mesh_component] = view.get<TransformComponent, StaticMeshComponent>(entity);
            if (ray.hit(static_mesh_component.get_bounding_oriented_box(transform_component.transform)))
            {
                selected_entity = EntityWrapper{ &registry_handle, entity };
                pick_anything = true;
                break;
            }
        }

        return pick_anything;
    }

    EntityWrapper SceneGraph::get_entity(uint32_t entity_id)
    {
        auto entity = static_cast<entt::entity>(entity_id);

        // Entity 0 and 1 are excluded
        if (entity_id == 0 || entity_id == 1)
        {
            return EntityWrapper{ nullptr, entity };
        } else
        {
            return EntityWrapper{ &registry_handle, entity };
        }
    }

    const std::vector<entt::entity> &SceneGraph::get_static_mesh_entities() const
    {
        return static_mesh_entities;
    }

    const DirectX::BoundingBox &SceneGraph::get_scene_bounding_box() const
    {
        return scene_bounding_box;
    }
}