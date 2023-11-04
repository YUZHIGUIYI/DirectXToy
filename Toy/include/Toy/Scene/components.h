//
// Created by ZZK on 2023/7/1.
//

#pragma once

#include <Toy/Scene/transform.h>
#include <Toy/Renderer/effect_interface.h>

namespace toy
{
    namespace model
    {
        struct Model;
    }

    class camera_c;

    struct CopyableAndMovable
    {
        CopyableAndMovable() = default;
        virtual ~CopyableAndMovable() = default;

        CopyableAndMovable(const CopyableAndMovable&) = default;
        CopyableAndMovable& operator=(const CopyableAndMovable&) = default;
        CopyableAndMovable(CopyableAndMovable&&) = default;
        CopyableAndMovable& operator=(CopyableAndMovable&&) = default;
    };

    struct TagComponent
    {
        std::string tag{};

        TagComponent() = default;
        TagComponent(const TagComponent&) = default;
        TagComponent& operator=(const TagComponent&) = default;

        TagComponent(TagComponent&&) = default;
        TagComponent& operator=(TagComponent&&) = default;

        TagComponent(std::string_view tag_) : tag(tag_) {}
    };

    struct TransformComponent
    {
        transform_c transform{};

        TransformComponent() = default;
        TransformComponent(const TransformComponent&) = default;
        TransformComponent& operator=(const TransformComponent&) = default;
    };

    struct CameraComponent
    {
        std::shared_ptr<camera_c> camera = nullptr;
    };

    struct StaticMeshComponent
    {
        model::Model* model_asset = nullptr;
        std::vector<bool> submodel_in_frustum{};
        bool in_frustum = true;
        bool is_skybox = false;     // TODO: Check whether static mesh component is skybox, bad design
        bool is_camera = false;

        StaticMeshComponent() = default;
        StaticMeshComponent(const StaticMeshComponent&) = default;
        StaticMeshComponent& operator=(const StaticMeshComponent&) = default;

        // Check insertion
        // Note: transform belongs to model asset
        void frustum_culling(const transform_c& transform, const DirectX::BoundingFrustum& frustum_in_world);

        // Render
        // Note: transform belongs to model asset
        void render(ID3D11DeviceContext *device_context, IEffect& effect, const transform_c& transform);

        // Bounding box
        DirectX::BoundingBox get_local_bounding_box() const;
        DirectX::BoundingBox get_local_bounding_box(size_t idx) const;
        DirectX::BoundingBox get_bounding_box(const transform_c& transform) const;
        DirectX::BoundingBox get_bounding_box(const transform_c& transform, size_t idx) const;
        DirectX::BoundingOrientedBox get_bounding_oriented_box(const transform_c& transform) const;
        DirectX::BoundingOrientedBox get_bounding_oriented_box(const transform_c& transform, size_t idx) const;
    };
}
































