//
// Created by ZZK on 2023/5/25.
//

#include <Toy/Renderer/render_states.h>

namespace toy
{
    com_ptr<ID3D11RasterizerState> render_states_c::rs_wireframe = nullptr;
    com_ptr<ID3D11RasterizerState> render_states_c::rs_no_cull = nullptr;
    com_ptr<ID3D11RasterizerState> render_states_c::rs_cull_clock_wise = nullptr;
    com_ptr<ID3D11RasterizerState> render_states_c::rs_shadow = nullptr;

    com_ptr<ID3D11SamplerState> render_states_c::ss_point_clamp = nullptr;
    com_ptr<ID3D11SamplerState> render_states_c::ss_linear_wrap = nullptr;
    com_ptr<ID3D11SamplerState> render_states_c::ss_linear_clamp = nullptr;
    com_ptr<ID3D11SamplerState> render_states_c::ss_anisotropic_wrap_16x = nullptr;
    com_ptr<ID3D11SamplerState> render_states_c::ss_anisotropic_clamp_2x = nullptr;
    com_ptr<ID3D11SamplerState> render_states_c::ss_anisotropic_clamp_4x = nullptr;
    com_ptr<ID3D11SamplerState> render_states_c::ss_anisotropic_clamp_8x = nullptr;
    com_ptr<ID3D11SamplerState> render_states_c::ss_anisotropic_clamp_16x = nullptr;
    com_ptr<ID3D11SamplerState> render_states_c::ss_shadow_pcf = nullptr;

    com_ptr<ID3D11BlendState> render_states_c::bs_alpha_weighted_additive = nullptr;
    com_ptr<ID3D11BlendState> render_states_c::bs_transparent = nullptr;
    com_ptr<ID3D11BlendState> render_states_c::bs_alpha_to_coverage = nullptr;
    com_ptr<ID3D11BlendState> render_states_c::bs_additive = nullptr;

    com_ptr<ID3D11DepthStencilState> render_states_c::dss_equal = nullptr;
    com_ptr<ID3D11DepthStencilState> render_states_c::dss_less_equal = nullptr;
    com_ptr<ID3D11DepthStencilState> render_states_c::dss_greater_equal = nullptr;
    com_ptr<ID3D11DepthStencilState> render_states_c::dss_no_depth_write = nullptr;
    com_ptr<ID3D11DepthStencilState> render_states_c::dss_no_depth_test = nullptr;
    com_ptr<ID3D11DepthStencilState> render_states_c::dss_write_stencil = nullptr;
    com_ptr<ID3D11DepthStencilState> render_states_c::dss_equal_stencil = nullptr;

