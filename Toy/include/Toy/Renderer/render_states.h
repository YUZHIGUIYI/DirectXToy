//
// Created by ZZK on 2023/5/25.
//

#pragma once

#include <Toy/Core/base.h>

namespace toy
{
    class RenderStates
    {
    public:
        static void init(ID3D11Device *device);

        inline static com_ptr<ID3D11RasterizerState> rs_wireframe = nullptr;             // Rasterizer state: wire frame
        inline static com_ptr<ID3D11RasterizerState> rs_no_cull = nullptr;               // Rasterizer state: no back-cull mode
        inline static com_ptr<ID3D11RasterizerState> rs_cull_clock_wise = nullptr;       // Rasterizer state: clock-wise-cull mode
        inline static com_ptr<ID3D11RasterizerState> rs_shadow = nullptr;                // Rasterizer state: depth bias mode

        inline static com_ptr<ID3D11SamplerState> ss_point_clamp = nullptr;              // Sampler state: point filter and clamp mode
        inline static com_ptr<ID3D11SamplerState> ss_linear_wrap = nullptr;              // Sampler state: linear filter and wrap mode
        inline static com_ptr<ID3D11SamplerState> ss_linear_clamp = nullptr;             // Sampler state: linear filter and clamp mode
        inline static com_ptr<ID3D11SamplerState> ss_linear_u_wrap_vw_clamp = nullptr;   // Sampler state: linear filter, warp address mode for u, clamp address mode for v and w
        inline static com_ptr<ID3D11SamplerState> ss_anisotropic_wrap_16x = nullptr;     // Sampler state: 16x anisotropic filter and wrap mode
        inline static com_ptr<ID3D11SamplerState> ss_anisotropic_clamp_2x = nullptr;     // Sampler state: 2x anisotropic filter and clamp mode
        inline static com_ptr<ID3D11SamplerState> ss_anisotropic_clamp_4x = nullptr;     // Sampler state: 4x anisotropic filter and clamp mode
        inline static com_ptr<ID3D11SamplerState> ss_anisotropic_clamp_8x = nullptr;     // Sampler state: 8x anisotropic filter and clamp mode
        inline static com_ptr<ID3D11SamplerState> ss_anisotropic_clamp_16x = nullptr;    // Sampler state: 16x anisotropic filter and clamp mode
        inline static com_ptr<ID3D11SamplerState> ss_shadow_pcf = nullptr;               // Sampler state: Depth comparison and border mode

        inline static com_ptr<ID3D11BlendState> bs_transparent = nullptr;                // Blend state: transparent blend
        inline static com_ptr<ID3D11BlendState> bs_alpha_to_coverage = nullptr;          // Blend state: alpha to coverage
        inline static com_ptr<ID3D11BlendState> bs_additive = nullptr;                   // Blend state: additive state
        inline static com_ptr<ID3D11BlendState> bs_alpha_weighted_additive = nullptr;    // Blend state: alpha weighted additive blend mode

        inline static com_ptr<ID3D11DepthStencilState> dss_equal = nullptr;              // Depth/stencil state: draw pixel which has equal depth value
        inline static com_ptr<ID3D11DepthStencilState> dss_less_equal = nullptr;         // Depth/stencil state: for normal-sky-box draw
        inline static com_ptr<ID3D11DepthStencilState> dss_greater_equal = nullptr;      // Depth/stencil state: for reverse z draw
        inline static com_ptr<ID3D11DepthStencilState> dss_no_depth_write = nullptr;     // Depth/stencil state: only depth test, less or equal, do not write depth value
        inline static com_ptr<ID3D11DepthStencilState> dss_no_depth_test = nullptr;      // Depth/stencil state: close depth test
        inline static com_ptr<ID3D11DepthStencilState> dss_write_stencil = nullptr;      // Depth/stencil state: no depth test, write into stencil value
        inline static com_ptr<ID3D11DepthStencilState> dss_equal_stencil = nullptr;      // Depth/stencil state: reverse z, test stencil value
    };
}
