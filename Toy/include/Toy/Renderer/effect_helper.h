//
// Created by ZHIKANG on 2023/5/27.
//

#pragma once

#include <Toy/Core/base.h>
#include <Toy/Renderer/misc.h>

namespace toy
{
    // Render pass description
    // Set the shader by specifying shader name
    struct effect_pass_desc_s
    {
        std::string_view nameVS;
        std::string_view nameDS;
        std::string_view nameHS;
        std::string_view nameGS;
        std::string_view namePS;
        std::string_view nameCS;
    };
    using EffectPassDesc = effect_pass_desc_s;

    class effect_helper_c;

    // Render pass
    struct effect_pass_interface_s
    {
        // Set rasterizer state
        virtual void set_rasterizer_state(ID3D11RasterizerState* rs_state) = 0;
        // Set blend state
        virtual void set_blend_state(ID3D11BlendState* bs_state, const float blend_factor[4], uint32_t sample_mask) = 0;
        // Set depth blend state
        virtual void set_depth_stencil_state(ID3D11DepthStencilState* ds_state, uint32_t stencil_value) = 0;

        // Obtain uniform argument of vertex shader
        virtual std::shared_ptr<IEffectConstantBufferVariable> get_vs_param_by_name(std::string_view param_name) = 0;
        // Obtain uniform argument of domain shader
        virtual std::shared_ptr<IEffectConstantBufferVariable> get_ds_param_by_name(std::string_view param_name) = 0;
        // Obtain uniform argument of hull shader
        virtual std::shared_ptr<IEffectConstantBufferVariable> get_hs_param_by_name(std::string_view param_name) = 0;
        // Obtain uniform argument of geometry shader
        virtual std::shared_ptr<IEffectConstantBufferVariable> get_gs_param_by_name(std::string_view param_name) = 0;
        // Obtain uniform argument of pixel shader
        virtual std::shared_ptr<IEffectConstantBufferVariable> get_ps_param_by_name(std::string_view param_name) = 0;
        // Obtain uniform argument of compute shader
        virtual std::shared_ptr<IEffectConstantBufferVariable> get_cs_param_by_name(std::string_view param_name) = 0;

        // Obtain effect helper
        virtual effect_helper_c* get_effect_helper() = 0;
        // Obtain effect name
        virtual const std::string& get_pass_name() = 0;

        // Apply shaders, constant buffers, sampler, shader resources and read-write resources to pipeline
        virtual void apply(ID3D11DeviceContext* device_context) = 0;

        // Dispatch compute shader
        virtual void dispatch(ID3D11DeviceContext* device_context, uint32_t thread_x, uint32_t thread_y, uint32_t thread_z) = 0;

        virtual ~effect_pass_interface_s() = default;
    };
    using IEffectPass = effect_pass_interface_s;

    struct EffectPass : public IEffectPass
    {
        EffectPass(
            effect_helper_c* _pEffectHelper,
            std::string_view _passName,
            std::unordered_map<uint32_t, CBufferData>& _cBuffers,
            std::unordered_map<uint32_t, ShaderResource>& _shaderResources,
            std::unordered_map<uint32_t, SamplerState>& _samplers,
            std::unordered_map<uint32_t, RWResource>& _rwResources)
            : pEffectHelper(_pEffectHelper), passName(_passName), cBuffers(_cBuffers), shaderResources(_shaderResources),
                samplers(_samplers), rwResources(_rwResources)
        {
        }
        ~EffectPass() override = default;

        void set_rasterizer_state(ID3D11RasterizerState* pRS) override;
        void set_blend_state(ID3D11BlendState* pBS, const float blendFactor[4], uint32_t sampleMask) override;
        void set_depth_stencil_state(ID3D11DepthStencilState* pDSS, uint32_t stencilRef) override;
        std::shared_ptr<IEffectConstantBufferVariable> get_vs_param_by_name(std::string_view paramName) override;
        std::shared_ptr<IEffectConstantBufferVariable> get_ds_param_by_name(std::string_view paramName) override;
        std::shared_ptr<IEffectConstantBufferVariable> get_hs_param_by_name(std::string_view paramName) override;
        std::shared_ptr<IEffectConstantBufferVariable> get_gs_param_by_name(std::string_view paramName) override;
        std::shared_ptr<IEffectConstantBufferVariable> get_ps_param_by_name(std::string_view paramName) override;
        std::shared_ptr<IEffectConstantBufferVariable> get_cs_param_by_name(std::string_view paramName) override;
        effect_helper_c* get_effect_helper() override;
        const std::string& get_pass_name() override;

        void apply(ID3D11DeviceContext * deviceContext) override;

        void dispatch(ID3D11DeviceContext* deviceContext, uint32_t threadX, uint32_t threadY, uint32_t threadZ) final;

        effect_helper_c* pEffectHelper = nullptr;
        std::string passName;

        // Render state
        com_ptr<ID3D11BlendState> pBlendState = nullptr;
        float blendFactor[4] = {};
        uint32_t sampleMask = 0xFFFFFFFF;

        com_ptr<ID3D11RasterizerState> pRasterizerState = nullptr;

        com_ptr<ID3D11DepthStencilState> pDepthStencilState = nullptr;
        uint32_t stencilRef = 0;

