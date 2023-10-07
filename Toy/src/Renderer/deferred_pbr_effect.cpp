//
// Created by ZZK on 2023/9/30.
//

#include <Toy/Renderer/effects.h>
#include <Toy/Renderer/render_states.h>
#include <Toy/Geometry/vertex.h>
#include <Toy/Model/mesh_data.h>
#include <Toy/Model/texture_manager.h>
#include <Toy/Model/material.h>
#include <Toy/Renderer/taa_settings.h>

namespace toy
{
    // Note: ensure material semantics must correspond one-to-one with shader semantics
    struct ShaderSemantics
    {
        std::string_view constant_buffer_semantics = {};
        std::string_view shader_resource_view_semantics = {};
    };

    static std::unordered_map<model::MaterialSemantics, ShaderSemantics> g_ms_ss_map{
        { model::MaterialSemantics::DiffuseMap,   ShaderSemantics{ "gNoDiffuseSrv", "gAlbedoMap" } },
        { model::MaterialSemantics::NormalMap,    ShaderSemantics{ "gNoNormalSrv", "gNormalMap" } },
        { model::MaterialSemantics::MetalnessMap, ShaderSemantics{ "gNoMetalnessSrv", "gMetalnessMap" } },
        { model::MaterialSemantics::RoughnessMap, ShaderSemantics{ "gNoRoughnessSrv", "gRoughnessMap" } },
    };

    struct DeferredPBREffect::EffectImpl
    {
        EffectImpl() = default;
        ~EffectImpl() = default;

        std::unique_ptr<EffectHelper> effect_helper = nullptr;
        std::shared_ptr<IEffectPass> cur_effect_pass = nullptr;
        com_ptr<ID3D11InputLayout> cur_vertex_layout = nullptr;
        com_ptr<ID3D11InputLayout> vertex_layout = nullptr;

        std::string_view geometry_pass = {};
        std::string_view deferred_lighting_pass = {};

        DirectX::XMFLOAT4X4 world_matrix = {};
        DirectX::XMFLOAT4X4 view_matrix = {};
        DirectX::XMFLOAT4X4 proj_matrix = {};

        DirectX::XMFLOAT4X4 pre_world_matrix = {};
        DirectX::XMFLOAT4X4 pre_view_proj_matrix = {};
        DirectX::XMFLOAT4X4 unjittered_view_proj_matrix = {};

        int32_t viewer_width = 0;
        int32_t viewer_height = 0;

        D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

        void set_material(const model::Material &material, model::MaterialSemantics material_semantics) const
        {
            using namespace toy::model;
            auto&& texture_manager = TextureManager::get();

            auto texture_map_name = material.try_get<std::string>(material_semantics_name(material_semantics));
            auto constant_buffer_semantics = g_ms_ss_map[material_semantics].constant_buffer_semantics;
            auto shader_resource_view_semantics = g_ms_ss_map[material_semantics].shader_resource_view_semantics;
            if (texture_map_name)
            {
                effect_helper->get_constant_buffer_variable(constant_buffer_semantics)->set_uint(0);
                effect_helper->set_shader_resource_by_name(shader_resource_view_semantics, texture_manager.get_texture(*texture_map_name));
            } else
            {
                effect_helper->get_constant_buffer_variable(constant_buffer_semantics)->set_uint(1);
                effect_helper->set_shader_resource_by_name(shader_resource_view_semantics, texture_manager.get_null_texture()); // White texture
                if (material_semantics == MaterialSemantics::DiffuseMap)
                {
                    // Currently do not consider opacity property
                    auto diffuse_color = material.try_get<DirectX::XMFLOAT4>(material_semantics_name(MaterialSemantics::DiffuseColor));
                    float base_color_opacity[4] = { diffuse_color->x, diffuse_color->y, diffuse_color->z, diffuse_color->w };
                    effect_helper->get_constant_buffer_variable("gBaseColorOpacity")->set_float_vector(4, base_color_opacity);
                } else if (material_semantics == MaterialSemantics::MetalnessMap)
                {
                    auto metalness_value = material.try_get<float>(material_semantics_name(MaterialSemantics::Metalness));
                    effect_helper->get_constant_buffer_variable("gMetalness")->set_float(*metalness_value);
                } else if (material_semantics == MaterialSemantics::RoughnessMap)
                {
                    auto roughness_value = material.try_get<float>(material_semantics_name(MaterialSemantics::Roughness));
                    effect_helper->get_constant_buffer_variable("gRoughness")->set_float(*roughness_value);
                }
            }
        }
    };

    DeferredPBREffect::DeferredPBREffect()
    {
        m_effect_impl = std::make_unique<EffectImpl>();
    }

