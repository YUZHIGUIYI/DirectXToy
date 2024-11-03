//
// Created by ZZK on 2024/10/15.
//

#pragma once

#include <Toy/Core/base.h>
#include <utility>
#include <vector>
#include <span>
#include <unordered_map>
#include <variant>
#include <array>

#include <wrl/client.h>

#include <d3d11.h>
#include <dxgi.h>
#include <Inc/dxcapi.h>
#include <Inc/d3d12shader.h>

template <typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

namespace toy
{
	// Shader type
	enum class ShaderType
	{
		VertexShader   = 0x1,
		HullShader     = 0x2,
		DomainShader   = 0x4,
		GeometryShader = 0x8,
		PixelShader    = 0x10,
		ComputeShader  = 0x20,
	};

	// Shader model target profile
	enum class ShaderTargetProfile
	{
		ShaderModel_5_0 = 0x40,
		ShaderModel_6_0 = 0x80,
		ShaderModel_6_1 = 0x100,
		ShaderModel_6_2 = 0x200,
		ShaderModel_6_3 = 0x400,
		ShaderModel_6_4 = 0x800,
		ShaderModel_6_5 = 0x1000,
		ShaderModel_6_6 = 0x2000
	};

	// Shader input parameter mask
	enum class ShaderInputParaMask
	{
		R = 1,
		RG = 3,
		RGB = 7,
		RGBA = 15
	};

	// Shader input parameter component type
	enum class ShaderInputParaType
	{
		Unknown = 0,
		UInt32 = 16,
		SInt32 = 32,
		Float32 = 64
	};

	// Shader input parameter DXGI format and its size in bytes
	struct DXGIFormatDesc
	{
		DXGI_FORMAT dxgi_format = DXGI_FORMAT_UNKNOWN;
		uint32_t size_in_bytes = 0;
	};

	// Dxc compiler result
	struct DxcShaderResult
	{
		ComPtr<IDxcBlob> shader_blob = nullptr;
		ComPtr<ID3D12ShaderReflection> shader_reflection = nullptr;
	};

	// Fxc compiler result
	struct FxcShaderResult
	{
		ComPtr<ID3DBlob> shader_blob = nullptr;
		ComPtr<ID3D11ShaderReflection> shader_reflection = nullptr;
	};

	// Helper function
	constexpr uint32_t operator|(uint32_t other, ShaderType shader_type);

	constexpr bool operator&(uint32_t other, ShaderType shader_type);

	// Dxc instance
	struct DxcInStance
	{
	private:
		using DxcCreateInstanceFn = decltype(&::DxcCreateInstance);

		ComPtr<IDxcUtils> utils = nullptr;
		ComPtr<IDxcCompiler3> compiler = nullptr;
		ComPtr<IDxcValidator> validator = nullptr;
		ComPtr<IDxcIncludeHandler> include_handler = nullptr;
		HMODULE compiler_hmodule = nullptr;
		DxcCreateInstanceFn dxc_create_instance_pfn = nullptr;

	private:
		DxcInStance();

	public:
		~DxcInStance();

		DxcInStance(const DxcInStance &) = delete;
		DxcInStance &operator=(const DxcInStance &) = delete;
		DxcInStance(DxcInStance &&) = delete;
		DxcInStance &operator=(DxcInStance &&) = delete;

		static DxcInStance &get();

		DxcShaderResult create_shader_from_file(std::wstring_view shader_filepath, ShaderType shader_type, ShaderTargetProfile shader_target_profile);
	};

	// Fxc instance
	struct FxcInstance
	{
	private:
		FxcInstance();

	public:
		~FxcInstance();

		FxcInstance(const FxcInstance &) = delete;
		FxcInstance &operator=(const FxcInstance &) = delete;
		FxcInstance(FxcInstance &&) = delete;
		FxcInstance &operator=(FxcInstance &&) = delete;

		static FxcInstance &get();

		FxcShaderResult create_shader_from_file(std::wstring_view shader_filepath, ShaderType shader_type, ShaderTargetProfile shader_target_profile);
	};

	// Constant buffer and its accessor
	struct ConstantBufferAccessor;

	struct ConstantBuffer
	{
	private:
		ComPtr<ID3D11Buffer> constant_buffer = nullptr;
		std::vector<uint8_t> upload_data = {};
		std::string constant_buffer_name = {};
		uint32_t binding_slot = 0;
		uint32_t shader_flag = 0;
		bool is_dirty = false;

		friend struct ConstantBufferAccessor;

	public:
		ConstantBuffer() = default;
		ConstantBuffer(const std::string &cb_name, uint32_t slot, uint32_t size_in_bytes, uint8_t *initial_data = nullptr);

		HRESULT create_buffer(ID3D11Device *device);

		void set_upload_data(std::span<uint8_t> in_data);

