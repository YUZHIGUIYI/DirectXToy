//
// Created by ZZK on 2024/10/27.
//

#pragma once

#include <Toy/NewFramework/atmosphere_properties.h>
#include <Toy/ECS/camera.h>

namespace toy
{
	struct AerialPerspectivePass
	{
	public:
		AerialPerspectivePass();
		~AerialPerspectivePass() noexcept;

		AerialPerspectivePass(AerialPerspectivePass &&other) noexcept;
		AerialPerspectivePass &operator=(AerialPerspectivePass &&other) noexcept;

		// * Initialize all resources and pipelines
		void init(ID3D11Device *device);

		// * Set camera
		void set_camera(const Camera &camera, float world_scale);

		// * Set sun direction
		void set_sun_direction(const DirectX::XMFLOAT3 &sun_direction);

		// * Set shadow map
		void set_shadow_map(const DirectX::XMMATRIX &shadow_view_proj, ID3D11ShaderResourceView *shadow_map);

		// * set marching parameters
		void set_marching_params(float max_distance, int32_t steps_per_slice);

		// * Set atmosphere properties
		void set_atmosphere_properties(const AtmosphereProperties &in_atmosphere_properties);

		// * Emit dispatch
		void emit_render_pass(ID3D11DeviceContext *device_context);

		static AerialPerspectivePass &get();

	private:
		struct PassImpl;
		std::unique_ptr<PassImpl> m_pass_impl;
	};
}