    DeferredPBREffect::~DeferredPBREffect() noexcept = default;

    DeferredPBREffect::DeferredPBREffect(toy::DeferredPBREffect &&other) noexcept
    {
        m_effect_impl.swap(other.m_effect_impl);
    }

    DeferredPBREffect& DeferredPBREffect::operator=(toy::DeferredPBREffect &&other) noexcept
    {
        m_effect_impl.swap(other.m_effect_impl);
        return *this;
    }

    DeferredPBREffect& DeferredPBREffect::get()
    {
        static DeferredPBREffect deferred_effect{};
        return deferred_effect;
    }

    void DeferredPBREffect::init(ID3D11Device *device)
    {
        m_effect_impl->effect_helper = std::make_unique<EffectHelper>();
        m_effect_impl->effect_helper->set_binary_cache_directory(DXTOY_HOME L"data/pbr/cache");

        // Set shader name and pass name
        std::string_view geometry_vs = "GeometryVS";
        std::string_view gbuffer_ps = "GBufferPS";
        std::string_view screen_triangle_vs = "ScreenTriangleVS";
        std::string_view deferred_pbr_ps = "DeferredPBRPS";
        m_effect_impl->geometry_pass = "GeometryPass";
        m_effect_impl->deferred_lighting_pass = "DeferredLightingPass";

        // Create vertex and pixel shaders and input layout
        com_ptr<ID3DBlob> blob = nullptr;
        m_effect_impl->effect_helper->create_shader_from_file(geometry_vs, DXTOY_HOME L"data/pbr/geometry_vs.hlsl", device,
                                                                "VS", "vs_5_0", nullptr, blob.GetAddressOf());
        m_effect_impl->effect_helper->create_shader_from_file(gbuffer_ps, DXTOY_HOME L"data/pbr/gbuffer.hlsl", device,
                                                                "PS", "ps_5_0");

        auto&& input_layout = VertexPosNormalTangentTex::get_input_layout();
        device->CreateInputLayout(input_layout.data(), uint32_t(input_layout.size()), blob->GetBufferPointer(), blob->GetBufferSize(),
                                    m_effect_impl->vertex_layout.ReleaseAndGetAddressOf());
        if (!m_effect_impl->vertex_layout)
        {
            DX_CORE_CRITICAL("Fail to create vertex layout");
        }

        m_effect_impl->effect_helper->create_shader_from_file(screen_triangle_vs, DXTOY_HOME L"data/pbr/screen_triangle_vs.hlsl", device,
                                                                "VS", "vs_5_0", nullptr, blob.ReleaseAndGetAddressOf());
        m_effect_impl->effect_helper->create_shader_from_file(deferred_pbr_ps, DXTOY_HOME L"data/pbr/deferred_pbr.hlsl", device,
                                                                "PS", "ps_5_0", nullptr, blob.ReleaseAndGetAddressOf());

        // Create geometry and deferred lighting passes
        EffectPassDesc pass_desc{};
        pass_desc.nameVS = geometry_vs;
        pass_desc.namePS = gbuffer_ps;
        m_effect_impl->effect_helper->add_effect_pass(m_effect_impl->geometry_pass, device, &pass_desc);
        {
            auto pass = m_effect_impl->effect_helper->get_effect_pass(m_effect_impl->geometry_pass);
            // Reverse z >= GREATER_EQUAL
            pass->set_depth_stencil_state(RenderStates::dss_less_equal.Get(), 0);
        }

        pass_desc.nameVS = screen_triangle_vs;
        pass_desc.namePS = deferred_pbr_ps;
        m_effect_impl->effect_helper->add_effect_pass(m_effect_impl->deferred_lighting_pass, device, &pass_desc);

        // Set sampler state
        m_effect_impl->effect_helper->set_sampler_state_by_name("gSamLinearWrap", RenderStates::ss_linear_wrap.Get());
        m_effect_impl->effect_helper->set_sampler_state_by_name("gSamAnisotropicWrap", RenderStates::ss_anisotropic_wrap_16x.Get());
    }

    void DeferredPBREffect::set_material(const model::Material &material)
    {
        using namespace toy::model;
        m_effect_impl->set_material(material, MaterialSemantics::DiffuseMap);
        m_effect_impl->set_material(material, MaterialSemantics::NormalMap);
        m_effect_impl->set_material(material, MaterialSemantics::MetalnessMap);
        m_effect_impl->set_material(material, MaterialSemantics::RoughnessMap);
    }

    void DeferredPBREffect::set_viewer_size(int32_t width, int32_t height)
    {
        m_effect_impl->viewer_width = width;
        m_effect_impl->viewer_height = height;
    }

