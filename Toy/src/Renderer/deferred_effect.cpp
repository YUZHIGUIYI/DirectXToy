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
    struct DeferredEffect::EffectImpl
    {
        EffectImpl() = default;
        ~EffectImpl() = default;

        std::unique_ptr<EffectHelper> m_effect_helper;
        std::shared_ptr<IEffectPass> m_cur_effect_pass;
        com_ptr<ID3D11InputLayout> m_cur_input_layout;
        D3D11_PRIMITIVE_TOPOLOGY m_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

        com_ptr<ID3D11InputLayout> m_vertex_pos_normal_tex_layout;
        DirectX::XMFLOAT4X4 m_world{};
        DirectX::XMFLOAT4X4 m_view{};
        DirectX::XMFLOAT4X4 m_proj{};
        uint32_t m_msaa_samples = 1;
    };

    DeferredEffect::DeferredEffect()
    {
        m_effect_impl = std::make_unique<EffectImpl>();
    }

    DeferredEffect::~DeferredEffect() noexcept = default;

    DeferredEffect::DeferredEffect(toy::DeferredEffect &&other) noexcept
    {
        m_effect_impl.swap(other.m_effect_impl);
    }

    DeferredEffect& DeferredEffect::operator=(toy::DeferredEffect &&other) noexcept
    {
        m_effect_impl.swap(other.m_effect_impl);
        return *this;
    }

    void DeferredEffect::init(ID3D11Device *device)
    {
        m_effect_impl->m_effect_helper = std::make_unique<EffectHelper>();
        m_effect_impl->m_effect_helper->set_binary_cache_directory(DXTOY_HOME L"data/defer/cache");

        com_ptr<ID3DBlob> blob = nullptr;
        D3D_SHADER_MACRO defines[] = {
            {"MSAA_SAMPLES", "1"},
            {nullptr, nullptr}
        };

        // Create vertex shader and input layout
        m_effect_impl->m_effect_helper->create_shader_from_file("FullScreenTriangleVS", DXTOY_HOME L"data/defer/full_screen_triangle.hlsl", device,
                                                                "FullScreenTriangleVS", "vs_5_0", defines);
        m_effect_impl->m_effect_helper->create_shader_from_file("GeometryVS", DXTOY_HOME L"data/defer/gbuffer.hlsl", device,
                                                                "GeometryVS", "vs_5_0", defines, blob.GetAddressOf());
        auto&& input_layout = VertexPosNormalTex::get_input_layout();
        device->CreateInputLayout(input_layout.data(), uint32_t(input_layout.size()), blob->GetBufferPointer(), blob->GetBufferSize(),
                                    m_effect_impl->m_vertex_pos_normal_tex_layout.ReleaseAndGetAddressOf());

        int32_t msaa_samples = 1;
        while (msaa_samples <= 8)
        {
            // Create pixel shaders and compute shaders
            std::string msaaSamplesStr = std::to_string(msaa_samples);
            defines[0].Definition = msaaSamplesStr.c_str();
            std::string shaderNames[] = {
                "GBuffer_" + msaaSamplesStr + "xMSAA_PS",
                "RequiresPerSampleShading_" + msaaSamplesStr + "xMSAA_PS",
                "BasicDeferred_" + msaaSamplesStr + "xMSAA_PS",
                "BasicDeferredPerSample_" + msaaSamplesStr + "xMSAA_PS",
                "DebugNormal_" + msaaSamplesStr + "xMSAA_PS",
                "DebugPosZGrad_" + msaaSamplesStr + "xMSAA_PS",
                "ComputeShaderTileDeferred_" + msaaSamplesStr + "xMSAA_CS"
            };

            m_effect_impl->m_effect_helper->create_shader_from_file(shaderNames[0], DXTOY_HOME L"data/defer/gbuffer.hlsl", device,
                                                                    "GBufferPS", "ps_5_0", defines);
            m_effect_impl->m_effect_helper->create_shader_from_file(shaderNames[1], DXTOY_HOME L"data/defer/gbuffer.hlsl", device,
                                                                    "RequiresPerSampleShadingPS", "ps_5_0", defines);
            m_effect_impl->m_effect_helper->create_shader_from_file(shaderNames[2], DXTOY_HOME L"data/defer/basic_deferred.hlsl", device,
                                                                    "BasicDeferredPS", "ps_5_0", defines);
            m_effect_impl->m_effect_helper->create_shader_from_file(shaderNames[3], DXTOY_HOME L"data/defer/basic_deferred.hlsl", device,
                                                                    "BasicDeferredPerSamplePS", "ps_5_0", defines);
            m_effect_impl->m_effect_helper->create_shader_from_file(shaderNames[4], DXTOY_HOME L"data/defer/gbuffer.hlsl", device,
                                                                    "DebugNormalPS", "ps_5_0", defines);
            m_effect_impl->m_effect_helper->create_shader_from_file(shaderNames[5], DXTOY_HOME L"data/defer/gbuffer.hlsl", device,
                                                                    "DebugPosZGradPS", "ps_5_0", defines);
            m_effect_impl->m_effect_helper->create_shader_from_file(shaderNames[6], DXTOY_HOME L"data/defer/compute_shader_tile.hlsl", device,
                                                                    "ComputeShaderTileDeferredCS", "cs_5_0", defines);

            // Create passes
            EffectPassDesc pass_desc{};
            pass_desc.nameVS = "GeometryVS";
            pass_desc.namePS = shaderNames[0];

            std::string passNames[] = {
                "GBuffer_" + msaaSamplesStr + "xMSAA",
                "Lighting_Basic_MaskStencil_" + msaaSamplesStr + "xMSAA",
                "Lighting_Basic_Deferred_PerPixel_" + msaaSamplesStr + "xMSAA",
                "Lighting_Basic_Deferred_PerSample_" + msaaSamplesStr + "xMSAA",
                "DebugNormal_" + msaaSamplesStr + "xMSAA",
                "DebugPosZGrad_" + msaaSamplesStr + "xMSAA",
                "ComputeShaderTileDeferred_" + msaaSamplesStr + "xMSAA"
            };

            m_effect_impl->m_effect_helper->add_effect_pass(passNames[0], device, &pass_desc);
            {
                auto pass = m_effect_impl->m_effect_helper->get_effect_pass(passNames[0]);
                // Reverse z >= GREATER_EQUAL
                pass->set_depth_stencil_state(RenderStates::dss_greater_equal.Get(), 0);
            }

            pass_desc.nameVS = "FullScreenTriangleVS";
            pass_desc.namePS = shaderNames[1];
            m_effect_impl->m_effect_helper->add_effect_pass(passNames[1], device, &pass_desc);
            {
                auto pass = m_effect_impl->m_effect_helper->get_effect_pass(passNames[1]);
                pass->set_depth_stencil_state(RenderStates::dss_write_stencil.Get(), 1);
            }

            pass_desc.nameVS = "FullScreenTriangleVS";
            pass_desc.namePS = shaderNames[2];
            m_effect_impl->m_effect_helper->add_effect_pass(passNames[2], device, &pass_desc);
            {
                auto pass = m_effect_impl->m_effect_helper->get_effect_pass(passNames[2]);
                pass->set_depth_stencil_state(RenderStates::dss_equal_stencil.Get(), 0);
                pass->set_blend_state(RenderStates::bs_additive.Get(), nullptr, 0xFFFFFFFF);
            }

            pass_desc.nameVS = "FullScreenTriangleVS";
            pass_desc.namePS = shaderNames[3];
            m_effect_impl->m_effect_helper->add_effect_pass(passNames[3], device, &pass_desc);
            {
                auto pass = m_effect_impl->m_effect_helper->get_effect_pass(passNames[3]);
                pass->set_depth_stencil_state(RenderStates::dss_equal_stencil.Get(), 1);
                pass->set_blend_state(RenderStates::bs_additive.Get(), nullptr, 0xFFFFFFFF);
            }

            pass_desc.nameVS = "FullScreenTriangleVS";
            pass_desc.namePS = shaderNames[4];
            m_effect_impl->m_effect_helper->add_effect_pass(passNames[4], device, &pass_desc);

            pass_desc.nameVS = "FullScreenTriangleVS";
            pass_desc.namePS = shaderNames[5];
            m_effect_impl->m_effect_helper->add_effect_pass(passNames[5], device, &pass_desc);

            pass_desc.nameVS = "";
            pass_desc.namePS = "";
            pass_desc.nameCS = shaderNames[6];
            m_effect_impl->m_effect_helper->add_effect_pass(passNames[6], device, &pass_desc);

            msaa_samples <<= 1;
        }

        m_effect_impl->m_effect_helper->set_sampler_state_by_name("g_Sam", RenderStates::ss_anisotropic_wrap_16x.Get());

        // TODO: set debug object name
    }

    void DeferredEffect::set_msaa_samples(uint32_t msaa_samples)
    {
        m_effect_impl->m_msaa_samples = msaa_samples;
    }

    void DeferredEffect::set_lighting_only(bool enable)
    {
        m_effect_impl->m_effect_helper->get_constant_buffer_variable("g_LightingOnly")->set_uint(enable);
    }

    void DeferredEffect::set_face_normals(bool enable)
    {
        m_effect_impl->m_effect_helper->get_constant_buffer_variable("g_FaceNormals")->set_uint(enable);
    }

    void DeferredEffect::set_visualize_light_count(bool enable)
    {
        m_effect_impl->m_effect_helper->get_constant_buffer_variable("g_VisualizeLightCount")->set_uint(enable);
    }

    void DeferredEffect::set_visualize_shading_freq(bool enable)
    {
        m_effect_impl->m_effect_helper->get_constant_buffer_variable("g_VisualizePerSampleShading")->set_uint(enable);
    }

    void DeferredEffect::set_camera_near_far(float nearz, float farz)
    {
        float near_far[4] = { nearz, farz };
        m_effect_impl->m_effect_helper->get_constant_buffer_variable("g_CameraNearFar")->set_float_vector(4, near_far);
    }

    void DeferredEffect::set_gbuffer_render()
    {
        std::string gbuffer_pass_str = "GBuffer_" + std::to_string(m_effect_impl->m_msaa_samples) + "xMSAA";
        m_effect_impl->m_cur_effect_pass = m_effect_impl->m_effect_helper->get_effect_pass(gbuffer_pass_str);
        m_effect_impl->m_cur_input_layout = m_effect_impl->m_vertex_pos_normal_tex_layout.Get();
        m_effect_impl->m_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    }

    void DeferredEffect::debug_normal_gbuffer(ID3D11DeviceContext *device_context, ID3D11RenderTargetView *rtv,
                                                ID3D11ShaderResourceView *normal_gbuffer, D3D11_VIEWPORT viewport)
    {
        // Set full-screen triangle
        device_context->IASetInputLayout(nullptr);
        device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        device_context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
        device_context->RSSetViewports(1, &viewport);

        m_effect_impl->m_effect_helper->set_shader_resource_by_name("g_GBufferTextures[0]", normal_gbuffer);
        std::string passStr = "DebugNormal_" + std::to_string(m_effect_impl->m_msaa_samples) + "xMSAA";
        auto pPass = m_effect_impl->m_effect_helper->get_effect_pass(passStr);
        pPass->apply(device_context);
        device_context->OMSetRenderTargets(1, &rtv, nullptr);
        device_context->Draw(3, 0);

        // Clear
        device_context->OMSetRenderTargets(0, nullptr, nullptr);
        m_effect_impl->m_effect_helper->set_shader_resource_by_name("g_GBufferTextures[0]", nullptr);
        pPass->apply(device_context);
    }

    void DeferredEffect::debug_pos_z_grad_gbuffer(ID3D11DeviceContext *device_context, ID3D11RenderTargetView *rtv,
                                                    ID3D11ShaderResourceView *pos_z_grad_gbuffer,
                                                    D3D11_VIEWPORT viewport)
    {
        // Set full-screen triangle
        device_context->IASetInputLayout(nullptr);
        device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        device_context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
        device_context->RSSetViewports(1, &viewport);

        m_effect_impl->m_effect_helper->set_shader_resource_by_name("g_GBufferTextures[2]", pos_z_grad_gbuffer);
        std::string passStr = "DebugPosZGrad_" + std::to_string(m_effect_impl->m_msaa_samples) + "xMSAA";
        auto pPass = m_effect_impl->m_effect_helper->get_effect_pass(passStr);
        pPass->apply(device_context);
        device_context->OMSetRenderTargets(1, &rtv, nullptr);
        device_context->Draw(3, 0);

        // Clear
        device_context->OMSetRenderTargets(0, nullptr, nullptr);
        int32_t slot = m_effect_impl->m_effect_helper->map_shader_resource_slot("g_GBufferTextures[2]");
        pos_z_grad_gbuffer = nullptr;
        device_context->PSSetShaderResources(slot, 1, &pos_z_grad_gbuffer);
    }

    void DeferredEffect::compute_lighting_default(ID3D11DeviceContext *device_context, ID3D11RenderTargetView *lit_buffer_rtv,
                                                    ID3D11DepthStencilView *depth_buffer_read_only_dsv,
                                                    ID3D11ShaderResourceView *light_buffer_srv,
                                                    ID3D11ShaderResourceView **gbuffers, D3D11_VIEWPORT viewport)
    {
        std::string passStrs[] = {
            "Lighting_Basic_MaskStencil_" + std::to_string(m_effect_impl->m_msaa_samples) + "xMSAA",
            "Lighting_Basic_Deferred_PerPixel_" + std::to_string(m_effect_impl->m_msaa_samples) + "xMSAA",
            "Lighting_Basic_Deferred_PerSample_" + std::to_string(m_effect_impl->m_msaa_samples) + "xMSAA"
        };

        // Clear screen
        const float zeros[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        device_context->ClearRenderTargetView(lit_buffer_rtv, zeros);
        // Full-screen triangle
        device_context->IASetInputLayout(nullptr);
        device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        device_context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
        device_context->RSSetViewports(1, &viewport);
        // Mark per-sample-render area using stencil
        m_effect_impl->m_effect_helper->set_shader_resource_by_name("g_GBufferTextures[0]", gbuffers[0]);
        m_effect_impl->m_effect_helper->set_shader_resource_by_name("g_GBufferTextures[1]", gbuffers[1]);
        m_effect_impl->m_effect_helper->set_shader_resource_by_name("g_GBufferTextures[2]", gbuffers[2]);
        m_effect_impl->m_effect_helper->set_shader_resource_by_name("g_GBufferTextures[3]", gbuffers[3]);
        m_effect_impl->m_effect_helper->set_shader_resource_by_name("g_Light", light_buffer_srv);
        if (m_effect_impl->m_msaa_samples > 1)
        {
            m_effect_impl->m_effect_helper->get_effect_pass(passStrs[0])->apply(device_context);
            device_context->OMSetRenderTargets(0, nullptr, depth_buffer_read_only_dsv);
            device_context->Draw(3, 0);
        }

        // 通过模板测试来绘制逐像素着色的区域
        ID3D11RenderTargetView* pRTVs[1] = { lit_buffer_rtv };
        device_context->OMSetRenderTargets(1, pRTVs, depth_buffer_read_only_dsv);

        m_effect_impl->m_effect_helper->get_effect_pass(passStrs[1])->apply(device_context);
        device_context->Draw(3, 0);

        // 通过模板测试来绘制逐样本着色的区域
        if (m_effect_impl->m_msaa_samples > 1)
        {
            m_effect_impl->m_effect_helper->get_effect_pass(passStrs[2])->apply(device_context);
            device_context->Draw(3, 0);
        }

        // 清空
        device_context->OMSetRenderTargets(0, nullptr, nullptr);
        ID3D11ShaderResourceView* nullSRVs[8]{};
        device_context->VSSetShaderResources(0, 8, nullSRVs);
        device_context->PSSetShaderResources(0, 8, nullSRVs);
    }

    void DeferredEffect::compute_tiled_light_culling(ID3D11DeviceContext *device_context,
                                                        ID3D11UnorderedAccessView *lit_flat_buffer_uav,
                                                        ID3D11ShaderResourceView *light_buffer_srv,
                                                        ID3D11ShaderResourceView **gbuffers)
    {
        // Do not need clear operation
        com_ptr<ID3D11Texture2D> tex = nullptr;
        gbuffers[0]->GetResource(reinterpret_cast<ID3D11Resource**>(tex.GetAddressOf()));
        D3D11_TEXTURE2D_DESC texDesc;
        tex->GetDesc(&texDesc);

        UINT dims[2] = { texDesc.Width, texDesc.Height };
        m_effect_impl->m_effect_helper->get_constant_buffer_variable("g_FramebufferDimensions")->set_uint_vector(2, dims);
        m_effect_impl->m_effect_helper->set_shader_resource_by_name("g_GBufferTextures[0]", gbuffers[0]);
        m_effect_impl->m_effect_helper->set_shader_resource_by_name("g_GBufferTextures[1]", gbuffers[1]);
        m_effect_impl->m_effect_helper->set_shader_resource_by_name("g_GBufferTextures[2]", gbuffers[2]);
        m_effect_impl->m_effect_helper->set_shader_resource_by_name("g_GBufferTextures[3]", gbuffers[3]);
        m_effect_impl->m_effect_helper->set_shader_resource_by_name("g_Light", light_buffer_srv);
        m_effect_impl->m_effect_helper->set_unordered_access_by_name("g_Framebuffer", lit_flat_buffer_uav, 0);

        std::string passName = "ComputeShaderTileDeferred_" + std::to_string(m_effect_impl->m_msaa_samples) + "xMSAA";
        auto pPass = m_effect_impl->m_effect_helper->get_effect_pass(passName);
        pPass->apply(device_context);
        pPass->dispatch(device_context, texDesc.Width, texDesc.Height, 1);

        // Clear
        int32_t slot = m_effect_impl->m_effect_helper->map_unordered_access_slot("g_Framebuffer");
        lit_flat_buffer_uav = nullptr;
        device_context->CSSetUnorderedAccessViews(slot, 1, &lit_flat_buffer_uav, nullptr);

        slot = m_effect_impl->m_effect_helper->map_shader_resource_slot("g_Light");
        light_buffer_srv = nullptr;
        device_context->CSSetShaderResources(slot, 1, &light_buffer_srv);

        ID3D11ShaderResourceView* nullSRVs[4] = {};
        slot = m_effect_impl->m_effect_helper->map_shader_resource_slot("g_GBufferTextures[0]");
        device_context->CSSetShaderResources(slot, 4, nullSRVs);
    }

    void XM_CALLCONV DeferredEffect::set_world_matrix(DirectX::FXMMATRIX world)
    {
        DirectX::XMStoreFloat4x4(&m_effect_impl->m_world, world);
    }

    void XM_CALLCONV DeferredEffect::set_view_matrix(DirectX::FXMMATRIX view)
    {
        DirectX::XMStoreFloat4x4(&m_effect_impl->m_view, view);
    }

    void XM_CALLCONV DeferredEffect::set_proj_matrix(DirectX::FXMMATRIX proj)
    {
        DirectX::XMStoreFloat4x4(&m_effect_impl->m_proj, proj);
    }

    void DeferredEffect::set_material(const model::Material &material)
    {
        auto&& texture_manager = model::TextureManagerHandle::get();

        auto texture_id_str = material.try_get<std::string>("$Diffuse");
        m_effect_impl->m_effect_helper->set_shader_resource_by_name("g_DiffuseMap",
                                                                    texture_id_str ? texture_manager.get_texture(*texture_id_str) : texture_manager.get_null_texture());
    }

    MeshDataInput DeferredEffect::get_input_data(const model::MeshData &mesh_data)
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

    void DeferredEffect::apply(ID3D11DeviceContext *device_context)
    {
        using namespace DirectX;
        XMMATRIX world = XMLoadFloat4x4(&m_effect_impl->m_world);
        XMMATRIX view = XMLoadFloat4x4(&m_effect_impl->m_view);
        XMMATRIX proj = XMLoadFloat4x4(&m_effect_impl->m_proj);

        XMMATRIX world_view = world * view;
        XMMATRIX world_view_proj = world_view * proj;
        XMMATRIX world_inv_t_view = XMath::inverse_transpose(world) * view;
        XMMATRIX inv_view = XMMatrixInverse(nullptr, view);
        XMMATRIX view_proj = view * proj;

        world_view = XMMatrixTranspose(world_view);
        world_view_proj = XMMatrixTranspose(world_view_proj);
        inv_view = XMMatrixTranspose(inv_view);
        world_inv_t_view = XMMatrixTranspose(world_inv_t_view);
        proj = XMMatrixTranspose(proj);
        view_proj = XMMatrixTranspose(view_proj);

        m_effect_impl->m_effect_helper->get_constant_buffer_variable("g_WorldInvTransposeView")->set_float_matrix(4, 4, (float*)&world_inv_t_view);
        m_effect_impl->m_effect_helper->get_constant_buffer_variable("g_WorldViewProj")->set_float_matrix(4, 4, (float*)&world_view_proj);
        m_effect_impl->m_effect_helper->get_constant_buffer_variable("g_WorldView")->set_float_matrix(4, 4, (float*)&world_view);
        m_effect_impl->m_effect_helper->get_constant_buffer_variable("g_InvView")->set_float_matrix(4, 4, (float*)&inv_view);
        m_effect_impl->m_effect_helper->get_constant_buffer_variable("g_ViewProj")->set_float_matrix(4, 4, (float*)&view_proj);
        m_effect_impl->m_effect_helper->get_constant_buffer_variable("g_Proj")->set_float_matrix(4, 4, (float*)&proj);

        if (m_effect_impl->m_cur_effect_pass)
        {
            m_effect_impl->m_cur_effect_pass->apply(device_context);
        }
    }
}










