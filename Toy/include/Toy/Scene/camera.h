//
// Created by ZZK on 2023/5/24.
//

#pragma once

#include <Toy/Scene/transform.h>

namespace toy
{
    class camera_c
    {
    public:
        camera_c() = default;
        virtual ~camera_c() = 0;

        //
        // Get camera position
        //
        DirectX::XMVECTOR get_position_xm() const;
        DirectX::XMFLOAT3 get_position() const;

        //
        // Get camera rotation
        //
        // 获取绕X轴旋转的欧拉角弧度
        float get_rotation_x() const;
        // 获取绕Y轴旋转的欧拉角弧度
        float get_rotation_y() const;

        //
        // 获取摄像机的坐标轴向量
        //
        DirectX::XMVECTOR get_right_axis_xm() const;
        DirectX::XMFLOAT3 get_right_axis() const;
        DirectX::XMVECTOR get_up_axis_xm() const;
        DirectX::XMFLOAT3 get_up_axis() const;
        DirectX::XMVECTOR get_look_axis_xm() const;
        DirectX::XMFLOAT3 get_look_axis() const;

        //
        // 获取矩阵
        //

        DirectX::XMMATRIX get_view_xm() const;
        DirectX::XMMATRIX get_proj_xm() const;
        DirectX::XMMATRIX get_view_proj_xm() const;

        // 获取视口
        D3D11_VIEWPORT get_viewport() const;

        // 设置视锥体
        void set_frustum(float fovY, float aspect, float nearZ, float farZ);

        // 设置视口
        void set_viewport(const D3D11_VIEWPORT& viewPort);
        void set_viewport(float topLeftX, float topLeftY, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f);

    protected:

        // 摄像机的变换
        transform_c class_transform = {};

        // 视锥体属性
        float class_near_z = 0.0f;
        float class_far_z = 0.0f;
        float class_aspect = 0.0f;
        float class_fov = 0.0f;

        // 当前视口
        D3D11_VIEWPORT class_viewport = {};
    };

    class first_person_camera_c : public camera_c
    {
    public:
        first_person_camera_c() = default;
        ~first_person_camera_c() override;

        // 设置摄像机位置
        void set_position(float x, float y, float z);
        void set_position(const DirectX::XMFLOAT3& pos);
        // 设置摄像机的朝向
        void look_at(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& target,const DirectX::XMFLOAT3& up);
        void look_to(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& to, const DirectX::XMFLOAT3& up);
        // 平移
        void strafe(float d);
        // 直行(平面移动)
        void walk(float d);
        // 前进(朝前向移动)
        void move_forward(float d);
        // 上下观察
        // 正rad值向上观察
        // 负rad值向下观察
        void pitch(float rad);
        // 左右观察
        // 正rad值向右观察
        // 负rad值向左观察
        void rotate_y(float rad);
    };

    class third_person_camera_c : public camera_c
    {
    public:
        third_person_camera_c() = default;
        ~third_person_camera_c() override;

        // 获取当前跟踪物体的位置
        [[nodiscard]] DirectX::XMFLOAT3 get_target_position() const;
        // 获取与物体的距离
        [[nodiscard]] float get_distance() const;
        // 绕物体垂直旋转(注意绕x轴旋转欧拉角弧度限制在[0, pi/3])
        void rotate_x(float rad);
        // 绕物体水平旋转
        void rotate_y(float rad);
        // 拉近物体
        void approach(float dist);
        // 设置初始绕X轴的弧度(注意绕x轴旋转欧拉角弧度限制在[0, pi/3])
        void set_rotation_x(float rad);
        // 设置初始绕Y轴的弧度
        void set_rotation_y(float rad);
        // 设置并绑定待跟踪物体的位置
        void set_target(const DirectX::XMFLOAT3& target);
        // 设置初始距离
        void set_distance(float dist);
        // 设置最小最大允许距离
        void set_distance_min_max(float minDist, float maxDist);

    private:
        DirectX::XMFLOAT3 class_target = {};
        float class_distance = 0.0f;
        // 最小允许距离，最大允许距离
        float class_min_dist = 0.0f, class_max_dist = 0.0f;
    };
}
