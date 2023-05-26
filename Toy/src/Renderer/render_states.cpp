//
// Created by ZZK on 2023/5/25.
//

#include <Toy/Renderer/render_states.h>

namespace toy
{
    com_ptr<ID3D11RasterizerState> render_states_c::rs_wireframe = nullptr;
    com_ptr<ID3D11RasterizerState> render_states_c::rs_no_cull = nullptr;
    com_ptr<ID3D11RasterizerState> render_states_c::rs_cull_clock_wise = nullptr;

    com_ptr<ID3D11SamplerState> render_states_c::ss_linear_wrap = nullptr;
    com_ptr<ID3D11SamplerState> render_states_c::ss_anisotropic_wrap = nullptr;

    com_ptr<ID3D11BlendState> render_states_c::bs_no_color_write = nullptr;
    com_ptr<ID3D11BlendState> render_states_c::bs_transparent = nullptr;
    com_ptr<ID3D11BlendState> render_states_c::bs_alpha_to_coverage = nullptr;
    com_ptr<ID3D11BlendState> render_states_c::bs_additive = nullptr;

    com_ptr<ID3D11DepthStencilState> render_states_c::ds_write_stencil = nullptr;
    com_ptr<ID3D11DepthStencilState> render_states_c::ds_draw_with_stencil = nullptr;
    com_ptr<ID3D11DepthStencilState> render_states_c::ds_no_double_blend = nullptr;
    com_ptr<ID3D11DepthStencilState> render_states_c::ds_no_depth_test = nullptr;
    com_ptr<ID3D11DepthStencilState> render_states_c::ds_no_depth_write = nullptr;
    com_ptr<ID3D11DepthStencilState> render_states_c::ds_no_depth_test_with_stencil = nullptr;
    com_ptr<ID3D11DepthStencilState> render_states_c::ds_no_depth_write_with_stencil = nullptr;

