//
// Created by ZZK on 2024/10/24.
//

#include <Toy/NewFramework/atmosphere_properties.h>
#include <Toy/Core/d3d_util.h>

namespace toy
{
	AtmosphereProperties AtmosphereProperties::convert_to_std_uint() const
	{
		using namespace toy::math;
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
}