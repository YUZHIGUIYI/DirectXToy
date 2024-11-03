//
// Created by ZZK on 2024/10/24.
//

#include <Toy/NewFramework/multi_scattering_pass.h>
#include <Toy/Renderer/render_states.h>
#include <Toy/Geometry/vertex.h>
#include <Toy/Model/mesh_data.h>
#include <Toy/Model/texture_manager.h>
#include <Toy/Model/model_manager.h>
#include <Toy/Model/material.h>
#include <Toy/ECS/components.h>

#include <Toy/Renderer/texture_2d.h>
#include <Toy/Renderer/buffer.h>
#include <Toy/NewFramework/sampling.h>

namespace toy
{
	static constexpr uint32_t s_dir_sample_count = 64;

	static constexpr uint32_t s_ray_march_step_count = 256;

	struct CSParams
	{
		DirectX::XMFLOAT3 terrain_albedo{};
		int32_t dir_sample_count = 0;
		DirectX::XMFLOAT3 sun_intensity{};
		int32_t ray_march_step_count = 0;
	};

	struct MultiScatteringPass::PassImpl
	{
		std::unique_ptr<ComputeEffect> compute_effect = nullptr;
		std::unique_ptr<Texture2D> multi_scattering_lut_tex = nullptr;
		std::unique_ptr<StructureBuffer<DirectX::XMFLOAT2>> samples_buffer = nullptr;

		AtmosphereProperties atmosphere_properties{};
		uint32_t ms_lut_res_x = 256;
		uint32_t ms_lut_res_y = 256;
	};

	MultiScatteringPass::MultiScatteringPass()
	: m_pass_impl(std::make_unique<PassImpl>())
	{

	}

	MultiScatteringPass::~MultiScatteringPass() noexcept = default;

	MultiScatteringPass::MultiScatteringPass(MultiScatteringPass &&other) noexcept
	{
		m_pass_impl.swap(other.m_pass_impl);
	}

	MultiScatteringPass &MultiScatteringPass::operator=(MultiScatteringPass &&other) noexcept
	{
		m_pass_impl.swap(other.m_pass_impl);
		return *this;
	}

	MultiScatteringPass &MultiScatteringPass::get()
	{
		static MultiScatteringPass multi_scattering_pass{};
		return multi_scattering_pass;
	}

	void MultiScatteringPass::init(ID3D11Device *device)
	{
		ComputePipelineStateObject pipeline_state_object{
			DXTOY_HOME L"data/pbr/multiscatter.hlsl",
			ShaderTargetProfile::ShaderModel_5_0
		};
		auto poisson_sampling_data = fast_poisson_disk_sampling(s_dir_sample_count);
		m_pass_impl->compute_effect = std::make_unique<ComputeEffect>(pipeline_state_object, device);
		m_pass_impl->samples_buffer = std::make_unique<StructureBuffer<DirectX::XMFLOAT2>>(device, s_dir_sample_count, D3D11_BIND_SHADER_RESOURCE, false, false, poisson_sampling_data.data());
		m_pass_impl->multi_scattering_lut_tex = std::make_unique<Texture2D>(device, m_pass_impl->ms_lut_res_x, m_pass_impl->ms_lut_res_y, DXGI_FORMAT_R32G32B32A32_FLOAT,
																	1, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS);
		create_texture_uav(device, m_pass_impl->multi_scattering_lut_tex.get(), 0);
	}

	void MultiScatteringPass::set_atmosphere_properties(const AtmosphereProperties &in_atmosphere_properties)
	{
		m_pass_impl->atmosphere_properties = in_atmosphere_properties;
	}

	void MultiScatteringPass::emit_render_pass(ID3D11DeviceContext *device_context)
	{
		using namespace model;
		auto &&texture_manager = TextureManager::get();
		CSParams cs_params{ DirectX::XMFLOAT3{ 0.3f, 0.3f, 0.3f }, s_dir_sample_count, DirectX::XMFLOAT3{ 1.0f, 1.0f, 1.0f }, s_ray_march_step_count };
		auto std_atmosphere_properties = m_pass_impl->atmosphere_properties.convert_to_std_uint();
		auto *cb_general_address = reinterpret_cast<uint8_t *>(std::addressof(std_atmosphere_properties));
		auto *cb_cs_address = reinterpret_cast<uint8_t *>(std::addressof(cs_params));
		auto *transmittance_map = texture_manager.get_texture(material_semantics_name(MaterialSemantics::TransmittanceLUT));

		m_pass_impl->compute_effect->set_constant_buffer_upload_data("CBAtmosphereParams", std::span{ cb_general_address, sizeof(AtmosphereProperties) });
		m_pass_impl->compute_effect->set_constant_buffer_upload_data("CBCSParams", std::span{ cb_cs_address, sizeof(CSParams) });
		m_pass_impl->compute_effect->set_shader_resource_view("gRawDirSamples", m_pass_impl->samples_buffer->get_shader_resource());
		m_pass_impl->compute_effect->set_shader_resource_view("gTransmittanceMap", transmittance_map);
		m_pass_impl->compute_effect->set_sampler("gSamTransmittance", RenderStates::ss_linear_clamp.Get());
		m_pass_impl->compute_effect->set_unordered_access_view("gMultiScatteringMap", m_pass_impl->multi_scattering_lut_tex->get_unordered_access());
		m_pass_impl->compute_effect->emit_compute_pipeline(device_context);
		m_pass_impl->compute_effect->dispatch(device_context, m_pass_impl->ms_lut_res_x, m_pass_impl->ms_lut_res_y, 1);

		m_pass_impl->compute_effect->reset_compute_pipeline(device_context);

		texture_manager.add_texture(material_semantics_name(MaterialSemantics::MultiScatteringLUT), m_pass_impl->multi_scattering_lut_tex->get_shader_resource());
	}

}