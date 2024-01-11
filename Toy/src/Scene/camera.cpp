//
// Created by ZZK on 2023/5/24.
//

#include <Toy/Scene/camera.h>

namespace toy
{
    // Camera implementation
    const Transform &Camera::get_transform() const
    {
        return m_transform;
    }

    Transform &Camera::get_transform()
    {
        return m_transform;
    }

    DirectX::XMVECTOR Camera::get_position_xm() const
    {
        return DirectX::XMLoadFloat3(&m_transform.position);
    }

    DirectX::XMFLOAT3 Camera::get_position() const
    {
        return m_transform.position;
    }

    float Camera::get_rotation_x() const
    {
        return m_transform.get_euler_angles().x;
    }

    float Camera::get_rotation_y() const
    {
        return m_transform.get_euler_angles().y;
    }

    DirectX::XMVECTOR Camera::get_right_axis_xm() const
    {
        return m_transform.get_right_axis_xm();
    }

    DirectX::XMFLOAT3 Camera::get_right_axis() const
    {
        return m_transform.get_right_axis();
    }

    DirectX::XMVECTOR Camera::get_up_axis_xm() const
    {
        return m_transform.get_up_axis_xm();
    }

    DirectX::XMFLOAT3 Camera::get_up_axis() const
    {
        return m_transform.get_up_axis();
    }

    DirectX::XMVECTOR Camera::get_look_axis_xm() const
    {
        return m_transform.get_forward_axis_xm();
    }

    DirectX::XMFLOAT3 Camera::get_look_axis() const
    {
        return m_transform.get_forward_axis();
    }

    DirectX::XMMATRIX Camera::get_local_to_world_xm() const
    {
        return m_transform.get_local_to_world_matrix_xm();
    }

    DirectX::XMMATRIX Camera::get_view_xm() const
    {
        return m_transform.get_world_to_local_matrix_xm();
    }

    DirectX::XMMATRIX Camera::get_proj_xm(bool reverse_z) const
    {
        if (reverse_z)
        {
            return DirectX::XMMatrixPerspectiveFovLH(m_fov_y, m_aspect, m_far_z, m_near_z);
        } else
        {
            return DirectX::XMMatrixPerspectiveFovLH(m_fov_y, m_aspect, m_near_z, m_far_z);
        }
    }

    DirectX::XMMATRIX Camera::get_view_proj_xm(bool reverse_z) const
    {
        using namespace DirectX;
        return get_view_xm() * get_proj_xm(reverse_z);
    }

    D3D11_VIEWPORT Camera::get_viewport() const
    {
        return m_viewport;
    }

    float Camera::get_near_z() const
    {
        return m_near_z;
    }

    float Camera::get_far_z() const
    {
        return m_far_z;
    }

    float Camera::get_fov_y() const
    {
        return m_fov_y;
    }

    float Camera::get_aspect_ratio() const
    {
        return m_aspect;
    }

    void Camera::move_local(const DirectX::XMFLOAT3 &magnitudes)
    {
        m_transform.move_local(magnitudes);
    }

    void Camera::set_frustum(float fov_y, float aspect, float near_z, float far_z)
    {
        m_fov_y = fov_y;
        m_aspect = aspect;
        m_near_z = near_z;
        m_far_z = far_z;
    }

    void Camera::set_viewport(const D3D11_VIEWPORT &viewPort)
    {
        m_viewport = viewPort;
    }

    void Camera::set_viewport(float top_left_x, float top_left_y, float width, float height, float min_depth, float max_depth)
    {
        m_viewport.TopLeftX = top_left_x;
        m_viewport.TopLeftY = top_left_y;
        m_viewport.Width = width;
        m_viewport.Height = height;
        m_viewport.MinDepth = min_depth;
        m_viewport.MaxDepth = max_depth;
    }

    // First person camera implementation
    void FirstPersonCamera::set_position(float x, float y, float z)
    {
        set_position(DirectX::XMFLOAT3{ x, y, z });
    }

    void FirstPersonCamera::set_position(const DirectX::XMFLOAT3 &pos)
    {
        m_transform.set_position(pos);
    }

    void FirstPersonCamera::look_at(const DirectX::XMFLOAT3 &pos, const DirectX::XMFLOAT3 &target, const DirectX::XMFLOAT3 &up)
    {
        m_transform.set_position(pos);
        m_transform.look_at(target, up);
    }

