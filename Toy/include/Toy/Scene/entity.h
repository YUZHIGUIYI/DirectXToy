//
// Created by ZZK on 2023/7/1.
//

#pragma once

#include <Toy/Core/base.h>
#include <Toy/Scene/scene.h>

#include <entt/entt.hpp>

namespace toy
{
    struct Entity
    {
        entt::entity entity_handle = { entt::null };
        Scene* scene_handle = nullptr;

        Entity() = default;
        Entity(entt::entity handle, Scene* scene);

        Entity(const Entity&) = default;
        Entity& operator=(const Entity&) = default;
        Entity(Entity&&) = default;
        Entity& operator=(Entity&&) = default;

        template<typename T, typename ... Args>
        T& add_component(Args&& ... args);

        template<typename T, typename ... Args>
        T& add_or_replace_component(Args&& ... args);

        template<typename T>
        T& get_component();

        template<typename T>
        bool has_component();

        template<typename T>
        void remove_component();

        bool operator==(const Entity& other) const;

        bool operator!=(const Entity& other) const;
    };

    template<typename T, typename ... Args>
    T& Entity::add_component(Args&& ... args)
    {
        T& component = scene_handle->registry_handle.emplace<T>(entity_handle, std::forward<Args>(args)...);
        return component;
    }

    template<typename T, typename ... Args>
    T& Entity::add_or_replace_component(Args&& ... args)
    {
        T& component = scene_handle->registry_handle.emplace_or_replace<T>(entity_handle, std::forward<Args>(args)...);
        return component;
    }

    template<typename T>
    T& Entity::get_component()
    {
        return scene_handle->registry_handle.get<T>(entity_handle);
    }

    template<typename T>
    bool Entity::has_component()
    {
        return scene_handle->registry_handle.all_of<T>(entity_handle);
    }

    template<typename T>
    void Entity::remove_component()
    {
        scene_handle->registry_handle.remove<T>(entity_handle);
    }
}


























