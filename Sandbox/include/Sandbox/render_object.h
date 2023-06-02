//
// Created by ZZK on 2023/5/24.
//

#pragma once

#include <Toy/toy.h>

namespace toy
{
    class RenderObject
    {
    public:
        RenderObject() = default;
        ~RenderObject() = default;

        RenderObject(const RenderObject&) = default;
        RenderObject& operator=(const RenderObject&) = default;

        RenderObject(RenderObject&&) = default;
        RenderObject& operator=(RenderObject&&) = default;

        // Object transform
        transform_c& get_transform();
        const transform_c& get_transform() const;

        // Check insertion
        void frustum_culling(const DirectX::BoundingFrustum& frustum_in_world);
        void cube_culling(const DirectX::BoundingOrientedBox& obb_in_world);
        void cube_culling(const DirectX::BoundingBox& aabb_in_world);
        [[nodiscard]] bool in_frustum() const { return m_in_frustum; }

        // Model
        void set_model(const model::Model* model_);
        const model::Model* get_model() const;

        DirectX::BoundingBox get_local_bounding_box() const;
        DirectX::BoundingBox get_local_bounding_box(size_t idx) const;
        DirectX::BoundingBox get_bounding_box() const;
        DirectX::BoundingBox get_bounding_box(size_t idx) const;
        DirectX::BoundingOrientedBox get_bounding_oriented_box() const;
        DirectX::BoundingOrientedBox get_bounding_oriented_box(size_t idx) const;

        // Draw
        void set_visible(bool is_visible);

        void draw(ID3D11DeviceContext *device_context, IEffect& effect);

        // Set debug object name
        void set_debug_object_name(const std::string& name);

    public:
        std::vector<bool> m_submodel_in_frustum;
        transform_c m_transform = {};
        const model::Model* m_model = nullptr;
        bool m_in_frustum = true;
    };
}
