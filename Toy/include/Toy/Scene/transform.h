//
// Created by ZZK on 2023/5/24.
//

#pragma once

#include <Toy/Core/base.h>

namespace toy
{
    class transform_c
    {
    public:
        transform_c() = default;
        transform_c(const DirectX::XMFLOAT3& scale, const DirectX::XMFLOAT3& rotation, const DirectX::XMFLOAT3& position);

        transform_c(const transform_c&) = default;
        transform_c& operator=(const transform_c&) = default;

        transform_c(transform_c&&) = default;
        transform_c& operator=(transform_c&&) = default;

        // Get scaling
        DirectX::XMFLOAT3& get_scale();
        DirectX::XMFLOAT3 get_scale() const;
        DirectX::XMVECTOR get_scale_xm() const;

        // Get euler angle - radian - z->x->y sequential rotation
        DirectX::XMFLOAT3& get_rotation();
        DirectX::XMFLOAT3 get_rotation() const;
        DirectX::XMVECTOR get_rotation_xm() const;

        // Get position
        DirectX::XMFLOAT3& get_position();
        DirectX::XMFLOAT3 get_position() const;
        DirectX::XMVECTOR get_position_xm() const;

        // Get right axis
        DirectX::XMFLOAT3 get_right_axis() const;
        DirectX::XMVECTOR get_right_axis_xm() const;

        // Get up axis
        DirectX::XMFLOAT3 get_up_axis() const;
        DirectX::XMVECTOR get_up_axis_xm() const;

        // Get forward axis
        DirectX::XMFLOAT3 get_forward_axis() const;
        DirectX::XMVECTOR get_forward_axis_xm() const;

        // Get world matrix
        DirectX::XMFLOAT4X4 get_local_to_world_matrix() const;
        DirectX::XMMATRIX get_local_to_world_matrix_xm() const;

        // Get inv world matrix
        DirectX::XMFLOAT4X4 get_world_to_local_matrix() const;
        DirectX::XMMATRIX get_world_to_local_matrix_xm() const;

        // Set scaling
        void set_scale(const DirectX::XMFLOAT3& scale);
        void set_scale(float x, float y, float z);

        // Set euler angle - radian
        void set_rotation(const DirectX::XMFLOAT3& euler_angle_in_radian);
        void set_rotation(float x, float y, float z);

        // Set position
        void set_position(const DirectX::XMFLOAT3& position);
        void set_position(float x, float y, float z);

        //
        void rotate(const DirectX::XMFLOAT3& euler_angle_in_radian);
        // Rotate around origin
        void rotate_axis(const DirectX::XMFLOAT3& axis, float radian);
        // Rotate around point
        void rotate_around(const DirectX::XMFLOAT3& point, const DirectX::XMFLOAT3& axis, float radian);

        // Translation along axis
        void translate(const DirectX::XMFLOAT3& direction, float magnitude);

        // Look at point
        void look_at(const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& up = { 0.0f, 1.0f, 0.0f });
        // Look at direction
        void look_to(const DirectX::XMFLOAT3& direction, const DirectX::XMFLOAT3& up = { 0.0f, 1.0f, 0.0f });

        // Get euler angle
        static DirectX::XMFLOAT3 get_euler_angle_from_rotation_matrix(const DirectX::XMFLOAT4X4& rotation_matrix);

    private:
        DirectX::XMFLOAT3 class_scale = { 1.0f, 1.0f, 1.0f };
        DirectX::XMFLOAT3 class_rotation = {};
        DirectX::XMFLOAT3 class_position = {};
    };
}
