//
// Created by ZZK on 2023/7/1.
//

#pragma once

#include <Toy/ECS/transform.h>
#include <Toy/Renderer/effect_interface.h>
#include <Toy/ECS/camera.h>

namespace toy::model
{
    struct Model;
}

namespace toy
{
    struct TagComponent
    {
        std::string tag = {};
    };

    struct TransformComponent
    {
        Transform transform = {};
    };

    struct DirectionalLightComponent
    {
        // View matrix, look axis helper generator
        Transform transform = {};
        // The starting point of a ray
        DirectX::XMFLOAT3 position = { 1.0f, 1.0f, 1.0f };
        // The end point of a ray
        DirectX::XMFLOAT3 target = { 0.0f, 0.0f, 0.0f };

        DirectX::XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
        float intensity = 1.0f;
    };

    struct CameraComponent
    {
        std::shared_ptr<Camera> camera = nullptr;
        CameraType camera_type = CameraType::DefaultCamera;
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
        void frustum_culling(const Transform& transform, const DirectX::BoundingFrustum& frustum_in_world);

        // Render
        // Note: transform belongs to model asset
        void render(ID3D11DeviceContext *device_context, IEffect& effect, const Transform& transform);

        // Bounding box
        [[nodiscard]] DirectX::BoundingBox get_local_bounding_box() const;
        [[nodiscard]] DirectX::BoundingBox get_local_bounding_box(size_t idx) const;
        [[nodiscard]] DirectX::BoundingBox get_bounding_box(const Transform& transform) const;
        [[nodiscard]] DirectX::BoundingBox get_bounding_box(const Transform& transform, size_t idx) const;
        [[nodiscard]] DirectX::BoundingOrientedBox get_bounding_oriented_box(const Transform& transform) const;
        [[nodiscard]] DirectX::BoundingOrientedBox get_bounding_oriented_box(const Transform& transform, size_t idx) const;
    };
}
































