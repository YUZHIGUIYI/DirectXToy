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

    struct TagComponent
    {
        std::string tag = {};
    };

    struct TransformComponent
    {
        transform_c transform = {};
    };

    struct CameraComponent
    {
        std::shared_ptr<camera_c> camera = nullptr;
    };

    struct StaticMeshComponent
    {
        model::Model* model_asset = nullptr;
        std::vector<bool> submodel_in_frustum = {};
        bool in_frustum = true;
        bool is_skybox = false;
        bool is_camera = false;

        // Check insertion
        // Note: transform belongs to model asset
        void frustum_culling(const transform_c& transform, const DirectX::BoundingFrustum& frustum_in_world);

        // Render
        // Note: transform belongs to model asset
        void render(ID3D11DeviceContext *device_context, IEffect& effect, const transform_c& transform);

        // Bounding box
        [[nodiscard]] DirectX::BoundingBox get_local_bounding_box() const;
        [[nodiscard]] DirectX::BoundingBox get_local_bounding_box(size_t idx) const;
        [[nodiscard]] DirectX::BoundingBox get_bounding_box(const transform_c& transform) const;
        [[nodiscard]] DirectX::BoundingBox get_bounding_box(const transform_c& transform, size_t idx) const;
        [[nodiscard]] DirectX::BoundingOrientedBox get_bounding_oriented_box(const transform_c& transform) const;
        [[nodiscard]] DirectX::BoundingOrientedBox get_bounding_oriented_box(const transform_c& transform, size_t idx) const;
    };
}
































