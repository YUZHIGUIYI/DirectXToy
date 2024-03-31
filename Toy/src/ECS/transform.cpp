//
// Created by ZZK on 2023/5/24.
//

#include <Toy/ECS/transform.h>
#include <Toy/Core/d3d_util.h>

namespace toy
{
    static bool all_equal(const DirectX::XMFLOAT2 &left_vec, const DirectX::XMFLOAT2 &right_vec)
    {
        bool result_one = (left_vec.x == right_vec.x);
        bool result_two = (left_vec.y == right_vec.y);
        return result_one && result_two;
    }

    DirectX::XMFLOAT3 Transform::get_euler_angles() const
    {
        using namespace DirectX;
        XMFLOAT3 euler_angles = {};

//        // Pitch - x-axis rotation
//        float y = 2.0f * (quaternion_rotation.y * quaternion_rotation.z + quaternion_rotation.w * quaternion_rotation.x);
//        float x = quaternion_rotation.w * quaternion_rotation.w - quaternion_rotation.x * quaternion_rotation.x - quaternion_rotation.y * quaternion_rotation.y + quaternion_rotation.z * quaternion_rotation.z;
//        if (all_equal(XMFLOAT2{ x, y }, XMFLOAT2{ 0.0f, 0.0f }))
//        {
//            // Avoid atan2(0.0, 0.0), handle singularity
//            euler_angles.x = 2.0f * std::atan2(quaternion_rotation.x, quaternion_rotation.w);
//        } else
//        {
//            euler_angles.x = std::atan2(y, x);
//        }
//
//        // Yaw - y-axis rotation
//        float yaw_value = -2.0f * (quaternion_rotation.x * quaternion_rotation.z - quaternion_rotation.w * quaternion_rotation.y);
//        yaw_value = std::clamp(yaw_value, -1.0f, 1.0f);
//        euler_angles.y = std::asin(yaw_value);
//
//        // Roll - z-axis rotation
//        y = 2.0f * (quaternion_rotation.x * quaternion_rotation.y + quaternion_rotation.w * quaternion_rotation.z);
//        x = quaternion_rotation.w * quaternion_rotation.w + quaternion_rotation.x * quaternion_rotation.x - quaternion_rotation.y * quaternion_rotation.y - quaternion_rotation.z * quaternion_rotation.z;
//        if (all_equal(XMFLOAT2{ x, y }, XMFLOAT2{ 0.0f, 0.0f }))
//        {
//            euler_angles.z = 0.0f;
//        } else
//        {
//            euler_angles.z = std::atan2(y, x);
//        }

        float sin_x = 2.0f * (quaternion_rotation.w * quaternion_rotation.x - quaternion_rotation.y * quaternion_rotation.z);
        float sin_y_cos_x = 2.0f * (quaternion_rotation.w * quaternion_rotation.y + quaternion_rotation.x * quaternion_rotation.z);
        float cos_y_cos_x = 1.0f - 2.0f * (quaternion_rotation.x * quaternion_rotation.x + quaternion_rotation.y * quaternion_rotation.y);
        float sin_z_cos_x = 2.0f * (quaternion_rotation.w * quaternion_rotation.z + quaternion_rotation.x * quaternion_rotation.y);
        float cos_z_cos_x = 1.0f - 2.0f * (quaternion_rotation.x * quaternion_rotation.x + quaternion_rotation.z * quaternion_rotation.z);

        // Roll (z-axis rotation)
        // Pitch (x-axis rotation)
        // Yaw (y-axis rotation)
        // Handle deadlock
        if (std::abs(std::abs(sin_x) - 1.0f) < 1e-5f)
        {
            euler_angles.x = std::copysign(XM_PI / 2.0f, sin_x);
            euler_angles.y = 2.0f * std::atan2(quaternion_rotation.y, quaternion_rotation.w);
            euler_angles.z = 0.0f;
        } else
        {
            euler_angles.x = std::asin(sin_x);
            euler_angles.y = std::atan2(sin_y_cos_x, cos_y_cos_x);
            euler_angles.z = std::atan2(sin_z_cos_x, cos_z_cos_x);
        }

        return euler_angles;
    }

    DirectX::XMFLOAT3 Transform::get_right_axis() const
    {
        using namespace DirectX;
        XMMATRIX rotation_matrix = XMMatrixRotationQuaternion(XMLoadFloat4(&quaternion_rotation));
        XMFLOAT3 right = {};
        XMStoreFloat3(&right, rotation_matrix.r[0]);

        return right;
    }

    DirectX::XMVECTOR Transform::get_right_axis_xm() const
    {
        using namespace DirectX;
        auto right = get_right_axis();

        return XMLoadFloat3(&right);
    }

    DirectX::XMFLOAT3 Transform::get_up_axis() const
    {
        using namespace DirectX;
        XMMATRIX rotation_matrix = XMMatrixRotationQuaternion(XMLoadFloat4(&quaternion_rotation));
        XMFLOAT3 up = {};
        XMStoreFloat3(&up, rotation_matrix.r[1]);

        return up;
    }

