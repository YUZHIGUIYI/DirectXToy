//
// Created by ZZK on 2023/10/15.
//

#pragma once

#include <Toy/Core/base.h>
#include <Toy/Scene/camera.h>
#include <TOy/Renderer/texture_2d.h>

namespace toy
{
    enum class CascadeSelection
    {
        CascadeSelection_Map,
        CascadeSelection_Interval
    };

    enum class CameraSelection
    {
        CameraSelection_Eye,
        CameraSelection_Light,
        CameraSelection_Cascade1,
        CameraSelection_Cascade2,
        CameraSelection_Cascade3,
        CameraSelection_Cascade4,
        CameraSelection_Cascade5,
        CameraSelection_Cascade6,
        CameraSelection_Cascade7,
        CameraSelection_Cascade8,
    };

    enum class FitNearFar
    {
        FitNearFar_ZeroOne,
        FitNearFar_CascadeAABB,
        FitNearFar_SceneAABB,
        FitNearFar_SceneAABB_Intersection
    };

    enum class FitProjection
    {
        FitProjection_ToCascade,
        FitProjection_ToScene
    };

    struct CascadedShadowManager
    {
    public:
        CascadedShadowManager();
        ~CascadedShadowManager();

        CascadedShadowManager(const CascadedShadowManager &) = delete;
        CascadedShadowManager& operator=(const CascadedShadowManager &) = delete;
        CascadedShadowManager(CascadedShadowManager &&) noexcept;
        CascadedShadowManager& operator=(CascadedShadowManager &&) noexcept;

        void init(ID3D11Device *device);

        void update_frame(const camera_c &viewer_camera, const camera_c &light_camera, const DirectX::BoundingBox &scene_bounding_box);

        ID3D11DepthStencilView *get_cascade_depth_stencil_view(size_t cascade_index) const;
        ID3D11ShaderResourceView *get_cascades_output() const;
        ID3D11ShaderResourceView *get_cascade_output(size_t cascade_index) const;

        const float *get_cascade_partitions() const;
        void get_cascade_partitions(float *output) const;

        DirectX::XMMATRIX get_shadow_project_xm(size_t cascade_index) const;
        const DirectX::BoundingBox &get_shadow_aabb(size_t cascade_index) const;
        DirectX::BoundingOrientedBox get_shadow_obb(size_t cascade_index) const;

        D3D11_VIEWPORT get_shadow_viewport() const;

        void XM_CALLCONV compute_near_far(float &out_near_plane, float &out_far_plane,
                                            DirectX::FXMVECTOR light_camera_orthographic_min_vec,
                                            DirectX::FXMVECTOR light_camera_orthographic_max_vec,
                                            DirectX::XMVECTOR *points_in_camera_view);

    public:
        int32_t shadow_size = 1024;
        int32_t cascade_levels = 4;
        int32_t pcf_kernel_size = 5;
        float pcf_depth_offset = 0.001f;
        float blend_between_cascades_range = 0.2f;
        bool derivative_based_offset = false;
        bool blend_between_cascades = true;
        bool fixed_size_frustum_aabb = true;
        bool move_light_texel_size = true;

        CameraSelection selected_camera = CameraSelection::CameraSelection_Eye;
        FitProjection selected_cascades_fit = FitProjection::FitProjection_ToCascade;
        FitNearFar selected_near_far_fit = FitNearFar::FitNearFar_SceneAABB_Intersection;
        CascadeSelection selected_cascade_selection = CascadeSelection::CascadeSelection_Map;

        D3D11_VIEWPORT shadow_viewport = {};
        std::array<float, 8> cascade_partitions_percentage = { 0.04f, 0.10f, 0.25f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
        std::array<float, 8> cascade_partitions_frustum = {};
        std::array<DirectX::XMFLOAT4X4, 8> shadow_proj = {};
        std::array<DirectX::BoundingBox, 8> shadow_proj_bounding_box = {};

        std::unique_ptr<Depth2DArray> csm_texture_array = nullptr;
    };
}