    void render_states_c::init(ID3D11Device *device)
    {
        // Initialize rasterizer state
        // Wire frame
        CD3D11_RASTERIZER_DESC rasterizer_desc{CD3D11_DEFAULT{}};
        rasterizer_desc.FillMode = D3D11_FILL_WIREFRAME;
        rasterizer_desc.CullMode = D3D11_CULL_NONE;
        device->CreateRasterizerState(&rasterizer_desc, rs_wireframe.GetAddressOf());
        // No back-cull mode
        rasterizer_desc.FillMode = D3D11_FILL_SOLID;
        rasterizer_desc.CullMode = D3D11_CULL_NONE;
        rasterizer_desc.FrontCounterClockwise = false;
        device->CreateRasterizerState(&rasterizer_desc, rs_no_cull.GetAddressOf());
        // Clockwise cull mode
        rasterizer_desc.FillMode = D3D11_FILL_SOLID;
        rasterizer_desc.CullMode = D3D11_CULL_BACK;
        rasterizer_desc.FrontCounterClockwise = true;
        device->CreateRasterizerState(&rasterizer_desc, rs_cull_clock_wise.GetAddressOf());
        // Depth bias mode
        rasterizer_desc.FillMode = D3D11_FILL_SOLID;
        rasterizer_desc.CullMode = D3D11_CULL_NONE;
        rasterizer_desc.FrontCounterClockwise = false;
        rasterizer_desc.DepthBias = 0;
        rasterizer_desc.DepthBiasClamp = 0.0f;
        rasterizer_desc.SlopeScaledDepthBias = 1.0f;
        device->CreateRasterizerState(&rasterizer_desc, rs_shadow.GetAddressOf());

        // Initialize sampler state
        CD3D11_SAMPLER_DESC sampler_desc{CD3D11_DEFAULT{}};
        // Point filter and clamp mode
        sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        device->CreateSamplerState(&sampler_desc, ss_point_clamp.GetAddressOf());
        // Linear filter and clamp mode
        sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        device->CreateSamplerState(&sampler_desc, ss_linear_clamp.GetAddressOf());
        // 2x anisotropic filter and clamp mode
        sampler_desc.Filter = D3D11_FILTER_ANISOTROPIC;
        sampler_desc.MaxAnisotropy = 2;
        device->CreateSamplerState(&sampler_desc, ss_anisotropic_clamp_2x.GetAddressOf());
        // 4x anisotropic filter and clamp mode
        sampler_desc.MaxAnisotropy = 4;
        device->CreateSamplerState(&sampler_desc, ss_anisotropic_clamp_4x.GetAddressOf());
        // 8x anisotropic filter and clamp mode
        sampler_desc.MaxAnisotropy = 8;
        device->CreateSamplerState(&sampler_desc, ss_anisotropic_clamp_8x.GetAddressOf());
        // 16x anisotropic filter and clamp mode
        sampler_desc.MaxAnisotropy = 16;
        device->CreateSamplerState(&sampler_desc, ss_anisotropic_clamp_16x.GetAddressOf());

        // Linear filter and wrap mode
        sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        sampler_desc.MaxAnisotropy = 0;
        sampler_desc.MinLOD = 0.0f;
        sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
        device->CreateSamplerState(&sampler_desc, ss_linear_wrap.GetAddressOf());

        // 16x anisotropic filter and wrap mode
        sampler_desc.Filter = D3D11_FILTER_ANISOTROPIC;
        sampler_desc.MaxAnisotropy = 16;
        device->CreateSamplerState(&sampler_desc, ss_anisotropic_wrap_16x.GetAddressOf());

        // Depth comparison and border mode
        sampler_desc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
        sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
        sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
        sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
        sampler_desc.ComparisonFunc = D3D11_COMPARISON_LESS;
        sampler_desc.MaxAnisotropy = 1;
        sampler_desc.MinLOD = 0.0f;
        sampler_desc.MaxLOD = 0.0f;
        sampler_desc.BorderColor[0] = 0.0f;
        sampler_desc.BorderColor[1] = 0.0f;
        sampler_desc.BorderColor[2] = 0.0f;
        sampler_desc.BorderColor[3] = 1.0f;
        device->CreateSamplerState(&sampler_desc, ss_shadow_pcf.GetAddressOf());

        // Initialize blend state
        CD3D11_BLEND_DESC blend_desc{CD3D11_DEFAULT{}};
        auto& rt_desc = blend_desc.RenderTarget[0];

        // Alpha to coverage mode
        blend_desc.AlphaToCoverageEnable = true;
        device->CreateBlendState(&blend_desc, bs_alpha_to_coverage.GetAddressOf());
        // Transparent mode
        // Color = SrcAlpha * SrcColor + (1 - SrcAlpha) * DestColor
        // Alpha = SrcAlpha
        blend_desc.AlphaToCoverageEnable = false;
        rt_desc.BlendEnable = true;
        rt_desc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
        rt_desc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        rt_desc.BlendOp = D3D11_BLEND_OP_ADD;
        rt_desc.SrcBlendAlpha = D3D11_BLEND_ONE;
        rt_desc.DestBlendAlpha = D3D11_BLEND_ZERO;
        rt_desc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
        device->CreateBlendState(&blend_desc, bs_transparent.GetAddressOf());

        // Additive mode
        // Color = SrcColor + DestColor
        // Alpha = SrcAlpha + DestAlpha
        rt_desc.SrcBlend = D3D11_BLEND_ONE;
        rt_desc.DestBlend = D3D11_BLEND_ONE;
        rt_desc.BlendOp = D3D11_BLEND_OP_ADD;
        rt_desc.SrcBlendAlpha = D3D11_BLEND_ONE;
        rt_desc.DestBlendAlpha = D3D11_BLEND_ONE;
        rt_desc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
        device->CreateBlendState(&blend_desc, bs_additive.GetAddressOf());

        // Weighted additive mode
        // Color = SrcAlpha * SrcColor + DestColor
        // Alpha = SrcAlpha
        rt_desc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
        rt_desc.DestBlend = D3D11_BLEND_ONE;
        rt_desc.BlendOp = D3D11_BLEND_OP_ADD;
        rt_desc.SrcBlendAlpha = D3D11_BLEND_ONE;
        rt_desc.DestBlendAlpha = D3D11_BLEND_ZERO;
        rt_desc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
        device->CreateBlendState(&blend_desc, bs_alpha_weighted_additive.GetAddressOf());

        // Initialize depth/stencil state
        CD3D11_DEPTH_STENCIL_DESC ds_desc{CD3D11_DEFAULT{}};
        // Only allow pixel with equal depth to write into depth/stencil state
        // Do not need to write depth value
        ds_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        ds_desc.DepthFunc = D3D11_COMPARISON_EQUAL;
        device->CreateDepthStencilState(&ds_desc, dss_equal.GetAddressOf());

        // Less equal
        ds_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        ds_desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        device->CreateDepthStencilState(&ds_desc, dss_less_equal.GetAddressOf());

        // Reverse z - greater equal
        ds_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        ds_desc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
        device->CreateDepthStencilState(&ds_desc, dss_greater_equal.GetAddressOf());

        // Perform depth test, but do not write depth value, disable stencil state
        // When draw opaque objects, use default state
        // When draw transparent objects, use this state
        ds_desc.DepthEnable = true;
        ds_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        ds_desc.DepthFunc = D3D11_COMPARISON_LESS;
        ds_desc.StencilEnable = false;
        device->CreateDepthStencilState(&ds_desc, dss_no_depth_write.GetAddressOf());

        // Close depth test
        // When draw transparent objects, draw in order
        // When draw opaque objects, draw in any order
        // Disable stencil state by default
        ds_desc.DepthEnable = false;
        device->CreateDepthStencilState(&ds_desc, dss_no_depth_test.GetAddressOf());

        // Reverse z depth test and stencil value comparison
        ds_desc.DepthEnable = true;
        ds_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        ds_desc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
        ds_desc.StencilEnable = true;
        ds_desc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
        ds_desc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;
        device->CreateDepthStencilState(&ds_desc, dss_equal_stencil.GetAddressOf());

        // No depth test, only write into stencil value
        ds_desc.DepthEnable = false;
        ds_desc.StencilEnable = true;
        ds_desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_REPLACE;
        ds_desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_REPLACE;
        ds_desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
        ds_desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
        ds_desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_REPLACE;
        ds_desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_REPLACE;
        ds_desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
        ds_desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
        device->CreateDepthStencilState(&ds_desc, dss_write_stencil.GetAddressOf());

        // Set debug object names
        // TODO
    }
}








