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
	DirectX::XMFLOAT3 operator*(const float scale_factor, const DirectX::XMFLOAT3 &input)
	{
		return DirectX::XMFLOAT3{ scale_factor * input.x, scale_factor * input.y, scale_factor * input.z };
	}

	struct AtmosphereProperties
	{
		DirectX::XMFLOAT3 scatter_rayleigh = { 5.802f, 13.558f, 33.1f };
		float h_density_rayleigh = 8.0f;

		float scatter_mie = 3.996f;
		float asymmetry_mie = 0.8f;
		float absorb_mie = 4.4f;
		float h_density_mie = 1.2f;

		DirectX::XMFLOAT3 absorb_ozone = { 0.65f, 1.881f, 0.085f };
		float ozone_center_height = 25.0f;

		float ozone_thickness = 30.0f;
		float planet_radius = 6360.0f;
		float atmosphere_radius = 6460.0f;
		float padding = 0.0f;

		AtmosphereProperties convert_to_std_uint() const
		{
			AtmosphereProperties std_atmosphere_properties{};
			std_atmosphere_properties.scatter_rayleigh    = 1e-6f * this->scatter_rayleigh;
			std_atmosphere_properties.h_density_rayleigh  = 1e3f  * this->h_density_rayleigh;
			std_atmosphere_properties.scatter_mie         = 1e-6f * this->scatter_mie;
			std_atmosphere_properties.absorb_mie          = 1e-6f * this->absorb_mie;
			std_atmosphere_properties.h_density_mie       = 1e3f  * this->h_density_mie;
			std_atmosphere_properties.absorb_ozone        = 1e-6f * this->absorb_ozone;
			std_atmosphere_properties.ozone_center_height = 1e3f  * this->ozone_center_height;
			std_atmosphere_properties.ozone_thickness     = 1e3f  * this->ozone_thickness;
			std_atmosphere_properties.planet_radius       = 1e3f  * this->planet_radius;
			std_atmosphere_properties.atmosphere_radius   = 1e3f  * this->atmosphere_radius;
			return std_atmosphere_properties;
		}
	};

	struct TransmittancePass::PassImpl
	{
		std::unique_ptr<ComputeEffect> compute_effect = nullptr;
		std::unique_ptr<Texture2D> transmittance_lut_tex = nullptr;

		// TODO: add
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
																	1, D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE);
		create_texture_uav(device, m_pass_impl->transmittance_lut_tex.get(), 0);
	}

	void TransmittancePass::emit_render_pass(ID3D11DeviceContext *device_context)
	{
		auto std_atmosphere_properties = m_pass_impl->atmosphere_properties.convert_to_std_uint();
		auto *cb_address = reinterpret_cast<uint8_t *>(std::addressof(std_atmosphere_properties));
		m_pass_impl->compute_effect->set_constant_buffer_upload_data("CBAtmosphereParams", std::span{ cb_address, sizeof(AtmosphereProperties) });
		m_pass_impl->compute_effect->set_unordered_access_view("gTransmittanceMap", m_pass_impl->transmittance_lut_tex->get_unordered_access());
		m_pass_impl->compute_effect->emit_compute_pipeline(device_context);
		m_pass_impl->compute_effect->dispatch(device_context, m_pass_impl->transmittance_lut_res_x, m_pass_impl->transmittance_lut_res_y, 1);
	}

}
