//
// Created by ZZK on 2024/10/15.
//

#include <Toy/NewFramework/effect.h>
#include <Toy/Core/d3d_util.h>

namespace toy
{
	constexpr uint32_t operator|(uint32_t other, ShaderType shader_type)
	{
		return (other | static_cast<uint32_t>(shader_type));
	}

	constexpr bool operator&(uint32_t other, ShaderType shader_type)
	{
		return (other & static_cast<uint32_t>(shader_type)) > 0;
	}

	constexpr uint32_t operator|(ShaderType shader_type, ShaderTargetProfile shader_target_profile)
	{
		return static_cast<uint32_t>(shader_type) | static_cast<uint32_t>(shader_target_profile);
	}

	constexpr uint32_t operator|(ShaderInputParaMask input_para_mask, ShaderInputParaType input_para_type)
	{
		return static_cast<uint32_t>(input_para_mask) | static_cast<uint32_t>(input_para_type);
	}

	constexpr uint32_t operator|(ShaderInputParaType input_para_type, ShaderInputParaMask input_para_mask)
	{
		return static_cast<uint32_t>(input_para_mask) | static_cast<uint32_t>(input_para_type);
	}

	// Convert dxc shader type to user defined shader type
	static ShaderType convert_to_internal_shader_type(D3D12_SHADER_VERSION_TYPE shader_version_type)
	{
		switch (shader_version_type)
		{
			case D3D12_SHVER_VERTEX_SHADER   : return ShaderType::VertexShader;
			case D3D12_SHVER_HULL_SHADER     : return ShaderType::HullShader;
			case D3D12_SHVER_DOMAIN_SHADER   : return ShaderType::DomainShader;
			case D3D12_SHVER_GEOMETRY_SHADER : return ShaderType::GeometryShader;
			case D3D12_SHVER_PIXEL_SHADER    : return ShaderType::PixelShader;
			case D3D12_SHVER_COMPUTE_SHADER  : return ShaderType::ComputeShader;
			default : assert(false && "Unsupported shader");
		}
	}

	// Get shader entry point via shader type
	std::wstring_view query_shader_entry_point(ShaderType shader_type)
	{
		switch (shader_type)
		{
			case ShaderType::VertexShader  : return L"VS";
			case ShaderType::HullShader    : return L"HS";
			case ShaderType::DomainShader  : return L"DS";
			case ShaderType::GeometryShader: return L"GS";
			case ShaderType::PixelShader   : return L"PS";
			case ShaderType::ComputeShader : return L"CS";
			default: assert(false && "Unsupported shader type");
		}
	}

	// Convert shader input parameter component type to internal component type
	static constexpr ShaderInputParaType convert_to_internal_component_type(D3D_REGISTER_COMPONENT_TYPE component_type)
	{
		switch (component_type)
		{
			case D3D_REGISTER_COMPONENT_UNKNOWN : return ShaderInputParaType::Unknown;
			case D3D_REGISTER_COMPONENT_UINT32  : return ShaderInputParaType::UInt32;
			case D3D_REGISTER_COMPONENT_SINT32  : return ShaderInputParaType::SInt32;
			case D3D_REGISTER_COMPONENT_FLOAT32 : return ShaderInputParaType::Float32;
			default: assert(false && "Unsupported shader component type");
		}
	}

	// Shader input parameter to dxgi mapping
	std::unordered_map<uint32_t, DXGIFormatDesc> s_shader_input_para_mapping {
		{ ShaderInputParaMask::R | ShaderInputParaType::UInt32, DXGIFormatDesc{ DXGI_FORMAT_R32_UINT, 4 } },
		{ ShaderInputParaMask::R | ShaderInputParaType::SInt32, DXGIFormatDesc{ DXGI_FORMAT_R32_SINT, 4 } },
		{ ShaderInputParaMask::R | ShaderInputParaType::Float32, DXGIFormatDesc{ DXGI_FORMAT_R32_FLOAT, 4 } },
		{ ShaderInputParaMask::RG | ShaderInputParaType::UInt32, DXGIFormatDesc{ DXGI_FORMAT_R32G32_UINT, 8 } },
		{ ShaderInputParaMask::RG | ShaderInputParaType::SInt32, DXGIFormatDesc{ DXGI_FORMAT_R32G32_SINT, 8 } },
		{ ShaderInputParaMask::RG | ShaderInputParaType::Float32, DXGIFormatDesc{ DXGI_FORMAT_R32G32_FLOAT, 8 } },
		{ ShaderInputParaMask::RGB | ShaderInputParaType::UInt32, DXGIFormatDesc{ DXGI_FORMAT_R32G32B32_UINT, 12 } },
		{ ShaderInputParaMask::RGB | ShaderInputParaType::SInt32, DXGIFormatDesc{ DXGI_FORMAT_R32G32B32_SINT, 12 } },
		{ ShaderInputParaMask::RGB | ShaderInputParaType::Float32, DXGIFormatDesc{ DXGI_FORMAT_R32G32B32_FLOAT, 12 } },
		{ ShaderInputParaMask::RGBA | ShaderInputParaType::UInt32, DXGIFormatDesc{ DXGI_FORMAT_R32G32B32A32_UINT, 16 } },
		{ ShaderInputParaMask::RGBA | ShaderInputParaType::SInt32, DXGIFormatDesc{ DXGI_FORMAT_R32G32B32A32_SINT, 16 } },
		{ ShaderInputParaMask::RGBA | ShaderInputParaType::Float32, DXGIFormatDesc{ DXGI_FORMAT_R32G32B32A32_FLOAT, 16 } },
	};

