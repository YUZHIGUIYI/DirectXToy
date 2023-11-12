//
// Created by ZZK on 2023/10/15.
//

#include <Toy/Renderer/cascaded_shadow_manager.h>

namespace toy
{
    struct Triangle
    {
        std::array<DirectX::XMVECTOR, 3> points = {};
        bool is_culled = false;
    };

    CascadedShadowManager::CascadedShadowManager() = default;

    CascadedShadowManager::~CascadedShadowManager() = default;

    CascadedShadowManager::CascadedShadowManager(CascadedShadowManager &&) noexcept = default;

    CascadedShadowManager &CascadedShadowManager::operator=(CascadedShadowManager &&) noexcept = default;

    void CascadedShadowManager::init(ID3D11Device *device)
    {
        csm_texture_array = std::make_unique<Depth2DArray>(device, shadow_size, shadow_size, cascade_levels);

        shadow_viewport.TopLeftX = 0.0f;
        shadow_viewport.TopLeftY = 0.0f;
        shadow_viewport.Width = static_cast<float>(shadow_size);
        shadow_viewport.Height = static_cast<float>(shadow_size);
        shadow_viewport.MinDepth = 0.0f;
        shadow_viewport.MaxDepth = 1.0f;
    }

    CascadedShadowManager &CascadedShadowManager::get()
    {
        static CascadedShadowManager cascaded_shadow_manager = {};
        return cascaded_shadow_manager;
    }

    ID3D11DepthStencilView *CascadedShadowManager::get_cascade_depth_stencil_view(size_t cascade_index) const
    {
        return csm_texture_array->get_depth_stencil(cascade_index);
    }

    ID3D11ShaderResourceView *CascadedShadowManager::get_cascades_output() const
    {
        return csm_texture_array->get_shader_resource();
    }

    ID3D11ShaderResourceView *CascadedShadowManager::get_cascade_output(size_t cascade_index) const
    {
        return csm_texture_array->get_shader_resource(cascade_index);
    }

    const float *CascadedShadowManager::get_cascade_partitions() const
    {
        return cascade_partitions_frustum.data();
    }

    void CascadedShadowManager::get_cascade_partitions(float *output) const
    {
        memcpy_s(output, sizeof(cascade_partitions_frustum), cascade_partitions_frustum.data(), sizeof(cascade_partitions_frustum));
    }

    DirectX::XMMATRIX CascadedShadowManager::get_shadow_project_xm(size_t cascade_index) const
    {
        using namespace DirectX;
        return XMLoadFloat4x4(&shadow_proj[cascade_index]);
    }

    const DirectX::BoundingBox &CascadedShadowManager::get_shadow_aabb(size_t cascade_index) const
    {
        return shadow_proj_bounding_box[cascade_index];
    }

    DirectX::BoundingOrientedBox CascadedShadowManager::get_shadow_obb(size_t cascade_index) const
    {
        DirectX::BoundingOrientedBox obb;
        DirectX::BoundingOrientedBox::CreateFromBoundingBox(obb, get_shadow_aabb(cascade_index));
        return obb;
    }

    D3D11_VIEWPORT CascadedShadowManager::get_shadow_viewport() const
    {
        return shadow_viewport;
    }