    void render_states_c::init(ID3D11Device *device)
    {
        // Initialize rasterizer state
        // Wire frame
        D3D11_RASTERIZER_DESC rasterizer_desc{};
        rasterizer_desc.FillMode = D3D11_FILL_WIREFRAME;
        rasterizer_desc.CullMode = D3D11_CULL_NONE;
        rasterizer_desc.FrontCounterClockwise = false;
        rasterizer_desc.DepthClipEnable = true;
        device->CreateRasterizerState(&rasterizer_desc, rs_wireframe.GetAddressOf());
        // No back-cull mode
        rasterizer_desc.FillMode = D3D11_FILL_SOLID;
        rasterizer_desc.CullMode = D3D11_CULL_NONE;
        rasterizer_desc.FrontCounterClockwise = false;
        rasterizer_desc.DepthClipEnable = true;
        device->CreateRasterizerState(&rasterizer_desc, rs_no_cull.GetAddressOf());
        // Clockwise cull mode
        rasterizer_desc.FillMode = D3D11_FILL_SOLID;
        rasterizer_desc.CullMode = D3D11_CULL_BACK;
        rasterizer_desc.FrontCounterClockwise = true;
        rasterizer_desc.DepthClipEnable = true;
        device->CreateRasterizerState(&rasterizer_desc, rs_cull_clock_wise.GetAddressOf());

        // Initialize sampler state
        D3D11_SAMPLER_DESC sampler_desc{};
        // Linear filter
        sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        sampler_desc.MinLOD = 0.0f;
        sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
        device->CreateSamplerState(&sampler_desc, ss_linear_wrap.GetAddressOf());
        // Anisotropic
        sampler_desc.Filter = D3D11_FILTER_ANISOTROPIC;
        sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        sampler_desc.MaxAnisotropy = 4;
        sampler_desc.MinLOD = 0.0f;
        sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
        device->CreateSamplerState(&sampler_desc, ss_anisotropic_wrap.GetAddressOf());

        // Initialize blend state
        D3D11_BLEND_DESC blend_desc{};
        auto& rt_desc = blend_desc.RenderTarget[0];
        // Alpha to coverage mode
        blend_desc.AlphaToCoverageEnable = true;
        blend_desc.IndependentBlendEnable = false;
        rt_desc.BlendEnable = false;
        rt_desc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        device->CreateBlendState(&blend_desc, bs_alpha_to_coverage.GetAddressOf());
        // Transparent mode
        // Color = SrcAlpha * SrcColor + (1 - SrcAlpha) * DestColor
        // Alpha = SrcAlpha
        blend_desc.AlphaToCoverageEnable = false;
        blend_desc.IndependentBlendEnable = false;
        rt_desc.BlendEnable = true;
        rt_desc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
        rt_desc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        rt_desc.BlendOp = D3D11_BLEND_OP_ADD;
        rt_desc.SrcBlendAlpha = D3D11_BLEND_ONE;
        rt_desc.DestBlendAlpha = D3D11_BLEND_ZERO;
        rt_desc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
        device->CreateBlendState(&blend_desc, bs_transparent.GetAddressOf());
        // Additive mode
        rt_desc.SrcBlend = D3D11_BLEND_ONE;
        rt_desc.DestBlend = D3D11_BLEND_ONE;
        rt_desc.BlendOp = D3D11_BLEND_OP_ADD;
        rt_desc.SrcBlendAlpha = D3D11_BLEND_ONE;
        rt_desc.DestBlendAlpha = D3D11_BLEND_ZERO;
        rt_desc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
        device->CreateBlendState(&blend_desc, bs_additive.GetAddressOf());
        // Do-not-write-color mode
        // Color = DestColor
        // Alpha = DestAlpha
        rt_desc.BlendEnable = false;
        rt_desc.SrcBlend = D3D11_BLEND_ZERO;
        rt_desc.DestBlend = D3D11_BLEND_ONE;
        rt_desc.BlendOp = D3D11_BLEND_OP_ADD;
        rt_desc.SrcBlendAlpha = D3D11_BLEND_ZERO;
        rt_desc.DestBlendAlpha = D3D11_BLEND_ONE;
        rt_desc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
        rt_desc.RenderTargetWriteMask = 0;
        device->CreateBlendState(&blend_desc, bs_no_color_write.GetAddressOf());

        // Initialize depth/stencil state
        D3D11_DEPTH_STENCIL_DESC ds_desc{};
        // Depth/stencil state: write stencil value
        // Do not write depth information
        // Both of front and back faces of specific area will be written StencilRef
        ds_desc.DepthEnable = true;
        ds_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        ds_desc.DepthFunc = D3D11_COMPARISON_LESS;

        ds_desc.StencilEnable = true;
        ds_desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
        ds_desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

        ds_desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        ds_desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
        ds_desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
        ds_desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
        // For back face geometry, since we do not render, so it doesn't matter whatever we set
        ds_desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        ds_desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
        ds_desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
        ds_desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
        device->CreateDepthStencilState(&ds_desc, ds_write_stencil.GetAddressOf());

        // Depth/stencil state: draw for area with specific stencil value
        // For area equals to specific stencil value, draw, and update depth value
        ds_desc.DepthEnable = true;
        ds_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        ds_desc.DepthFunc = D3D11_COMPARISON_LESS;

        ds_desc.StencilEnable = true;
        ds_desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
        ds_desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

        ds_desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        ds_desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
        ds_desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        ds_desc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
        // For back face geometry, since we do not render, so it doesn't matter whatever we set
        ds_desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        ds_desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
        ds_desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        ds_desc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;
        device->CreateDepthStencilState(&ds_desc, ds_draw_with_stencil.GetAddressOf());

        // Depth/stencil state: without twice blend
        // Allow default depth test
        // By auto-increasing, the StencilRef value can only be used once, so that achieves only one blending
        ds_desc.DepthEnable = true;
        ds_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        ds_desc.DepthFunc = D3D11_COMPARISON_LESS;

        ds_desc.StencilEnable = true;
        ds_desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
        ds_desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

        ds_desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        ds_desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
        ds_desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
        ds_desc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
        // For back face geometry, since we do not render, so it doesn't matter whatever we set
        ds_desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        ds_desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
        ds_desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
        ds_desc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;
        device->CreateDepthStencilState(&ds_desc, ds_no_double_blend.GetAddressOf());

        // Depth/stencil state: close depth test
        // When draw opaque objects, must obey draw-order
        // While transparent objects do not matter
        // Stencil test is closed by default
        ds_desc.DepthEnable = false;
        ds_desc.StencilEnable = false;
        device->CreateDepthStencilState(&ds_desc, ds_no_depth_test.GetAddressOf());

        // Depth/stencil state: close depth test, draw for area with specific stencil value
        // When draw opaque objects, must obey draw-order
        // While transparent objects do not matter
        // Only area equals to specific stencil value will be rendered
        ds_desc.StencilEnable = true;
        ds_desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
        ds_desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

        ds_desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        ds_desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
        ds_desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        ds_desc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
        // For back face geometry, since we do not render, so it doesn't matter whatever we set
        ds_desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        ds_desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
        ds_desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        ds_desc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;
        device->CreateDepthStencilState(&ds_desc, ds_no_depth_test_with_stencil.GetAddressOf());

        // Depth/stencil state: only depth test, do not write depth value
        // For opaque objects, use default state to draw
        // When draw transparent objects, use this state will make blend state well
        // Opaque objects can block other objects behind them
        ds_desc.DepthEnable = true;
        ds_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        ds_desc.DepthFunc = D3D11_COMPARISON_LESS;
        ds_desc.StencilEnable = false;
        device->CreateDepthStencilState(&ds_desc, ds_no_depth_write.GetAddressOf());

        // Depth/stencil state: only depth test, do not write depth value, draw for area with specific stencil value
        // For opaque objects, use default state to draw
        // When draw transparent objects, use this state will make blend state well
        // Opaque objects can block other objects behind them
        // Draw for area that equals to specific stencil value
        ds_desc.StencilEnable = true;
        ds_desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
        ds_desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

        ds_desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        ds_desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
        ds_desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        ds_desc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
        // For back face geometry, since we do not render, so it doesn't matter whatever we set
        ds_desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        ds_desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
        ds_desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        ds_desc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;
        device->CreateDepthStencilState(&ds_desc, ds_no_depth_write_with_stencil.GetAddressOf());

        // Set debug object names
        // TODO
    }
}








