//
// Created by ZZK on 2023/10/2.
//

#include <Toy/Renderer/effects.h>
#include <Toy/Renderer/render_states.h>
#include <Toy/Geometry/vertex.h>
#include <Toy/Model/mesh_data.h>
#include <Toy/Model/texture_manager.h>
#include <Toy/Model/model_manager.h>
#include <Toy/Model/material.h>
#include <Toy/ECS/components.h>

namespace toy
{
    struct SimpleSkyboxEffect::EffectImpl
    {
        EffectImpl() = default;
        ~EffectImpl() = default;

        std::unique_ptr<EffectHelper> effect_helper = nullptr;
        std::shared_ptr<IEffectPass>  cur_effect_pass = nullptr;
        com_ptr<ID3D11InputLayout>    cur_vertex_layout = nullptr;
        com_ptr<ID3D11InputLayout>    vertex_layout = nullptr;

        std::string_view skybox_pass = {};

        DirectX::XMFLOAT4X4 view_matrix = {};
        DirectX::XMFLOAT4X4 proj_matrix = {};

        D3D11_PRIMITIVE_TOPOLOGY cur_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    };

    SimpleSkyboxEffect::SimpleSkyboxEffect()
    {
        m_effect_impl = std::make_unique<EffectImpl>();
    }

    SimpleSkyboxEffect::~SimpleSkyboxEffect() noexcept = default;

    SimpleSkyboxEffect::SimpleSkyboxEffect(toy::SimpleSkyboxEffect &&other) noexcept
    {
        m_effect_impl.swap(other.m_effect_impl);
    }

    SimpleSkyboxEffect& SimpleSkyboxEffect::operator=(toy::SimpleSkyboxEffect &&other) noexcept
    {
        m_effect_impl.swap(other.m_effect_impl);
        return *this;
    }

    SimpleSkyboxEffect& SimpleSkyboxEffect::get()
    {
        static SimpleSkyboxEffect simple_skybox_effect = {};
        return simple_skybox_effect;
    }

    void SimpleSkyboxEffect::init(ID3D11Device *device)
    {
        m_effect_impl->effect_helper = std::make_unique<EffectHelper>();
        m_effect_impl->effect_helper->set_binary_cache_directory(DXTOY_HOME L"data/pbr/cache");
        m_effect_impl->skybox_pass = "SkyboxPass";

        std::string_view skybox_vs = "SkyboxVS";
        std::string_view skybox_ps = "SkyboxPS";

        // Create vertex and pixel shaders and input layout
        com_ptr<ID3DBlob> blob = nullptr;
        m_effect_impl->effect_helper->create_shader_from_file(skybox_vs, DXTOY_HOME L"data/pbr/skybox.hlsl", device,
                                                                "VS", "vs_5_0", nullptr, blob.GetAddressOf());
        m_effect_impl->effect_helper->create_shader_from_file(skybox_ps, DXTOY_HOME L"data/pbr/skybox.hlsl", device,
                                                                "PS", "ps_5_0");
        auto&& input_layout = VertexPosTex::get_input_layout();
        device->CreateInputLayout(input_layout.data(), (uint32_t)input_layout.size(),
                                    blob->GetBufferPointer(), blob->GetBufferSize(),
                                    m_effect_impl->vertex_layout.ReleaseAndGetAddressOf());
        if (!m_effect_impl->vertex_layout)
        {
            DX_CORE_CRITICAL("Fail to create vertex layout");
        }

        // Create skybox pass ans rasterizer state
        // Note: skybox pass shares the same depth-stencil state with deferred pbr pass
        EffectPassDesc pass_desc = {};
        pass_desc.nameVS = skybox_vs;
        pass_desc.namePS = skybox_ps;
        m_effect_impl->effect_helper->add_effect_pass(m_effect_impl->skybox_pass, device, &pass_desc);
        {
            auto pass = m_effect_impl->effect_helper->get_effect_pass(m_effect_impl->skybox_pass);
            pass->set_rasterizer_state(RenderStates::rs_no_cull.Get());
            pass->set_depth_stencil_state(RenderStates::dss_less_equal.Get(), 0);
        }

        // Set sampler state
        m_effect_impl->effect_helper->set_sampler_state_by_name("gSamAnisotropicWrap", RenderStates::ss_anisotropic_wrap_16x.Get());
    }

