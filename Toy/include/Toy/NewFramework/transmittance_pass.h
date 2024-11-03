//
// Created by ZZK on 2024/10/21.
//

#pragma once

#include <Toy/NewFramework/effect.h>
#include <Toy/NewFramework/atmosphere_properties.h>

namespace toy
{
	struct TransmittancePass
	{
	public:
		TransmittancePass();
		~TransmittancePass() noexcept;

		TransmittancePass(TransmittancePass &&other) noexcept;
		TransmittancePass &operator=(TransmittancePass &&other) noexcept;

		// * Initialize all resources and pipelines
		void init(ID3D11Device *device);

		// * Set atmosphere properties
		void set_atmosphere_properties(const AtmosphereProperties &in_atmosphere_properties);

		// * Emit dispatch
		void emit_render_pass(ID3D11DeviceContext *device_context);

		static TransmittancePass &get();

	private:
		struct PassImpl;
		std::unique_ptr<PassImpl> m_pass_impl;
	};
}
