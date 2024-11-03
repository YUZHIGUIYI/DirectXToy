//
// Created by ZZK on 2024/10/27.
//

#include <Toy/NewFramework/sky_lut_pass.h>
#include <Toy/NewFramework/effect.h>
#include <Toy/Renderer/render_states.h>
#include <Toy/Model/texture_manager.h>
#include <Toy/Model/material.h>

#include <Toy/Renderer/texture_2d.h>

namespace toy
{
	struct PSParams
	{
		DirectX::XMFLOAT3 atmosphere_eye_position{};
		int32_t low_res_march_step_count = 0;

		DirectX::XMFLOAT3 sun_direction{};
		int32_t enable_multi_scattering = 0;

		DirectX::XMFLOAT3 sun_intensity{};
		float padding = 0.0f;
	};

	struct SkyLUTPass::PassImpl
	{
		std::unique_ptr<GraphicsEffect> graphics_effect = nullptr;
		std::unique_ptr<Texture2D> sky_lut_tex = nullptr;

		AtmosphereProperties atmosphere_properties{};
		PSParams ps_params{};
		float world_scale = 1.0f;
		uint32_t sky_lut_res_x = 64;
		uint32_t sky_lut_res_y = 64;
	};

	SkyLUTPass::SkyLUTPass()
	: m_pass_impl(std::make_unique<PassImpl>())
	{

	}

	SkyLUTPass::~SkyLUTPass() noexcept = default;

	SkyLUTPass::SkyLUTPass(SkyLUTPass &&other) noexcept
	{
		m_pass_impl.swap(other.m_pass_impl);
	}

	SkyLUTPass &SkyLUTPass::operator=(SkyLUTPass &&other) noexcept
	{
		m_pass_impl.swap(other.m_pass_impl);
		return *this;
	}

	SkyLUTPass &SkyLUTPass::get()
	{
		static SkyLUTPass sky_pass{};
		return sky_pass;
	}

	void SkyLUTPass::init(ID3D11Device *device)
	{
		GraphicsPipelineStateObject graphics_pipeline_state_object{};
		graphics_pipeline_state_object.vs_path = DXTOY_HOME L"data/pbr/sky_lut.hlsl";
		graphics_pipeline_state_object.ps_path = DXTOY_HOME L"data/pbr/sky_lut.hlsl";
		graphics_pipeline_state_object.shader_target_profile = ShaderTargetProfile::ShaderModel_5_0;
		m_pass_impl->graphics_effect = std::make_unique<GraphicsEffect>(graphics_pipeline_state_object, device);
		m_pass_impl->sky_lut_tex = std::make_unique<Texture2D>(device, m_pass_impl->sky_lut_res_x, m_pass_impl->sky_lut_res_y, DXGI_FORMAT_R32G32B32A32_FLOAT, 1);
	}

	void SkyLUTPass::set_camera(const DirectX::XMFLOAT3 &atmosphere_eye_position)
	{
		m_pass_impl->ps_params.atmosphere_eye_position = atmosphere_eye_position;
	}

	void SkyLUTPass::set_world_scale(float world_scale)
	{
		m_pass_impl->world_scale = world_scale;
	}

	void SkyLUTPass::set_ray_marching(int32_t step_count)
	{
		m_pass_impl->ps_params.low_res_march_step_count = step_count;
	}

	void SkyLUTPass::set_atmosphere_properties(const AtmosphereProperties &in_atmosphere_properties)
	{
		m_pass_impl->atmosphere_properties = in_atmosphere_properties;
	}

	void SkyLUTPass::set_sun_params(const DirectX::XMFLOAT3 &sun_direction, const DirectX::XMFLOAT3 &sun_intensity)
	{
		m_pass_impl->ps_params.sun_direction = sun_direction;
		m_pass_impl->ps_params.sun_intensity = sun_intensity;
	}

	void SkyLUTPass::emit_render_pass(ID3D11DeviceContext *device_context)
	{
		using namespace model;
		auto &&texture_manager = TextureManager::get();

		D3D11_VIEWPORT viewport{ 0.0f, 0.0f, static_cast<float>(m_pass_impl->sky_lut_res_x), static_cast<float>(m_pass_impl->sky_lut_res_y), 0.0f, 1.0f };
		auto std_atmosphere_properties = m_pass_impl->atmosphere_properties.convert_to_std_uint();
		auto *cb_atmosphere_address = reinterpret_cast<uint8_t *>(std::addressof(std_atmosphere_properties));
		auto *cb_ps_address = reinterpret_cast<uint8_t *>(std::addressof(m_pass_impl->ps_params));
		auto *transmittance_map = texture_manager.get_texture(material_semantics_name(MaterialSemantics::TransmittanceLUT));
		auto *multi_scattering_map = texture_manager.get_texture(material_semantics_name(MaterialSemantics::MultiScatteringLUT));
		auto sky_lut_rtv = m_pass_impl->sky_lut_tex->get_render_target();

		m_pass_impl->graphics_effect->set_constant_buffer_upload_data("CBAtmosphereParams", std::span{ cb_atmosphere_address, sizeof(AtmosphereProperties) });
		m_pass_impl->graphics_effect->set_constant_buffer_upload_data("CBPSParams", std::span{ cb_ps_address, sizeof(PSParams) });
		m_pass_impl->graphics_effect->set_shader_resource_view("gTransmittanceMap", transmittance_map);
		m_pass_impl->graphics_effect->set_shader_resource_view("gMultiScatteringMap", multi_scattering_map);
		m_pass_impl->graphics_effect->set_sampler("gSamMT", RenderStates::ss_linear_clamp.Get());
		m_pass_impl->graphics_effect->set_render_viewports(std::span{ std::addressof(viewport), 1 });
		m_pass_impl->graphics_effect->set_render_target_views(std::span{ std::addressof(sky_lut_rtv), 1 });
		m_pass_impl->graphics_effect->set_primitive_topology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_pass_impl->graphics_effect->emit_graphics_pipeline(device_context);
		m_pass_impl->graphics_effect->draw(device_context, 3, 0);

		m_pass_impl->graphics_effect->reset_graphics_pipeline(device_context);

		texture_manager.add_texture(material_semantics_name(MaterialSemantics::SkyLUT), m_pass_impl->sky_lut_tex->get_shader_resource());
	}

}