    DirectX::XMVECTOR Transform::get_up_axis_xm() const
    {
        using namespace DirectX;
        auto up = get_up_axis();

        return XMLoadFloat3(&up);
    }

    DirectX::XMFLOAT3 Transform::get_forward_axis() const
    {
        using namespace DirectX;
        XMMATRIX rotation_matrix = XMMatrixRotationQuaternion(XMLoadFloat4(&quaternion_rotation));
        XMFLOAT3 forward = {};
        XMStoreFloat3(&forward, rotation_matrix.r[2]);

        return forward;
    }

    DirectX::XMVECTOR Transform::get_forward_axis_xm() const
    {
        using namespace DirectX;
        auto forward = get_forward_axis();

        return XMLoadFloat3(&forward);
    }

    DirectX::XMFLOAT4X4 Transform::get_local_to_world_matrix() const
    {
        using namespace DirectX;
        XMFLOAT4X4 matrix = {};
        XMStoreFloat4x4(&matrix, get_local_to_world_matrix_xm());

        return matrix;
    }

    DirectX::XMMATRIX Transform::get_local_to_world_matrix_xm() const
    {
        using namespace DirectX;
        XMVECTOR scale_vec = XMLoadFloat3(&scale);
        XMVECTOR position_vec = XMLoadFloat3(&position);
        XMVECTOR rotation_vec = XMLoadFloat4(&quaternion_rotation);
        XMMATRIX world_matrix = XMMatrixAffineTransformation(scale_vec, g_XMZero, rotation_vec, position_vec);

        return world_matrix;
    }

    DirectX::XMFLOAT4X4 Transform::get_world_to_local_matrix() const
    {
        using namespace DirectX;
        XMFLOAT4X4 local_matrix = {};
        XMStoreFloat4x4(&local_matrix, get_world_to_local_matrix_xm());

        return local_matrix;
    }

    DirectX::XMMATRIX Transform::get_world_to_local_matrix_xm() const
    {
        using namespace DirectX;
        XMMATRIX inv_world_matrix = XMMatrixInverse(nullptr, get_local_to_world_matrix_xm());

        return inv_world_matrix;
    }

    void Transform::set_scale(const DirectX::XMFLOAT3 &scale_vec)
    {
        scale = scale_vec;
    }

    void Transform::set_scale(float x, float y, float z)
    {
        scale = DirectX::XMFLOAT3{ x, y, z };
    }

