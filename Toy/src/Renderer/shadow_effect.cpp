//
// Created by ZZK on 2023/10/16.
//

#include <Toy/Renderer/effects.h>
#include <Toy/Renderer/render_states.h>
#include <Toy/Geometry/vertex.h>
#include <Toy/Model/mesh_data.h>
#include <Toy/Model/texture_manager.h>
#include <Toy/Model/material.h>

namespace toy
{
    struct ShadowEffect::EffectImpl
    {
        EffectImpl() = default;
        ~EffectImpl() = default;

        std::unique_ptr<EffectHelper> effect_helper = nullptr;
        std::shared_ptr<IEffectPass> cur_effect_pass = nullptr;
        com_ptr<ID3D11InputLayout> cur_input_layout = nullptr;

        DirectX::XMFLOAT4X4 world_matrix = {};
        DirectX::XMFLOAT4X4 view_matrix = {};
        DirectX::XMFLOAT4X4 proj_matrix = {};

        std::string_view shadow_pass = {};
        std::string_view shadow_alpha_clip_pass = {};
        std::string_view shadow_debug_pass = {};

        D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
    };

    ShadowEffect::ShadowEffect()
    {
        m_effect_impl = std::make_unique<EffectImpl>();
    }

    ShadowEffect::~ShadowEffect() = default;

    ShadowEffect::ShadowEffect(toy::ShadowEffect &&other) noexcept
    {
        m_effect_impl.swap(other.m_effect_impl);
    }

    ShadowEffect &ShadowEffect::operator=(toy::ShadowEffect &&other) noexcept
    {
        m_effect_impl.swap(other.m_effect_impl);
        return *this;
    }

    ShadowEffect &ShadowEffect::get()
    {
        static ShadowEffect shadow_effect = {};
        return shadow_effect;
    }

    void ShadowEffect::init(ID3D11Device *device)
    {
        m_effect_impl->effect_helper = std::make_unique<EffectHelper>();
        m_effect_impl->effect_helper->set_binary_cache_directory(DXTOY_HOME L"data/pbr/cache");

        std::string_view shadow_vs = "ShadowVS";
        std::string_view screen_triangle_vs = "ShadowScreenVS";
        std::string_view shadow_ps = "ShadowPS";
        std::string_view debug_ps = "ShadowDebugPS";

        m_effect_impl->shadow_pass = "ShadowPass";
        m_effect_impl->shadow_alpha_clip_pass = "ShadowAlphaClipPass";
        m_effect_impl->shadow_debug_pass = "ShadowDebugPass";

        com_ptr<ID3DBlob> blob = nullptr;

        m_effect_impl->effect_helper->create_shader_from_file(shadow_vs, DXTOY_HOME L"data/pbr/shadow_vs.hlsl", device,
                                                                "VS", "vs_5_0", nullptr, blob.GetAddressOf());
        auto&& input_layout = VertexPosNormalTex::get_input_layout();
        device->CreateInputLayout(input_layout.data(), uint32_t(input_layout.size()), blob->GetBufferPointer(),
                                    blob->GetBufferSize(), m_effect_impl->cur_input_layout.GetAddressOf());
        m_effect_impl->effect_helper->create_shader_from_file(screen_triangle_vs, DXTOY_HOME L"data/pbr/screen_triangle_vs.hlsl", device,
                                                                "VS", "vs_5_0");

        m_effect_impl->effect_helper->create_shader_from_file(shadow_ps, DXTOY_HOME L"data/pbr/shadow_ps.hlsl", device,
                                                                "PS", "ps_5_0");
        m_effect_impl->effect_helper->create_shader_from_file(debug_ps, DXTOY_HOME L"data/pbr/shadow_ps.hlsl", device,
                                                                "DebugPS", "ps_5_0");

        EffectPassDesc pass_desc = {};
        pass_desc.nameVS = shadow_vs;
        m_effect_impl->effect_helper->add_effect_pass(m_effect_impl->shadow_pass, device, &pass_desc);
        auto pass = m_effect_impl->effect_helper->get_effect_pass(m_effect_impl->shadow_pass);
        pass->set_rasterizer_state(RenderStates::rs_shadow.Get());

        pass_desc.nameVS = shadow_vs;
        pass_desc.namePS = shadow_ps;
        m_effect_impl->effect_helper->add_effect_pass(m_effect_impl->shadow_alpha_clip_pass, device, &pass_desc);
        pass = m_effect_impl->effect_helper->get_effect_pass(m_effect_impl->shadow_alpha_clip_pass);
        pass->set_rasterizer_state(RenderStates::rs_no_cull.Get());

        pass_desc.nameVS = screen_triangle_vs;
        pass_desc.namePS = debug_ps;
        m_effect_impl->effect_helper->add_effect_pass(m_effect_impl->shadow_debug_pass, device, &pass_desc);

        m_effect_impl->effect_helper->set_sampler_state_by_name("gSamLinearWrap", RenderStates::ss_linear_wrap.Get());
    }

