//
// Created by ZHIKANG on 2023/5/27.
//

#pragma once

#include <Toy/Renderer/effect_helper.h>
#include <Toy/Renderer/constant_data.h>
#include <Toy/Renderer/effect_interface.h>

namespace toy
{
    class BasicEffect : public IEffect, public IEffectTransform, public IEffectMaterial, public IEffectMeshData
    {
    public:
        BasicEffect();
        ~BasicEffect() override = default;

        BasicEffect(BasicEffect&& other) noexcept;
        BasicEffect& operator=(BasicEffect&& other) noexcept;

        // Init all resources
        void init(ID3D11Device* device);

        // Transform
        void XM_CALLCONV set_world_matrix(DirectX::FXMMATRIX world) override;
        void XM_CALLCONV set_view_matrix(DirectX::FXMMATRIX view) override;
        void XM_CALLCONV set_proj_matrix(DirectX::FXMMATRIX proj) override;

        // Material
        void set_material(const model::Material& material) override;

        // Mesh data
        MeshDataInput get_input_data(const model::MeshData& mesh_data) override;

        // Draw by default state
        void set_default_render();

        void set_texture_cube(ID3D11ShaderResourceView* texture_cube);

        // Light
        void set_dir_light(uint32_t pos, const DirectionalLight& dir_light);
        void set_point_light(uint32_t pos, const PointLight& point_light);

        void set_eye_pos(const DirectX::XMFLOAT3& eye_pos);

        void set_reflection_enabled(bool enable);
        void set_refraction_enabled(bool enable);
        void set_refraction_eta(float eta);     // Air/medium refractive ratio

        // Update and bind constant buffers
        void apply(ID3D11DeviceContext * device_context) override;

    private:
        struct effect_impl
        {
            effect_impl() = default;
            ~effect_impl() = default;

            std::unique_ptr<EffectHelper> m_effect_helper;
            std::shared_ptr<IEffectPass> m_curr_effect_pass;
            com_ptr<ID3D11InputLayout> m_vertex_pos_normal_tex_layout;
            com_ptr<ID3D11InputLayout> m_curr_input_layout;

            D3D11_PRIMITIVE_TOPOLOGY m_curr_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            DirectX::XMFLOAT4X4 m_world{};
            DirectX::XMFLOAT4X4 m_view{};
            DirectX::XMFLOAT4X4 m_proj{};
        };

        std::unique_ptr<effect_impl> m_effect_impl;
    };

    class SkyboxEffect : public IEffect, public IEffectTransform, public IEffectMaterial, public IEffectMeshData
    {
    public:
        SkyboxEffect();
        ~SkyboxEffect() override = default;

        SkyboxEffect(SkyboxEffect&& other) noexcept;
        SkyboxEffect& operator=(SkyboxEffect&& other) noexcept;

        void init(ID3D11Device* device);

        // Useless
        void XM_CALLCONV set_world_matrix(DirectX::FXMMATRIX world) override;
        void XM_CALLCONV set_view_matrix(DirectX::FXMMATRIX view) override;
        void XM_CALLCONV set_proj_matrix(DirectX::FXMMATRIX proj) override;

        // Material
        void set_material(const model::Material& material) override;

        // Mesh data
        MeshDataInput get_input_data(const model::MeshData& mesh_data) override;

        // Draw by default state
        void set_default_render();

        // Set depth resource view
        void set_depth_texture(ID3D11ShaderResourceView* depth_texture);
        // Set scene rendering resource view
        void set_lit_texture(ID3D11ShaderResourceView* lit_texture);
        // Set TBDR scene rendering resource view
        void set_flat_lit_texture(ID3D11ShaderResourceView* flat_lit_texture, uint32_t width, uint32_t height);

        // Set MSAA level
        void set_msaa_samples(uint32_t msaa_samples);

        // Update and bind constant buffers
        void apply(ID3D11DeviceContext * device_context) override;

    private:
        struct effect_impl
        {
            effect_impl()
            {
                DirectX::XMStoreFloat4x4(&m_view, DirectX::XMMatrixIdentity());
                DirectX::XMStoreFloat4x4(&m_proj, DirectX::XMMatrixIdentity());
            }
            ~effect_impl() = default;


            std::unique_ptr<EffectHelper> m_effect_helper;

