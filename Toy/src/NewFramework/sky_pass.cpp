//
// Created by ZZK on 2024/11/3.
//

#include <Toy/NewFramework/sky_pass.h>
#include <Toy/NewFramework/effect.h>
#include <Toy/Renderer/render_states.h>
#include <Toy/Model/texture_manager.h>
#include <Toy/Model/material.h>

#include <Toy/Renderer/texture_2d.h>

namespace toy
{
	struct PSParams
	{
		DirectX::XMFLOAT3 frustum_a{};
		float padding0 = 0.0f;

		DirectX::XMFLOAT3 frustum_b{};
		float padding1 = 0.0f;

		DirectX::XMFLOAT3 frustum_c{};
		float padding2 = 0.0f;

		DirectX::XMFLOAT3 frustum_d{};
		float padding3 = 0.0f;
	};

	struct SkyPass::PassImpl
	{
		std::unique_ptr<GraphicsEffect> graphics_effect = nullptr;
		ID3D11RenderTargetView *render_target_view = nullptr;
		ID3D11DepthStencilView *depth_stencil_view = nullptr;

		PSParams ps_params{};
		D3D11_VIEWPORT render_viewport{};
	};

	SkyPass::SkyPass()
	: m_pass_impl(std::make_unique<PassImpl>())
	{

	}

	SkyPass::~SkyPass() noexcept = default;

	SkyPass::SkyPass(SkyPass &&other) noexcept
	{
		m_pass_impl.swap(other.m_pass_impl);
	}

	SkyPass &SkyPass::operator=(SkyPass &&other) noexcept
	{
		m_pass_impl.swap(other.m_pass_impl);
		return *this;
	}

	SkyPass &SkyPass::get()
	{
		static SkyPass sky_pass{};
		return sky_pass;
	}

	void SkyPass::init(ID3D11Device *device)
	{
		GraphicsPipelineStateObject graphics_pipeline_state_object{};
		graphics_pipeline_state_object.vs_path = DXTOY_HOME L"data/pbr/sky.hlsl";
		graphics_pipeline_state_object.ps_path = DXTOY_HOME L"data/pbr/sky.hlsl";
		graphics_pipeline_state_object.depth_stencil_state = RenderStates::dss_no_depth_write;
		graphics_pipeline_state_object.shader_target_profile = ShaderTargetProfile::ShaderModel_5_0;
		m_pass_impl->graphics_effect = std::make_unique<GraphicsEffect>(graphics_pipeline_state_object, device);
	}

	void SkyPass::set_camera(const Camera &camera)
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

		XMStoreFloat3(&m_pass_impl->ps_params.frustum_a, a_vec3);
		XMStoreFloat3(&m_pass_impl->ps_params.frustum_b, b_vec3);
		XMStoreFloat3(&m_pass_impl->ps_params.frustum_c, c_vec3);
		XMStoreFloat3(&m_pass_impl->ps_params.frustum_d, d_vec3);
	}

	void SkyPass::set_viewport(const D3D11_VIEWPORT &viewport)
	{
		m_pass_impl->render_viewport = viewport;
	}

	void SkyPass::set_output_merger(ID3D11RenderTargetView *render_target_view, ID3D11DepthStencilView *depth_stencil_view)
	{
		m_pass_impl->render_target_view = render_target_view;
		m_pass_impl->depth_stencil_view = depth_stencil_view;
	}

	void SkyPass::emit_render_pass(ID3D11DeviceContext *device_context)
	{
		using namespace model;
		auto &&texture_manager = TextureManager::get();

		DX_CORE_ASSERT(m_pass_impl->render_target_view != nullptr, "Output merger in sky pass should be set");

		auto *cb_ps_address = reinterpret_cast<uint8_t *>(std::addressof(m_pass_impl->ps_params));
		auto *sky_lut_map = texture_manager.get_texture(material_semantics_name(MaterialSemantics::SkyLUT));

		m_pass_impl->graphics_effect->set_constant_buffer_upload_data("CBPSParams", std::span{ cb_ps_address, sizeof(PSParams) });
		m_pass_impl->graphics_effect->set_shader_resource_view("gSkyLutMap", sky_lut_map);
		m_pass_impl->graphics_effect->set_sampler("gSamSkyView", RenderStates::ss_linear_u_wrap_vw_clamp.Get());
		m_pass_impl->graphics_effect->set_render_viewports(std::span{ std::addressof(m_pass_impl->render_viewport), 1 });
		m_pass_impl->graphics_effect->set_render_target_views(std::span{ std::addressof(m_pass_impl->render_target_view), 1 });
		m_pass_impl->graphics_effect->set_depth_stencil_view(m_pass_impl->depth_stencil_view);
		m_pass_impl->graphics_effect->set_primitive_topology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_pass_impl->graphics_effect->emit_graphics_pipeline(device_context);
		m_pass_impl->graphics_effect->draw(device_context, 3, 0);

		m_pass_impl->graphics_effect->reset_graphics_pipeline(device_context);
	}

}