	// shader target profile
	std::unordered_map<uint32_t, std::wstring_view> s_shader_target_profile_mapping {
		{ ShaderType::VertexShader | ShaderTargetProfile::ShaderModel_5_0, L"vs_5_0" },
		{ ShaderType::HullShader | ShaderTargetProfile::ShaderModel_5_0, L"hs_5_0" },
		{ ShaderType::DomainShader | ShaderTargetProfile::ShaderModel_5_0, L"ds_5_0" },
		{ ShaderType::GeometryShader | ShaderTargetProfile::ShaderModel_5_0, L"gs_5_0" },
		{ ShaderType::PixelShader | ShaderTargetProfile::ShaderModel_5_0, L"ps_5_0" },
		{ ShaderType::ComputeShader | ShaderTargetProfile::ShaderModel_5_0, L"cs_5_0" },

		{ ShaderType::VertexShader | ShaderTargetProfile::ShaderModel_6_0, L"vs_6_0" },
		{ ShaderType::HullShader | ShaderTargetProfile::ShaderModel_6_0, L"hs_6_0" },
		{ ShaderType::DomainShader | ShaderTargetProfile::ShaderModel_6_0, L"ds_6_0" },
		{ ShaderType::GeometryShader | ShaderTargetProfile::ShaderModel_6_0, L"gs_6_0" },
		{ ShaderType::PixelShader | ShaderTargetProfile::ShaderModel_6_0, L"ps_6_0" },
		{ ShaderType::ComputeShader | ShaderTargetProfile::ShaderModel_6_0, L"cs_6_0" },
	};

	static DXGIFormatDesc query_dxgi_format_desc(ShaderInputParaMask input_para_mask, ShaderInputParaType input_para_type)
	{
		const auto input_para_key = input_para_mask | input_para_type;
		return s_shader_input_para_mapping[input_para_key];
	}

	static std::wstring_view query_shader_target_profile(ShaderType shader_type, ShaderTargetProfile shader_target_profile)
	{
		const auto profile_key = shader_type | shader_target_profile;
		return s_shader_target_profile_mapping[profile_key];
	}