            std::shared_ptr<IEffectPass> m_curr_effect_pass;
            com_ptr<ID3D11InputLayout> m_curr_input_layout;
            com_ptr<ID3D11InputLayout> m_vertex_pos_normal_tex_layout;

            DirectX::XMFLOAT4X4 m_view, m_proj;
            D3D11_PRIMITIVE_TOPOLOGY m_curr_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            uint32_t m_msaa_levels = 1;
        };
        std::unique_ptr<effect_impl> m_effect_impl;
    };

    class PostProcessEffect
    {
    public:
        PostProcessEffect();
        ~PostProcessEffect() = default;

        PostProcessEffect(PostProcessEffect&& other) noexcept;
        PostProcessEffect& operator=(PostProcessEffect&& other) noexcept;

        // Initialize all resources
        void init(ID3D11Device* device);

        // Multiply the components of two images
        // If input2 is nullptr, input1 will pass through directly
        void render_composite(ID3D11DeviceContext* device_context, ID3D11ShaderResourceView* input1,
                                ID3D11ShaderResourceView* input2, ID3D11RenderTargetView* output, const D3D11_VIEWPORT& viewport);

        // Sobel filter
        void compute_sobel(ID3D11DeviceContext* device_context, ID3D11ShaderResourceView* input, ID3D11UnorderedAccessView* output,
                            uint32_t width, uint32_t height);

        // Gaussian filter
        // Size: odd number, 3-19
        void set_blur_kernel_size(int32_t size);
        void set_blur_sigma(float sigma);

        void compute_gaussian_blur_x(ID3D11DeviceContext* device_context, ID3D11ShaderResourceView* input, ID3D11UnorderedAccessView* output,
                                        uint32_t width, uint32_t height);
        void compute_gaussian_blur_y(ID3D11DeviceContext* device_context, ID3D11ShaderResourceView* input, ID3D11UnorderedAccessView* output,
                                        uint32_t width, uint32_t height);

    private:
        struct effect_impl
        {
            effect_impl() = default;
            ~effect_impl() = default;

            std::unique_ptr<EffectHelper> m_effect_helper;

            std::vector<float> m_weights;
            int32_t m_blur_radius = 3;
            float m_blur_sigma = 1.0f;
        };

        std::unique_ptr<effect_impl> m_effect_impl;
    };

    // Forward effect
    class ForwardEffect : public IEffect, public IEffectTransform, public IEffectMaterial, public IEffectMeshData
    {
    public:
        ForwardEffect();
        ~ForwardEffect() override = default;

        ForwardEffect(ForwardEffect&& other) noexcept;
        ForwardEffect& operator=(ForwardEffect&& other) noexcept;

        // Initialize all resources
        void init(ID3D11Device* device);

        // MVP matrix
        void XM_CALLCONV set_world_matrix(DirectX::FXMMATRIX world) override;
        void XM_CALLCONV set_view_matrix(DirectX::FXMMATRIX view) override;
        void XM_CALLCONV set_proj_matrix(DirectX::FXMMATRIX proj) override;

        // Material
        void set_material(const model::Material& material) override;

        // Mesh data
        MeshDataInput get_input_data(const model::MeshData& mesh_data) override;

        // Forward effect
        void set_msaa_samples(uint32_t msaa_samples);

        void set_light_buffer(ID3D11ShaderResourceView* light_buffer);
        void set_tile_buffer(ID3D11ShaderResourceView* tile_buffer);
        void set_camera_near_far(float nearz, float farz);

        void set_lighting_only(bool enable);
        void set_face_normals(bool enable);
        void set_visualize_light_count(bool enable);

        // Default render
        void set_default_render();

        // Pre-Z pass render
        void set_pre_z_pass_render();

        // Tile-based lighting culling
        void compute_tiled_light_culling(ID3D11DeviceContext* device_context, ID3D11UnorderedAccessView* tile_info_buffer_uav,
                                            ID3D11ShaderResourceView* light_buffer_srv, ID3D11ShaderResourceView* depth_buffer_srv);

        // Render based on tile-based lighting culling
        void set_tiled_light_culling_render();

        // Constant buffers and resources change
        void apply(ID3D11DeviceContext* device_context) override;

