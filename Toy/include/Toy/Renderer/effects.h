//
// Created by ZHIKANG on 2023/5/27.
//

#pragma once

#include <Toy/Renderer/effect_helper.h>
#include <Toy/Renderer/constant_data.h>
#include <Toy/Renderer/effect_interface.h>

namespace toy
{
    class BasicEffect final : public IEffect, public IEffectTransform, public IEffectMaterial, public IEffectMeshData
    {
    public:
        BasicEffect();
        ~BasicEffect() override;

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

        // Singleton
        static BasicEffect &get();

    private:
        struct EffectImpl;

        std::unique_ptr<EffectImpl> m_effect_impl;
    };

    class SkyboxEffect final : public IEffect, public IEffectTransform, public IEffectMaterial, public IEffectMeshData
    {
    public:
        SkyboxEffect();
        ~SkyboxEffect() override;

        SkyboxEffect(SkyboxEffect&& other) noexcept;
        SkyboxEffect& operator=(SkyboxEffect&& other) noexcept;

        void init(ID3D11Device* device);

        // * Currently useless
        void XM_CALLCONV set_world_matrix(DirectX::FXMMATRIX world) override;
        void XM_CALLCONV set_view_matrix(DirectX::FXMMATRIX view) override;
        void XM_CALLCONV set_proj_matrix(DirectX::FXMMATRIX proj) override;

        // Material
        void set_material(const model::Material& material) override;

        // Mesh data
        MeshDataInput get_input_data(const model::MeshData& mesh_data) override;

        // Draw by default state
        void set_default_render();

        // Set texture cube
        void set_texture_cube(ID3D11ShaderResourceView* skybox_texture);
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

        // Singleton
        static SkyboxEffect &get();

    private:
        struct EffectImpl;

        std::unique_ptr<EffectImpl> m_effect_impl;
    };

    class PostProcessEffect
    {
    public:
        PostProcessEffect();
        ~PostProcessEffect();

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

        // Singleton
        static PostProcessEffect &get();

    private:
        struct EffectImpl;

        std::unique_ptr<EffectImpl> m_effect_impl;
    };

    // Forward effect
    class ForwardEffect final : public IEffect, public IEffectTransform, public IEffectMaterial, public IEffectMeshData
    {
    public:
        ForwardEffect();
        ~ForwardEffect() override;

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

        // Singleton
        static ForwardEffect &get();

    private:
        struct EffectImpl;

        std::unique_ptr<EffectImpl> m_effect_impl;
    };

    // Deferred effect
    class DeferredEffect final : public IEffect, public IEffectTransform, public IEffectMaterial, public IEffectMeshData
    {
    public:
        DeferredEffect();
        ~DeferredEffect() override;

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

        // Singleton
        static DeferredEffect &get();

    private:
        struct EffectImpl;

        std::unique_ptr<EffectImpl> m_effect_impl;
    };

    // FXAA effect - Post processing
    class FXAAEffect
    {
    public:
        FXAAEffect();
        ~FXAAEffect();

        FXAAEffect(FXAAEffect&& other) noexcept;
        FXAAEffect& operator=(FXAAEffect&& other) noexcept;

        // Initialize all resources
        void init(ID3D11Device* device);

        // FXAA Quality
        // major = 1, low quality, minor = 0 ... 5
        // major = 2, middle quality, minor = 0 ... 9
        // major = 3, high quality, minor = 9
        void set_quality(int32_t major, int32_t minor);
        void set_quality_sub_pix(float value);
        void set_quality_edge_threshold(float threshold);
        void set_quality_edge_threshold_min(float min_threshold);

        void enable_debug(bool enabled);

        void render_fxaa(ID3D11DeviceContext* device_context, ID3D11ShaderResourceView* input_srv, ID3D11RenderTargetView* output_rtv, const D3D11_VIEWPORT& viewport);

        // Singleton
        static FXAAEffect &get();

    private:
        struct EffectImpl;

        std::unique_ptr<EffectImpl> m_effect_impl;
    };

    // PBR pre-processing effect
    class PreProcessEffect
    {
    public:
        PreProcessEffect();
        ~PreProcessEffect();

        PreProcessEffect(PreProcessEffect&& other) noexcept;
        PreProcessEffect& operator=(PreProcessEffect&& other) noexcept;

        // * Initialize all resources
        void init(ID3D11Device* device);