    void ShadowEffect::set_default_render()
    {
        m_effect_impl->cur_effect_pass = m_effect_impl->effect_helper->get_effect_pass(m_effect_impl->shadow_pass);
        m_effect_impl->topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    }

    void ShadowEffect::set_alpha_clip_render(float alpha_clip_value)
    {
        m_effect_impl->cur_effect_pass = m_effect_impl->effect_helper->get_effect_pass(m_effect_impl->shadow_alpha_clip_pass);
        m_effect_impl->cur_effect_pass->get_ps_param_by_name("gClipValue")->set_float(alpha_clip_value);
        m_effect_impl->topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    }

    void ShadowEffect::render_depth_to_texture(ID3D11DeviceContext *device_context, ID3D11ShaderResourceView *input_srv,
                                                ID3D11RenderTargetView *output_rtv, const D3D11_VIEWPORT &viewport)
    {
        device_context->IASetInputLayout(nullptr);
        device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_effect_impl->cur_effect_pass = m_effect_impl->effect_helper->get_effect_pass(m_effect_impl->shadow_debug_pass);
        m_effect_impl->effect_helper->set_shader_resource_by_name("gAlbedoMap", input_srv);    // TODO: modify name
        m_effect_impl->cur_effect_pass->apply(device_context);
        device_context->OMSetRenderTargets(1, &output_rtv, nullptr);
        device_context->RSSetViewports(1, &viewport);
        device_context->Draw(3, 0);

        // Clear
        auto slot = m_effect_impl->effect_helper->map_shader_resource_slot("gAlbedoMap");  // TODO: modify name
        input_srv = nullptr;
        device_context->PSSetShaderResources(static_cast<uint32_t>(slot), 1, &input_srv);
        device_context->OMSetRenderTargets(0, nullptr, nullptr);
    }

    void ShadowEffect::set_material(const model::Material &material)
    {

    }

    MeshDataInput ShadowEffect::get_input_data(const model::MeshData &mesh_data)
    {
        MeshDataInput input;
        input.input_layout = m_effect_impl->cur_input_layout.Get();
        input.topology = m_effect_impl->topology;
        input.vertex_buffers = {
            mesh_data.vertices.Get(),
            mesh_data.normals.Get(),
            mesh_data.texcoord_arrays.empty() ? nullptr : mesh_data.texcoord_arrays[0].Get()
        };
        input.strides = { 12, 12, 8 };
        input.offsets = { 0, 0, 0 };

        input.index_buffer = mesh_data.indices.Get();
        input.index_count = mesh_data.index_count;

        return input;
    }

    void ShadowEffect::apply(ID3D11DeviceContext *device_context)
    {
        using namespace DirectX;
        XMMATRIX world_view_proj = XMLoadFloat4x4(&m_effect_impl->world_matrix) * XMLoadFloat4x4(&m_effect_impl->view_matrix) *
                                    XMLoadFloat4x4(&m_effect_impl->proj_matrix);
        world_view_proj = XMMatrixTranspose(world_view_proj);
        m_effect_impl->effect_helper->get_constant_buffer_variable("gWorldViewProj")->set_float_matrix(4, 4, (const float *)&world_view_proj);

        m_effect_impl->cur_effect_pass->apply(device_context);
    }

    void XM_CALLCONV ShadowEffect::set_world_matrix(DirectX::FXMMATRIX world)
    {
        DirectX::XMStoreFloat4x4(&m_effect_impl->world_matrix, world);
    }

    void XM_CALLCONV ShadowEffect::set_view_matrix(DirectX::FXMMATRIX view)
    {
        DirectX::XMStoreFloat4x4(&m_effect_impl->view_matrix, view);
    }

    void XM_CALLCONV ShadowEffect::set_proj_matrix(DirectX::FXMMATRIX proj)
    {
        DirectX::XMStoreFloat4x4(&m_effect_impl->proj_matrix, proj);
    }
}






