    void DeferredPBREffect::set_camera_near_far(float nearz, float farz)
    {
//        m_effect_impl->effect_helper->get_constant_buffer_variable("gNearZ")->set_float(nearz);
//        m_effect_impl->effect_helper->get_constant_buffer_variable("gFarZ")->set_float(farz);
    }

    void DeferredPBREffect::set_camera_position(DirectX::XMFLOAT3 camera_position)
    {
        float camera_world_position[4] = { camera_position.x, camera_position.y, camera_position.z, 1.0f };
        m_effect_impl->effect_helper->get_constant_buffer_variable("gEyeWorldPos")->set_float_vector(4, camera_world_position);
    }

    MeshDataInput DeferredPBREffect::get_input_data(const model::MeshData &mesh_data)
    {
        // TODO: check layout
        MeshDataInput input;
        input.input_layout = m_effect_impl->cur_vertex_layout.Get();
        input.topology = m_effect_impl->topology;
        input.vertex_buffers = {
            mesh_data.vertices.Get(),
            mesh_data.normals.Get(),
            mesh_data.tangents.Get(),
            mesh_data.texcoord_arrays.empty() ? nullptr : mesh_data.texcoord_arrays[0].Get()
        };
        input.strides = { 12, 12, 16, 8 };
        input.offsets = { 0, 0, 0, 0 };

        input.index_buffer = mesh_data.indices.Get();
        input.index_count = mesh_data.index_count;

        return input;
    }

    void DeferredPBREffect::set_gbuffer_render()
    {
        m_effect_impl->cur_effect_pass = m_effect_impl->effect_helper->get_effect_pass(m_effect_impl->geometry_pass);
        m_effect_impl->cur_vertex_layout = m_effect_impl->vertex_layout.Get();
        m_effect_impl->topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    }

    void DeferredPBREffect::apply(ID3D11DeviceContext *device_context)
    {
        using namespace DirectX;

        static bool first_frame = true;
        static uint32_t taa_frame_counter = 0;

        XMMATRIX world = XMLoadFloat4x4(&m_effect_impl->world_matrix);
        XMMATRIX view = XMLoadFloat4x4(&m_effect_impl->view_matrix);
        XMMATRIX proj = XMLoadFloat4x4(&m_effect_impl->proj_matrix);
        XMMATRIX unjittered_view_proj =  XMMatrixTranspose(view * proj);

        if (first_frame)
        {
            XMStoreFloat4x4(&m_effect_impl->pre_view_proj_matrix, unjittered_view_proj);
            XMStoreFloat4x4(&m_effect_impl->pre_world_matrix, XMMatrixTranspose(world));
            first_frame = false;
        }

        float jitter_x = taa::s_halton_2[taa_frame_counter] / static_cast<float>(m_effect_impl->viewer_width) * taa::s_taa_jitter_distance;
        float jitter_y = taa::s_halton_3[taa_frame_counter] / static_cast<float>(m_effect_impl->viewer_height) * taa::s_taa_jitter_distance;
        proj.r[2].m128_f32[0] += jitter_x;
        proj.r[2].m128_f32[1] += jitter_y;
//        taa::s_taa_jitter_x = jitter_x / 2.0f;
//        taa::s_taa_jitter_y = -jitter_y / 2.0f;

        XMMATRIX world_view = world * view;
        XMMATRIX world_view_proj = world_view * proj;
//        XMMATRIX world_inv_t_view = XMath::inverse_transpose(world) * view;
//        XMMATRIX inv_view = XMMatrixInverse(nullptr, view);
        XMMATRIX view_proj = view * proj;

//        world_view = XMMatrixTranspose(world_view);
        world_view_proj = XMMatrixTranspose(world_view_proj);
//        inv_view = XMMatrixTranspose(inv_view);
//        world_inv_t_view = XMMatrixTranspose(world_inv_t_view);
//        proj = XMMatrixTranspose(proj);
        view_proj = XMMatrixTranspose(view_proj);
        world = XMMatrixTranspose(world);

        m_effect_impl->effect_helper->get_constant_buffer_variable("gWorld")->set_float_matrix(4, 4, (float*)&world);
        m_effect_impl->effect_helper->get_constant_buffer_variable("gViewProj")->set_float_matrix(4, 4, (float*)&view_proj);
        m_effect_impl->effect_helper->get_constant_buffer_variable("gWorldViewProj")->set_float_matrix(4, 4, (float*)&world_view_proj);
        XMMATRIX pre_world = XMLoadFloat4x4(&m_effect_impl->pre_world_matrix);
        XMMATRIX pre_view_proj = XMLoadFloat4x4(&m_effect_impl->pre_view_proj_matrix);
        m_effect_impl->effect_helper->get_constant_buffer_variable("gPreWorld")->set_float_matrix(4, 4, (float*)&pre_world);
        m_effect_impl->effect_helper->get_constant_buffer_variable("gPreViewProj")->set_float_matrix(4, 4, (float*)&pre_view_proj);
        m_effect_impl->effect_helper->get_constant_buffer_variable("gUnjitteredViewProj")->set_float_matrix(4, 4, (float*)&unjittered_view_proj);

        if (m_effect_impl->cur_effect_pass)
        {
            m_effect_impl->cur_effect_pass->apply(device_context);
        }

        XMStoreFloat4x4(&m_effect_impl->pre_world_matrix, world);
        XMStoreFloat4x4(&m_effect_impl->pre_view_proj_matrix, unjittered_view_proj);
        taa_frame_counter = (taa_frame_counter + 1) % taa::s_taa_sample;
    }