	// Dxc instance
	constexpr std::string_view s_compiler_path = DXTOY_HOME "external/dxc/bin/x64/dxcompiler.dll";
	constexpr std::wstring_view s_search_path = DXTOY_HOME L"data/pbr";
	DxcInStance::DxcInStance()
	{
		compiler_hmodule = LoadLibraryA(s_compiler_path.data());
		dxc_create_instance_pfn = reinterpret_cast<DxcCreateInstanceFn>(GetProcAddress(compiler_hmodule, "DxcCreateInstance"));
		dxc_create_instance_pfn(CLSID_DxcUtils, IID_PPV_ARGS(&utils));
		dxc_create_instance_pfn(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));
		dxc_create_instance_pfn(CLSID_DxcValidator, IID_PPV_ARGS(validator.GetAddressOf()));
		utils->CreateDefaultIncludeHandler(include_handler.GetAddressOf());
	}

	DxcInStance::~DxcInStance()
	{
		if (compiler_hmodule)
		{
			FreeLibrary(compiler_hmodule);
			dxc_create_instance_pfn = nullptr;
		}
	}

	DxcInStance &DxcInStance::get()
	{
		static DxcInStance dxc_instance{};
		return dxc_instance;
	}

	DxcShaderResult DxcInStance::create_shader_from_file(std::wstring_view shader_filepath, ShaderType shader_type, ShaderTargetProfile shader_target_profile)
	{
		DxcShaderResult shader_result{};
		auto entry_point = query_shader_entry_point(shader_type);
		auto target_profile = query_shader_target_profile(shader_type, shader_target_profile);
		std::vector<const wchar_t *> compilation_arguments{
			L"-E", entry_point.data(),
			L"-T", target_profile.data(),
			L"-I", s_search_path.data(),
			DXC_ARG_PACK_MATRIX_ROW_MAJOR,
			DXC_ARG_WARNINGS_ARE_ERRORS,
			DXC_ARG_ALL_RESOURCES_BOUND,
		};
#if defined(_DEBUG)
		compilation_arguments.push_back(DXC_ARG_DEBUG);
#else
		compilation_arguments.push_back(DXC_ARG_OPTIMIZATION_LEVEL1);
#endif

		// Load the shader source file to a blob.
		ComPtr<IDxcBlobEncoding> source_blob = nullptr;
		utils->LoadFile(shader_filepath.data(), nullptr, source_blob.GetAddressOf());
		const DxcBuffer source_buffer{
			.Ptr = source_blob->GetBufferPointer(),
			.Size = source_blob->GetBufferSize(),
			.Encoding = 0U,
		};

		// Compile shader
		ComPtr<IDxcResult> compiled_shader_buffer = nullptr;
		const HRESULT hr = compiler->Compile(&source_buffer,
								compilation_arguments.data(),
								static_cast<uint32_t>(compilation_arguments.size()),
								include_handler.Get(),
								IID_PPV_ARGS(compiled_shader_buffer.GetAddressOf()));
		if (FAILED(hr))
		{
			std::cout << std::format("Failed to compile shader with path\n");
		}

		// Get compilation errors (if any).
		ComPtr<IDxcBlobUtf8> errors = nullptr;
		compiled_shader_buffer->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(errors.GetAddressOf()), nullptr);
		if (errors != nullptr && errors->GetStringLength() > 0LLU)
		{
			const char *errorMessage = errors->GetStringPointer();
			std::cout << std::format("{}\n", errorMessage);
		}

		// Get shader blob
		compiled_shader_buffer->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(shader_result.shader_blob.GetAddressOf()), nullptr);
		if (shader_result.shader_blob == nullptr)
		{
			std::cout << std::format("Failed to get shader blob\n");
		}

		// Get shader reflection data.
		ComPtr<IDxcBlob> reflection_blob = nullptr;
		compiled_shader_buffer->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(reflection_blob.GetAddressOf()), nullptr);
		const DxcBuffer reflection_buffer{
			.Ptr = reflection_blob->GetBufferPointer(),
			.Size = reflection_blob->GetBufferSize(),
			.Encoding = 0U,
		};
		utils->CreateReflection(&reflection_buffer, IID_PPV_ARGS(shader_result.shader_reflection.GetAddressOf()));
		if (shader_result.shader_reflection == nullptr)
		{
			std::cout << std::format("Failed to get shader reflection");
		}

		return shader_result;
	}

	// Fxc shader result
	FxcInstance::FxcInstance() = default;

	FxcInstance::~FxcInstance() = default;

	FxcInstance &FxcInstance::get()
	{
		static FxcInstance fxc_instance{};
		return fxc_instance;
	}

	FxcShaderResult FxcInstance::create_shader_from_file(std::wstring_view shader_filepath, ShaderType shader_type, ShaderTargetProfile shader_target_profile)
	{
		// TODO: add shader cache
		FxcShaderResult fxc_shader_result{};
		auto entry_point = query_shader_entry_point(shader_type);
		auto target_profile = query_shader_target_profile(shader_type, shader_target_profile);
		ComPtr<ID3DBlob> error_blob = nullptr;
		ComPtr<ID3DBlob> blob_in = nullptr;
		if (const auto hr = D3DReadFileToBlob(shader_filepath.data(), blob_in.GetAddressOf()); FAILED(hr))
		{
			return fxc_shader_result;
		}

		uint32_t shader_compile_flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
		shader_compile_flags |= D3DCOMPILE_DEBUG;
		shader_compile_flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
		auto shader_filepath_u8_str = wstring_to_utf8(shader_filepath);
		auto entry_point_u8_str = wstring_to_utf8(entry_point);
		auto target_profile_u8_str = wstring_to_utf8(target_profile);
		auto hr = D3DCompile(blob_in->GetBufferPointer(), blob_in->GetBufferSize(), shader_filepath_u8_str.data(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entry_point_u8_str.data(), target_profile_u8_str.data(), shader_compile_flags, 0, fxc_shader_result.shader_blob.GetAddressOf(), error_blob.GetAddressOf());
		if (FAILED(hr) && error_blob != nullptr)
		{
			DX_CORE_ERROR("Failed to compiler shader via fxc: {}", static_cast<const char *>(error_blob->GetBufferPointer()));
			return fxc_shader_result;
		}

		// Reflection
		hr = D3DReflect(fxc_shader_result.shader_blob->GetBufferPointer(), fxc_shader_result.shader_blob->GetBufferSize(), IID_PPV_ARGS(fxc_shader_result.shader_reflection.GetAddressOf()));
		if (FAILED(hr))
		{
			DX_CORE_ERROR("Failed to create shader reflection");
			return fxc_shader_result;
		}
		return fxc_shader_result;
	}

	// Constant buffer
	ConstantBuffer::ConstantBuffer(const std::string &cb_name, uint32_t slot, uint32_t size_in_bytes, uint8_t *initial_data)
	: constant_buffer(nullptr), upload_data(size_in_bytes), constant_buffer_name(cb_name), binding_slot(slot), is_dirty(false)
	{
		if (initial_data != nullptr)
		{
			std::memcpy(upload_data.data(), initial_data, size_in_bytes);
			is_dirty = true;
		}
	}

	HRESULT ConstantBuffer::create_buffer(ID3D11Device *device)
	{
		if (device != nullptr)
		{
			D3D11_BUFFER_DESC constant_buffer_desc{};
			constant_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
			constant_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			constant_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			constant_buffer_desc.ByteWidth = static_cast<uint32_t>(upload_data.size());
			return device->CreateBuffer(&constant_buffer_desc, nullptr, constant_buffer.GetAddressOf());
		}
		return S_FALSE;
	}

	void ConstantBuffer::set_upload_data(std::span<uint8_t> in_data)
	{
		const auto size_in_bytes = (std::min)(in_data.size(), upload_data.size());
		std::memcpy(upload_data.data(), in_data.data(), size_in_bytes);
		is_dirty = true;
	}

	void ConstantBuffer::transmit_upload_data(ConstantBuffer &other) const
	{
		const size_t min_size = (std::min)(upload_data.size(), other.upload_data.size());
		std::memcpy(other.upload_data.data(), upload_data.data(), min_size);
		other.is_dirty = true;
	}

	void ConstantBuffer::update_buffer(ID3D11DeviceContext *device_context)
	{
		if (is_dirty)
		{
			is_dirty = false;
			D3D11_MAPPED_SUBRESOURCE mapped_data{};
			device_context->Map(constant_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_data);
			std::memcpy(mapped_data.pData, upload_data.data(), upload_data.size());
			device_context->Unmap(constant_buffer.Get(), 0);
		}
	}

	void ConstantBuffer::set_shader_flag(ShaderType shader_type)
	{
		shader_flag = (shader_flag | shader_type);
	}

	void ConstantBuffer::emit_constant_buffer(ID3D11DeviceContext *device_context)
	{
		if (shader_flag & ShaderType::VertexShader) {
			device_context->VSSetConstantBuffers(binding_slot, 1, constant_buffer.GetAddressOf());
		}
		if (shader_flag & ShaderType::HullShader) {
			device_context->HSSetConstantBuffers(binding_slot, 1, constant_buffer.GetAddressOf());
		}
		if (shader_flag & ShaderType::DomainShader) {
			device_context->DSSetConstantBuffers(binding_slot, 1, constant_buffer.GetAddressOf());
		}
		if (shader_flag & ShaderType::GeometryShader) {
			device_context->GSSetConstantBuffers(binding_slot, 1, constant_buffer.GetAddressOf());
		}
		if (shader_flag & ShaderType::PixelShader) {
			device_context->PSSetConstantBuffers(binding_slot, 1, constant_buffer.GetAddressOf());
		}
		if (shader_flag & ShaderType::ComputeShader) {
			device_context->CSSetConstantBuffers(binding_slot, 1, constant_buffer.GetAddressOf());
		}
	}

	// Constant buffer accessor
	ConstantBufferAccessor::ConstantBufferAccessor(ConstantBuffer *input_constant_buffer, const std::string &in_component_name, uint32_t in_offset, uint32_t in_size)
	: constant_buffer_ref(input_constant_buffer), component_name(in_component_name), component_offset(in_offset), component_size(in_size)
	{

	}

	void ConstantBufferAccessor::set_raw(const uint8_t *data, uint32_t offset_in_bytes, uint32_t size_in_bytes)
	{
		if (data == nullptr || offset_in_bytes > component_size) {
			return;
		}

		if ((offset_in_bytes + size_in_bytes) > component_size) {
			size_in_bytes = component_size - offset_in_bytes;
		}

		std::memcpy(constant_buffer_ref->upload_data.data() + component_offset + offset_in_bytes, data, size_in_bytes);
		constant_buffer_ref->is_dirty = true;
	}

	void ConstantBufferAccessor::set_matrix_in_bytes(const uint8_t *no_padding_data, uint32_t rows, uint32_t cols)
	{
		if (rows == 0 || rows > 4 || cols == 0 || cols > 4) {
			return;
		}

		uint32_t remain_bytes = component_size < 64 ? component_size : 64;
		uint8_t *dst_data = constant_buffer_ref->upload_data.data() + component_offset;
		while (remain_bytes > 0) {
			constexpr uint32_t stride = 16;
			uint32_t row_pitch = sizeof(uint32_t) * cols < remain_bytes ? sizeof(uint32_t) * cols : remain_bytes;
			std::memcpy(dst_data, no_padding_data, row_pitch);
			dst_data += stride;
			no_padding_data += sizeof(uint32_t) * cols;
			remain_bytes = remain_bytes < stride ? 0 : (remain_bytes - stride);
		}
		constant_buffer_ref->is_dirty = true;
	}

	void ConstantBufferAccessor::set_sint_matrix(const int32_t *no_padding_data, uint32_t rows, uint32_t cols)
	{
		set_matrix_in_bytes(reinterpret_cast<const uint8_t *>(no_padding_data), rows, cols);
	}

	void ConstantBufferAccessor::set_uint_matrix(const uint32_t *no_padding_data, uint32_t rows, uint32_t cols)
	{
		set_matrix_in_bytes(reinterpret_cast<const uint8_t *>(no_padding_data), rows, cols);
	}

	void ConstantBufferAccessor::set_float_matrix(const float *no_padding_data, uint32_t rows, uint32_t cols)
	{
		set_matrix_in_bytes(reinterpret_cast<const uint8_t *>(no_padding_data), rows, cols);
	}

	void ConstantBufferAccessor::set_sint_vector(std::span<int32_t> data)
	{
		auto components = (std::min)(static_cast<uint32_t>(data.size()), 4U);
		auto size_in_bytes = (std::min)(static_cast<uint32_t>(components * sizeof(uint32_t)), component_size);
		set_raw(reinterpret_cast<const uint8_t *>(data.data()), 0, size_in_bytes);
	}

	void ConstantBufferAccessor::set_uint_vector(std::span<uint32_t> data)
	{
		auto components = (std::min)(static_cast<uint32_t>(data.size()), 4U);
		auto size_in_bytes = (std::min)(static_cast<uint32_t>(components * sizeof(uint32_t)), component_size);
		set_raw(reinterpret_cast<const uint8_t *>(data.data()), 0, size_in_bytes);
	}

	void ConstantBufferAccessor::set_float_vector(std::span<float> data)
	{
		auto components = (std::min)(static_cast<uint32_t>(data.size()), 4U);
		auto size_in_bytes = (std::min)(static_cast<uint32_t>(components * sizeof(uint32_t)), component_size);
		set_raw(reinterpret_cast<const uint8_t *>(data.data()), 0, size_in_bytes);
	}

	void ConstantBufferAccessor::set_sint(int32_t data)
	{
		set_raw(reinterpret_cast<const uint8_t *>(&data), 0, sizeof(int32_t));
	}

	void ConstantBufferAccessor::set_uint(uint32_t data)
	{
		set_raw(reinterpret_cast<const uint8_t *>(&data), 0, sizeof(uint32_t));
	}

	void ConstantBufferAccessor::set_float(float data)
	{
		set_raw(reinterpret_cast<const uint8_t *>(&data), 0, sizeof(float));
	}

	// EmitShader
	void EmitShader::operator()(const VertexShaderConf &vertex_shader) const
	{
		device_context->VSSetShader(vertex_shader.vs.Get(), nullptr, 0);
	}

	void EmitShader::operator()(const HullShaderConf &hull_shader) const
	{
		device_context->HSSetShader(hull_shader.hs.Get(), nullptr, 0);
	}

	void EmitShader::operator()(const DomainShaderConf &domain_shader) const
	{
		device_context->DSSetShader(domain_shader.ds.Get(), nullptr, 0);
	}

	void EmitShader::operator()(const GeometryShaderConf &geometry_shader) const
	{
		device_context->GSSetShader(geometry_shader.gs.Get(), nullptr, 0);
	}

	void EmitShader::operator()(const PixelShaderConf &pixel_shader) const
	{
		device_context->PSSetShader(pixel_shader.ps.Get(), nullptr, 0);
	}

	void EmitShader::operator()(const ComputeShaderConf &compute_shader) const
	{
		device_context->CSSetShader(compute_shader.cs.Get(), nullptr, 0);
	}

	// Emit shader resource view
	static void emit_shader_resource_view(const ShaderResourceConf &shader_resource, ID3D11DeviceContext *device_context)
	{
		if (shader_resource.shader_flag == ShaderType::VertexShader) {
			device_context->VSSetShaderResources(shader_resource.bind_slot, 1, &shader_resource.srv);
		} else if (shader_resource.shader_flag == ShaderType::HullShader) {
			device_context->HSSetShaderResources(shader_resource.bind_slot, 1, &shader_resource.srv);
		} else if (shader_resource.shader_flag == ShaderType::DomainShader) {
			device_context->DSSetShaderResources(shader_resource.bind_slot, 1, &shader_resource.srv);
		} else if (shader_resource.shader_flag == ShaderType::GeometryShader) {
			device_context->GSSetShaderResources(shader_resource.bind_slot, 1, &shader_resource.srv);
		} else if (shader_resource.shader_flag == ShaderType::PixelShader) {
			device_context->PSSetShaderResources(shader_resource.bind_slot, 1, &shader_resource.srv);
		} else {
			device_context->CSSetShaderResources(shader_resource.bind_slot, 1, &shader_resource.srv);
		}
	}

	// Emit sampler
	static void emit_sampler_state(const SamplerStateConf &sampler_state, ID3D11DeviceContext *device_context)
	{
		if (sampler_state.shader_flag == ShaderType::VertexShader) {
			device_context->VSSetSamplers(sampler_state.bind_slot, 1, &sampler_state.sampler);
		} else if (sampler_state.shader_flag == ShaderType::HullShader) {
			device_context->HSSetSamplers(sampler_state.bind_slot, 1, &sampler_state.sampler);
		} else if (sampler_state.shader_flag == ShaderType::DomainShader) {
			device_context->DSSetSamplers(sampler_state.bind_slot, 1, &sampler_state.sampler);
		} else if (sampler_state.shader_flag == ShaderType::GeometryShader) {
			device_context->GSSetSamplers(sampler_state.bind_slot, 1, &sampler_state.sampler);
		} else if (sampler_state.shader_flag == ShaderType::PixelShader) {
			device_context->PSSetSamplers(sampler_state.bind_slot, 1, &sampler_state.sampler);
		} else {
			device_context->CSSetSamplers(sampler_state.bind_slot, 1, &sampler_state.sampler);
		}
	}

	// Emit unordered access view
	static void emit_unordered_access_view(RWResourceConf &rw_resource, ID3D11DeviceContext *device_context)
	{
		if (rw_resource.shader_flag == ShaderType::PixelShader) {
			const bool need_init = rw_resource.first_init;
			rw_resource.first_init = false;
			device_context->OMSetRenderTargetsAndUnorderedAccessViews(D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL, nullptr, nullptr,
																	rw_resource.bind_slot, 1, &rw_resource.uav, (need_init ? &rw_resource.initial_count : nullptr));
		} else if (rw_resource.shader_flag == ShaderType::ComputeShader) {
			const bool need_init = rw_resource.enable_counter && rw_resource.first_init;
			rw_resource.first_init = false;
			device_context->CSSetUnorderedAccessViews(rw_resource.bind_slot, 1, &rw_resource.uav,
														(need_init ? &rw_resource.initial_count : nullptr));
		}
	}

	// Effect
	Effect::Effect() = default;

	Effect::~Effect() = default;

	void Effect::update_shader_reflection(std::wstring_view shader_name, ID3D11Device *device, const FxcShaderResult &shader_result)
	{
		auto &&shader_reflection = shader_result.shader_reflection;
		D3D11_SHADER_DESC shader_desc{};
		if (FAILED(shader_reflection->GetDesc(&shader_desc))) {
			std::cout << std::format("Failed to get shader reflection desc\n");
			return;
		}

		auto shader_type = static_cast<D3D11_SHADER_VERSION_TYPE>(D3D11_SHVER_GET_TYPE(shader_desc.Version));
		auto dx12_shader_type = static_cast<D3D12_SHADER_VERSION_TYPE>(static_cast<uint32_t>(shader_type));
		auto inner_shader_type = convert_to_internal_shader_type(dx12_shader_type);
		// Bound resources
		for (uint32_t i = 0; i < shader_desc.BoundResources; ++i)
		{
			D3D11_SHADER_INPUT_BIND_DESC shader_input_bind_desc{};
			shader_reflection->GetResourceBindingDesc(i, &shader_input_bind_desc);
			auto bind_type = shader_input_bind_desc.Type;
			if (bind_type == D3D_SIT_CBUFFER)
			{
				ID3D11ShaderReflectionConstantBuffer* shader_reflection_constant_buffer = shader_reflection->GetConstantBufferByName(shader_input_bind_desc.Name);
				D3D11_SHADER_BUFFER_DESC shader_buffer_desc{};
				shader_reflection_constant_buffer->GetDesc(&shader_buffer_desc);

				auto constant_buffer_id = string_to_id(shader_input_bind_desc.Name);
				ConstantBuffer *constant_buffer_ref = nullptr;
				if (constant_buffer_manager.contains(constant_buffer_id)) {
					constant_buffer_ref = constant_buffer_manager[constant_buffer_id].get();
					constant_buffer_ref->set_shader_flag(inner_shader_type);
				} else {
					constant_buffer_manager[constant_buffer_id] = std::make_unique<ConstantBuffer>(shader_input_bind_desc.Name, shader_input_bind_desc.BindPoint, shader_buffer_desc.Size);
					constant_buffer_ref = constant_buffer_manager[constant_buffer_id].get();
					constant_buffer_ref->create_buffer(device);
					constant_buffer_ref->set_shader_flag(inner_shader_type);
				}

				for (uint32_t j = 0; j < shader_buffer_desc.Variables; ++j)
				{
					ID3D11ShaderReflectionVariable* shader_reflection_variable = shader_reflection_constant_buffer->GetVariableByIndex(j);
					D3D11_SHADER_VARIABLE_DESC shader_variable_desc{};
					shader_reflection_variable->GetDesc(&shader_variable_desc);

					auto constant_buffer_var_id = string_to_id(shader_variable_desc.Name);
					if (!constant_buffer_accessor_manager.contains(constant_buffer_var_id)) {
						constant_buffer_accessor_manager[constant_buffer_var_id] = std::make_unique<ConstantBufferAccessor>(constant_buffer_ref, shader_variable_desc.Name,
																					shader_variable_desc.StartOffset, shader_variable_desc.Size);
					}
				}
				continue;
			}

			if (bind_type == D3D_SIT_TEXTURE || bind_type == D3D_SIT_TBUFFER || bind_type == D3D_SIT_STRUCTURED || bind_type == D3D_SIT_BYTEADDRESS)
			{
				auto srv_id = string_to_id(shader_input_bind_desc.Name);
				if (!shader_resource_manager.contains(srv_id)) {
					shader_resource_manager.try_emplace(srv_id, nullptr,  shader_input_bind_desc.Dimension, shader_input_bind_desc.BindPoint, inner_shader_type);
				}
				continue;
			}

			if (bind_type == D3D_SIT_UAV_RWTYPED || bind_type == D3D_SIT_UAV_RWSTRUCTURED || bind_type == D3D_SIT_UAV_RWBYTEADDRESS || bind_type == D3D_SIT_UAV_FEEDBACKTEXTURE ||
				bind_type == D3D_SIT_UAV_APPEND_STRUCTURED || bind_type == D3D_SIT_UAV_CONSUME_STRUCTURED || bind_type == D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER) {
				auto uav_id = string_to_id(shader_input_bind_desc.Name);
				if (!unordered_access_manager.contains(uav_id)) {
					unordered_access_manager.try_emplace(uav_id, nullptr, static_cast<D3D11_UAV_DIMENSION>(shader_input_bind_desc.Dimension), 0, shader_input_bind_desc.BindPoint,
											inner_shader_type, bind_type == D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER, false);
				}
				continue;
			}

			if (bind_type == D3D_SIT_SAMPLER)
			{
				auto sampler_id = string_to_id(shader_input_bind_desc.Name);
				if (!sampler_manager.contains(sampler_id)) {
					sampler_manager.try_emplace(sampler_id, nullptr, shader_input_bind_desc.BindPoint, inner_shader_type);
				}
			}
		}
	}

	ConstantBufferAccessor *Effect::query_constant_buffer_accessor(std::string_view variable_name)
	{
		if (const auto constant_buffer_var_id = string_to_id(variable_name); constant_buffer_accessor_manager.contains(constant_buffer_var_id))
		{
			return constant_buffer_accessor_manager[constant_buffer_var_id].get();
		}
		return nullptr;
	}

	void Effect::set_constant_buffer_upload_data(std::string_view constant_buffer_name, std::span<uint8_t> in_data)
	{
		if (const auto constant_buffer_id = string_to_id(constant_buffer_name); constant_buffer_manager.contains(constant_buffer_id))
		{
			constant_buffer_manager[constant_buffer_id]->set_upload_data(in_data);
		}
	}

	void Effect::transmit_constant_buffer(Effect &other, std::string_view constant_buffer_name)
	{
		if (const auto constant_buffer_id = string_to_id(constant_buffer_name); constant_buffer_manager.contains(constant_buffer_id) && other.constant_buffer_manager.contains(constant_buffer_id))
		{
			constant_buffer_manager[constant_buffer_id]->transmit_upload_data(*other.constant_buffer_manager[constant_buffer_id]);
		}
	}

	void Effect::set_shader_resource_view(std::string_view srv_name, ID3D11ShaderResourceView *srv)
	{
		auto shader_resource_id = string_to_id(srv_name);
		if (shader_resource_manager.contains(shader_resource_id)) {
			shader_resource_manager[shader_resource_id].srv = srv;
		}
	}

	void Effect::set_sampler(std::string_view sampler_name, ID3D11SamplerState *sampler)
	{
		auto sampler_id = string_to_id(sampler_name);
		if (sampler_manager.contains(sampler_id)) {
			sampler_manager[sampler_id].sampler = sampler;
		}
	}

	void Effect::set_unordered_access_view(std::string_view uav_name, ID3D11UnorderedAccessView *uav)
	{
		auto uav_id = string_to_id(uav_name);
		if (unordered_access_manager.contains(uav_id)) {
			unordered_access_manager[uav_id].uav = uav;
		}
	}

	void Effect::emit_pipeline(ID3D11DeviceContext *device_context)
	{
		for (auto &&shader_info : pipeline_shader_manager)
		{
			std::visit(EmitShader{ device_context }, shader_info);
		}

		for (auto &&constant_buffer_info : constant_buffer_manager)
		{
			constant_buffer_info.second->update_buffer(device_context);
			constant_buffer_info.second->emit_constant_buffer(device_context);
		}

		for (auto &&shader_resource_info : shader_resource_manager)
		{
			emit_shader_resource_view(shader_resource_info.second, device_context);
		}

		for (auto &&sampler_state_info : sampler_manager)
		{
			emit_sampler_state(sampler_state_info.second, device_context);
		}

		for (auto &&rw_resource_info : unordered_access_manager)
		{
			emit_unordered_access_view(rw_resource_info.second, device_context);
		}
	}

	void Effect::reset_pipeline(ID3D11DeviceContext *device_context)
	{
		for (auto &&shader_resource_info : shader_resource_manager)
		{
			shader_resource_info.second.srv = nullptr;
			emit_shader_resource_view(shader_resource_info.second, device_context);
		}

		for (auto &&sampler_state_info : sampler_manager)
		{
			sampler_state_info.second.sampler = nullptr;
			emit_sampler_state(sampler_state_info.second, device_context);
		}

		for (auto &&rw_resource_info : unordered_access_manager)
		{
			rw_resource_info.second.uav = nullptr;
			emit_unordered_access_view(rw_resource_info.second, device_context);
		}
	}

	// Graphics effect
	GraphicsEffect::GraphicsEffect(const PipelineStateObject &pipeline_state_object, ID3D11Device *device)
	{
		auto &&fxc_instance = FxcInstance::get();
		if (std::holds_alternative<GraphicsPipelineStateObject>(pipeline_state_object))
		{
			auto &&graphics_pipeline_state_object = std::get<GraphicsPipelineStateObject>(pipeline_state_object);
			rasterizer_state = graphics_pipeline_state_object.rasterizer_state;
			depth_stencil_state = graphics_pipeline_state_object.depth_stencil_state;
			blend_state = graphics_pipeline_state_object.blend_state;
			auto shader_target_profile = graphics_pipeline_state_object.shader_target_profile;
			// VS
			if (!graphics_pipeline_state_object.vs_path.empty())
			{
				auto fxc_shader_result = fxc_instance.create_shader_from_file(graphics_pipeline_state_object.vs_path, ShaderType::VertexShader, shader_target_profile);
				update_shader_reflection(graphics_pipeline_state_object.vs_path, device, fxc_shader_result);

				VertexShaderConf vs_info{};
				auto &&shader_blob = fxc_shader_result.shader_blob;
				device->CreateVertexShader(shader_blob->GetBufferPointer(), shader_blob->GetBufferSize(), nullptr, vs_info.vs.GetAddressOf());
				pipeline_shader_manager.emplace_back(std::move(vs_info));

				// Create vertex input layout if possible
				auto &&shader_reflection = fxc_shader_result.shader_reflection;
				D3D11_SHADER_DESC shader_desc{};
				if (FAILED(shader_reflection->GetDesc(&shader_desc)))
				{
					std::cout << std::format("Failed to get shader reflection desc\n");
					return;
				}
				std::vector<D3D11_INPUT_ELEMENT_DESC> input_elements{};
				input_elements.reserve(shader_desc.InputParameters);
				uint32_t input_slot = 0;
				for (uint32_t index = 0; index < shader_desc.InputParameters; ++index)
				{
					D3D11_SIGNATURE_PARAMETER_DESC signature_parameter_desc{};
					shader_reflection->GetInputParameterDesc(index, &signature_parameter_desc);
					auto shader_input_para_type = convert_to_internal_component_type(signature_parameter_desc.ComponentType);
					auto shader_input_para_mask = static_cast<ShaderInputParaMask>(signature_parameter_desc.Mask);
					auto dxgi_format_desc = query_dxgi_format_desc(shader_input_para_mask, shader_input_para_type);
					D3D11_INPUT_ELEMENT_DESC input_element_desc{ signature_parameter_desc.SemanticName, signature_parameter_desc.SemanticIndex, dxgi_format_desc.dxgi_format, input_slot,
														0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
					input_elements.emplace_back(input_element_desc);
					++input_slot;
				}
				if (shader_desc.InputParameters > 0)
				{
					device->CreateInputLayout(input_elements.data(), static_cast<uint32_t>(input_elements.size()), shader_blob->GetBufferPointer(), shader_blob->GetBufferSize(),
											vertex_input_layout.GetAddressOf());
				}
			} else {
				pipeline_shader_manager.emplace_back(VertexShaderConf{});
			}

			// HS
			if (!graphics_pipeline_state_object.hs_path.empty()) {
				auto fxc_shader_result = fxc_instance.create_shader_from_file(graphics_pipeline_state_object.hs_path, ShaderType::HullShader, shader_target_profile);
				update_shader_reflection(graphics_pipeline_state_object.hs_path, device, fxc_shader_result);
				HullShaderConf hs_info{};
				device->CreateHullShader(fxc_shader_result.shader_blob->GetBufferPointer(), fxc_shader_result.shader_blob->GetBufferSize(), nullptr, hs_info.hs.GetAddressOf());
				pipeline_shader_manager.emplace_back(std::move(hs_info));
			} else {
				pipeline_shader_manager.emplace_back(HullShaderConf{});
			}

			// DS
			if (!graphics_pipeline_state_object.ds_path.empty()) {
				auto fxc_shader_result = fxc_instance.create_shader_from_file(graphics_pipeline_state_object.ds_path, ShaderType::DomainShader, shader_target_profile);
				update_shader_reflection(graphics_pipeline_state_object.ds_path, device, fxc_shader_result);
				DomainShaderConf ds_info{};
				device->CreateDomainShader(fxc_shader_result.shader_blob->GetBufferPointer(), fxc_shader_result.shader_blob->GetBufferSize(), nullptr, ds_info.ds.GetAddressOf());
				pipeline_shader_manager.emplace_back(std::move(ds_info));
			} else {
				pipeline_shader_manager.emplace_back(DomainShaderConf{});
			}

			// GS
			if (!graphics_pipeline_state_object.gs_path.empty()) {
				auto fxc_shader_result = fxc_instance.create_shader_from_file(graphics_pipeline_state_object.gs_path, ShaderType::GeometryShader, shader_target_profile);
				update_shader_reflection(graphics_pipeline_state_object.gs_path, device, fxc_shader_result);
				GeometryShaderConf gs_info{};
				device->CreateGeometryShader(fxc_shader_result.shader_blob->GetBufferPointer(), fxc_shader_result.shader_blob->GetBufferSize(), nullptr, gs_info.gs.GetAddressOf());
				pipeline_shader_manager.emplace_back(std::move(gs_info));
			} else {
				pipeline_shader_manager.emplace_back(GeometryShaderConf{});
			}

			// PS
			if (!graphics_pipeline_state_object.ps_path.empty()) {
				auto fxc_shader_result = fxc_instance.create_shader_from_file(graphics_pipeline_state_object.ps_path, ShaderType::PixelShader, shader_target_profile);
				update_shader_reflection(graphics_pipeline_state_object.ps_path, device, fxc_shader_result);
				PixelShaderConf ps_info{};
				device->CreatePixelShader(fxc_shader_result.shader_blob->GetBufferPointer(), fxc_shader_result.shader_blob->GetBufferSize(), nullptr, ps_info.ps.GetAddressOf());
				pipeline_shader_manager.emplace_back(std::move(ps_info));
			} else {
				pipeline_shader_manager.emplace_back(PixelShaderConf{});
			}
		}
	}

	void GraphicsEffect::set_stencil_ref(uint32_t stencil_value)
	{
		stencil_ref = stencil_value;
	}

	void GraphicsEffect::set_blend_factor(std::span<float> blend_value)
	{
		const uint32_t size_in_bytes = (std::min)(static_cast<uint32_t>(blend_value.size()), 4U) * sizeof(float);
		std::memcpy(blend_factor.data(), blend_value.data(), size_in_bytes);
	}

	void GraphicsEffect::emit_graphics_pipeline(ID3D11DeviceContext *device_context)
	{
		Effect::emit_pipeline(device_context);
		device_context->IASetInputLayout(vertex_input_layout.Get());
		device_context->RSSetState(rasterizer_state.Get());
		device_context->OMSetDepthStencilState(depth_stencil_state.Get(), stencil_ref);
		device_context->OMSetBlendState(blend_state.Get(), blend_factor.data(), sample_mask);
	}

	void GraphicsEffect::reset_graphics_pipeline(ID3D11DeviceContext *device_context)
	{
		Effect::reset_pipeline(device_context);
	}

	// Compute pipeline
	ComputeEffect::ComputeEffect(const PipelineStateObject &pipeline_state_object, ID3D11Device *device)
	{
		auto &&fxc_instance = FxcInstance::get();
		if (std::holds_alternative<ComputePipelineStateObject>(pipeline_state_object))
		{
			auto &&compute_pipeline_state_object = std::get<ComputePipelineStateObject>(pipeline_state_object);
			auto shader_target_profile = compute_pipeline_state_object.shader_target_profile;

			// CS
			if (!compute_pipeline_state_object.cs_path.empty()) {
				auto fxc_shader_result = fxc_instance.create_shader_from_file(compute_pipeline_state_object.cs_path, ShaderType::ComputeShader, shader_target_profile);
				update_shader_reflection(compute_pipeline_state_object.cs_path, device, fxc_shader_result);
				ComputeShaderConf cs_info{};
				fxc_shader_result.shader_reflection->GetThreadGroupSize(&cs_info.thread_group_conf.thread_group_size_x, &cs_info.thread_group_conf.thread_group_size_y, &cs_info.thread_group_conf.thread_group_size_z);
				thread_group_conf = cs_info.thread_group_conf;
				device->CreateComputeShader(fxc_shader_result.shader_blob->GetBufferPointer(), fxc_shader_result.shader_blob->GetBufferSize(), nullptr, cs_info.cs.GetAddressOf());
				pipeline_shader_manager.emplace_back(std::move(cs_info));
			} else {
				pipeline_shader_manager.emplace_back(ComputeShaderConf{});
			}
		}
	}

	void ComputeEffect::emit_compute_pipeline(ID3D11DeviceContext *device_context)
	{
		Effect::emit_pipeline(device_context);
	}

	void ComputeEffect::dispatch(ID3D11DeviceContext *device_context, uint32_t thread_x, uint32_t thread_y, uint32_t thread_z)
	{
		uint32_t thread_group_count_x = (thread_x + thread_group_conf.thread_group_size_x - 1) / thread_group_conf.thread_group_size_x;
		uint32_t thread_group_count_y = (thread_y + thread_group_conf.thread_group_size_y - 1) / thread_group_conf.thread_group_size_y;
		uint32_t thread_group_count_z = (thread_z + thread_group_conf.thread_group_size_z - 1) / thread_group_conf.thread_group_size_z;
		device_context->Dispatch(thread_group_count_x, thread_group_count_y, thread_group_count_z);
	}

	void ComputeEffect::reset_compute_pipeline(ID3D11DeviceContext *device_context)
	{
		Effect::reset_pipeline(device_context);
	}
}








































