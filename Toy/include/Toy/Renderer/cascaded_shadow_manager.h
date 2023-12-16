//
// Created by ZZK on 2023/10/15.
//

#pragma once

#include <Toy/Core/base.h>
#include <Toy/Scene/camera.h>
#include <Toy/Renderer/texture_2d.h>
#include <Toy/Renderer/cascaded_shadow_defines.h>

namespace toy
{
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

        static CascadedShadowManager &get();

        void update_frame(const camera_c &viewer_camera, const camera_c &light_camera, const DirectX::BoundingBox &scene_bounding_box);

        [[nodiscard]] ID3D11RenderTargetView *get_cascade_render_target_view(size_t cascade_index) const;
        [[nodiscard]] ID3D11ShaderResourceView *get_cascades_output() const;
        [[nodiscard]] ID3D11ShaderResourceView *get_cascade_output(size_t cascade_index) const;

        [[nodiscard]] ID3D11DepthStencilView *get_depth_buffer_dsv() const;
        [[nodiscard]] ID3D11ShaderResourceView *get_depth_buffer_srv() const;

        [[nodiscard]] ID3D11RenderTargetView *get_temp_texture_rtv() const;
        [[nodiscard]] ID3D11ShaderResourceView *get_temp_texture_output() const;

        [[nodiscard]] std::span<float> get_cascade_partitions();
        void get_cascade_partitions(float *output) const;

        [[nodiscard]] DirectX::XMMATRIX get_shadow_project_xm(size_t cascade_index) const;
        [[nodiscard]] const DirectX::BoundingBox &get_shadow_aabb(size_t cascade_index) const;
        [[nodiscard]] DirectX::BoundingOrientedBox get_shadow_obb(size_t cascade_index) const;

        [[nodiscard]] D3D11_VIEWPORT get_shadow_viewport() const;

        void XM_CALLCONV compute_near_far(float &out_near_plane, float &out_far_plane,
                                            DirectX::FXMVECTOR light_camera_orthographic_min_vec,
                                            DirectX::FXMVECTOR light_camera_orthographic_max_vec,
                                            DirectX::XMVECTOR *points_in_camera_view);

    public:
        int32_t shadow_size = 1024;
        int32_t shadow_bits = 4;
        int32_t cascade_levels = 4;
        int32_t blur_kernel_size = 9;

        float gaussian_blur_sigma = 3.0f;
        float pcf_depth_bias = 0.001f;
        float blend_between_cascades_range = 0.2f;
        float light_bleeding_reduction = 0.8f;

        float magic_power = 160.0f;
        float positive_exponent = 5.0f;
        float negative_exponent = 5.0f;

        bool fixed_size_frustum_aabb = true;
        bool move_light_texel_size = true;
        bool generate_mips = false;

        CameraSelection selected_camera = CameraSelection::CameraSelection_Eye;
        FitProjection selected_cascades_fit = FitProjection::FitProjection_ToCascade;
        FitNearFar selected_near_far_fit = FitNearFar::FitNearFar_SceneAABB_Intersection;
        CascadeSelection selected_cascade_selection = CascadeSelection::CascadeSelection_Map;
        ShadowType shadow_type = ShadowType::ShadowType_EVSM4;

        D3D11_VIEWPORT shadow_viewport = {};
        std::array<float, 8> cascade_partitions_percentage = { 0.04f, 0.10f, 0.25f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
        std::array<float, 8> cascade_partitions_frustum = {};
        std::array<DirectX::XMFLOAT4X4, 8> shadow_proj = {};
        std::array<DirectX::BoundingBox, 8> shadow_proj_bounding_box = {};

        std::unique_ptr<Texture2DArray> csm_texture_array = nullptr;
        std::unique_ptr<Texture2D> csm_temp_texture = nullptr;
        std::unique_ptr<Depth2D> csm_depth_buffer = nullptr;
    };
}
