//
// Created by ZZK on 2023/5/24.
//

#include <Toy/Scene/camera.h>

namespace toy
{
    // Camera
    using namespace DirectX;
    camera_c::~camera_c()
    {
    }

    XMVECTOR camera_c::get_position_xm() const
    {
        return class_transform.get_position_xm();
    }

    XMFLOAT3 camera_c::get_position() const
    {
        return class_transform.get_position();
    }

    float camera_c::get_rotation_x() const
    {
        return class_transform.get_rotation().x;
    }

    float camera_c::get_rotation_y() const
    {
        return class_transform.get_rotation().y;
    }


    XMVECTOR camera_c::get_right_axis_xm() const
    {
        return class_transform.get_right_axis_xm();
    }

    XMFLOAT3 camera_c::get_right_axis() const
    {
        return class_transform.get_right_axis();
    }

    XMVECTOR camera_c::get_up_axis_xm() const
    {
        return class_transform.get_up_axis_xm();
    }

    XMFLOAT3 camera_c::get_up_axis() const
    {
        return class_transform.get_up_axis();
    }

    XMVECTOR camera_c::get_look_axis_xm() const
    {
        return class_transform.get_forward_axis_xm();
    }

    XMFLOAT3 camera_c::get_look_axis() const
    {
        return class_transform.get_forward_axis();
    }

    XMMATRIX camera_c::get_view_xm() const
    {
        return class_transform.get_world_to_local_matrix_xm();
    }

    XMMATRIX camera_c::get_proj_xm() const
    {
        return XMMatrixPerspectiveFovLH(class_fov, class_aspect, class_near_z, class_far_z);
    }

    XMMATRIX camera_c::get_view_proj_xm() const
    {
        return get_view_xm() * get_proj_xm();
    }

    D3D11_VIEWPORT camera_c::get_viewport() const
    {
        return class_viewport;
    }

    void camera_c::set_frustum(float fovY, float aspect, float nearZ, float farZ)
    {
        class_fov = fovY;
        class_aspect = aspect;
        class_near_z = nearZ;
        class_far_z = farZ;
    }

    void camera_c::set_viewport(const D3D11_VIEWPORT & viewPort)
    {
        class_viewport = viewPort;
    }

    void camera_c::set_viewport(float topLeftX, float topLeftY, float width, float height, float minDepth, float maxDepth)
    {
        class_viewport.TopLeftX = topLeftX;
        class_viewport.TopLeftY = topLeftY;
        class_viewport.Width = width;
        class_viewport.Height = height;
        class_viewport.MinDepth = minDepth;
        class_viewport.MaxDepth = maxDepth;
    }

    // First person camera
    first_person_camera_c::~first_person_camera_c()
    {
    }

    void first_person_camera_c::set_position(float x, float y, float z)
    {
        set_position(XMFLOAT3(x, y, z));
    }

    void first_person_camera_c::set_position(const XMFLOAT3& pos)
    {
        class_transform.set_position(pos);
    }

    void first_person_camera_c::look_at(const XMFLOAT3 & pos, const XMFLOAT3 & target,const XMFLOAT3 & up)
    {
        class_transform.set_position(pos);
        class_transform.look_at(target, up);
    }

    void first_person_camera_c::look_to(const XMFLOAT3 & pos, const XMFLOAT3 & to, const XMFLOAT3 & up)
    {
        class_transform.set_position(pos);
        class_transform.look_to(to, up);
    }

    void first_person_camera_c::strafe(float d)
    {
        class_transform.translate(class_transform.get_right_axis(), d);
    }

    void first_person_camera_c::walk(float d)
    {
        XMVECTOR rightVec = class_transform.get_right_axis_xm();
        XMVECTOR frontVec = XMVector3Normalize(XMVector3Cross(rightVec, g_XMIdentityR1));
        XMFLOAT3 front{};
        XMStoreFloat3(&front, frontVec);
        class_transform.translate(front, d);
    }

    void first_person_camera_c::move_forward(float d)
    {
        class_transform.translate(class_transform.get_forward_axis(), d);
    }

    void first_person_camera_c::pitch(float rad)
    {
        XMFLOAT3 rotation = class_transform.get_rotation();
        // 将绕x轴旋转弧度限制在[-7pi/18, 7pi/18]之间
        rotation.x += rad;
        if (rotation.x > XM_PI * 7 / 18)
            rotation.x = XM_PI * 7 / 18;
        else if (rotation.x < -XM_PI * 7 / 18)
            rotation.x = -XM_PI * 7 / 18;

        class_transform.set_rotation(rotation);
    }

    void first_person_camera_c::rotate_y(float rad)
    {
        XMFLOAT3 rotation = class_transform.get_rotation();
        rotation.y = XMScalarModAngle(rotation.y + rad);
        class_transform.set_rotation(rotation);
    }

    // Third person camera
    third_person_camera_c::~third_person_camera_c()
    {
    }

    XMFLOAT3 third_person_camera_c::get_target_position() const
    {
        return class_target;
    }

    float third_person_camera_c::get_distance() const
    {
        return class_distance;
    }

    void third_person_camera_c::rotate_x(float rad)
    {
        XMFLOAT3 rotation = class_transform.get_rotation();
        // 将绕x轴旋转弧度限制在[0, pi/3]之间
        rotation.x += rad;
        if (rotation.x < 0.0f)
            rotation.x = 0.0f;
        else if (rotation.x > XM_PI / 3)
            rotation.x = XM_PI / 3;

        class_transform.set_rotation(rotation);
        class_transform.set_position(class_target);
        class_transform.translate(class_transform.get_forward_axis(), -class_distance);
    }

    void third_person_camera_c::rotate_y(float rad)
    {
        XMFLOAT3 rotation = class_transform.get_rotation();
        rotation.y = XMScalarModAngle(rotation.y + rad);

        class_transform.set_rotation(rotation);
        class_transform.set_position(class_target);
        class_transform.translate(class_transform.get_forward_axis(), -class_distance);
    }

    void third_person_camera_c::approach(float dist)
    {
        class_distance += dist;
        // 限制距离在[m_MinDist, m_MaxDist]之间
        if (class_distance < class_min_dist)
            class_distance = class_min_dist;
        else if (class_distance > class_max_dist)
            class_distance = class_max_dist;

        class_transform.set_position(class_target);
        class_transform.translate(class_transform.get_forward_axis(), -class_distance);
    }

    void third_person_camera_c::set_rotation_x(float rad)
    {
        XMFLOAT3 rotation = class_transform.get_rotation();
        // 将绕x轴旋转弧度限制在[0, pi/3]之间
        rotation.x = rad;
        if (rotation.x < 0.0f)
            rotation.x = 0.0f;
        else if (rotation.x > XM_PI / 3)
            rotation.x = XM_PI / 3;

        class_transform.set_rotation(rotation);
        class_transform.set_position(class_target);
        class_transform.translate(class_transform.get_forward_axis(), -class_distance);
    }

    void third_person_camera_c::set_rotation_y(float rad)
    {
        XMFLOAT3 rotation = class_transform.get_rotation();
        rotation.y = XMScalarModAngle(rad);
        class_transform.set_rotation(rotation);
        class_transform.set_position(class_target);
        class_transform.translate(class_transform.get_forward_axis(), -class_distance);
    }

    void third_person_camera_c::set_target(const XMFLOAT3 & target)
    {
        class_target = target;
    }

    void third_person_camera_c::set_distance(float dist)
    {
        class_distance = dist;
    }

    void third_person_camera_c::set_distance_min_max(float minDist, float maxDist)
    {
        class_min_dist = minDist;
        class_max_dist = maxDist;
    }
}