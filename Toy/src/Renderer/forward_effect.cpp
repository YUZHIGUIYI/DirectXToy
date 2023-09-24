//
// Created by ZHIKANG on 2023/6/11.
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
    struct ForwardEffect::EffectImpl
    {
        EffectImpl() = default;
        ~EffectImpl() = default;

        std::unique_ptr<EffectHelper> m_effect_helper;
        std::shared_ptr<IEffectPass> m_cur_effect_pass;
        com_ptr<ID3D11InputLayout> m_cur_input_layout;
        com_ptr<ID3D11InputLayout> m_vertex_pos_normal_tex_layout;
        D3D11_PRIMITIVE_TOPOLOGY  m_topology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;

        DirectX::XMFLOAT4X4 m_world{};
        DirectX::XMFLOAT4X4 m_view{};
        DirectX::XMFLOAT4X4 m_proj{};

        uint32_t m_msaa_samples = 1;
    };

    ForwardEffect::ForwardEffect()
    {
        m_effect_impl = std::make_unique<EffectImpl>();
    }

    ForwardEffect::~ForwardEffect() noexcept = default;

    ForwardEffect::ForwardEffect(toy::ForwardEffect &&other) noexcept
    {
        m_effect_impl.swap(other.m_effect_impl);
    }

    ForwardEffect& ForwardEffect::operator=(toy::ForwardEffect &&other) noexcept
    {
        m_effect_impl.swap(other.m_effect_impl);
        return *this;
    }

    ForwardEffect& ForwardEffect::get()
    {
        static ForwardEffect forward_effect{};
        return forward_effect;
    }

    void ForwardEffect::init(ID3D11Device* device)
    {
        m_effect_impl->m_effect_helper = std::make_unique<EffectHelper>();
        m_effect_impl->m_effect_helper->set_binary_cache_directory(L"../data/defer/cache");

        com_ptr<ID3DBlob> blob = nullptr;
        D3D_SHADER_MACRO defines[] = {
            {"MSAA_SAMPLES", "1"},
            {nullptr, nullptr}
        };
        // Create vertex shader and input layout
        m_effect_impl->m_effect_helper->create_shader_from_file("GeometryVS", L"../data/defer/forward.hlsl", device,
                                                                "GeometryVS", "vs_5_0", nullptr, blob.GetAddressOf());
        auto&& input_layout = VertexPosNormalTex::get_input_layout();
        device->CreateInputLayout(input_layout.data(), (uint32_t)input_layout.size(), blob->GetBufferPointer(),
                                    blob->GetBufferSize(), m_effect_impl->m_vertex_pos_normal_tex_layout.ReleaseAndGetAddressOf());

        // Create pixel shader
        m_effect_impl->m_effect_helper->create_shader_from_file("ForwardPS", L"../data/defer/forward.hlsl", device,
                                                                "ForwardPS", "ps_5_0");
        m_effect_impl->m_effect_helper->create_shader_from_file("ForwardPlusPS", L"../data/defer/forward.hlsl", device,
                                                                "ForwardPlusPS", "ps_5_0");

        // Create pass
        EffectPassDesc pass_desc{};
        pass_desc.nameVS = "GeometryVS";
        pass_desc.namePS = "ForwardPS";
        m_effect_impl->m_effect_helper->add_effect_pass("Forward", device, &pass_desc);
        {
            auto pass = m_effect_impl->m_effect_helper->get_effect_pass("Forward");
            // Reverse Z => GREATER_EQUAL
            pass->set_depth_stencil_state(RenderStates::dss_greater_equal.Get(), 0);
        }

        pass_desc.namePS = "ForwardPlusPS";
        m_effect_impl->m_effect_helper->add_effect_pass("ForwardPlus", device, &pass_desc);
        {
            auto pass = m_effect_impl->m_effect_helper->get_effect_pass("ForwardPlus");
            // Reverse Z => GREATER_EQUAL
            pass->set_depth_stencil_state(RenderStates::dss_greater_equal.Get(), 0);
        }

        pass_desc.namePS = "";
        m_effect_impl->m_effect_helper->add_effect_pass("PreZ", device, &pass_desc);
        {
            auto pass = m_effect_impl->m_effect_helper->get_effect_pass("PreZ");
            // Reverse Z => GREATER_EQUAL
            pass->set_depth_stencil_state(RenderStates::dss_greater_equal.Get(), 0);
        }

        m_effect_impl->m_effect_helper->set_sampler_state_by_name("g_Sam", RenderStates::ss_anisotropic_wrap_16x.Get());

        // Create compute shaders and passes
        uint32_t msaa_samples = 1;
        pass_desc.nameVS = "";
        pass_desc.namePS = "";
        while (msaa_samples <= 8)
        {
            std::string msaa_samples_str = std::to_string(msaa_samples);
            defines[0].Definition = msaa_samples_str.c_str();
            std::string shader_name = "ComputeShaderTileForward_" + msaa_samples_str + "xMSAA_CS";
            m_effect_impl->m_effect_helper->create_shader_from_file(shader_name, L"../data/defer/compute_shader_tile.hlsl", device,
                                                                    "ComputeShaderTileForwardCS", "cs_5_0", defines);
            pass_desc.nameCS = shader_name;
            std::string pass_name = "ComputeShaderTileForward_" + std::to_string(msaa_samples) + "xMSAA";
            m_effect_impl->m_effect_helper->add_effect_pass(pass_name, device, &pass_desc);

            msaa_samples <<= 1;
        }

        // TODO: set debug object name
    }

    void ForwardEffect::set_msaa_samples(uint32_t msaa_samples)
    {
        m_effect_impl->m_msaa_samples = msaa_samples;
    }

    void ForwardEffect::set_light_buffer(ID3D11ShaderResourceView *light_buffer)
    {
        m_effect_impl->m_effect_helper->set_shader_resource_by_name("g_Light", light_buffer);
    }

    void ForwardEffect::set_tile_buffer(ID3D11ShaderResourceView *tile_buffer)
    {
        m_effect_impl->m_effect_helper->set_shader_resource_by_name("g_Tilebuffer", tile_buffer);
    }

    void ForwardEffect::set_camera_near_far(float nearz, float farz)
    {
        float near_far[4] = { nearz, farz };
        m_effect_impl->m_effect_helper->get_constant_buffer_variable("g_CameraNearFar")->set_float_vector(4, near_far);
    }

    void ForwardEffect::set_lighting_only(bool enable)
    {
        m_effect_impl->m_effect_helper->get_constant_buffer_variable("g_LightingOnly")->set_uint(enable);
    }

    void ForwardEffect::set_face_normals(bool enable)
    {
        m_effect_impl->m_effect_helper->get_constant_buffer_variable("g_FaceNormals")->set_uint(enable);
    }

    void ForwardEffect::set_visualize_light_count(bool enable)
    {
        m_effect_impl->m_effect_helper->get_constant_buffer_variable("g_VisualizeLightCount")->set_uint(enable);
    }

    void ForwardEffect::set_pre_z_pass_render()
    {
        m_effect_impl->m_cur_effect_pass = m_effect_impl->m_effect_helper->get_effect_pass("PreZ");
        m_effect_impl->m_cur_input_layout = m_effect_impl->m_vertex_pos_normal_tex_layout.Get();
        m_effect_impl->m_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    }

    void ForwardEffect::set_default_render()
    {
        m_effect_impl->m_cur_effect_pass = m_effect_impl->m_effect_helper->get_effect_pass("Forward");
        m_effect_impl->m_cur_input_layout = m_effect_impl->m_vertex_pos_normal_tex_layout.Get();
        m_effect_impl->m_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    }

    void ForwardEffect::compute_tiled_light_culling(ID3D11DeviceContext *device_context,
                                                    ID3D11UnorderedAccessView *tile_info_buffer_uav,
                                                    ID3D11ShaderResourceView *light_buffer_srv,
                                                    ID3D11ShaderResourceView *depth_buffer_srv)
    {
        com_ptr<ID3D11Texture2D> tex = nullptr;
        depth_buffer_srv->GetResource(reinterpret_cast<ID3D11Resource**>(tex.GetAddressOf()));
        D3D11_TEXTURE2D_DESC texDesc{};
        tex->GetDesc(&texDesc);

        uint32_t dims[2] = { texDesc.Width, texDesc.Height };
        m_effect_impl->m_effect_helper->get_constant_buffer_variable("g_FramebufferDimensions")->set_uint_vector(2, dims);
        uint32_t initCount = 0;
        m_effect_impl->m_effect_helper->set_unordered_access_by_name("g_TilebufferRW", tile_info_buffer_uav, &initCount);
        m_effect_impl->m_effect_helper->set_shader_resource_by_name("g_Light", light_buffer_srv);
        m_effect_impl->m_effect_helper->set_shader_resource_by_name("g_GBufferTextures[3]", depth_buffer_srv);

        std::string pass_name = "ComputeShaderTileForward_" + std::to_string(m_effect_impl->m_msaa_samples) + "xMSAA";
        auto pass = m_effect_impl->m_effect_helper->get_effect_pass(pass_name);
        pass->apply(device_context);

        // Dispatch
        pass->dispatch(device_context, texDesc.Width, texDesc.Height, 1);

        // Clear
        auto& effect_helper = m_effect_impl->m_effect_helper;
        tile_info_buffer_uav = nullptr;
        device_context->CSSetUnorderedAccessViews(effect_helper->map_unordered_access_slot("g_TilebufferRW"), 1, &tile_info_buffer_uav, nullptr);
        light_buffer_srv = nullptr;
        device_context->CSSetShaderResources(effect_helper->map_shader_resource_slot("g_Light"), 1, &light_buffer_srv);
        device_context->CSSetShaderResources(effect_helper->map_shader_resource_slot("g_GBufferTextures[3]"), 1, &light_buffer_srv);
    }

    void ForwardEffect::set_tiled_light_culling_render()
    {
        m_effect_impl->m_cur_effect_pass = m_effect_impl->m_effect_helper->get_effect_pass("ForwardPlus");
        m_effect_impl->m_cur_input_layout = m_effect_impl->m_vertex_pos_normal_tex_layout.Get();
        m_effect_impl->m_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    }

    void XM_CALLCONV ForwardEffect::set_world_matrix(DirectX::FXMMATRIX world)
    {
        DirectX::XMStoreFloat4x4(&m_effect_impl->m_world, world);
    }

    void XM_CALLCONV ForwardEffect::set_view_matrix(DirectX::FXMMATRIX view)
    {
        DirectX::XMStoreFloat4x4(&m_effect_impl->m_view, view);
    }

    void XM_CALLCONV ForwardEffect::set_proj_matrix(DirectX::FXMMATRIX proj)
    {
        DirectX::XMStoreFloat4x4(&m_effect_impl->m_proj, proj);
    }

    void ForwardEffect::set_material(const model::Material &material)
    {
        auto&& texture_manager = model::TextureManager::get();

        auto texture_id_str = material.try_get<std::string>("$Diffuse");
        m_effect_impl->m_effect_helper->set_shader_resource_by_name("g_DiffuseMap",
                                                                    texture_id_str ? texture_manager.get_texture(*texture_id_str) : texture_manager.get_null_texture());
    }

    MeshDataInput ForwardEffect::get_input_data(const model::MeshData &mesh_data)
    {
        MeshDataInput input;
        input.input_layout = m_effect_impl->m_cur_input_layout.Get();
        input.topology = m_effect_impl->m_topology;
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

    void ForwardEffect::apply(ID3D11DeviceContext *device_context)
    {
        using namespace DirectX;
        XMMATRIX world = XMLoadFloat4x4(&m_effect_impl->m_world);
        XMMATRIX view = XMLoadFloat4x4(&m_effect_impl->m_view);
        XMMATRIX proj = XMLoadFloat4x4(&m_effect_impl->m_proj);

        XMMATRIX world_view = world * view;
        XMMATRIX world_view_proj = world_view * proj;
        XMMATRIX world_inv_t_view = XMath::inverse_transpose(world) * view;
        XMMATRIX view_proj = view * proj;

        world_view = XMMatrixTranspose(world_view);
        world_view_proj = XMMatrixTranspose(world_view_proj);
        world_inv_t_view = XMMatrixTranspose(world_inv_t_view);
        proj = XMMatrixTranspose(proj);
        view_proj = XMMatrixTranspose(view_proj);

        m_effect_impl->m_effect_helper->get_constant_buffer_variable("g_WorldInvTransposeView")->set_float_matrix(4, 4, (float*)&world_inv_t_view);
        m_effect_impl->m_effect_helper->get_constant_buffer_variable("g_WorldViewProj")->set_float_matrix(4, 4, (float*)&world_view_proj);
        m_effect_impl->m_effect_helper->get_constant_buffer_variable("g_WorldView")->set_float_matrix(4, 4, (float*)&world_view);
        m_effect_impl->m_effect_helper->get_constant_buffer_variable("g_ViewProj")->set_float_matrix(4, 4, (float*)&view_proj);
        m_effect_impl->m_effect_helper->get_constant_buffer_variable("g_Proj")->set_float_matrix(4, 4, (float*)&proj);

        if (m_effect_impl->m_cur_effect_pass)
        {
            m_effect_impl->m_cur_effect_pass->apply(device_context);
        }
    }
}














