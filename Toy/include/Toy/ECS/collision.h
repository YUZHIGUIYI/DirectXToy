//
// Created by ZHIKANG on 2023/6/23.
//

#pragma once

#include <Toy/ECS/camera.h>
#include <Toy/Geometry/vertex.h>

namespace toy
{
    struct Ray
    {
        Ray();
        Ray(const DirectX::XMFLOAT3& origin, const DirectX::XMFLOAT3& direction);

        static Ray screen_to_ray(const Camera& camera, float screenX, float screenY);

        bool hit(const DirectX::BoundingBox& box, float* pOutDist = nullptr, float maxDist = FLT_MAX);
        bool hit(const DirectX::BoundingOrientedBox& box, float* pOutDist = nullptr, float maxDist = FLT_MAX);
        bool hit(const DirectX::BoundingSphere& sphere, float* pOutDist = nullptr, float maxDist = FLT_MAX);
        bool XM_CALLCONV hit(DirectX::FXMVECTOR V0, DirectX::FXMVECTOR V1, DirectX::FXMVECTOR V2, float* pOutDist = nullptr, float maxDist = FLT_MAX);

        DirectX::XMFLOAT3 origin;		// Ray origin
        DirectX::XMFLOAT3 direction;	// Unit direction vector
    };

    class Collision
    {
    public:
        // 线框顶点/索引数组
        struct WireFrameData
        {
            std::vector<VertexPosColor> vertexVec;		// 顶点数组
            std::vector<uint32_t> indexVec;				// 索引数组
        };

        //
        // 包围盒线框的创建
        //
        // 创建AABB盒线框
        static WireFrameData create_bounding_box(const DirectX::BoundingBox& box, const DirectX::XMFLOAT4& color);
        // 创建OBB盒线框
        static WireFrameData create_bounding_oriented_box(const DirectX::BoundingOrientedBox& box, const DirectX::XMFLOAT4& color);
        // 创建包围球线框
        static WireFrameData create_bounding_sphere(const DirectX::BoundingSphere& sphere, const DirectX::XMFLOAT4& color, int slices = 20);
        // 创建视锥体线框
        static WireFrameData create_bounding_frustum(const DirectX::BoundingFrustum& frustum, const DirectX::XMFLOAT4& color);

        // 视锥体裁剪
        static std::vector<Transform> XM_CALLCONV frustum_culling(
                const std::vector<Transform>& transforms, const DirectX::BoundingBox& localBox, DirectX::FXMMATRIX View, DirectX::CXMMATRIX Proj);

        // 视锥体裁剪
        static void XM_CALLCONV frustum_culling(
                std::vector<Transform>& dest, const std::vector<Transform>& src,
        const DirectX::BoundingBox& localBox, DirectX::FXMMATRIX View, DirectX::CXMMATRIX Proj);

    private:
        static WireFrameData create_from_corners(const DirectX::XMFLOAT3(&corners)[8], const DirectX::XMFLOAT4& color);
    };
}
