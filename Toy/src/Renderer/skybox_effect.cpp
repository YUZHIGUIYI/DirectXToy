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
    struct SkyboxEffect::EffectImpl
    {
        EffectImpl()
        {
            DirectX::XMStoreFloat4x4(&m_view, DirectX::XMMatrixIdentity());
            DirectX::XMStoreFloat4x4(&m_proj, DirectX::XMMatrixIdentity());
        }
        ~EffectImpl() = default;


        std::unique_ptr<EffectHelper> m_effect_helper;

        std::shared_ptr<IEffectPass> m_curr_effect_pass;
        com_ptr<ID3D11InputLayout> m_curr_input_layout;
        com_ptr<ID3D11InputLayout> m_vertex_pos_normal_tex_layout;

        DirectX::XMFLOAT4X4 m_view, m_proj;
        D3D11_PRIMITIVE_TOPOLOGY m_curr_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        uint32_t m_msaa_levels = 1;
    };

    SkyboxEffect::SkyboxEffect()
    {
        m_effect_impl = std::make_unique<EffectImpl>();
    }

    SkyboxEffect::~SkyboxEffect() noexcept = default;

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
        m_effect_impl->m_effect_helper->set_binary_cache_directory(DXTOY_HOME L"data/defer/cache");

        com_ptr<ID3DBlob> blob = nullptr;
        D3D_SHADER_MACRO defines[] = {
            {"MSAA_SAMPLES", "1"},
            {nullptr, nullptr}
        };
        // Create vertex shader
        m_effect_impl->m_effect_helper->create_shader_from_file("SkyboxVS", DXTOY_HOME L"data/defer/skybox.hlsl", device,
                                                                "SkyboxVS", "vs_5_0", defines, blob.GetAddressOf());
        auto&& input_layout = VertexPosNormalTex::get_input_layout();
        device->CreateInputLayout(input_layout.data(), (uint32_t)input_layout.size(),
                                    blob->GetBufferPointer(), blob->GetBufferSize(),
                                    m_effect_impl->m_vertex_pos_normal_tex_layout.ReleaseAndGetAddressOf());

        int32_t msaa_samples = 1;
        while (msaa_samples <= 8)
        {
            // ******************
            // Create pixel shaders
            //
            std::string msaaSamplesStr = std::to_string(msaa_samples);
            defines[0].Definition = msaaSamplesStr.c_str();
            std::string shaderName = "Skybox_" + msaaSamplesStr + "xMSAA_PS";
            m_effect_impl->m_effect_helper->create_shader_from_file(shaderName, DXTOY_HOME L"data/defer/skybox.hlsl",
                                                            device, "SkyboxPS", "ps_5_0", defines);

            // ******************
            // Create pass
            //
            std::string passName = "Skybox_" + msaaSamplesStr + "xMSAA";
            EffectPassDesc passDesc;
            passDesc.nameVS = "SkyboxVS";
            passDesc.namePS = shaderName;
            m_effect_impl->m_effect_helper->add_effect_pass(passName, device, &passDesc);
            {
                auto pPass = m_effect_impl->m_effect_helper->get_effect_pass(passName);
                pPass->set_rasterizer_state(RenderStates::rs_no_cull.Get());
            }

            msaa_samples <<= 1;
        }

        m_effect_impl->m_effect_helper->set_sampler_state_by_name("g_Sam", RenderStates::ss_linear_wrap.Get());

        // TODO: Set debug object name
#if defined(GRAPHICS_DEBUGGER_OBJECT_NAME)
        set_debug_object_name(m_effect_impl->m_vertex_pos_normal_tex_layout.Get(), "SkyboxEffect.VertexPosNormalTexLayout");
#endif
    }

    void SkyboxEffect::set_default_render()
    {
        std::string passName = "Skybox_" + std::to_string(m_effect_impl->m_msaa_levels) + "xMSAA";
        m_effect_impl->m_curr_effect_pass = m_effect_impl->m_effect_helper->get_effect_pass(passName);
        m_effect_impl->m_curr_input_layout = m_effect_impl->m_vertex_pos_normal_tex_layout;
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

        if (auto env_map_srv = PreProcessEffect::get().get_environment_srv())
        {
            m_effect_impl->m_effect_helper->set_shader_resource_by_name("g_SkyboxTexture",env_map_srv);
        } else
        {
            auto texture_id_str = material.try_get<std::string>("$Skybox");
            m_effect_impl->m_effect_helper->set_shader_resource_by_name("g_SkyboxTexture",
                                                                        texture_id_str ? texture_manager.get_texture(*texture_id_str) : nullptr);
        }
    }

    MeshDataInput SkyboxEffect::get_input_data(const model::MeshData &mesh_data)
    {
        MeshDataInput input;
        input.input_layout = m_effect_impl->m_curr_input_layout.Get();
        input.topology = m_effect_impl->m_curr_topology;
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

    void SkyboxEffect::set_texture_cube(ID3D11ShaderResourceView* skybox_texture)
    {
        m_effect_impl->m_effect_helper->set_shader_resource_by_name("g_SkyboxTexture", skybox_texture);
    }

    void SkyboxEffect::set_depth_texture(ID3D11ShaderResourceView *depth_texture)
    {
        m_effect_impl->m_effect_helper->set_shader_resource_by_name("g_DepthTexture", depth_texture);
    }

    void SkyboxEffect::set_lit_texture(ID3D11ShaderResourceView *lit_texture)
    {
        m_effect_impl->m_effect_helper->set_shader_resource_by_name("g_LitTexture", lit_texture);
    }

    void SkyboxEffect::set_flat_lit_texture(ID3D11ShaderResourceView *flat_lit_texture, uint32_t width, uint32_t height)
    {
        m_effect_impl->m_effect_helper->set_shader_resource_by_name("g_FlatLitTexture", flat_lit_texture);
        uint32_t wh[2] = { width, height };
        m_effect_impl->m_effect_helper->get_constant_buffer_variable("g_FramebufferDimensions")->set_uint_vector(2, wh);
    }

    void SkyboxEffect::set_msaa_samples(uint32_t msaa_samples)
    {
        m_effect_impl->m_msaa_levels = msaa_samples;
    }

    void SkyboxEffect::apply(ID3D11DeviceContext *device_context)
    {
        using namespace DirectX;
        XMMATRIX view_proj = XMLoadFloat4x4(&m_effect_impl->m_view) * XMLoadFloat4x4(&m_effect_impl->m_proj);
        view_proj = XMMatrixTranspose(view_proj);
        m_effect_impl->m_effect_helper->get_constant_buffer_variable("g_ViewProj")->set_float_matrix(4, 4, (const float *)&view_proj);

        m_effect_impl->m_curr_effect_pass->apply(device_context);
    }
}
