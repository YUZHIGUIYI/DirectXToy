//
// Created by ZZK on 2024/10/27.
//

#pragma once

#include <Toy/NewFramework/atmosphere_properties.h>
#include <Toy/ECS/camera.h>

namespace toy
{
	struct SkyLUTPass
	{
	public:
		SkyLUTPass();
		~SkyLUTPass() noexcept;

		SkyLUTPass(SkyLUTPass &&other) noexcept;
		SkyLUTPass &operator=(SkyLUTPass &&other) noexcept;

		// * Initialize all resources and pipelines
		void init(ID3D11Device *device);

		// * Set camera position with scaling
		void set_camera(const DirectX::XMFLOAT3 &atmosphere_eye_position);

		// * Set sun direction and intensity
		void set_sun_params(const DirectX::XMFLOAT3 &sun_direction, const DirectX::XMFLOAT3 &sun_intensity);

		// * Set world scale
		void set_world_scale(float world_scale);

		// * set ray marching step count
		void set_ray_marching(int32_t step_count);

		// * Set atmosphere properties
		void set_atmosphere_properties(const AtmosphereProperties &in_atmosphere_properties);

		// * Emit draw
		void emit_render_pass(ID3D11DeviceContext *device_context);

		static SkyLUTPass &get();

	private:
		struct PassImpl;
		std::unique_ptr<PassImpl> m_pass_impl;
	};
}
