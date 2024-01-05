//
// Created by ZHIKANG on 2023/6/23.
//

#include <Toy/Scene/collision.h>

namespace toy
{
    // Ray
    Ray::Ray()
    : origin(), direction(0.0f, 0.0f, 1.0f)
    {
    }

    Ray::Ray(const DirectX::XMFLOAT3 &origin, const DirectX::XMFLOAT3 &direction)
    : origin(origin)
    {
        using namespace DirectX;

        XMVECTOR dirVec = XMLoadFloat3(&direction);
        assert(XMVector3NotEqual(dirVec, g_XMZero));
        XMStoreFloat3(&this->direction, XMVector3Normalize(dirVec));
    }

    Ray Ray::screen_to_ray(const Camera &camera, float screenX, float screenY)
    {
        using namespace DirectX;

        // 参考DirectX::XMVector3Unproject函数，并省略了从世界坐标系到局部坐标系的变换

        // 将屏幕坐标点从视口变换回NDC坐标系
        static const XMVECTORF32 D = { { { -1.0f, 1.0f, 0.0f, 0.0f } } };
        XMVECTOR V = XMVectorSet(screenX, screenY, 0.0f, 1.0f);
        D3D11_VIEWPORT viewPort = camera.get_viewport();

        XMVECTOR Scale = XMVectorSet(viewPort.Width * 0.5f, -viewPort.Height * 0.5f, viewPort.MaxDepth - viewPort.MinDepth, 1.0f);
        Scale = XMVectorReciprocal(Scale);

        XMVECTOR Offset = XMVectorSet(-viewPort.TopLeftX, -viewPort.TopLeftY, -viewPort.MinDepth, 0.0f);
        Offset = XMVectorMultiplyAdd(Scale, Offset, D.v);

        // 从NDC坐标系变换回世界坐标系
        XMMATRIX Transform = XMMatrixMultiply(camera.get_view_xm(), camera.get_proj_xm());
        Transform = XMMatrixInverse(nullptr, Transform);

        XMVECTOR Target = XMVectorMultiplyAdd(V, Scale, Offset);
        Target = XMVector3TransformCoord(Target, Transform);

        // 求出射线
        XMFLOAT3 direction{};
        XMStoreFloat3(&direction, Target - camera.get_position_xm());
        return Ray{camera.get_position(), direction };
    }

    bool Ray::hit(const DirectX::BoundingBox &box, float *pOutDist, float maxDist)
    {

        float dist;
        bool res = box.Intersects(XMLoadFloat3(&origin), XMLoadFloat3(&direction), dist);
        if (pOutDist)
        {
            *pOutDist = dist;
        }
        return dist > maxDist ? false : res;
    }

    bool Ray::hit(const DirectX::BoundingOrientedBox &box, float *pOutDist, float maxDist)
    {
        float dist;
        bool res = box.Intersects(XMLoadFloat3(&origin), XMLoadFloat3(&direction), dist);
        if (pOutDist)
        {
            *pOutDist = dist;
        }
        return dist > maxDist ? false : res;
    }

    bool Ray::hit(const DirectX::BoundingSphere &sphere, float *pOutDist, float maxDist)
    {
        float dist;
        bool res = sphere.Intersects(XMLoadFloat3(&origin), XMLoadFloat3(&direction), dist);
        if (pOutDist)
            *pOutDist = dist;
        return dist > maxDist ? false : res;
    }

    bool XM_CALLCONV Ray::hit(DirectX::FXMVECTOR V0, DirectX::FXMVECTOR V1, DirectX::FXMVECTOR V2, float * pOutDist, float maxDist)
    {
        using namespace DirectX;
        float dist;
        bool res = TriangleTests::Intersects(XMLoadFloat3(&origin), XMLoadFloat3(&direction), V0, V1, V2, dist);
        if (pOutDist)
        *pOutDist = dist;
        return dist > maxDist ? false : res;
    }

    // Collision
    Collision::WireFrameData Collision::create_bounding_box(const DirectX::BoundingBox& box, const DirectX::XMFLOAT4& color)
    {
        using namespace DirectX;

        XMFLOAT3 corners[8];
        box.GetCorners(corners);
        return create_from_corners(corners, color);
    }

    Collision::WireFrameData Collision::create_bounding_oriented_box(const DirectX::BoundingOrientedBox& box, const DirectX::XMFLOAT4& color)
    {
        using namespace DirectX;

        XMFLOAT3 corners[8];
        box.GetCorners(corners);
        return create_from_corners(corners, color);
    }