        // * Convert HDR image to cube map
        void compute_cubemap(ID3D11Device *device, ID3D11DeviceContext *device_context, std::string_view file_path);

        void compute_sp_env_map(ID3D11Device *device, ID3D11DeviceContext *device_context);

        void compute_irradiance_map(ID3D11Device *device, ID3D11DeviceContext *device_context);

        void compute_brdf_lut(ID3D11Device *device, ID3D11DeviceContext *device_context);

        // * Get environment map shader resource view
        ID3D11ShaderResourceView* get_environment_srv() const;

        // * Get irradiance map shader resource view
        ID3D11ShaderResourceView* get_irradiance_srv() const;

        // * Get BRDF shader resource view
        ID3D11ShaderResourceView* get_brdf_srv() const;

        // * Check whether pre-process has been completed
        bool is_ready() const;

        // * Singleton
        static PreProcessEffect& get();

    private:
        struct EffectImpl;

        std::unique_ptr<EffectImpl> m_effect_impl;
    };

    // Deferred PBR effect
    class DeferredPBREffect final : public IEffect, public IEffectTransform, public IEffectMaterial, public IEffectMeshData
    {
    public:
        DeferredPBREffect();
        ~DeferredPBREffect() override;

        DeferredPBREffect(DeferredPBREffect&& other) noexcept;
        DeferredPBREffect& operator=(DeferredPBREffect&& other) noexcept;

        // * Initialize all resources and shaders
        void init(ID3D11Device* device);

        // * Set material for geometry pass
        // * Note: called by render object automatically
        void set_material(const model::Material& material) override;

        // * Get mesh data
        // * Note: called by render object automatically
        MeshDataInput get_input_data(const model::MeshData& mesh_data) override;

        // * Set camera near and far parameters for geometry pass
        void set_camera_near_far(float nearz, float farz);

        // * Set camera world position
        void set_camera_position(DirectX::XMFLOAT3 camera_position);

        // * Render GBuffer for geometry pass - set vertex layout and skybox pass and topology
        void set_gbuffer_render();

        // * Apply constant buffers and resources for geometry pass
        // * Note: called by render object automatically
        void apply(ID3D11DeviceContext* device_context) override;

        // * Render to lit texture
        // * Note: default method of deferred lighting pass
        void deferred_lighting_pass(ID3D11DeviceContext* device_context, ID3D11RenderTargetView* lit_buffer_rtv,
                                    ID3D11ShaderResourceView** gbuffers, D3D11_VIEWPORT viewport);

        // * Singleton
        static DeferredPBREffect &get();

        // * Set MVP matrix
        void XM_CALLCONV set_world_matrix(DirectX::FXMMATRIX world) override;
        void XM_CALLCONV set_view_matrix(DirectX::FXMMATRIX view) override;
        void XM_CALLCONV set_proj_matrix(DirectX::FXMMATRIX proj) override;

    private:
        struct EffectImpl;

        std::unique_ptr<EffectImpl> m_effect_impl;
    };

    class SimpleSkyboxEffect final : public IEffect, public IEffectTransform, public IEffectMaterial, public IEffectMeshData
    {
    public:
        SimpleSkyboxEffect();
        ~SimpleSkyboxEffect() override;

        SimpleSkyboxEffect(SimpleSkyboxEffect&& other) noexcept;
        SimpleSkyboxEffect& operator=(SimpleSkyboxEffect&& other) noexcept;

        // * Initialize all resources and shaders
        void init(ID3D11Device* device);

        // * Set material for geometry pass
        // * Note: called by render object automatically
        void set_material(const model::Material& material) override;

        // * Get mesh data
        // * Note: called by render object automatically
        MeshDataInput get_input_data(const model::MeshData& mesh_data) override;

        // * Set vertex layout and skybox pass and topology
        void set_skybox_render();

        // * Apply constant buffers and resources for geometry pass
        // * Note: called by render object automatically
        void apply(ID3D11DeviceContext* device_context) override;

        // * Singleton
        static SimpleSkyboxEffect &get();

        // * Set MVP matrix
        void XM_CALLCONV set_world_matrix(DirectX::FXMMATRIX world) override;
        void XM_CALLCONV set_view_matrix(DirectX::FXMMATRIX view) override;
        void XM_CALLCONV set_proj_matrix(DirectX::FXMMATRIX proj) override;

    private:
        struct EffectImpl;

        std::unique_ptr<EffectImpl> m_effect_impl;
    };
}







