		void update_buffer(ID3D11DeviceContext *device_context);

		void transmit_upload_data(ConstantBuffer &other) const;

		void set_shader_flag(ShaderType shader_type);

		void emit_constant_buffer(ID3D11DeviceContext *device_context);
	};

	struct ConstantBufferAccessor
	{
	private:
		ConstantBuffer *constant_buffer_ref;
		std::string component_name = {};
		uint32_t component_offset = 0;
		uint32_t component_size = 0;

	public:
		explicit ConstantBufferAccessor(ConstantBuffer *input_constant_buffer, const std::string &in_component_name, uint32_t in_offset, uint32_t in_size);

		void set_raw(const uint8_t *data, uint32_t offset_in_bytes, uint32_t size_in_bytes);

		void set_matrix_in_bytes(const uint8_t *no_padding_data, uint32_t rows, uint32_t cols);

		void set_sint_matrix(const int32_t *no_padding_data, uint32_t rows, uint32_t cols);

		void set_uint_matrix(const uint32_t *no_padding_data, uint32_t rows, uint32_t cols);

		void set_float_matrix(const float *no_padding_data, uint32_t rows, uint32_t cols);

		void set_sint_vector(std::span<int32_t> data);

		void set_uint_vector(std::span<uint32_t> data);

		void set_float_vector(std::span<float> data);

		void set_sint(int32_t data);

		void set_uint(uint32_t data);

		void set_float(float data);
	};

	// Shader info
	struct VertexShaderConf
	{
		ComPtr<ID3D11VertexShader> vs = nullptr;
	};

	struct HullShaderConf
	{
		ComPtr<ID3D11HullShader> hs = nullptr;
	};

	struct DomainShaderConf
	{
		ComPtr<ID3D11DomainShader> ds = nullptr;
	};

	struct GeometryShaderConf
	{
		ComPtr<ID3D11GeometryShader> gs = nullptr;
	};

	struct PixelShaderConf
	{
		ComPtr<ID3D11PixelShader> ps = nullptr;
	};

	struct ThreadGroupConf
	{
		uint32_t thread_group_size_x = 1;
		uint32_t thread_group_size_y = 1;
		uint32_t thread_group_size_z = 1;
	};

	struct ComputeShaderConf
	{
		ComPtr<ID3D11ComputeShader> cs = nullptr;
		ThreadGroupConf thread_group_conf{};
	};

	// Shader resource view info
	struct ShaderResourceConf
	{
		ID3D11ShaderResourceView *srv = nullptr;
		D3D11_SRV_DIMENSION srv_dimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		uint32_t bind_slot = 0;
		ShaderType shader_flag = ShaderType::VertexShader;
	};

	// Unordered access view info
	struct RWResourceConf
	{
		ID3D11UnorderedAccessView *uav = nullptr;
		D3D11_UAV_DIMENSION uav_dimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		uint32_t initial_count = 0;
		uint32_t bind_slot = 0;
		ShaderType shader_flag = ShaderType::VertexShader;
		bool enable_counter = false;
		bool first_init = false;
	};

	// Sampler state
	struct SamplerStateConf
	{
		ID3D11SamplerState *sampler = nullptr;
		uint32_t bind_slot = 0;
		ShaderType shader_flag = ShaderType::VertexShader;
	};

	using ShaderInfo = std::variant<VertexShaderConf, HullShaderConf, DomainShaderConf, GeometryShaderConf, PixelShaderConf, ComputeShaderConf>;

	// Bind shaders to pipeline
	struct EmitShader
	{
		ID3D11DeviceContext *device_context = nullptr;

		void operator()(const VertexShaderConf &vertex_shader) const;

		void operator()(const HullShaderConf &hull_shader) const;

		void operator()(const DomainShaderConf &domain_shader) const;

		void operator()(const GeometryShaderConf &geometry_shader) const;

		void operator()(const PixelShaderConf &pixel_shader) const;

		void operator()(const ComputeShaderConf &compute_shader) const;
	};

	// Pipeline state object
	struct GraphicsPipelineStateObject
	{
		ComPtr<ID3D11RasterizerState> rasterizer_state = nullptr;
		ComPtr<ID3D11DepthStencilState> depth_stencil_state = nullptr;
		ComPtr<ID3D11BlendState> blend_state = nullptr;
		std::wstring_view vs_path{};
		std::wstring_view hs_path{};
		std::wstring_view ds_path{};
		std::wstring_view gs_path{};
		std::wstring_view ps_path{};
		ShaderTargetProfile shader_target_profile = ShaderTargetProfile::ShaderModel_5_0;
	};

	struct ComputePipelineStateObject
	{
		std::wstring_view cs_path{};
		ShaderTargetProfile shader_target_profile = ShaderTargetProfile::ShaderModel_5_0;
	};