    void CascadedShadowManager::update_frame(const toy::camera_c &viewer_camera, const toy::camera_c &light_camera,
                                                const DirectX::BoundingBox &scene_bounding_box)
    {
        using namespace DirectX;

        XMMATRIX ViewerProj = viewer_camera.get_proj_xm();
        XMMATRIX ViewerView = viewer_camera.get_view_xm();
        XMMATRIX LightView = light_camera.get_view_xm();
        XMMATRIX ViewerInvView = XMMatrixInverse(nullptr, ViewerView);

        float frustumIntervalBegin, frustumIntervalEnd;
        XMVECTOR lightCameraOrthographicMinVec;     // 视锥体在光照空间下的AABB vMin
        XMVECTOR lightCameraOrthographicMaxVec;     // 视锥体在光照空间下的AABB vMax
        float cameraNearFarRange = viewer_camera.get_far_z() - viewer_camera.get_near_z();

        XMVECTOR worldUnitsPerTexelVec = g_XMZero;
        //
        // 为每个级联计算光照空间下的正交投影矩阵
        //
        for (size_t cascadeIndex = 0; cascadeIndex < cascade_levels; ++cascadeIndex)
        {
            // 计算当前级联覆盖的视锥体区间。我们以沿着Z轴最小/最大距离来衡量级联覆盖的区间
            if (selected_cascades_fit == FitProjection::FitProjection_ToCascade)
            {
                // 因为我们希望让正交投影矩阵在级联周围紧密贴合，我们将最小级联值
                // 设置为上一级联的区间末端
                if (cascadeIndex == 0)
                    frustumIntervalBegin = 0.0f;
                else
                    frustumIntervalBegin = cascade_partitions_percentage[cascadeIndex - 1];
            } else
            {
                // 在FIT_PROJECTION_TO_SCENE中，这些级联相互重叠
                // 比如级联1-8覆盖了区间1
                // 级联2-8覆盖了区间2
                frustumIntervalBegin = 0.0f;
            }

            // 算出视锥体Z区间
            frustumIntervalEnd = cascade_partitions_percentage[cascadeIndex];
            frustumIntervalBegin = frustumIntervalBegin * cameraNearFarRange;
            frustumIntervalEnd = frustumIntervalEnd * cameraNearFarRange;

            XMFLOAT3 viewerFrustumPoints[8];
            BoundingFrustum viewerFrustum(ViewerProj);
            viewerFrustum.Near = frustumIntervalBegin;
            viewerFrustum.Far = frustumIntervalEnd;
            // 将局部视锥体变换到世界空间后，再变换到光照空间
            viewerFrustum.Transform(viewerFrustum, ViewerInvView * LightView);
            viewerFrustum.GetCorners(viewerFrustumPoints);
            // 计算视锥体在光照空间下的AABB和vMax, vMin
            BoundingBox viewerFrustumBox;
            BoundingBox::CreateFromPoints(viewerFrustumBox, 8, viewerFrustumPoints, sizeof(XMFLOAT3));
            lightCameraOrthographicMaxVec = XMLoadFloat3(&viewerFrustumBox.Center) + XMLoadFloat3(&viewerFrustumBox.Extents);
            lightCameraOrthographicMinVec = XMLoadFloat3(&viewerFrustumBox.Center) - XMLoadFloat3(&viewerFrustumBox.Extents);

            // 消除由于光线改变或摄像机视角变化导致阴影边缘出现的闪烁效果
            if (fixed_size_frustum_aabb)
            {
                // 使用max(子视锥体的斜对角线, 远平面对角线)的长度作为XY的宽高，从而固定AABB的宽高
                // 并且它的宽高足够大，且总是能覆盖当前级联的视锥体

                //     Near    Far
                //    0----1  4----5
                //    |    |  |    |
                //    |    |  |    |
                //    3----2  7----6
                XMVECTOR diagVec = XMLoadFloat3(viewerFrustumPoints + 7) - XMLoadFloat3(viewerFrustumPoints + 1);   // 子视锥体的斜对角线
                XMVECTOR diag2Vec = XMLoadFloat3(viewerFrustumPoints + 7) - XMLoadFloat3(viewerFrustumPoints + 5);  // 远平面对角线
                // 找到较长的对角线作为AABB的宽高
                XMVECTOR lengthVec = XMVectorMax(XMVector3Length(diagVec), XMVector3Length(diag2Vec));

                // 计算出的偏移量会填充正交投影
                XMVECTOR borderOffsetVec = (lengthVec - (lightCameraOrthographicMaxVec - lightCameraOrthographicMinVec)) * g_XMOneHalf.v;
                // 仅对XY方向进行填充
                static const XMVECTORF32 xyzw1100Vec = { {1.0f, 1.0f, 0.0f, 0.0f} };
                lightCameraOrthographicMaxVec += borderOffsetVec * xyzw1100Vec.v;
                lightCameraOrthographicMinVec -= borderOffsetVec * xyzw1100Vec.v;
            }

            // 基于PCF核的大小再计算一个边界扩充值使得包围盒稍微放大一些。
            // 等比缩放不会影响前面固定大小的AABB
            {
                float scaleDuetoBlur = static_cast<float>(pcf_kernel_size) / static_cast<float>(shadow_size);
                XMVECTORF32 scaleDuetoBlurVec = { {scaleDuetoBlur, scaleDuetoBlur, 0.0f, 0.0f} };

                XMVECTOR borderOffsetVec = lightCameraOrthographicMaxVec - lightCameraOrthographicMinVec;
                borderOffsetVec *= g_XMOneHalf.v;
                borderOffsetVec *= scaleDuetoBlurVec.v;
                lightCameraOrthographicMaxVec += borderOffsetVec;
                lightCameraOrthographicMinVec -= borderOffsetVec;
            }


            if (move_light_texel_size)
            {
                // 计算阴影图中每个texel对应世界空间的宽高，用于后续避免阴影边缘的闪烁
                float normalizeByBufferSize = 1.0f / static_cast<float>(shadow_size);
                XMVECTORF32 normalizeByBufferSizeVec = { {normalizeByBufferSize, normalizeByBufferSize, 0.0f, 0.0f} };
                worldUnitsPerTexelVec = lightCameraOrthographicMaxVec - lightCameraOrthographicMinVec;
                worldUnitsPerTexelVec *= normalizeByBufferSize;

                // worldUnitsPerTexel
                // | |                     光照空间
                // [x][x][ ]    [ ][x][x]  x是阴影texel
                // [x][x][ ] => [ ][x][x]
                // [ ][ ][ ]    [ ][ ][ ]
                // 在摄像机移动的时候，视锥体在光照空间下的AABB并不会立马跟着移动
                // 而是累积到texel对应世界空间的宽高的变化时，AABB才会发生一次texel大小的跃动
                // 所以移动摄像机的时候不会出现阴影的抖动
                lightCameraOrthographicMinVec /= worldUnitsPerTexelVec;
                lightCameraOrthographicMinVec = XMVectorFloor(lightCameraOrthographicMinVec);
                lightCameraOrthographicMinVec *= worldUnitsPerTexelVec;

                lightCameraOrthographicMaxVec /= worldUnitsPerTexelVec;
                lightCameraOrthographicMaxVec = XMVectorFloor(lightCameraOrthographicMaxVec);
                lightCameraOrthographicMaxVec *= worldUnitsPerTexelVec;
            }

            float nearPlane = 0.0f;
            float farPlane = 0.0f;

            // 将场景AABB的角点变换到光照空间
            XMVECTOR sceneAABBPointsLightSpace[8]{};
            {
                XMFLOAT3 corners[8];
                scene_bounding_box.GetCorners(corners);
                for (size_t i = 0; i < 8; ++i)
                {
                    XMVECTOR v = XMLoadFloat3(corners + i);
                    sceneAABBPointsLightSpace[i] = XMVector3Transform(v, LightView);
                }
            }

            if (selected_near_far_fit == FitNearFar::FitNearFar_ZeroOne)
            {
                // 下面的值用于展示计算准确的近平面和远平面的重要性，否则会出现可怕的结果
                nearPlane = 0.1f;
                farPlane = 10000.0f;
            }
            if (selected_near_far_fit == FitNearFar::FitNearFar_CascadeAABB)
            {
                // 由于缺乏视锥体AABB外的信息，位于近平面外的物体无法投射到shadow map
                // 故无法产生遮挡
                nearPlane = XMVectorGetZ(lightCameraOrthographicMinVec);
                farPlane = XMVectorGetZ(lightCameraOrthographicMaxVec);
            } else if (selected_near_far_fit == FitNearFar::FitNearFar_SceneAABB)
            {
                XMVECTOR lightSpaceSceneAABBminValueVec = g_XMFltMax.v;
                XMVECTOR lightSpaceSceneAABBmaxValueVec = -g_XMFltMax.v;
                // 计算光照空间下场景的min max向量
                // 其中光照空间AABB的minZ和maxZ可以用于近平面和远平面
                // 这比场景与AABB的相交测试简单，在某些情况下也能提供相似的结果
                for (size_t i = 0; i < 8; ++i)
                {
                    lightSpaceSceneAABBminValueVec = XMVectorMin(sceneAABBPointsLightSpace[i], lightSpaceSceneAABBminValueVec);
                    lightSpaceSceneAABBmaxValueVec = XMVectorMax(sceneAABBPointsLightSpace[i], lightSpaceSceneAABBmaxValueVec);
                }
                nearPlane = XMVectorGetZ(lightSpaceSceneAABBminValueVec);
                farPlane = XMVectorGetZ(lightSpaceSceneAABBmaxValueVec);
            } else if (selected_near_far_fit == FitNearFar::FitNearFar_SceneAABB_Intersection)
            {
                // 通过光照空间下视锥体的AABB 与 变换到光照空间的场景AABB 的相交测试，可以得到一个更紧密的近平面和远平面
                compute_near_far(nearPlane, farPlane, lightCameraOrthographicMinVec,
                                    lightCameraOrthographicMaxVec,sceneAABBPointsLightSpace);
            }

            XMStoreFloat4x4(shadow_proj.data() + cascadeIndex,
                            XMMatrixOrthographicOffCenterLH(XMVectorGetX(lightCameraOrthographicMinVec), XMVectorGetX(lightCameraOrthographicMaxVec),
                                                            XMVectorGetY(lightCameraOrthographicMinVec), XMVectorGetY(lightCameraOrthographicMaxVec),
                                                            nearPlane, farPlane));

            // 创建最终的正交投影AABB
            lightCameraOrthographicMinVec = XMVectorSetZ(lightCameraOrthographicMinVec, nearPlane);
            lightCameraOrthographicMaxVec = XMVectorSetZ(lightCameraOrthographicMaxVec, farPlane);
            BoundingBox::CreateFromPoints(shadow_proj_bounding_box[cascadeIndex],
                                            lightCameraOrthographicMinVec, lightCameraOrthographicMaxVec);

            cascade_partitions_frustum[cascadeIndex] = frustumIntervalEnd;
        }
    }

