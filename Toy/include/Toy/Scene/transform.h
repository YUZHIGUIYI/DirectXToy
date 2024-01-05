//
// Created by ZZK on 2023/5/24.
//

#pragma once

#include <Toy/Core/base.h>

namespace toy
{
    struct Transform
    {
    public:
        // Get euler angles in radian, in z-x-y order
        [[nodiscard]] DirectX::XMFLOAT3 get_euler_angles() const;

        // Get right axis
        [[nodiscard]] DirectX::XMFLOAT3 get_right_axis() const;
        [[nodiscard]] DirectX::XMVECTOR get_right_axis_xm() const;

        // Get up axis
        [[nodiscard]] DirectX::XMFLOAT3 get_up_axis() const;
        [[nodiscard]] DirectX::XMVECTOR get_up_axis_xm() const;

        // Get forward axis
        [[nodiscard]] DirectX::XMFLOAT3 get_forward_axis() const;
        [[nodiscard]] DirectX::XMVECTOR get_forward_axis_xm() const;

        // Get world matrix
        [[nodiscard]] DirectX::XMFLOAT4X4 get_local_to_world_matrix() const;
        [[nodiscard]] DirectX::XMMATRIX get_local_to_world_matrix_xm() const;

        // Get inverse world matrix
        [[nodiscard]] DirectX::XMFLOAT4X4 get_world_to_local_matrix() const;
        [[nodiscard]] DirectX::XMMATRIX get_world_to_local_matrix_xm() const;

        // Set scale
        void set_scale(const DirectX::XMFLOAT3 &scale_vec);
        void set_scale(float x, float y, float z);

        // Set euler angles in radian, in z-x-y order
        void set_rotation(const DirectX::XMFLOAT3 &euler_angles_in_radian);
        void set_rotation(float x, float y, float z);

        // Set euler angles in quaternion
        void set_rotation_in_quaternion(const DirectX::XMFLOAT4 &quaternion);
        void set_rotation_in_quaternion(float x, float y, float z, float w);

        // Set euler angles in degree, in z-x-y order
        void set_rotation_in_degree(const DirectX::XMFLOAT3 &euler_angles);
        void set_rotation_in_degree(float x, float y, float z);

        // Set position
        void set_position(const DirectX::XMFLOAT3 &position_vec);
        void set_position(float x, float y, float z);

        // Rotate with specified euler angles
        void rotate(const DirectX::XMFLOAT3 &euler_angle_in_radian);
        // Rotate around an axis centered on the origin
        void rotate_axis(const DirectX::XMFLOAT3 &axis, float radian);
        // Rotate around an axis with "point" as the center of rotation
        void rotate_around(const DirectX::XMFLOAT3 &point, const DirectX::XMFLOAT3 &axis, float radian);

        // Translation in a certain direction
        void translate(const DirectX::XMFLOAT3 &direction, float magnitude);

        // Look at point
        void look_at(const DirectX::XMFLOAT3 &target, const DirectX::XMFLOAT3 &up = { 0.0f, 1.0f, 0.0f });
        // Look in a certain direction
        void look_to(const DirectX::XMFLOAT3 &direction, const DirectX::XMFLOAT3 &up = { 0.0f, 1.0f, 0.0f });

        // Obtain rotation euler angles from a rotation matrix
        static DirectX::XMFLOAT3 get_euler_angles_from_rotation_matrix(const DirectX::XMFLOAT4X4 &rotation_matrix);

    public:
        DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
        DirectX::XMFLOAT3 scale = { 1.0f, 1.0f, 1.0f };
        DirectX::XMFLOAT4 quaternion_rotation = { 0.0f, 0.0f, 0.0f, 1.0f };
    };
}