    void Transform::set_rotation(const DirectX::XMFLOAT3 &euler_angles_in_radian)
    {
        using namespace DirectX;
        auto quaternion_vec = XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&euler_angles_in_radian));
        XMStoreFloat4(&quaternion_rotation, quaternion_vec);
    }

    void Transform::set_rotation(float x, float y, float z)
    {
        using namespace DirectX;
        auto quaternion_vec = XMQuaternionRotationRollPitchYaw(x, y, z);
        XMStoreFloat4(&quaternion_rotation, quaternion_vec);
    }

    void Transform::set_rotation_in_quaternion(const DirectX::XMFLOAT4 &quaternion)
    {
        quaternion_rotation = quaternion;
    }

    void Transform::set_rotation_in_quaternion(float x, float y, float z, float w)
    {
        quaternion_rotation = DirectX::XMFLOAT4{ x, y, z, w };
    }

    void Transform::set_rotation_in_degree(const DirectX::XMFLOAT3 &euler_angles)
    {
        auto float3_radians = math::float3_radians(euler_angles);
        set_rotation(float3_radians);
    }

    void Transform::set_rotation_in_degree(float x, float y, float z)
    {
        auto x_radian = math::radians(x);
        auto y_radian = math::radians(y);
        auto z_radian = math::radians(z);
        set_rotation(x_radian, y_radian, z_radian);
    }

    void Transform::set_position(const DirectX::XMFLOAT3 &position_vec)
    {
        position = position_vec;
    }

    void Transform::set_position(float x, float y, float z)
    {
        position = DirectX::XMFLOAT3{ x, y, z };
    }

    void Transform::rotate(const DirectX::XMFLOAT3 &euler_angle_in_radian)
    {
        using namespace DirectX;
        XMVECTOR old_quaternion_rotation = XMLoadFloat4(&quaternion_rotation);
        XMVECTOR increment_quaternion_rotation = XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&euler_angle_in_radian));
        XMVECTOR new_quaternion_rotation = XMQuaternionMultiply(old_quaternion_rotation, increment_quaternion_rotation);
        XMStoreFloat4(&quaternion_rotation, XMQuaternionMultiply(old_quaternion_rotation, new_quaternion_rotation));
    }

    void Transform::rotate_axis(const DirectX::XMFLOAT3 &axis, float radian)
    {
        using namespace DirectX;
        XMVECTOR old_quaternion_rotation = XMLoadFloat4(&quaternion_rotation);
        XMVECTOR new_quaternion_rotation = XMQuaternionRotationAxis(XMLoadFloat3(&axis), radian);
        XMStoreFloat4(&quaternion_rotation, XMQuaternionMultiply(old_quaternion_rotation, new_quaternion_rotation));
    }

    void Transform::rotate_around(const DirectX::XMFLOAT3 &point, const DirectX::XMFLOAT3 &axis, float radian)
    {
        using namespace DirectX;
        XMVECTOR rotation_vec = XMLoadFloat4(&quaternion_rotation);
        XMVECTOR position_vec = XMLoadFloat3(&position);
        XMVECTOR center_vec = XMLoadFloat3(&point);

        // Take point as origin
        XMMATRIX rt_matrix = XMMatrixRotationQuaternion(rotation_vec) * XMMatrixTranslationFromVector(position_vec - center_vec);
        rt_matrix *= XMMatrixRotationAxis(XMLoadFloat3(&axis), radian);
        rt_matrix *= XMMatrixTranslationFromVector(center_vec);

        XMVECTOR new_quaternion_rotation = XMQuaternionRotationMatrix(rt_matrix);
        XMStoreFloat4(&quaternion_rotation, new_quaternion_rotation);
        XMStoreFloat3(&position, rt_matrix.r[3]);
    }

    void Transform::translate(const DirectX::XMFLOAT3 &direction, float magnitude)
    {
        using namespace DirectX;
        XMVECTOR direction_vec = XMVector3Normalize(XMLoadFloat3(&direction));
        XMVECTOR new_position = XMVectorMultiplyAdd(XMVectorReplicate(magnitude), direction_vec, XMLoadFloat3(&position));
        XMStoreFloat3(&position, new_position);
    }

    void Transform::move_local(const DirectX::XMFLOAT3 &magnitudes)
    {
        using namespace DirectX;

        XMVECTOR new_position_vec = XMLoadFloat3(&position);

        XMVECTOR x_axis_increment = XMVectorReplicate(magnitudes.x);
        XMVECTOR y_axis_increment = XMVectorReplicate(magnitudes.y);
        XMVECTOR z_axis_increment = XMVectorReplicate(magnitudes.z);

        auto world_matrix = get_local_to_world_matrix_xm();
        auto unit_x_axis= XMVector3Normalize(world_matrix.r[0]);
        auto unit_y_axis = XMVector3Normalize(world_matrix.r[1]);
        auto unit_z_axis = XMVector3Normalize(world_matrix.r[2]);

        new_position_vec += (unit_x_axis * x_axis_increment);
        new_position_vec += (unit_y_axis * y_axis_increment);
        new_position_vec += (unit_z_axis * z_axis_increment);

        XMStoreFloat3(&position, new_position_vec);
    }

    void Transform::look_at(const DirectX::XMFLOAT3 &target, const DirectX::XMFLOAT3 &up)
    {
        using namespace DirectX;
        XMMATRIX view_matrix = XMMatrixLookAtLH(XMLoadFloat3(&position),
                                        XMLoadFloat3(&target), XMLoadFloat3(&up));
        XMMATRIX inverse_view_matrix = XMMatrixInverse(nullptr, view_matrix);
        XMVECTOR new_quaternion_rotation = XMQuaternionRotationMatrix(inverse_view_matrix);
        XMStoreFloat4(&quaternion_rotation, new_quaternion_rotation);
    }

    void Transform::look_to(const DirectX::XMFLOAT3 &direction, const DirectX::XMFLOAT3 &up)
    {
        using namespace DirectX;
        XMMATRIX view_matrix = XMMatrixLookToLH(XMLoadFloat3(&position),
                                                XMLoadFloat3(&direction), XMLoadFloat3(&up));
        XMMATRIX inverse_view_matrix = XMMatrixInverse(nullptr, view_matrix);
        XMVECTOR new_quaternion_rotation = XMQuaternionRotationMatrix(inverse_view_matrix);
        XMStoreFloat4(&quaternion_rotation, new_quaternion_rotation);
    }

    DirectX::XMFLOAT3 Transform::get_euler_angles_from_rotation_matrix(const DirectX::XMFLOAT4X4 &rotation_matrix)
    {
        using namespace DirectX;

        XMFLOAT3 euler_angles = {};

        // Handle deadlock
        if (std::abs(1.0f - std::abs(rotation_matrix(2, 1))) < 1e-5f)
        {
            euler_angles.x = std::copysign(XM_PIDIV2, -rotation_matrix(2, 1));
            euler_angles.y = -std::atan2(rotation_matrix(0, 2), rotation_matrix(0, 0));
            return euler_angles;
        }

        // Calculate euler angles through rotation matrix
        float c = std::sqrt(1.0f - rotation_matrix(2, 1) * rotation_matrix(2, 1));
        // Prohibit a case that r[2][1] > 1.0f
        if (std::isnan(c)) c = 0.0f;

        euler_angles.z = std::atan2(rotation_matrix(0, 1), rotation_matrix(1, 1));
        euler_angles.x = std::atan2(-rotation_matrix(2, 1), c);
        euler_angles.y = std::atan2(rotation_matrix(2, 0), rotation_matrix(2, 2));

        return euler_angles;
    }
}






