    //--------------------------------------------------------------------------------------
    // 计算一个准确的近平面/远平面可以减少surface acne和Peter-panning
    // 通常偏移量会用于PCF滤波来解决阴影问题
    // 而准确的近平面/远平面可以提升精度
    // 这个概念并不复杂，但相交测试的代码比较复杂
    //--------------------------------------------------------------------------------------
    void XM_CALLCONV CascadedShadowManager::compute_near_far(float &out_near_plane, float &out_far_plane,
                                                                DirectX::FXMVECTOR light_camera_orthographic_min_vec,
                                                                DirectX::FXMVECTOR light_camera_orthographic_max_vec,
                                                                DirectX::XMVECTOR *points_in_camera_view)
    {
        using namespace DirectX;

        // 核心思想
        // 1. 对AABB的所有12个三角形进行迭代
        // 2. 每个三角形分别对正交投影的4个侧面进行裁剪。裁剪过程中可能会出现这些情况：
        //    - 0个点在该侧面的内部，该三角形可以剔除
        //    - 1个点在该侧面的内部，计算该点与另外两个点在侧面上的交点得到新三角形
        //    - 2个点在该侧面的内部，计算这两个点与另一个点在侧面上的交点，分裂得到2个新三角形
        //    - 3个点都在该侧面的内部
        //    遍历中的三角形与新生产的三角形都要进行剩余侧面的裁剪
        // 3. 在这些三角形中找到最小/最大的Z值作为近平面/远平面

        out_near_plane = FLT_MAX;
        out_far_plane = -FLT_MAX;
        Triangle triangleList[16]{};
        int numTriangles;

        //      4----5
        //     /|   /|
        //    0-+--1 |
        //    | 7--|-6
        //    |/   |/
        //    3----2
        static const int all_indices[][3] = {
            {4,7,6}, {6,5,4},
            {5,6,2}, {2,1,5},
            {1,2,3}, {3,0,1},
            {0,3,7}, {7,4,0},
            {7,3,2}, {2,6,7},
            {0,4,5}, {5,1,0}
        };
        bool triPointPassCollision[3]{};
        const float minX = XMVectorGetX(light_camera_orthographic_min_vec);
        const float maxX = XMVectorGetX(light_camera_orthographic_max_vec);
        const float minY = XMVectorGetY(light_camera_orthographic_min_vec);
        const float maxY = XMVectorGetY(light_camera_orthographic_max_vec);

        for (auto& indices : all_indices)
        {
            triangleList[0].points[0] = points_in_camera_view[indices[0]];
            triangleList[0].points[1] = points_in_camera_view[indices[1]];
            triangleList[0].points[2] = points_in_camera_view[indices[2]];
            numTriangles = 1;
            triangleList[0].is_culled = false;

            // 每个三角形都需要对4个视锥体侧面进行裁剪
            for (size_t planeIdx = 0; planeIdx < 4; ++planeIdx)
            {
                float edge;
                int component;
                switch (planeIdx)
                {
                    case 0: edge = minX; component = 0; break;
                    case 1: edge = maxX; component = 0; break;
                    case 2: edge = minY; component = 1; break;
                    case 3: edge = maxY; component = 1; break;
                    default: break;
                }

                for (size_t triIdx = 0; triIdx < numTriangles; ++triIdx)
                {
                    // 跳过裁剪的三角形
                    if (triangleList[triIdx].is_culled)
                        continue;

                    int insideVertexCount = 0;

                    for (size_t triVtxIdx = 0; triVtxIdx < 3; ++triVtxIdx)
                    {
                        switch (planeIdx)
                        {
                            case 0: triPointPassCollision[triVtxIdx] = (XMVectorGetX(triangleList[triIdx].points[triVtxIdx]) > minX); break;
                            case 1: triPointPassCollision[triVtxIdx] = (XMVectorGetX(triangleList[triIdx].points[triVtxIdx]) < maxX); break;
                            case 2: triPointPassCollision[triVtxIdx] = (XMVectorGetY(triangleList[triIdx].points[triVtxIdx]) > minY); break;
                            case 3: triPointPassCollision[triVtxIdx] = (XMVectorGetY(triangleList[triIdx].points[triVtxIdx]) < maxY); break;
                            default: break;
                        }
                        insideVertexCount += triPointPassCollision[triVtxIdx];
                    }

                    // 将通过视锥体测试的点挪到数组前面
                    if (triPointPassCollision[1] && !triPointPassCollision[0])
                    {
                        std::swap(triangleList[triIdx].points[0], triangleList[triIdx].points[1]);
                        triPointPassCollision[0] = true;
                        triPointPassCollision[1] = false;
                    }
                    if (triPointPassCollision[2] && !triPointPassCollision[1])
                    {
                        std::swap(triangleList[triIdx].points[1], triangleList[triIdx].points[2]);
                        triPointPassCollision[1] = true;
                        triPointPassCollision[2] = false;
                    }
                    if (triPointPassCollision[1] && !triPointPassCollision[0])
                    {
                        std::swap(triangleList[triIdx].points[0], triangleList[triIdx].points[1]);
                        triPointPassCollision[0] = true;
                        triPointPassCollision[1] = false;
                    }

                    // 裁剪测试
                    triangleList[triIdx].is_culled = (insideVertexCount == 0);
                    if (insideVertexCount == 1)
                    {
                        // 找出三角形与当前平面相交的另外两个点
                        XMVECTOR v0v1Vec = triangleList[triIdx].points[1] - triangleList[triIdx].points[0];
                        XMVECTOR v0v2Vec = triangleList[triIdx].points[2] - triangleList[triIdx].points[0];

                        float hitPointRatio = edge - XMVectorGetByIndex(triangleList[triIdx].points[0], component);
                        float distAlong_v0v1 = hitPointRatio / XMVectorGetByIndex(v0v1Vec, component);
                        float distAlong_v0v2 = hitPointRatio / XMVectorGetByIndex(v0v2Vec, component);
                        v0v1Vec = distAlong_v0v1 * v0v1Vec + triangleList[triIdx].points[0];
                        v0v2Vec = distAlong_v0v2 * v0v2Vec + triangleList[triIdx].points[0];

                        triangleList[triIdx].points[1] = v0v2Vec;
                        triangleList[triIdx].points[2] = v0v1Vec;
                    }
                    else if (insideVertexCount == 2)
                    {
                        // 裁剪后需要分开成两个三角形

                        // 把当前三角形后面的三角形(如果存在的话)复制出来，这样
                        // 可以用算出来的新三角形覆盖它
                        triangleList[numTriangles] = triangleList[triIdx + 1];
                        triangleList[triIdx + 1].is_culled = false;

                        // 找出三角形与当前平面相交的另外两个点
                        XMVECTOR v2v0Vec = triangleList[triIdx].points[0] - triangleList[triIdx].points[2];
                        XMVECTOR v2v1Vec = triangleList[triIdx].points[1] - triangleList[triIdx].points[2];

                        float hitPointRatio = edge - XMVectorGetByIndex(triangleList[triIdx].points[2], component);
                        float distAlong_v2v0 = hitPointRatio / XMVectorGetByIndex(v2v0Vec, component);
                        float distAlong_v2v1 = hitPointRatio / XMVectorGetByIndex(v2v1Vec, component);
                        v2v0Vec = distAlong_v2v0 * v2v0Vec + triangleList[triIdx].points[2];
                        v2v1Vec = distAlong_v2v1 * v2v1Vec + triangleList[triIdx].points[2];

                        // 添加三角形
                        triangleList[triIdx + 1].points[0] = triangleList[triIdx].points[0];
                        triangleList[triIdx + 1].points[1] = triangleList[triIdx].points[1];
                        triangleList[triIdx + 1].points[2] = v2v0Vec;

                        triangleList[triIdx].points[0] = triangleList[triIdx + 1].points[1];
                        triangleList[triIdx].points[1] = triangleList[triIdx + 1].points[2];
                        triangleList[triIdx].points[2] = v2v1Vec;

                        // 添加三角形数目，跳过我们刚插入的三角形
                        ++numTriangles;
                        ++triIdx;
                    }
                }
            }

            for (size_t triIdx = 0; triIdx < numTriangles; ++triIdx)
            {
                if (!triangleList[triIdx].is_culled)
                {
                    for (size_t vtxIdx = 0; vtxIdx < 3; ++vtxIdx)
                    {
                        float z = XMVectorGetZ(triangleList[triIdx].points[vtxIdx]);

                        out_near_plane = std::min(out_near_plane, z);
                        out_far_plane = std::max(out_far_plane, z);
                    }
                }
            }
        }
    }
}