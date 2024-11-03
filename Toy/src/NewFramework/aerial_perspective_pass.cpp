//
// Created by ZZK on 2024/10/27.
//

#include <Toy/NewFramework/aerial_perspective_pass.h>
#include <Toy/NewFramework/effect.h>
#include <Toy/Renderer/render_states.h>
#include <Toy/Model/texture_manager.h>
#include <Toy/Model/material.h>
#include <Toy/ECS/components.h>

#include <Toy/Renderer/texture_2d.h>

namespace toy
{
	struct CSParams
	{
		DirectX::XMFLOAT3 sun_direction{};
		float sun_theta = 0.0f;

		DirectX::XMFLOAT3 frustum_a{};
		float max_distance = 0.0f;

		DirectX::XMFLOAT3 frustum_b{};
		int32_t per_slice_step_count = 0;

		DirectX::XMFLOAT3 frustum_c{};
		int32_t enable_multi_scattering = 1;

		DirectX::XMFLOAT3 frustum_d{};
		float eye_position_y = 0.0f;

		DirectX::XMFLOAT3 shadow_eye_position{};
		int32_t enable_shadow = 0;

		DirectX::XMMATRIX shadow_view_proj_matrix{};

		float world_scale = 1.0f;
		DirectX::XMFLOAT3 float3_padding{};
	};

	struct AerialPerspectivePass::PassImpl
	{
		std::unique_ptr<ComputeEffect> compute_effect = nullptr;
		std::unique_ptr<Texture3D> aerial_perspective_lut_tex = nullptr;
		ComPtr<ID3D11UnorderedAccessView> aerial_perspective_lut_uav = nullptr;

		AtmosphereProperties atmosphere_properties{};
		CSParams cs_params{};
		uint32_t aerial_perspective_lut_res_x = 200;
		uint32_t aerial_perspective_lut_res_y = 150;
		uint32_t aerial_perspective_lut_res_z = 32;
	};

	AerialPerspectivePass::AerialPerspectivePass()
	: m_pass_impl(std::make_unique<PassImpl>())
	{

	}

	AerialPerspectivePass::~AerialPerspectivePass() noexcept = default;

	AerialPerspectivePass::AerialPerspectivePass(AerialPerspectivePass &&other) noexcept
	{
		m_pass_impl.swap(other.m_pass_impl);
	}

	AerialPerspectivePass &AerialPerspectivePass::operator=(AerialPerspectivePass &&other) noexcept
	{
		m_pass_impl.swap(other.m_pass_impl);
		return *this;
	}

	AerialPerspectivePass &AerialPerspectivePass::get()
	{
		static AerialPerspectivePass aerial_perspective_pass{};
		return aerial_perspective_pass;
	}

	void AerialPerspectivePass::init(ID3D11Device *device)
	{
		ComputePipelineStateObject pipeline_state_object{
			DXTOY_HOME L"data/pbr/aerial_lut.hlsl",
			ShaderTargetProfile::ShaderModel_5_0
		};
		m_pass_impl->compute_effect = std::make_unique<ComputeEffect>(pipeline_state_object, device);
		m_pass_impl->aerial_perspective_lut_tex = std::make_unique<Texture3D>(device, m_pass_impl->aerial_perspective_lut_res_x, m_pass_impl->aerial_perspective_lut_res_y,
													m_pass_impl->aerial_perspective_lut_res_z, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE);
		m_pass_impl->aerial_perspective_lut_uav = m_pass_impl->aerial_perspective_lut_tex->create_unordered_access(device);
	}

