//
// Created by ZZK on 2024/11/3.
//

#pragma once

#include <Toy/NewFramework/atmosphere_properties.h>
#include <Toy/ECS/camera.h>

namespace toy
{
	struct SkyPass
	{
	public:
		SkyPass();
		~SkyPass() noexcept;

		SkyPass(SkyPass &&other) noexcept;
		SkyPass &operator=(SkyPass &&other) noexcept;

		// * Initialize all resources and pipelines
		void init(ID3D11Device *device);

		// * Set camera frustum directions
		void set_camera(const Camera &camera);

		// * Set render area
		void set_viewport(const D3D11_VIEWPORT &viewport);

		// * Set render target view and depth stencil view
		void set_output_merger(ID3D11RenderTargetView *render_target_view, ID3D11DepthStencilView *depth_stencil_view);

		// * Emit draw
		void emit_render_pass(ID3D11DeviceContext *device_context);

		static SkyPass &get();

	private:
		struct PassImpl;
		std::unique_ptr<PassImpl> m_pass_impl;
	};
}
