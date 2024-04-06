//
// Created by ZZK on 2024/3/17.
//

#pragma once

#include <Toy/Core/base.h>
#include <entt/entt.hpp>

namespace toy
{
    struct EntityWrapper
    {
        entt::registry* registry_handle = nullptr;
        entt::entity entity_inst = entt::null;

        EntityWrapper() = default;

        explicit EntityWrapper(entt::registry *input_registry_handle, entt::entity input_entity_inst);

        [[nodiscard]] bool is_valid() const;

        template<typename T, typename ... Args>
        T& add_component(Args&& ... args);

        template<typename T, typename ... Args>
        T& add_or_replace_component(Args&& ... args);

        template<typename T>
        T& get_component();

        template<typename T>
        [[nodiscard]] bool has_component() const;

        template<typename T>
        void remove_component();

        bool operator==(const EntityWrapper& other) const;

        bool operator!=(const EntityWrapper& other) const;
    };

    inline EntityWrapper::EntityWrapper(entt::registry *input_registry_handle, entt::entity input_entity_inst)
    : registry_handle(input_registry_handle), entity_inst(input_entity_inst)
    {

    }

    inline bool EntityWrapper::is_valid() const
    {
        return entity_inst != entt::null && registry_handle != nullptr;
    }

    inline bool EntityWrapper::operator==(const EntityWrapper &other) const
    {
        return entity_inst == other.entity_inst && registry_handle == other.registry_handle;
    }

    inline bool EntityWrapper::operator!=(const EntityWrapper &other) const
    {
        return !(*this == other);
    }

    template<typename T, typename ... Args>
    T& EntityWrapper::add_component(Args&& ... args)
    {
        static_assert(std::is_constructible_v<T, Args...>, "Failed to construct component T from Args");
        T& component = registry_handle->emplace<T>(entity_inst, std::forward<Args>(args)...);
        return component;
    }

    template<typename T, typename ... Args>
    T& EntityWrapper::add_or_replace_component(Args&& ... args)
    {
        static_assert(std::is_constructible_v<T, Args...>, "Failed to construct component T from Args");
        T& component = registry_handle->emplace_or_replace<T>(entity_inst, std::forward<Args>(args)...);
        return component;
    }

    template<typename T>
    T& EntityWrapper::get_component()
    {
        return registry_handle->get<T>(entity_inst);
    }

    template<typename T>
    bool EntityWrapper::has_component() const
    {
        return registry_handle->all_of<T>(entity_inst);
    }

    template<typename T>
    void EntityWrapper::remove_component()
    {
        registry_handle->remove<T>(entity_inst);
    }
}
