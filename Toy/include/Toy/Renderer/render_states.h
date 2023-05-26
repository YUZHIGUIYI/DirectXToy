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

        static com_ptr<ID3D11SamplerState> ss_linear_wrap;              // Sampler state: linear filter
        static com_ptr<ID3D11SamplerState> ss_anisotropic_wrap;         // Sampler state: anisotropic filter

        static com_ptr<ID3D11BlendState> bs_no_color_write;             // Blend state: do not write colors
        static com_ptr<ID3D11BlendState> bs_transparent;                // Blend state: transparent blend
        static com_ptr<ID3D11BlendState> bs_alpha_to_coverage;          // Blend state: alpha to coverage
        static com_ptr<ID3D11BlendState> bs_additive;                   // Blend state: additive state

        static com_ptr<ID3D11DepthStencilState> ds_write_stencil;       // Depth/stencil state: write stencil value
        static com_ptr<ID3D11DepthStencilState> ds_draw_with_stencil;   // Depth/stencil state: draw for area with specific stencil value
        static com_ptr<ID3D11DepthStencilState> ds_no_double_blend;     // Depth/stencil state: without twice blend area
        static com_ptr<ID3D11DepthStencilState> ds_no_depth_test;       // Depth/stencil state: close depth test
        static com_ptr<ID3D11DepthStencilState> ds_no_depth_write;      // Depth/stencil state: only depth test, do not write depth value
        static com_ptr<ID3D11DepthStencilState> ds_no_depth_test_with_stencil;  // Depth/stencil state: close depth test, draw for area with specific stencil value
        static com_ptr<ID3D11DepthStencilState> ds_no_depth_write_with_stencil; // Depth/stencil state: only depth test, do not write depth value, draw for area with specific stencil value
    };

    using RenderStates = render_states_c;
}