        // Shader info
        std::shared_ptr<VertexShaderInfo> pVSInfo = nullptr;
        std::shared_ptr<HullShaderInfo> pHSInfo = nullptr;
        std::shared_ptr<DomainShaderInfo> pDSInfo = nullptr;
        std::shared_ptr<GeometryShaderInfo> pGSInfo = nullptr;
        std::shared_ptr<PixelShaderInfo> pPSInfo = nullptr;
        std::shared_ptr<ComputeShaderInfo> pCSInfo = nullptr;

        // Shader argument constant buffer data(do not create constant buffer)
        std::unique_ptr<CBufferData> pVSParamData = nullptr;
        std::unique_ptr<CBufferData> pHSParamData = nullptr;
        std::unique_ptr<CBufferData> pDSParamData = nullptr;
        std::unique_ptr<CBufferData> pGSParamData = nullptr;
        std::unique_ptr<CBufferData> pPSParamData = nullptr;
        std::unique_ptr<CBufferData> pCSParamData = nullptr;

        // Resources and sampler state
        std::unordered_map<uint32_t, CBufferData>& cBuffers;
        std::unordered_map<uint32_t, ShaderResource>& shaderResources;
        std::unordered_map<uint32_t, SamplerState>& samplers;
        std::unordered_map<uint32_t, RWResource>& rwResources;
    };

    // Effect helper
    // Manage shader, sampler, resource, constant buffer, shader argument, read-write resource and render state
    class effect_helper_c
    {
    public:
        effect_helper_c();
        ~effect_helper_c();

        // Disable copy, allow move
        effect_helper_c(const effect_helper_c&) = delete;
        effect_helper_c& operator=(const effect_helper_c&) = delete;
        effect_helper_c(effect_helper_c&&) = default;
        effect_helper_c& operator=(effect_helper_c&&) = default;

        // Set compiled shader file cache path and create it
        // If set to "", turn off cache
        // If force_write is true, it will overwrite the save every time when the program is running
        // By default, compiled shaders will not be cached
        // When the shader has not been modified, force_write should be enabled
        void set_binary_cache_directory(std::wstring_view cache_dir, bool force_write = false);

        // Compile shader or read shader binary codes
        HRESULT create_shader_from_file(std::string_view shader_name, std::wstring_view file_name, ID3D11Device* device,
                                        const char* entry_point = nullptr, const char* shader_model = nullptr, const D3D_SHADER_MACRO* p_defines = nullptr, ID3DBlob** pp_shader_byte_code = nullptr);

        // Only compile shader
        static HRESULT compile_shader_from_file(std::wstring_view file_name, const char* entry_point, const char* shader_model, ID3DBlob** pp_shader_byte_code, ID3DBlob** pp_error_blob = nullptr,
                                                const D3D_SHADER_MACRO* p_defines = nullptr, ID3DInclude* p_include = D3D_COMPILE_STANDARD_FILE_INCLUDE);

        // Add compiled shader binary information and set an identifier for it
        // Will not save the shader binary encoding to a file
        HRESULT add_shader(std::string_view name, ID3D11Device* device, ID3DBlob* blob);

        // Add geometry shader with stream output and set an identifier for it
        // Will not save the shader binary encoding to a file
        HRESULT add_geometry_shader_with_stream_output(std::string_view name, ID3D11Device* device, ID3D11GeometryShader* gs_with_so, ID3DBlob* blob);

        // Clear all
        void clear();

        // Create render pass
        HRESULT add_effect_pass(std::string_view effect_pass_name, ID3D11Device* device, const EffectPassDesc* p_effect_desc);

        // Obtain specific render pass
        std::shared_ptr<IEffectPass> get_effect_pass(std::string_view effect_pass_name);

        // Obtain constant buffer and set value
        std::shared_ptr<IEffectConstantBufferVariable> get_constant_buffer_variable(std::string_view name);

        // Set sampler state by slot
        void set_sampler_state_by_slot(uint32_t slot, ID3D11SamplerState* sampler_state);
        // Set sampler state by name
        void set_sampler_state_by_name(std::string_view name, ID3D11SamplerState* sampler_state);
        // Obtain sampler state slot, return -1 if no found
        int32_t map_sampler_state_slot(std::string_view name);

        // Set shader resource view by slot
        void set_shader_resource_by_slot(uint32_t slot, ID3D11ShaderResourceView* srv);
        // Set shader resource view by name
        void set_shader_resource_by_name(std::string_view name, ID3D11ShaderResourceView* srv);
        // Obtain shader resource view slot, return -1 if no found
        int32_t map_shader_resource_slot(std::string_view name);

        // Set readable and writable resource by slot
        void set_unordered_access_by_slot(uint32_t slot, ID3D11UnorderedAccessView* uav, uint32_t* p_initial_count = nullptr);
        // Set readable and writable resource by name
        void set_unordered_access_by_name(std::string_view name, ID3D11UnorderedAccessView* uav, uint32_t* p_initial_count = nullptr);
        // Obtain readable and writable resource
        int32_t map_unordered_access_slot(std::string_view name);

    private:
        struct EffectHelperImpl;

        std::unique_ptr<EffectHelperImpl> p_impl_ = nullptr;
    };

    using EffectHelper = effect_helper_c;
}











