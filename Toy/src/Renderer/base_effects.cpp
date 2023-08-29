//
// Created by ZHIKANG on 2023/5/27.
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
    struct BasicEffect::EffectImpl
    {
        EffectImpl() = default;
        ~EffectImpl() = default;

        std::unique_ptr<EffectHelper> m_effect_helper;
        std::shared_ptr<IEffectPass> m_curr_effect_pass;
        com_ptr<ID3D11InputLayout> m_vertex_pos_normal_tex_layout;
        com_ptr<ID3D11InputLayout> m_curr_input_layout;

        D3D11_PRIMITIVE_TOPOLOGY m_curr_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        DirectX::XMFLOAT4X4 m_world{};
        DirectX::XMFLOAT4X4 m_view{};
        DirectX::XMFLOAT4X4 m_proj{};
    };

    BasicEffect::BasicEffect()
    {
        m_effect_impl = std::make_unique<EffectImpl>();
    }

    BasicEffect::~BasicEffect() noexcept = default;

    BasicEffect::BasicEffect(toy::BasicEffect &&other) noexcept
    {
        m_effect_impl.swap(other.m_effect_impl);
    }

    BasicEffect& BasicEffect::operator=(toy::BasicEffect &&other) noexcept
    {
        m_effect_impl.swap(other.m_effect_impl);
        return *this;
    }

    void BasicEffect::init(ID3D11Device *device)
    {
        m_effect_impl->m_effect_helper = std::make_unique<EffectHelper>();

        com_ptr<ID3DBlob> blob = nullptr;
        // Create vertex shader, requires a compiled vertex shader file
        m_effect_impl->m_effect_helper->create_shader_from_file("base_3d_vs", L"../data/shaders/base_3d_vs.cso", device,
                                                                nullptr, nullptr, nullptr, blob.GetAddressOf());
        // Create vertex layout
        auto&& input_layout = VertexPosNormalTex::get_input_layout();
        device->CreateInputLayout(input_layout.data(), (uint32_t)input_layout.size(),
                                        blob->GetBufferPointer(), blob->GetBufferSize(),
                                        m_effect_impl->m_vertex_pos_normal_tex_layout.GetAddressOf());

        // Create pixel shader, requires a compiled pixel shader file
        m_effect_impl->m_effect_helper->create_shader_from_file("base_3d_ps", L"../data/shaders/base_3d_ps.cso", device);

        // Create render pass
        EffectPassDesc pass_desc{};
        pass_desc.nameVS = "base_3d_vs";
        pass_desc.namePS = "base_3d_ps";
        m_effect_impl->m_effect_helper->add_effect_pass("base", device, &pass_desc);
        m_effect_impl->m_effect_helper->set_sampler_state_by_name("g_Sam", RenderStates::ss_linear_wrap.Get());

        // TODO: Set debug object name
#if defined(GRAPHICS_DEBUGGER_OBJECT_NAME)
        set_debug_object_name(m_effect_impl->m_vertex_pos_normal_tex_layout.Get(), "BasicEffect.VertexPosNormalTexLayout");
#endif
    }

    void XM_CALLCONV BasicEffect::set_world_matrix(DirectX::FXMMATRIX world)
    {
        DirectX::XMStoreFloat4x4(&m_effect_impl->m_world, world);
    }

    void XM_CALLCONV BasicEffect::set_view_matrix(DirectX::FXMMATRIX view)
    {
        DirectX::XMStoreFloat4x4(&m_effect_impl->m_view, view);
    }

    void XM_CALLCONV BasicEffect::set_proj_matrix(DirectX::FXMMATRIX proj)
    {
        DirectX::XMStoreFloat4x4(&m_effect_impl->m_proj, proj);
    }

    void BasicEffect::set_material(const model::Material &material)
    {
        auto&& texture_manager = model::TextureManagerHandle::get();

        // Color value
        PhongMaterial phong_mat{};
        phong_mat.ambient = material.get<DirectX::XMFLOAT4>("$AmbientColor");
        phong_mat.diffuse = material.get<DirectX::XMFLOAT4>("$DiffuseColor");
        phong_mat.diffuse.w = material.get<float>("$Opacity");
        phong_mat.specular = material.get<DirectX::XMFLOAT4>("$SpecularColor");
        phong_mat.specular.w = material.has<float>("$SpecularFactor") ? material.get<float>("$SpecularFactor") : 1.0f;
        phong_mat.reflect = material.get<DirectX::XMFLOAT4>("$ReflectColor");
        m_effect_impl->m_effect_helper->get_constant_buffer_variable("g_Material")->set_raw(&phong_mat);
        // Texture
        auto texture_str_id = material.try_get<std::string>("$Diffuse");
        m_effect_impl->m_effect_helper->set_shader_resource_by_name("g_DiffuseMap", texture_str_id ? texture_manager.get_texture(*texture_str_id) : texture_manager.get_null_texture());
    }

    MeshDataInput BasicEffect::get_input_data(const model::MeshData &mesh_data)
    {
        // Attention: as we have used multiple vertex buffers, such as position buffer, normal buffer, etc.
        // A stride array and offset array must be specified
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

    void BasicEffect::set_dir_light(uint32_t pos, const toy::DirectionalLight &dir_light)
    {
        m_effect_impl->m_effect_helper->get_constant_buffer_variable("g_DirLight")->set_raw(&dir_light,
                                                                                        sizeof(dir_light) * pos, sizeof(dir_light));
    }

    void BasicEffect::set_point_light(uint32_t pos, const toy::PointLight &point_light)
    {
        m_effect_impl->m_effect_helper->get_constant_buffer_variable("g_PointLight")->set_raw(&point_light,
                                                                                            sizeof(point_light) * pos, sizeof(point_light));
    }

    void BasicEffect::set_eye_pos(const DirectX::XMFLOAT3 &eye_pos)
    {
        m_effect_impl->m_effect_helper->get_constant_buffer_variable("g_EyePosW")->set_float_vector(3, reinterpret_cast<const float *>(&eye_pos));
    }

    void BasicEffect::set_reflection_enabled(bool enable)
    {
        m_effect_impl->m_effect_helper->get_constant_buffer_variable("g_ReflectionEnabled")->set_sint(enable);
    }

    void BasicEffect::set_refraction_enabled(bool enable)
    {
        m_effect_impl->m_effect_helper->get_constant_buffer_variable("g_RefractionEnabled")->set_sint(enable);
    }

    void BasicEffect::set_refraction_eta(float eta)
    {
        m_effect_impl->m_effect_helper->get_constant_buffer_variable("g_Eta")->set_float(eta);
    }

    void BasicEffect::set_default_render()
    {
        m_effect_impl->m_curr_effect_pass = m_effect_impl->m_effect_helper->get_effect_pass("base");
        m_effect_impl->m_curr_input_layout = m_effect_impl->m_vertex_pos_normal_tex_layout;
        m_effect_impl->m_curr_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    }

    void BasicEffect::set_texture_cube(ID3D11ShaderResourceView *texture_cube)
    {
        m_effect_impl->m_effect_helper->set_shader_resource_by_name("g_TexCube", texture_cube);
    }

    void BasicEffect::apply(ID3D11DeviceContext *device_context)
    {
        using namespace DirectX;

        XMMATRIX world = XMLoadFloat4x4(&m_effect_impl->m_world);
        XMMATRIX view = XMLoadFloat4x4(&m_effect_impl->m_view);
        XMMATRIX proj = XMLoadFloat4x4(&m_effect_impl->m_proj);

        XMMATRIX view_proj = view * proj;
        XMMATRIX world_inv_tran = XMath::inverse_transpose(world);

        world = XMMatrixTranspose(world);
        view_proj = XMMatrixTranspose(view_proj);
        world_inv_tran = XMMatrixTranspose(world_inv_tran);

        m_effect_impl->m_effect_helper->get_constant_buffer_variable("g_WorldInvTranspose")->set_float_matrix(4, 4, (float*)&world_inv_tran);
        m_effect_impl->m_effect_helper->get_constant_buffer_variable("g_ViewProj")->set_float_matrix(4, 4, (float*)&view_proj);
        m_effect_impl->m_effect_helper->get_constant_buffer_variable("g_World")->set_float_matrix(4, 4, (float*)&world);

        if (m_effect_impl->m_curr_effect_pass)
        {
            m_effect_impl->m_curr_effect_pass->apply(device_context);
        }
    }
}









