    void SimpleSkyboxEffect::set_material(const model::Material &material)
    {
        auto&& texture_manager = model::TextureManager::get();

        if (auto env_map_srv = texture_manager.get_texture(model::material_semantics_name(model::MaterialSemantics::PrefilteredSpecularMap)))
        {
            m_effect_impl->effect_helper->set_shader_resource_by_name("gSkyboxMap",env_map_srv);
        } else
        {
            m_effect_impl->effect_helper->set_shader_resource_by_name("gSkyboxMap", nullptr);
        }
    }

    void SimpleSkyboxEffect::set_depth_texture(ID3D11ShaderResourceView *depth_srv)
    {
        m_effect_impl->effect_helper->set_shader_resource_by_name("gDepthMap", depth_srv);
    }

    void SimpleSkyboxEffect::set_scene_texture(ID3D11ShaderResourceView *scene_texture)
    {
        m_effect_impl->effect_helper->set_shader_resource_by_name("gSceneMap", scene_texture);
    }

    MeshDataInput SimpleSkyboxEffect::get_input_data(const model::MeshData &mesh_data)
    {
        MeshDataInput input;
        input.input_layout = m_effect_impl->cur_vertex_layout.Get();
        input.topology = m_effect_impl->cur_topology;
        input.vertex_buffers = {
            mesh_data.vertices.Get(),
            mesh_data.texcoord_arrays.empty() ? nullptr : mesh_data.texcoord_arrays[0].Get()
        };
        input.strides = { 12, 8 };
        input.offsets = { 0, 0 };

        input.index_buffer = mesh_data.indices.Get();
        input.index_count = mesh_data.index_count;

        return input;
    }

    void SimpleSkyboxEffect::set_skybox_render()
    {
        m_effect_impl->cur_effect_pass = m_effect_impl->effect_helper->get_effect_pass(m_effect_impl->skybox_pass);
        m_effect_impl->cur_vertex_layout = m_effect_impl->vertex_layout;
        m_effect_impl->cur_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    }

    void SimpleSkyboxEffect::apply(ID3D11DeviceContext *device_context)
    {
        using namespace DirectX;
        XMMATRIX view_proj = XMLoadFloat4x4(&m_effect_impl->view_matrix) * XMLoadFloat4x4(&m_effect_impl->proj_matrix);
        view_proj = XMMatrixTranspose(view_proj);
        m_effect_impl->effect_helper->get_constant_buffer_variable("gViewProj")->set_float_matrix(4, 4, (const float *)&view_proj);

        m_effect_impl->cur_effect_pass->apply(device_context);
    }

    void SimpleSkyboxEffect::emit_render_pass(ID3D11DeviceContext *device_context, const Transform &transform, const model::Model &model_data)
    {
        size_t meshes_size = model_data.meshes.size();
        for (size_t i = 0; i < meshes_size; ++i)
        {
            set_material(model_data.materials[model_data.meshes[i].material_index]);
            set_world_matrix(transform.get_local_to_world_matrix_xm());
            apply(device_context);

            MeshDataInput input = get_input_data(model_data.meshes[i]);
            device_context->IASetInputLayout(input.input_layout);
            device_context->IASetPrimitiveTopology(input.topology);
            device_context->IASetVertexBuffers(0, static_cast<uint32_t>(input.vertex_buffers.size()),
                                                input.vertex_buffers.data(), input.strides.data(), input.offsets.data());
            device_context->IASetIndexBuffer(input.index_buffer, input.index_count > std::numeric_limits<uint16_t>::max() ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT, 0);
            device_context->DrawIndexed(input.index_count, 0, 0);
        }
    }

    void XM_CALLCONV SimpleSkyboxEffect::set_world_matrix(DirectX::FXMMATRIX world)
    {
        // Do nothing
    }

    void XM_CALLCONV SimpleSkyboxEffect::set_view_matrix(DirectX::FXMMATRIX view)
    {
        DirectX::XMStoreFloat4x4(&m_effect_impl->view_matrix, view);
    }

    void XM_CALLCONV SimpleSkyboxEffect::set_proj_matrix(DirectX::FXMMATRIX proj)
    {
        DirectX::XMStoreFloat4x4(&m_effect_impl->proj_matrix, proj);
    }
}