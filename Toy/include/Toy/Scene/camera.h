//
// Created by ZZK on 2023/5/24.
//

#pragma once

#include <Toy/Scene/transform.h>

namespace toy
{
    class Camera
    {
    public:
        Camera() = default;
        virtual ~Camera() = default;

        // Get camera transform
        [[nodiscard]] const Transform& get_transform() const;
        Transform& get_transform();

        // Get camera position
        [[nodiscard]] DirectX::XMVECTOR get_position_xm() const;
        [[nodiscard]] DirectX::XMFLOAT3 get_position() const;

        // Get euler angler(in radian) of rotation around x-axis
        [[nodiscard]] float get_rotation_x() const;
        // Get euler angler(in radian) of rotation around y-axis
        [[nodiscard]] float get_rotation_y() const;

        // Get the coordinate axis vector of camera
        [[nodiscard]] DirectX::XMVECTOR get_right_axis_xm() const;
        [[nodiscard]] DirectX::XMFLOAT3 get_right_axis() const;
        [[nodiscard]] DirectX::XMVECTOR get_up_axis_xm() const;
        [[nodiscard]] DirectX::XMFLOAT3 get_up_axis() const;
        [[nodiscard]] DirectX::XMVECTOR get_look_axis_xm() const;
        [[nodiscard]] DirectX::XMFLOAT3 get_look_axis() const;

        // Get matrix
        [[nodiscard]] DirectX::XMMATRIX get_local_to_world_xm() const;
        [[nodiscard]] DirectX::XMMATRIX get_view_xm() const;
        [[nodiscard]] DirectX::XMMATRIX get_proj_xm(bool reverse_z = false) const;
        [[nodiscard]] DirectX::XMMATRIX get_view_proj_xm(bool reverse_z = false) const;

        // Get viewport
        [[nodiscard]] D3D11_VIEWPORT get_viewport() const;
        [[nodiscard]] float get_near_z() const;
        [[nodiscard]] float get_far_z() const;
        [[nodiscard]] float get_fov_y() const;
        [[nodiscard]] float get_aspect_ratio() const;

        // Check dirty flag
        [[nodiscard]] bool is_dirty() const;

        // Move locally in x-y-z directions
        void move_local(const DirectX::XMFLOAT3 &magnitudes);

        // Set frustum
        void set_frustum(float fov_y, float aspect, float near_z, float far_z);

        // Set viewport
        void set_viewport(const D3D11_VIEWPORT &viewPort);
        void set_viewport(float top_left_x, float top_left_y, float width, float height, float min_depth = 0.0f, float max_depth = 1.0f);

        // Set dirty flag
        void set_dirty_flag(bool is_dirty);

    private:
        Transform m_transform = {};

        // Frustum properties
        float m_near_z = 0.0f;
        float m_far_z = 0.0f;
        float m_aspect = 0.0f;
        float m_fov_y = 0.0f;

        // Current viewport
        D3D11_VIEWPORT m_viewport = {};

        // Dirty flag
        bool m_dirty = false;

        // Friends
        friend class FirstPersonCamera;
        friend class ThirdPersonCamera;
    };

    class FirstPersonCamera final : public Camera
    {
    public:
        FirstPersonCamera() = default;
        ~FirstPersonCamera() override = default;

        // Set camera position
        void set_position(float x, float y, float z);
        void set_position(const DirectX::XMFLOAT3 &pos);

        // Set camera orientation
        void look_at(const DirectX::XMFLOAT3 &pos, const DirectX::XMFLOAT3 &target,const DirectX::XMFLOAT3 &up);
        void look_to(const DirectX::XMFLOAT3 &pos, const DirectX::XMFLOAT3 &to, const DirectX::XMFLOAT3 &up);

        // Translation
        void strafe(float delta);

        // Planar movement
        void walk(float delta);

        // Go forward
        void move_forward(float delta);

        // Translation
        void translate(const DirectX::XMFLOAT3 &direction, float magnitude);

        // Observe up and down, positive value means observe up, negative value means observe down
        void pitch(float radian);

        // Observe left and right, positive value means observe right, negative value means observe left
        void rotate_y(float radian);
    };

    class ThirdPersonCamera final : public Camera
    {
    public:
        ThirdPersonCamera() = default;
        ~ThirdPersonCamera() override = default;

        // Get the position of current tracked target
        [[nodiscard]] DirectX::XMFLOAT3 get_target_position() const;

        // Get the distance from the current tracked target
        [[nodiscard]] float get_distance() const;

        // Vertical rotation around target(x-axis rotation euler radian is clamped to [0, pi/3])
        void rotate_x(float radian);

        // Horizontal rotation around target
        void rotate_y(float radian);

        // Approach target
        void approach(float distance);

        // Set the initial radian around x-axis(x-axis rotation euler radian is clamped to [0, pi/3])
        void set_rotation_x(float radian);

        // Set the initial radian around y-axis
        void set_rotation_y(float radian);

        // Set and bind the position of target to be tracked
        void set_target(const DirectX::XMFLOAT3 &target);

        // Set the initial distance
        void set_distance(float distance);

        // Set minimum and maximum allowed distances
        void set_min_max_distance(float min_distance, float max_distance);

    private:
        DirectX::XMFLOAT3 m_target = {};
        float m_distance = 0.0f;
        float m_min_distance = 0.0f;
        float m_max_distance = 0.0f;
    };
}
