//
// Created by ZZK on 2023/7/1.
//

#include <Toy/Scene/entity.h>

namespace toy
{
    Entity::Entity(entt::entity handle, Scene *scene)
    : entity_handle(handle), scene_handle(scene)
    {

    }

    bool Entity::operator==(const Entity &other) const
    {
        return entity_handle == other.entity_handle && scene_handle == other.scene_handle;
    }

    bool Entity::operator!=(const Entity &other) const
    {
        return !(*this == other);
    }
}