    Collision::WireFrameData Collision::create_bounding_sphere(const DirectX::BoundingSphere& sphere, const DirectX::XMFLOAT4& color, int slices)
    {
        using namespace DirectX;

        WireFrameData data;
        XMVECTOR center = XMLoadFloat3(&sphere.Center), posVec;
        XMFLOAT3 pos{};
        float theta = 0.0f;
        for (int i = 0; i < slices; ++i)
        {
            posVec = XMVector3Transform(center + XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f), XMMatrixRotationY(theta));
            XMStoreFloat3(&pos, posVec);
            data.vertexVec.push_back({ pos, color });
            posVec = XMVector3Transform(center + XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f), XMMatrixRotationZ(theta));
            XMStoreFloat3(&pos, posVec);
            data.vertexVec.push_back({ pos, color });
            posVec = XMVector3Transform(center + XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f), XMMatrixRotationX(theta));
            XMStoreFloat3(&pos, posVec);
            data.vertexVec.push_back({ pos, color });
            theta += XM_2PI / slices;
        }
        for (int i = 0; i < slices; ++i)
        {
            data.indexVec.push_back(i * 3);
            data.indexVec.push_back((i + 1) % slices * 3);

            data.indexVec.push_back(i * 3 + 1);
            data.indexVec.push_back((i + 1) % slices * 3 + 1);

            data.indexVec.push_back(i * 3 + 2);
            data.indexVec.push_back((i + 1) % slices * 3 + 2);
        }


        return data;
    }

    Collision::WireFrameData Collision::create_bounding_frustum(const DirectX::BoundingFrustum& frustum, const DirectX::XMFLOAT4& color)
    {
        using namespace DirectX;

        XMFLOAT3 corners[8];
        frustum.GetCorners(corners);
        return create_from_corners(corners, color);
    }

    std::vector<Transform> XM_CALLCONV Collision::frustum_culling(
            const std::vector<Transform>& transforms, const DirectX::BoundingBox& localBox, DirectX::FXMMATRIX View, DirectX::CXMMATRIX Proj)
    {
        using namespace DirectX;

        std::vector<Transform> acceptedData;

        BoundingFrustum frustum;
        BoundingFrustum::CreateFromMatrix(frustum, Proj);

        BoundingOrientedBox localOrientedBox, orientedBox;
        BoundingOrientedBox::CreateFromBoundingBox(localOrientedBox, localBox);
        for (auto& t : transforms)
        {
            XMMATRIX W = t.get_local_to_world_matrix_xm();
            // 将有向包围盒从局部坐标系变换到视锥体所在的局部坐标系(观察坐标系)中
            localOrientedBox.Transform(orientedBox, W * View);
            // 相交检测
            if (frustum.Intersects(orientedBox))
                acceptedData.push_back(t);
        }

        return acceptedData;
    }

    void XM_CALLCONV Collision::frustum_culling(
            std::vector<Transform>& dest, const std::vector<Transform>& src, const DirectX::BoundingBox& localBox, DirectX::FXMMATRIX View, DirectX::CXMMATRIX Proj)
    {
        using namespace DirectX;

        dest.clear();

        BoundingFrustum frustum;
        BoundingFrustum::CreateFromMatrix(frustum, Proj);

        BoundingOrientedBox localOrientedBox, orientedBox;
        BoundingOrientedBox::CreateFromBoundingBox(localOrientedBox, localBox);
        for (auto& t : src)
        {
            XMMATRIX W = t.get_local_to_world_matrix_xm();
            // 将有向包围盒从局部坐标系变换到视锥体所在的局部坐标系(观察坐标系)中
            localOrientedBox.Transform(orientedBox, W * View);
            // 相交检测
            if (frustum.Intersects(orientedBox))
            dest.push_back(t);
        }
    }

    Collision::WireFrameData Collision::create_from_corners(const DirectX::XMFLOAT3(&corners)[8], const DirectX::XMFLOAT4& color)
    {
        WireFrameData data;
        // AABB/OBB顶点索引如下    视锥体顶点索引如下
        //     3_______2             4__________5
        //    /|      /|             |\        /|
        //  7/_|____6/ |             | \      / |
        //  |  |____|__|            7|_0\____/1_|6
        //  | /0    | /1              \ |    | /
        //  |/______|/                 \|____|/
        //  4       5                   3     2
        for (int i = 0; i < 8; ++i)
        {
            data.vertexVec.push_back({ corners[i], color });
        }
        for (int i = 0; i < 4; ++i)
        {
            data.indexVec.push_back(i);
            data.indexVec.push_back(i + 4);

            data.indexVec.push_back(i);
            data.indexVec.push_back((i + 1) % 4);

            data.indexVec.push_back(i + 4);
            data.indexVec.push_back((i + 1) % 4 + 4);
        }
        return data;
    }
}




































