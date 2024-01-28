//
// Created by ZZK on 2023/7/1.
//

#include <Toy/Scene/scene.h>
#include <Toy/Scene/entity.h>
#include <Toy/Scene/collision.h>

#include <Toy/Scene/components.h>
#include <Toy/Model/model_manager.h>
#include <Toy/Scene/camera.h>

namespace toy
{
    Scene::Scene() = default;

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

    Entity Scene::get_skybox_entity()
    {
        return Entity{ skybox_entity, this };
    }

    void Scene::update_cascaded_shadow(std::function<void(const DirectX::BoundingBox &)> &&update_func)
    {
        update_func(scene_bounding_box);
    }

    void Scene::frustum_culling(const DirectX::BoundingFrustum &frustum_in_world)
    {
        using namespace DirectX;
        general_static_mesh_entities.clear();
        entities_in_viewer.clear();
        illuminant_entities_in_viewer.clear();

        uint32_t static_mesh_entity_index = 0;
        auto view = registry_handle.view<TransformComponent, StaticMeshComponent>();
        for (auto entity : view)
        {
            const auto [transform_component, static_mesh_component] = view.get<TransformComponent, StaticMeshComponent>(entity);

            if (!static_mesh_component.is_skybox)
            {
                static_mesh_component.frustum_culling(transform_component.transform, frustum_in_world);
                general_static_mesh_entities.push_back(entity);
                if (static_mesh_component.in_frustum) entities_in_viewer.push_back(entity);
                if (static_mesh_component.in_frustum && static_mesh_component.is_camera) illuminant_entities_in_viewer.push_back(entity);

                if (static_mesh_component.is_camera) continue;
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
            } else
            {
                skybox_entity = entity;
            }
        }
    }

    void Scene::render_skybox(ID3D11DeviceContext *device_context, toy::IEffect &effect)
    {
        // Skybox model
        if (skybox_entity != entt::null)
        {
            auto&& static_mesh_component = registry_handle.get<StaticMeshComponent>(skybox_entity);
            auto&& transform_component = registry_handle.get<TransformComponent>(skybox_entity);
            static_mesh_component.render(device_context, effect, transform_component.transform);
        }
    }

    void Scene::render_static_mesh_shadow(ID3D11DeviceContext *device_context, IEffect &effect)
    {
        // Exclude illuminant
        auto view = registry_handle.view<TransformComponent, StaticMeshComponent>();
        for (auto entity : view)
        {
            const auto [transform_component, static_mesh_component] = view.get<TransformComponent, StaticMeshComponent>(entity);
            if (!static_mesh_component.is_camera)
            {
                static_mesh_component.render(device_context, effect, transform_component.transform);
            }
        }
    }

    void Scene::render_static_mesh(ID3D11DeviceContext* device_context, IEffect &effect)
    {
        // Static mesh in viewer
        auto view = registry_handle.view<TransformComponent, StaticMeshComponent>();
        for (auto entity : entities_in_viewer)
        {
            const auto [transform_component, static_mesh_component] = view.get<TransformComponent, StaticMeshComponent>(entity);
            static_mesh_component.render(device_context, effect, transform_component.transform);
        }

        adjust_illuminant();
    }

    bool Scene::pick_entity(Entity& selected_entity, const Camera &camera, float mouse_pos_x, float mouse_pos_y)
    {
        Ray ray = Ray::screen_to_ray(camera, mouse_pos_x, mouse_pos_y);

        auto view = registry_handle.view<TransformComponent, StaticMeshComponent>();
        bool pick_anything = false;
        for (auto entity : entities_in_viewer)
        {
            const auto [transform_component, static_mesh_component] = view.get<TransformComponent, StaticMeshComponent>(entity);
            if (ray.hit(static_mesh_component.get_bounding_oriented_box(transform_component.transform)))
            {
                selected_entity = { entity, this };
                pick_anything = true;
                break;
            }
        }

        return pick_anything;
    }

    Entity Scene::get_entity(uint32_t entity_id)
    {
        auto entity = static_cast<entt::entity>(entity_id);

        if (entity_id == 0)
        {
            // For skybox, return an invalid entity
            return Entity{ entity, nullptr };
        } else
        {
            // For non-skybox, return a valid entity
            return Entity{ entity, this};
        }
    }

    void Scene::adjust_illuminant()
    {
        // Adjust illuminant position, update view matrix
        using namespace DirectX;
        auto view = registry_handle.view<TransformComponent, CameraComponent>();
        for (auto entity : illuminant_entities_in_viewer)
        {
            const auto [transform_component, camera_component] = view.get<TransformComponent, CameraComponent>(entity);
            auto first_person_camera = std::static_pointer_cast<FirstPersonCamera>(camera_component.camera);
            first_person_camera->look_at(transform_component.transform.position, XMFLOAT3{ 0.0f, 0.0f, 0.0f }, XMFLOAT3{ 0.0f, 1.0f, 0.0f });
        }
    }
}





