    private:
        struct effect_impl
        {
            effect_impl() = default;
            ~effect_impl() = default;

            std::unique_ptr<EffectHelper> m_effect_helper;
            std::shared_ptr<IEffectPass> m_cur_effect_pass;
            com_ptr<ID3D11InputLayout> m_cur_input_layout;
            com_ptr<ID3D11InputLayout> m_vertex_pos_normal_tex_layout;
            D3D11_PRIMITIVE_TOPOLOGY  m_topology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;

            DirectX::XMFLOAT4X4 m_world{};
            DirectX::XMFLOAT4X4 m_view{};
            DirectX::XMFLOAT4X4 m_proj{};

            uint32_t m_msaa_samples = 1;
        };

        std::unique_ptr<effect_impl> m_effect_impl;
    };

    // Deferred effect
    class DeferredEffect : public IEffect, public IEffectTransform, public IEffectMaterial, public IEffectMeshData
    {
    public:
        DeferredEffect();
        ~DeferredEffect() override = default;

        DeferredEffect(DeferredEffect&& other) noexcept;
        DeferredEffect& operator=(DeferredEffect&& other) noexcept;

        // Initialize all resources
        void init(ID3D11Device* device);

        // MVP matrix
        void XM_CALLCONV set_world_matrix(DirectX::FXMMATRIX world) override;
        void XM_CALLCONV set_view_matrix(DirectX::FXMMATRIX view) override;
        void XM_CALLCONV set_proj_matrix(DirectX::FXMMATRIX proj) override;

        // Material
        void set_material(const model::Material& material) override;

        // Mesh data
        MeshDataInput get_input_data(const model::MeshData& mesh_data) override;

        // Deferred effect
        void set_msaa_samples(uint32_t msaa_samples);

        void set_lighting_only(bool enable);
        void set_face_normals(bool enable);
        void set_visualize_light_count(bool enable);
        void set_visualize_shading_freq(bool enable);

        void set_camera_near_far(float nearz, float farz);

        // Render G-Buffer
        void set_gbuffer_render();

        // Render normal of G-Buffer to target texture
        void debug_normal_gbuffer(ID3D11DeviceContext* device_context,
                                    ID3D11RenderTargetView* rtv,
                                    ID3D11ShaderResourceView* normal_gbuffer,
                                    D3D11_VIEWPORT viewport);

        // Render depth gradient of G-Buffer to target texture
        void debug_pos_z_grad_gbuffer(ID3D11DeviceContext* device_context,
                                        ID3D11RenderTargetView* rtv,
                                        ID3D11ShaderResourceView* pos_z_grad_gbuffer,
                                        D3D11_VIEWPORT viewport);

        // Traditional deferred render
        void compute_lighting_default(ID3D11DeviceContext* device_context,
                                        ID3D11RenderTargetView* lit_buffer_rtv,
                                        ID3D11DepthStencilView* depth_buffer_read_only_dsv,
                                        ID3D11ShaderResourceView* light_buffer_srv,
                                        ID3D11ShaderResourceView* gbuffers[4],
                                        D3D11_VIEWPORT viewport);

        // Perform tile-based lighting culling
        void compute_tiled_light_culling(ID3D11DeviceContext* device_context,
                                            ID3D11UnorderedAccessView* lit_flat_buffer_uav,
                                            ID3D11ShaderResourceView* light_buffer_srv,
                                            ID3D11ShaderResourceView* gbuffers[4]);

        // Apply constant buffers and resources
        void apply(ID3D11DeviceContext* device_context) override;

    private:
        struct effect_impl
        {
            effect_impl() = default;
            ~effect_impl() = default;

            std::unique_ptr<EffectHelper> m_effect_helper;
            std::shared_ptr<IEffectPass> m_cur_effect_pass;
            com_ptr<ID3D11InputLayout> m_cur_input_layout;
            D3D11_PRIMITIVE_TOPOLOGY m_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

            com_ptr<ID3D11InputLayout> m_vertex_pos_normal_tex_layout;
            DirectX::XMFLOAT4X4 m_world{};
            DirectX::XMFLOAT4X4 m_view{};
            DirectX::XMFLOAT4X4 m_proj{};
            uint32_t m_msaa_samples = 1;
        };

        std::unique_ptr<effect_impl> m_effect_impl;
    };
}







