	void AerialPerspectivePass::set_camera(const Camera &camera, float world_scale)
	{
		// TODO: fix
		using namespace DirectX;
		auto view_proj_matrix = camera.get_view_proj_xm();
		auto inv_view_proj_matrix = DirectX::XMMatrixInverse(nullptr, view_proj_matrix);
		auto a0_vec4 = XMVectorSet(-1.0f, 1.0f, 0.2f, 1.0f);
		auto a1_vec4 = XMVectorSet(-1.0f, 1.0f, 0.5f, 1.0f);
		auto b0_vec4 = XMVectorSet(1.0f, 1.0f, 0.2f, 1.0f);
		auto b1_vec4 = XMVectorSet(1.0f, 1.0f, 0.5f, 1.0f);
		auto c0_vec4 = XMVectorSet(-1.0f, -1.0f, 0.2f, 1.0f);
		auto c1_vec4 = XMVectorSet(-1.0f, -1.0f, 0.5f, 1.0f);
		auto d0_vec4 = XMVectorSet(1.0f, -1.0f, 0.2f, 1.0f);
		auto d1_vec4 = XMVectorSet(1.0f, -1.0f, 0.5f, 1.0f);
		a0_vec4 = XMVector4Transform(a0_vec4, inv_view_proj_matrix);
		a1_vec4 = XMVector4Transform(a1_vec4, inv_view_proj_matrix);
		b0_vec4 = XMVector4Transform(b0_vec4, inv_view_proj_matrix);
		b1_vec4 = XMVector4Transform(b1_vec4, inv_view_proj_matrix);
		c0_vec4 = XMVector4Transform(c0_vec4, inv_view_proj_matrix);
		c1_vec4 = XMVector4Transform(c1_vec4, inv_view_proj_matrix);
		d0_vec4 = XMVector4Transform(d0_vec4, inv_view_proj_matrix);
		d1_vec4 = XMVector4Transform(d1_vec4, inv_view_proj_matrix);

		a0_vec4 /= a0_vec4.m128_f32[3];
		a1_vec4 /= a1_vec4.m128_f32[3];
		b0_vec4 /= b0_vec4.m128_f32[3];
		b1_vec4 /= b1_vec4.m128_f32[3];
		c0_vec4 /= c0_vec4.m128_f32[3];
		c1_vec4 /= c1_vec4.m128_f32[3];
		d0_vec4 /= d0_vec4.m128_f32[3];
		d1_vec4 /= d1_vec4.m128_f32[3];

		auto a_vec3 = XMVector3Normalize(a1_vec4 - a0_vec4);
		auto b_vec3 = XMVector3Normalize(b1_vec4 - b0_vec4);
		auto c_vec3 = XMVector3Normalize(c1_vec4 - c0_vec4);
		auto d_vec3 = XMVector3Normalize(d1_vec4 - d0_vec4);

		XMStoreFloat3(&m_pass_impl->cs_params.frustum_a, a_vec3);
		XMStoreFloat3(&m_pass_impl->cs_params.frustum_b, b_vec3);
		XMStoreFloat3(&m_pass_impl->cs_params.frustum_c, c_vec3);
		XMStoreFloat3(&m_pass_impl->cs_params.frustum_d, d_vec3);

		const auto eye_position = camera.get_position();
		m_pass_impl->cs_params.eye_position_y = world_scale * eye_position.y;
		m_pass_impl->cs_params.shadow_eye_position = eye_position;
		m_pass_impl->cs_params.world_scale = world_scale;
	}

	void AerialPerspectivePass::set_sun_direction(const DirectX::XMFLOAT3 &sun_direction)
	{
		m_pass_impl->cs_params.sun_direction = sun_direction;
		m_pass_impl->cs_params.sun_theta = std::asin(-sun_direction.y);
	}

	void AerialPerspectivePass::set_shadow_map(const DirectX::XMMATRIX &shadow_view_proj, ID3D11ShaderResourceView *shadow_map)
	{
		// TODO: support shadow map
		m_pass_impl->cs_params.enable_shadow = 0;
	}

	void AerialPerspectivePass::set_marching_params(float max_distance, int32_t steps_per_slice)
	{
		m_pass_impl->cs_params.max_distance = max_distance;
		m_pass_impl->cs_params.per_slice_step_count = steps_per_slice;
	}

	void AerialPerspectivePass::set_atmosphere_properties(const AtmosphereProperties &in_atmosphere_properties)
	{
		m_pass_impl->atmosphere_properties = in_atmosphere_properties;
	}

	void AerialPerspectivePass::emit_render_pass(ID3D11DeviceContext *device_context)
	{
		using namespace model;
		auto &&texture_manager = TextureManager::get();

		auto std_atmosphere_properties = m_pass_impl->atmosphere_properties.convert_to_std_uint();
		auto *cb_atmosphere_address = reinterpret_cast<uint8_t *>(std::addressof(std_atmosphere_properties));
		auto *cb_cs_address = reinterpret_cast<uint8_t *>(std::addressof(m_pass_impl->cs_params));
		auto *transmittance_map = texture_manager.get_texture(material_semantics_name(MaterialSemantics::TransmittanceLUT));
		auto *multi_scattering_map = texture_manager.get_texture(material_semantics_name(MaterialSemantics::MultiScatteringLUT));

		m_pass_impl->compute_effect->set_constant_buffer_upload_data("CBAtmosphereParams", std::span{ cb_atmosphere_address, sizeof(AtmosphereProperties) });
		m_pass_impl->compute_effect->set_constant_buffer_upload_data("CBCSParams", std::span{ cb_cs_address, sizeof(CSParams) });
		m_pass_impl->compute_effect->set_shader_resource_view("gMultiScatteringMap", multi_scattering_map);
		m_pass_impl->compute_effect->set_shader_resource_view("gTransmittanceMap", transmittance_map);
		m_pass_impl->compute_effect->set_shader_resource_view("gShadowMap", nullptr);
		m_pass_impl->compute_effect->set_unordered_access_view("gAerialPerspectiveLUT", m_pass_impl->aerial_perspective_lut_uav.Get());
		m_pass_impl->compute_effect->set_sampler("gSamMT", RenderStates::ss_linear_clamp.Get());
		m_pass_impl->compute_effect->set_sampler("gSamShadow", RenderStates::ss_point_clamp.Get());
		m_pass_impl->compute_effect->emit_compute_pipeline(device_context);
		m_pass_impl->compute_effect->dispatch(device_context, m_pass_impl->aerial_perspective_lut_res_x, m_pass_impl->aerial_perspective_lut_res_y, 1);

		m_pass_impl->compute_effect->reset_compute_pipeline(device_context);

		texture_manager.add_texture(material_semantics_name(MaterialSemantics::AerialPerspectiveLUT), m_pass_impl->aerial_perspective_lut_tex->get_shader_resource());
	}

}