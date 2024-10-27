//
// Created by ZZK on 2024/10/24.
//

#pragma once

#include <Toy/Core/base.h>

namespace toy
{
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

		[[nodiscard]] AtmosphereProperties convert_to_std_uint() const;
	};
}
