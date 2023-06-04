//
// Created by ZZK on 2023/5/25.
//

#pragma once

#include <Toy/Core/base.h>

namespace toy
{
    class render_states_c
    {
    public:
        static void init(ID3D11Device *device);

    public:
        static com_ptr<ID3D11RasterizerState> rs_wireframe;             // Rasterizer state: wire frame
        static com_ptr<ID3D11RasterizerState> rs_no_cull;               // Rasterizer state: no back-cull mode
        static com_ptr<ID3D11RasterizerState> rs_cull_clock_wise;       // Rasterizer state: clock-wise-cull mode
        static com_ptr<ID3D11RasterizerState> rs_shadow;                // Rasterizer state: depth bias mode

        static com_ptr<ID3D11SamplerState> ss_point_clamp;              // Sampler state: point filter and clamp mode
        static com_ptr<ID3D11SamplerState> ss_linear_wrap;              // Sampler state: linear filter and wrap mode
        static com_ptr<ID3D11SamplerState> ss_linear_clamp;             // Sampler state: linear filter and clamp mode
        static com_ptr<ID3D11SamplerState> ss_anisotropic_wrap_16x;     // Sampler state: 16x anisotropic filter and wrap mode
        static com_ptr<ID3D11SamplerState> ss_anisotropic_clamp_2x;     // Sampler state: 2x anisotropic filter and clamp mode
        static com_ptr<ID3D11SamplerState> ss_anisotropic_clamp_4x;     // Sampler state: 4x anisotropic filter and clamp mode
        static com_ptr<ID3D11SamplerState> ss_anisotropic_clamp_8x;     // Sampler state: 8x anisotropic filter and clamp mode
        static com_ptr<ID3D11SamplerState> ss_anisotropic_clamp_16x;    // Sampler state: 16x anisotropic filter and clamp mode
        static com_ptr<ID3D11SamplerState> ss_shadow_pcf;               // Sampler state: Depth comparison and border mode

        static com_ptr<ID3D11BlendState> bs_transparent;                // Blend state: transparent blend
        static com_ptr<ID3D11BlendState> bs_alpha_to_coverage;          // Blend state: alpha to coverage
        static com_ptr<ID3D11BlendState> bs_additive;                   // Blend state: additive state
        static com_ptr<ID3D11BlendState> bs_alpha_weighted_additive;    // Blend state: alpha weighted additive blend mode

        static com_ptr<ID3D11DepthStencilState> dss_equal;              // Depth/stencil state: draw pixel which has equal depth value
        static com_ptr<ID3D11DepthStencilState> dss_less_equal;         // Depth/stencil state: for normal-sky-box draw
        static com_ptr<ID3D11DepthStencilState> dss_greater_equal;      // Depth/stencil state: for reverse z draw
        static com_ptr<ID3D11DepthStencilState> dss_no_depth_write;     // Depth/stencil state: only test, do not write depth value
        static com_ptr<ID3D11DepthStencilState> dss_no_depth_test;      // Depth/stencil state: close depth test
        static com_ptr<ID3D11DepthStencilState> dss_write_stencil;      // Depth/stencil state: no depth test, write into stencil value
        static com_ptr<ID3D11DepthStencilState> dss_equal_stencil;      // Depth/stencil state: reverse z, test stencil value
    };

    using RenderStates = render_states_c;
}