	using PipelineStateObject = std::variant<GraphicsPipelineStateObject, ComputePipelineStateObject>;

	// Effect
	struct Effect
	{
	protected:
		std::unordered_map<size_t, std::unique_ptr<ConstantBuffer>> constant_buffer_manager;
		std::unordered_map<size_t, std::unique_ptr<ConstantBufferAccessor>> constant_buffer_accessor_manager;
		std::unordered_map<size_t, ShaderResourceConf> shader_resource_manager;
		std::unordered_map<size_t, RWResourceConf> unordered_access_manager;
		std::unordered_map<size_t, SamplerStateConf> sampler_manager;
		std::vector<ShaderInfo> pipeline_shader_manager;

	public:
		Effect();
		virtual ~Effect();

		ConstantBufferAccessor *query_constant_buffer_accessor(std::string_view variable_name);

		void set_constant_buffer_upload_data(std::string_view constant_buffer_name, std::span<uint8_t> in_data);

		void transmit_constant_buffer(Effect &other, std::string_view constant_buffer_name);

		void set_shader_resource_view(std::string_view srv_name, ID3D11ShaderResourceView *srv);

		void set_sampler(std::string_view sampler_name, ID3D11SamplerState *sampler);

		void set_unordered_access_view(std::string_view uav_name, ID3D11UnorderedAccessView *uav);

		virtual void emit_graphics_pipeline(ID3D11DeviceContext *device_context) {};

		virtual void emit_compute_pipeline(ID3D11DeviceContext *device_context) {};

		virtual void reset_graphics_pipeline(ID3D11DeviceContext *device_context) {}

		virtual void reset_compute_pipeline(ID3D11DeviceContext *device_context) {}

	protected:
		void emit_pipeline(ID3D11DeviceContext *device_context);

		void reset_pipeline(ID3D11DeviceContext *device_context);

		void update_shader_reflection(std::wstring_view shader_name, ID3D11Device *device, const FxcShaderResult &shader_result);
	};

	struct GraphicsEffect final : Effect
	{
	private:
		ComPtr<ID3D11RasterizerState> rasterizer_state = nullptr;
		ComPtr<ID3D11DepthStencilState> depth_stencil_state = nullptr;
		ComPtr<ID3D11BlendState> blend_state = nullptr;
		ComPtr<ID3D11InputLayout> vertex_input_layout = nullptr;
		std::array<float, 4> blend_factor{ 0.0f, 0.0f, 0.0f, 0.0f };
		std::span<ID3D11RenderTargetView *> render_target_views{};
		std::span<D3D11_VIEWPORT> viewports{};
		ID3D11DepthStencilView* depth_stencil_view = nullptr;
		D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		uint32_t sample_mask = 0xffffffff;
		uint32_t stencil_ref = 0;

	public:
		explicit GraphicsEffect(const PipelineStateObject &pipeline_state_object, ID3D11Device *device);

		~GraphicsEffect() override = default;
		GraphicsEffect(const GraphicsEffect &) = delete;
		GraphicsEffect &operator=(const GraphicsEffect &) = delete;
		GraphicsEffect(GraphicsEffect &&) = delete;
		GraphicsEffect &operator=(GraphicsEffect &&) = delete;

		void set_primitive_topology(D3D11_PRIMITIVE_TOPOLOGY in_topology);

		void set_render_viewports(std::span<D3D11_VIEWPORT> in_viewports);

		void set_render_target_views(std::span<ID3D11RenderTargetView *> in_render_target_views);

		void set_depth_stencil_view(ID3D11DepthStencilView *in_depth_stencil_view);

		void set_stencil_ref(uint32_t stencil_value);

		void set_blend_factor(std::span<float> blend_value);

		void draw(ID3D11DeviceContext *device_context, uint32_t vertex_count, uint32_t start_vertex_location);

		void emit_graphics_pipeline(ID3D11DeviceContext *device_context) override;

		void reset_graphics_pipeline(ID3D11DeviceContext *device_context) override;
	};

	struct ComputeEffect final : Effect
	{
	private:
		ThreadGroupConf thread_group_conf{};

	public:
		explicit ComputeEffect(const PipelineStateObject &pipeline_state_object, ID3D11Device *device);

		~ComputeEffect() override = default;
		ComputeEffect(const ComputeEffect &) = delete;
		ComputeEffect &operator=(const ComputeEffect &) = delete;
		ComputeEffect(ComputeEffect &&) = delete;
		ComputeEffect &operator=(ComputeEffect &&) = delete;

		void emit_compute_pipeline(ID3D11DeviceContext *device_context) override;

		void reset_compute_pipeline(ID3D11DeviceContext *device_context) override;

		void dispatch(ID3D11DeviceContext *device_context, uint32_t thread_x, uint32_t thread_y, uint32_t thread_z);
	};
}














