    void DeferredPBREffect::deferred_lighting_pass(ID3D11DeviceContext *device_context, ID3D11RenderTargetView *lit_buffer_rtv,
                                                    ID3D11ShaderResourceView **gbuffers, D3D11_VIEWPORT viewport)
    {
        // Clear render target view
//        static const float zeros[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
//        device_context->ClearRenderTargetView(lit_buffer_rtv, zeros);

        // Full-screen triangle
        device_context->IASetInputLayout(nullptr);
        device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        device_context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
        device_context->RSSetViewports(1, &viewport);

        // Bind GBuffer shader resource view
        m_effect_impl->effect_helper->set_shader_resource_by_name("gGeometryAlbedoMetalness", gbuffers[0]);
        m_effect_impl->effect_helper->set_shader_resource_by_name("gGeometryNormalRoughness", gbuffers[1]);
        m_effect_impl->effect_helper->set_shader_resource_by_name("gGeometryWorldPosition", gbuffers[2]);
        if (auto&& preprocess_effect = PreProcessEffect::get(); preprocess_effect.is_ready())
        {
            m_effect_impl->effect_helper->get_constant_buffer_variable("gNoPreprocess")->set_uint(0);
            m_effect_impl->effect_helper->set_shader_resource_by_name("gPrefilteredSpecularMap", preprocess_effect.get_environment_srv());
            m_effect_impl->effect_helper->set_shader_resource_by_name("gIrradianceMap", preprocess_effect.get_irradiance_srv());
            m_effect_impl->effect_helper->set_shader_resource_by_name("gBRDFLUT", preprocess_effect.get_brdf_srv());
        } else
        {
            m_effect_impl->effect_helper->get_constant_buffer_variable("gNoPreprocess")->set_uint(1);
        }

        // Bind render target view
        device_context->OMSetRenderTargets(1, &lit_buffer_rtv, nullptr);

        // Apply pass and draw
        auto pass = m_effect_impl->effect_helper->get_effect_pass(m_effect_impl->deferred_lighting_pass);
        pass->apply(device_context);
        device_context->Draw(3, 0);

        // Clear bindings
        device_context->OMSetRenderTargets(0, nullptr, nullptr);
        m_effect_impl->effect_helper->set_shader_resource_by_name("gGeometryAlbedoMetalness", nullptr);
        m_effect_impl->effect_helper->set_shader_resource_by_name("gGeometryNormalRoughness", nullptr);
        m_effect_impl->effect_helper->set_shader_resource_by_name("gGeometryWorldPosition", nullptr);
        m_effect_impl->effect_helper->set_shader_resource_by_name("gPrefilteredSpecularMap", nullptr);
        m_effect_impl->effect_helper->set_shader_resource_by_name("gIrradianceMap", nullptr);
        m_effect_impl->effect_helper->set_shader_resource_by_name("gBRDFLUT", nullptr);
        pass->apply(device_context);
    }

    void XM_CALLCONV DeferredPBREffect::set_world_matrix(DirectX::FXMMATRIX world)
    {
        DirectX::XMStoreFloat4x4(&m_effect_impl->world_matrix, world);
    }

    void XM_CALLCONV DeferredPBREffect::set_view_matrix(DirectX::FXMMATRIX view)
    {
        DirectX::XMStoreFloat4x4(&m_effect_impl->view_matrix, view);
    }

    void XM_CALLCONV DeferredPBREffect::set_proj_matrix(DirectX::FXMMATRIX proj)
    {
        DirectX::XMStoreFloat4x4(&m_effect_impl->proj_matrix, proj);
    }
}






















