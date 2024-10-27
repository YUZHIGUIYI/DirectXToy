//
// Created by ZZK on 2024/10/21.
//

#include <Toy/NewFramework/transmittance_pass.h>
#include <Toy/Renderer/render_states.h>
#include <Toy/Geometry/vertex.h>
#include <Toy/Model/mesh_data.h>
#include <Toy/Model/texture_manager.h>
#include <Toy/Model/model_manager.h>
#include <Toy/Model/material.h>
#include <Toy/ECS/components.h>

#include <Toy/Renderer/texture_2d.h>

namespace toy
{
	struct TransmittancePass::PassImpl
	{
		std::unique_ptr<ComputeEffect> compute_effect = nullptr;
		std::unique_ptr<Texture2D> transmittance_lut_tex = nullptr;

		AtmosphereProperties atmosphere_properties{};
		uint32_t transmittance_lut_res_x = 256;
		uint32_t transmittance_lut_res_y = 256;
	};

	TransmittancePass::TransmittancePass()
	: m_pass_impl(std::make_unique<PassImpl>())
	{

	}

	TransmittancePass::~TransmittancePass() noexcept = default;

	TransmittancePass::TransmittancePass(TransmittancePass &&other) noexcept
	{
		m_pass_impl.swap(other.m_pass_impl);
	}

	TransmittancePass &TransmittancePass::operator=(TransmittancePass &&other) noexcept
	{
		m_pass_impl.swap(other.m_pass_impl);
		return *this;
	}

	TransmittancePass &TransmittancePass::get()
	{
		static TransmittancePass transmittance_pass{};
		return transmittance_pass;
	}

	void TransmittancePass::init(ID3D11Device *device)
	{
		ComputePipelineStateObject pipeline_state_object{
			DXTOY_HOME L"data/pbr/transmittance.hlsl",
			ShaderTargetProfile::ShaderModel_5_0
		};
		m_pass_impl->compute_effect = std::make_unique<ComputeEffect>(pipeline_state_object, device);
		m_pass_impl->transmittance_lut_tex = std::make_unique<Texture2D>(device, m_pass_impl->transmittance_lut_res_x, m_pass_impl->transmittance_lut_res_y, DXGI_FORMAT_R32G32B32A32_FLOAT,
																	1, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS);
		create_texture_uav(device, m_pass_impl->transmittance_lut_tex.get(), 0);
	}

	void TransmittancePass::set_atmosphere_properties(const AtmosphereProperties &in_atmosphere_properties)
	{
		m_pass_impl->atmosphere_properties = in_atmosphere_properties;
	}

	void TransmittancePass::emit_render_pass(ID3D11DeviceContext *device_context)
	{
		using namespace model;
		auto std_atmosphere_properties = m_pass_impl->atmosphere_properties.convert_to_std_uint();
		auto *cb_address = reinterpret_cast<uint8_t *>(std::addressof(std_atmosphere_properties));

		m_pass_impl->compute_effect->set_constant_buffer_upload_data("CBAtmosphereParams", std::span{ cb_address, sizeof(AtmosphereProperties) });
		m_pass_impl->compute_effect->set_unordered_access_view("gTransmittanceMap", m_pass_impl->transmittance_lut_tex->get_unordered_access());
		m_pass_impl->compute_effect->emit_compute_pipeline(device_context);
		m_pass_impl->compute_effect->dispatch(device_context, m_pass_impl->transmittance_lut_res_x, m_pass_impl->transmittance_lut_res_y, 1);

		m_pass_impl->compute_effect->reset_compute_pipeline(device_context);

		TextureManager::get().add_texture(material_semantics_name(MaterialSemantics::TransmittanceLUT), m_pass_impl->transmittance_lut_tex->get_shader_resource());
	}
}