    void FirstPersonCamera::look_to(const DirectX::XMFLOAT3 &pos, const DirectX::XMFLOAT3 &to, const DirectX::XMFLOAT3 &up)
    {
        m_transform.set_position(pos);
        m_transform.look_to(to, up);
    }

    void FirstPersonCamera::strafe(float delta)
    {
        m_transform.translate(m_transform.get_right_axis(), delta);
    }

    void FirstPersonCamera::walk(float delta)
    {
        using namespace DirectX;
        XMVECTOR right_vec = m_transform.get_right_axis_xm();
        XMVECTOR front_vec = XMVector3Normalize(XMVector3Cross(right_vec, g_XMIdentityR1));
        XMFLOAT3 front = {};
        XMStoreFloat3(&front, front_vec);
        m_transform.translate(front, delta);
    }

    void FirstPersonCamera::move_forward(float delta)
    {
        m_transform.translate(m_transform.get_forward_axis(), delta);
    }

    void FirstPersonCamera::translate(const DirectX::XMFLOAT3 &direction, float magnitude)
    {
        m_transform.translate(direction, magnitude);
    }

    void FirstPersonCamera::pitch(float radian)
    {
        using namespace DirectX;
        XMFLOAT3 rotation = m_transform.get_euler_angles();
        // Clamp to [-7pi / 18, 7pi / 18]
        rotation.x += radian;
        rotation.x = std::clamp(rotation.x, -XM_PI * 7.0f / 18.0f, XM_PI * 7.0f / 18.0f);

        m_transform.set_rotation(rotation);
    }

    void FirstPersonCamera::rotate_y(float radian)
    {
        using namespace DirectX;
        XMFLOAT3 rotation = m_transform.get_euler_angles();
        rotation.y = XMScalarModAngle(rotation.y + radian);
        m_transform.set_rotation(rotation);
    }

    // Third person camera implementation
    DirectX::XMFLOAT3 ThirdPersonCamera::get_target_position() const
    {
        return m_target;
    }

    float ThirdPersonCamera::get_distance() const
    {
        return m_distance;
    }

    void ThirdPersonCamera::rotate_x(float radian)
    {
        using namespace DirectX;

        auto rotation = m_transform.get_euler_angles();

        rotation.x += radian;
        rotation.x = std::clamp(rotation.x, 0.0f, XM_PI / 3.0f);

        m_transform.set_rotation(rotation);
        m_transform.set_position(m_target);
        m_transform.translate(m_transform.get_forward_axis(), -m_distance);
    }

    void ThirdPersonCamera::rotate_y(float radian)
    {
        using namespace DirectX;

        auto rotation = m_transform.get_euler_angles();
        rotation.y = XMScalarModAngle(rotation.y + radian);

        m_transform.set_rotation(rotation);
        m_transform.set_position(m_target);
        m_transform.translate(m_transform.get_forward_axis(), -m_distance);
    }

    void ThirdPersonCamera::approach(float distance)
    {
        m_distance += distance;
        m_distance = std::clamp(m_distance, m_min_distance, m_max_distance);

        m_transform.set_position(m_target);
        m_transform.translate(m_transform.get_forward_axis(), -m_distance);
    }

    void ThirdPersonCamera::set_rotation_x(float radian)
    {
        using namespace DirectX;

        auto rotation = m_transform.get_euler_angles();

        rotation.x = radian;
        rotation.x = std::clamp(rotation.x, 0.0f, XM_PI / 3.0f);

        m_transform.set_rotation(rotation);
        m_transform.set_position(m_target);
        m_transform.translate(m_transform.get_forward_axis(), -m_distance);
    }

    void ThirdPersonCamera::set_rotation_y(float radian)
    {
        using namespace DirectX;

        auto rotation = m_transform.get_euler_angles();
        rotation.y = XMScalarModAngle(radian);

        m_transform.set_rotation(rotation);
        m_transform.set_position(m_target);
        m_transform.translate(m_transform.get_forward_axis(), -m_distance);
    }

    void ThirdPersonCamera::set_target(const DirectX::XMFLOAT3 &target)
    {
        m_target = target;
    }

    void ThirdPersonCamera::set_distance(float distance)
    {
        m_distance = distance;
    }

    void ThirdPersonCamera::set_min_max_distance(float min_distance, float max_distance)
    {
        m_min_distance = min_distance;
        m_max_distance = max_distance;
    }
}