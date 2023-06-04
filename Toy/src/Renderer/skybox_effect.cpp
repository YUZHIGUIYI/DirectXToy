//
// Created by ZZK on 2023/6/3.
//

#include <Toy/Renderer/effects.h>
#include <Toy/Renderer/render_states.h>
#include <Toy/Core/d3d_util.h>
#include <Toy/Geometry/vertex.h>
#include <Toy/Model/mesh_data.h>
#include <Toy/Model/texture_manager.h>
#include <Toy/Model/material.h>

namespace toy
{
    SkyboxEffect::SkyboxEffect()
    {
        m_effect_impl = std::make_unique<effect_impl>();
    }

    SkyboxEffect::SkyboxEffect(toy::SkyboxEffect &&other) noexcept
    {
        m_effect_impl.swap(other.m_effect_impl);
    }

    SkyboxEffect& SkyboxEffect::operator=(toy::SkyboxEffect &&other) noexcept
    {
        m_effect_impl.swap(other.m_effect_impl);
        return *this;
    }

    void SkyboxEffect::init(ID3D11Device *device)
    {
        m_effect_impl->m_effect_helper = std::make_unique<EffectHelper>();

        com_ptr<ID3DBlob> blob = nullptr;
        // Create vertex shader
        m_effect_impl->m_effect_helper->create_shader_from_file("skybox_vs", L"../data/shaders/skybox_vs.cso", device,
                                                                "VS", "vs_5_0", nullptr, blob.GetAddressOf());
        auto&& input_layout = VertexPos::get_input_layout();
        device->CreateInputLayout(input_layout.data(), (uint32_t)input_layout.size(),
                                    blob->GetBufferPointer(), blob->GetBufferSize(),
                                    m_effect_impl->m_vertex_pos_layout.GetAddressOf());

        // Create pixel shader
        m_effect_impl->m_effect_helper->create_shader_from_file("skybox_ps", L"../data/shaders/skybox_ps.cso", device);

        // Create pass
        EffectPassDesc pass_desc{};
        pass_desc.nameVS = "skybox_vs";
        pass_desc.namePS = "skybox_ps";
        m_effect_impl->m_effect_helper->add_effect_pass("skybox", device, &pass_desc);
        {
            auto pass = m_effect_impl->m_effect_helper->get_effect_pass("skybox");
            pass->set_rasterizer_state(RenderStates::rs_no_cull.Get());
            pass->set_depth_stencil_state(RenderStates::dss_less_equal.Get(), 0);
        }
        m_effect_impl->m_effect_helper->set_sampler_state_by_name("g_Sam", RenderStates::ss_linear_wrap.Get());

        // TODO: Set debug object name
#if defined(GRAPHICS_DEBUGGER_OBJECT_NAME)
        set_debug_object_name(m_effect_impl->m_vertex_pos_layout.Get(), "SkyboxEffect.VertexPosLayout");
#endif
    }

    void SkyboxEffect::set_default_render()
    {
        m_effect_impl->m_curr_effect_pass = m_effect_impl->m_effect_helper->get_effect_pass("skybox");
        m_effect_impl->m_curr_input_layout = m_effect_impl->m_vertex_pos_layout;
        m_effect_impl->m_curr_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    }

    void XM_CALLCONV SkyboxEffect::set_world_matrix(DirectX::FXMMATRIX world)
    {
        UNREFERENCED_PARAMETER(world);
    }

    void XM_CALLCONV SkyboxEffect::set_view_matrix(DirectX::FXMMATRIX view)
    {
        DirectX::XMStoreFloat4x4(&m_effect_impl->m_view, view);
    }

    void XM_CALLCONV SkyboxEffect::set_proj_matrix(DirectX::FXMMATRIX proj)
    {
        DirectX::XMStoreFloat4x4(&m_effect_impl->m_proj, proj);
    }

    void SkyboxEffect::set_material(const model::Material &material)
    {
        auto&& texture_manager = model::TextureManagerHandle::get();

        const std::string& texture_id_str = material.get<std::string>("$Skybox");
        m_effect_impl->m_effect_helper->set_shader_resource_by_name("g_TexCube", texture_manager.get_texture(texture_id_str));
    }

    MeshDataInput SkyboxEffect::get_input_data(const model::MeshData &mesh_data)
    {
        MeshDataInput input;
        input.input_layout = m_effect_impl->m_curr_input_layout.Get();
        input.topology = m_effect_impl->m_curr_topology;
        input.vertex_buffers = {
            mesh_data.vertices.Get()
        };
        input.strides = { 12 };
        input.offsets = { 0 };

        input.index_buffer = mesh_data.indices.Get();
        input.index_count = mesh_data.index_count;

        return input;
    }

    void SkyboxEffect::apply(ID3D11DeviceContext *device_context)
    {
        using namespace DirectX;
        XMMATRIX view = XMLoadFloat4x4(&m_effect_impl->m_view);
        view.r[3] = g_XMIdentityR3;
        XMMATRIX view_proj = view * XMLoadFloat4x4(&m_effect_impl->m_proj);

        view_proj = XMMatrixTranspose(view_proj);
        m_effect_impl->m_effect_helper->get_constant_buffer_variable("g_WorldViewProj")->set_float_matrix(4, 4, (const float *)&view_proj);
        m_effect_impl->m_curr_effect_pass->apply(device_context);
    }
}
