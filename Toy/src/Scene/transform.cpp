//
// Created by ZZK on 2023/5/24.
//

#include <Toy/Scene/transform.h>

namespace toy
{
    transform_c::transform_c(const DirectX::XMFLOAT3 &scale, const DirectX::XMFLOAT3 &rotation,
                                const DirectX::XMFLOAT3 &position)
    : class_scale(scale), class_rotation(rotation), class_position(position)
    {

    }

    DirectX::XMFLOAT3 transform_c::get_scale() const { return class_scale; }
    DirectX::XMVECTOR transform_c::get_scale_xm() const { return DirectX::XMLoadFloat3(&class_scale); }

    DirectX::XMFLOAT3 transform_c::get_rotation() const { return class_rotation; }
    DirectX::XMVECTOR transform_c::get_rotation_xm() const { return DirectX::XMLoadFloat3(&class_rotation); }

    DirectX::XMFLOAT3 transform_c::get_position() const { return class_position; }
    DirectX::XMVECTOR transform_c::get_position_xm() const { return DirectX::XMLoadFloat3(&class_position); }

    DirectX::XMFLOAT3 transform_c::get_right_axis() const
    {
        DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&class_rotation));
        DirectX::XMFLOAT3 right{};
        DirectX::XMStoreFloat3(&right, rotation.r[0]);
        return right;
    }

    DirectX::XMVECTOR transform_c::get_right_axis_xm() const
    {
        DirectX::XMFLOAT3 right = get_right_axis();
        return DirectX::XMLoadFloat3(&right);
    }

    DirectX::XMFLOAT3 transform_c::get_up_axis() const
    {
        DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&class_rotation));
        DirectX::XMFLOAT3 up{};
        DirectX::XMStoreFloat3(&up, rotation.r[1]);
        return up;
    }

    DirectX::XMVECTOR transform_c::get_up_axis_xm() const
    {
        DirectX::XMFLOAT3 up = get_up_axis();
        return DirectX::XMLoadFloat3(&up);
    }

    DirectX::XMFLOAT3 transform_c::get_forward_axis() const
    {
        DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&class_rotation));
        DirectX::XMFLOAT3 forward{};
        DirectX::XMStoreFloat3(&forward, rotation.r[2]);
        return forward;
    }

    DirectX::XMVECTOR transform_c::get_forward_axis_xm() const
    {
        DirectX::XMFLOAT3 forward = get_forward_axis();
        return DirectX::XMLoadFloat3(&forward);
    }

    DirectX::XMFLOAT4X4 transform_c::get_local_to_world_matrix() const
    {
        DirectX::XMFLOAT4X4 res{};
        DirectX::XMStoreFloat4x4(&res, get_local_to_world_matrix_xm());
        return res;
    }

    DirectX::XMMATRIX transform_c::get_local_to_world_matrix_xm() const
    {
        DirectX::XMVECTOR scale_vec = DirectX::XMLoadFloat3(&class_scale);
        DirectX::XMVECTOR rotation_vec = DirectX::XMLoadFloat3(&class_rotation);
        DirectX::XMVECTOR position_vec = DirectX::XMLoadFloat3(&class_position);
        DirectX::XMMATRIX world = DirectX::XMMatrixScalingFromVector(scale_vec) *
                DirectX::XMMatrixRotationRollPitchYawFromVector(rotation_vec) *
                DirectX::XMMatrixTranslationFromVector(position_vec);
        return world;
    }

    DirectX::XMFLOAT4X4 transform_c::get_world_to_local_matrix() const
    {
        DirectX::XMFLOAT4X4 res{};
        DirectX::XMStoreFloat4x4(&res, get_world_to_local_matrix_xm());
        return res;
    }

    DirectX::XMMATRIX transform_c::get_world_to_local_matrix_xm() const
    {
        DirectX::XMMATRIX inv_world = DirectX::XMMatrixInverse(nullptr, get_local_to_world_matrix_xm());
        return inv_world;
    }

    void transform_c::set_scale(const DirectX::XMFLOAT3 &scale)
    {
        class_scale = scale;
    }

    void transform_c::set_scale(float x, float y, float z)
    {
        class_scale = DirectX::XMFLOAT3(x, y, z);
    }

    void transform_c::set_rotation(const DirectX::XMFLOAT3 &euler_angle_in_radian)
    {
        class_rotation = euler_angle_in_radian;
    }

    void transform_c::set_rotation(float x, float y, float z)
    {
        class_rotation = DirectX::XMFLOAT3(x, y, z);
    }

    void transform_c::set_position(const DirectX::XMFLOAT3 &position)
    {
        class_position = position;
    }

    void transform_c::set_position(float x, float y, float z)
    {
        class_position = DirectX::XMFLOAT3(x, y, z);
    }

    void transform_c::rotate(const DirectX::XMFLOAT3 &euler_angle_in_radian)
    {
        DirectX::XMVECTOR new_rotation_vec = DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&class_rotation),
                                                                    DirectX::XMLoadFloat3(&euler_angle_in_radian));
        DirectX::XMStoreFloat3(&class_rotation, new_rotation_vec);
    }

    void transform_c::rotate_axis(const DirectX::XMFLOAT3 &axis, float radian)
    {
        DirectX::XMVECTOR rotation_vec = XMLoadFloat3(&class_rotation);
        DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYawFromVector(rotation_vec) *
                            DirectX::XMMatrixRotationAxis(XMLoadFloat3(&axis), radian);
        DirectX::XMFLOAT4X4 rot_matrix{};
        DirectX::XMStoreFloat4x4(&rot_matrix, rotation);
        class_rotation = get_euler_angle_from_rotation_matrix(rot_matrix);
    }

    void transform_c::rotate_around(const DirectX::XMFLOAT3 &point, const DirectX::XMFLOAT3 &axis, float radian)
    {
        DirectX::XMVECTOR rotation_vec = XMLoadFloat3(&class_rotation);
        DirectX::XMVECTOR position_vec = XMLoadFloat3(&class_position);
        DirectX::XMVECTOR center_vec = XMLoadFloat3(&point);
        DirectX::XMFLOAT3 diff = DirectX::XMFLOAT3(class_position.x - point.x, class_position.y - point.y, class_position.z - point.z);
        DirectX::XMVECTOR diff_vec = XMLoadFloat3(&diff);

        // Take point as origin
        DirectX::XMMATRIX RT = DirectX::XMMatrixRotationRollPitchYawFromVector(rotation_vec) * DirectX::XMMatrixTranslationFromVector(diff_vec);
        RT *= DirectX::XMMatrixRotationAxis(XMLoadFloat3(&axis), radian);
        RT *= DirectX::XMMatrixTranslationFromVector(center_vec);
        DirectX::XMFLOAT4X4 rotMatrix{};
        DirectX::XMStoreFloat4x4(&rotMatrix, RT);
        class_rotation = get_euler_angle_from_rotation_matrix(rotMatrix);
        DirectX::XMStoreFloat3(&class_position, RT.r[3]);
    }

    void transform_c::translate(const DirectX::XMFLOAT3 &direction, float magnitude)
    {
        DirectX::XMVECTOR directionVec = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&direction));
        DirectX::XMVECTOR newPosition = DirectX::XMVectorMultiplyAdd(DirectX::XMVectorReplicate(magnitude), directionVec, DirectX::XMLoadFloat3(&class_position));
        DirectX::XMStoreFloat3(&class_position, newPosition);
    }

    void transform_c::look_at(const DirectX::XMFLOAT3 &target, const DirectX::XMFLOAT3 &up)
    {
        DirectX::XMMATRIX View = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat3(&class_position),
                                                            DirectX::XMLoadFloat3(&target), DirectX::XMLoadFloat3(&up));
        DirectX::XMMATRIX InvView = DirectX::XMMatrixInverse(nullptr, View);
        DirectX::XMFLOAT4X4 rotMatrix{};
        DirectX::XMStoreFloat4x4(&rotMatrix, InvView);
        class_rotation = get_euler_angle_from_rotation_matrix(rotMatrix);
    }

    void transform_c::look_to(const DirectX::XMFLOAT3 &direction, const DirectX::XMFLOAT3 &up)
    {
        DirectX::XMMATRIX View = DirectX::XMMatrixLookToLH(DirectX::XMLoadFloat3(&class_position),
                                                            DirectX::XMLoadFloat3(&direction), DirectX::XMLoadFloat3(&up));
        DirectX::XMMATRIX InvView = DirectX::XMMatrixInverse(nullptr, View);
        DirectX::XMFLOAT4X4 rotMatrix{};
        DirectX::XMStoreFloat4x4(&rotMatrix, InvView);
        class_rotation = get_euler_angle_from_rotation_matrix(rotMatrix);
    }

    DirectX::XMFLOAT3 transform_c::get_euler_angle_from_rotation_matrix(const DirectX::XMFLOAT4X4 &rotation_matrix)
    {
        DirectX::XMFLOAT3 rotation{};
        // 死锁特殊处理
        if (std::abs(1.0f - std::abs(rotation_matrix(2, 1))) < 1e-5f)
        {
            rotation.x = std::copysign(DirectX::XM_PIDIV2, -rotation_matrix(2, 1));
            rotation.y = -std::atan2(rotation_matrix(0, 2), rotation_matrix(0, 0));
            return rotation;
        }

        // 通过旋转矩阵反求欧拉角
        float c = std::sqrt(1.0f - rotation_matrix(2, 1) * rotation_matrix(2, 1));
        // 防止r[2][1]出现大于1的情况
        if (isnan(c))
            c = 0.0f;

        rotation.z = std::atan2(rotation_matrix(0, 1), rotation_matrix(1, 1));
        rotation.x = std::atan2(-rotation_matrix(2, 1), c);
        rotation.y = std::atan2(rotation_matrix(2, 0), rotation_matrix(2, 2));
        return rotation;
    }
}






































