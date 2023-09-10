//
// Created by ZZK on 2023/9/3.
//

#include <Toy/Renderer/effects.h>
#include <Toy/Renderer/render_states.h>
#include <Toy/Geometry/vertex.h>
#include <Toy/Model/mesh_data.h>
#include <Toy/Model/texture_manager.h>
#include <Toy/Model/material.h>
#include <Toy/Renderer/buffer.h>
#include <Toy/Renderer/texture_2d.h>

namespace toy
{
    struct SpecularMapFilterSettingCB
    {
        float roughness = 0.0f;
        float padding[3] = { 0.0f, 0.0f, 0.0f };
    };

    struct PreProcessEffect::EffectImpl
    {
        EffectImpl() = default;
        ~EffectImpl() = default;

        std::unique_ptr<TextureCube> cube_texture = nullptr;
        std::unique_ptr<TextureCube> env_texture = nullptr;
        std::unique_ptr<TextureCube> ir_map_texture = nullptr;
        std::unique_ptr<Texture2D> sp_brdf_texture = nullptr;
        std::unique_ptr<Buffer> sp_map_cb = nullptr;

        std::unique_ptr<EffectHelper> m_effect_helper = nullptr;

        std::string_view equirect_to_cube_pass{};
        std::string_view sp_env_map_pass{};
        std::string_view ir_map_pass{};
        std::string_view brdf_lut_pass{};
    };

    PreProcessEffect::PreProcessEffect() : m_effect_impl(std::make_unique<EffectImpl>()) {}

    PreProcessEffect::~PreProcessEffect() = default;

    PreProcessEffect::PreProcessEffect(toy::PreProcessEffect &&other) noexcept
    {
        m_effect_impl.swap(other.m_effect_impl);
    }

    PreProcessEffect& PreProcessEffect::operator=(toy::PreProcessEffect &&other) noexcept
    {
        m_effect_impl.swap(other.m_effect_impl);
        return *this;
    }

    PreProcessEffect& PreProcessEffect::get()
    {
        static PreProcessEffect pre_process_effect{};
        return pre_process_effect;
    }

    void PreProcessEffect::init(ID3D11Device *device)
    {
        m_effect_impl->m_effect_helper = std::make_unique<EffectHelper>();
        m_effect_impl->m_effect_helper->set_binary_cache_directory(DXTOY_HOME L"data/defer/cache");

        // Create computer shaders
        std::string_view equirect_to_cube_cs = "EquirectToCube";
        std::string_view sp_env_map_cs = "SpEnvMap";
        std::string_view irradiance_map_cs = "IrradianceMap";
        std::string_view brdf_lut_cs = "BRDF_LUT";
        m_effect_impl->equirect_to_cube_pass = "EqToCubePass";
        m_effect_impl->sp_env_map_pass = "SpEnvMapPass";
        m_effect_impl->ir_map_pass = "IrMapPass";
        m_effect_impl->brdf_lut_pass = "BRDFLUTPass";

        m_effect_impl->m_effect_helper->create_shader_from_file(equirect_to_cube_cs, DXTOY_HOME L"data/defer/equirect_to_cube.hlsl", device,
                                                                "main", "cs_5_0");
        m_effect_impl->m_effect_helper->create_shader_from_file(sp_env_map_cs, DXTOY_HOME L"data/defer/sp_env_map.hlsl", device,
                                                                "main", "cs_5_0");
        m_effect_impl->m_effect_helper->create_shader_from_file(irradiance_map_cs, DXTOY_HOME L"data/defer/irradiance_map.hlsl", device,
                                                                "main", "cs_5_0");
        m_effect_impl->m_effect_helper->create_shader_from_file(brdf_lut_cs, DXTOY_HOME L"data/defer/sp_brdf.hlsl", device,
                                                                "main", "cs_5_0");

        // Create computer passes
        EffectPassDesc pass_desc{};
        pass_desc.nameCS = equirect_to_cube_cs;
        m_effect_impl->m_effect_helper->add_effect_pass(m_effect_impl->equirect_to_cube_pass, device, &pass_desc);
        pass_desc.nameCS = sp_env_map_cs;
        m_effect_impl->m_effect_helper->add_effect_pass(m_effect_impl->sp_env_map_pass, device, &pass_desc);
        pass_desc.nameCS = irradiance_map_cs;
        m_effect_impl->m_effect_helper->add_effect_pass(m_effect_impl->ir_map_pass, device, &pass_desc);
        pass_desc.nameCS = brdf_lut_cs;
        m_effect_impl->m_effect_helper->add_effect_pass(m_effect_impl->brdf_lut_pass, device, &pass_desc);
    }

    void PreProcessEffect::compute_cubemap(ID3D11Device *device, ID3D11DeviceContext *device_context, std::string_view file_path)
    {
        // Load and convert equirectangular environment map to a cubemap texture
        auto&& texture_manager = model::TextureManagerHandle::get();
        auto hdr_srv = texture_manager.create_from_file(file_path);

        if (!m_effect_impl->cube_texture)
        {
            m_effect_impl->cube_texture = std::make_unique<TextureCube>(device, 1024, 1024, DXGI_FORMAT_R16G16B16A16_FLOAT, 0,
                                                                        D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS);
            create_texture_uav(device, m_effect_impl->cube_texture.get(), 0);
        }

        auto width = m_effect_impl->cube_texture->get_width();
        auto height = m_effect_impl->cube_texture->get_height();
        auto cube_uav = m_effect_impl->cube_texture->get_unordered_access();

        auto pass = m_effect_impl->m_effect_helper->get_effect_pass(m_effect_impl->equirect_to_cube_pass);
        m_effect_impl->m_effect_helper->set_shader_resource_by_name("inputTexture", hdr_srv);
        m_effect_impl->m_effect_helper->set_unordered_access_by_name("outputTexture", cube_uav);
        m_effect_impl->m_effect_helper->set_sampler_state_by_name("computeSampler", RenderStates::ss_linear_wrap.Get());
        pass->apply(device_context);
        pass->dispatch(device_context, width, height, 6);

        //// Clear
        cube_uav = nullptr;
        hdr_srv = nullptr;
        auto srv_slot = m_effect_impl->m_effect_helper->map_shader_resource_slot("inputTexture");
        auto uav_slot = m_effect_impl->m_effect_helper->map_unordered_access_slot("outputTexture");
        DX_CORE_INFO("Texture cube's shader resource view slot: {}; unordered access view slot: {}", srv_slot, uav_slot);
        device_context->CSSetShaderResources(srv_slot, 1, &hdr_srv);
        device_context->CSSetUnorderedAccessViews(uav_slot, 1, &cube_uav, nullptr);

        //// Generate mip-map of texture cube
        device_context->GenerateMips(m_effect_impl->cube_texture->get_shader_resource());
    }

    void PreProcessEffect::compute_sp_env_map(ID3D11Device *device, ID3D11DeviceContext *device_context)
    {
        // Compute pre-filtered specular environment map
        if (!m_effect_impl->sp_map_cb)
        {
            D3D11_BUFFER_DESC desc{};
            desc.ByteWidth = sizeof(SpecularMapFilterSettingCB);
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

            m_effect_impl->sp_map_cb = std::make_unique<Buffer>(device, CD3D11_BUFFER_DESC(desc));
        }

        if (!m_effect_impl->env_texture)
        {
            m_effect_impl->env_texture = std::make_unique<TextureCube>(device, 1024, 1024, DXGI_FORMAT_R16G16B16A16_FLOAT, 0,
                                                                        D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS);
        }

        //// Copy 0th mip-map level into destination environment map
        auto env_resource = m_effect_impl->env_texture->get_texture();
        auto cube_resource = m_effect_impl->cube_texture->get_texture();
        for (uint32_t array_slice = 0; array_slice < 6; ++array_slice)
        {
            const uint32_t subresource_index = D3D11CalcSubresource(0, array_slice, m_effect_impl->env_texture->get_mip_levels());
            device_context->CopySubresourceRegion(env_resource, subresource_index, 0, 0, 0,
                                                    cube_resource, subresource_index,nullptr);
        }

        //// Pre-filter rest of the mip-chain
        auto pass = m_effect_impl->m_effect_helper->get_effect_pass(m_effect_impl->sp_env_map_pass);
        auto cube_srv = m_effect_impl->cube_texture->get_shader_resource();
        ID3D11UnorderedAccessView* env_uav = nullptr;
        auto sp_map_cb = m_effect_impl->sp_map_cb->get_buffer();

        m_effect_impl->m_effect_helper->set_shader_resource_by_name("inputTexture", cube_srv);
        m_effect_impl->m_effect_helper->set_sampler_state_by_name("computeSampler", RenderStates::ss_linear_wrap.Get());

        auto mip_levels = m_effect_impl->env_texture->get_mip_levels();
        const float delta_roughness = 1.0f / std::max(float(mip_levels - 1), 1.0f);
        for (uint32_t level = 1, size = 512; level < mip_levels; ++level, size /= 2)
        {
            const SpecularMapFilterSettingCB sp_map_constants{ float(level) * delta_roughness };

            create_texture_uav(device, m_effect_impl->env_texture.get(), level);
            env_uav = m_effect_impl->env_texture->get_unordered_access();

            device_context->UpdateSubresource(sp_map_cb, 0, nullptr, &sp_map_constants, 0, 0);
            device_context->CSSetConstantBuffers(0, 1, &sp_map_cb);
            m_effect_impl->m_effect_helper->set_unordered_access_by_name("outputTexture", env_uav);
            pass->apply(device_context);
            pass->dispatch(device_context, size, size, 6);
        }

        //// Clear
        cube_srv = nullptr;
        env_uav = nullptr;
        sp_map_cb = nullptr;
        auto srv_slot = m_effect_impl->m_effect_helper->map_shader_resource_slot("inputTexture");
        auto uav_slot = m_effect_impl->m_effect_helper->map_unordered_access_slot("outputTexture");
        DX_CORE_INFO("Environment map's shader resource view slot: {}; unordered access view slot: {}", srv_slot, uav_slot);
        device_context->CSSetConstantBuffers(0, 1, &sp_map_cb);
        device_context->CSSetShaderResources(srv_slot, 1, &cube_srv);
        device_context->CSSetUnorderedAccessViews(uav_slot, 1, &env_uav, nullptr);
    }

    void PreProcessEffect::compute_irradiance_map(ID3D11Device *device, ID3D11DeviceContext *device_context)
    {
        // Compute diffuse irradiance cube map
        if (!m_effect_impl->ir_map_texture)
        {
            m_effect_impl->ir_map_texture = std::make_unique<TextureCube>(device, 32, 32, DXGI_FORMAT_R16G16B16A16_FLOAT, 1,
                                                                            D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS);
            create_texture_uav(device, m_effect_impl->ir_map_texture.get(), 0);
        }

        auto width = m_effect_impl->ir_map_texture->get_width();
        auto height = m_effect_impl->ir_map_texture->get_height();
        auto ir_map_uav = m_effect_impl->ir_map_texture->get_unordered_access();
        auto env_srv = m_effect_impl->env_texture->get_shader_resource();
        auto pass = m_effect_impl->m_effect_helper->get_effect_pass(m_effect_impl->ir_map_pass);
        m_effect_impl->m_effect_helper->set_shader_resource_by_name("inputTexture", env_srv);
        m_effect_impl->m_effect_helper->set_unordered_access_by_name("outputTexture", ir_map_uav);
        m_effect_impl->m_effect_helper->set_sampler_state_by_name("computeSampler", RenderStates::ss_linear_wrap.Get());
        pass->apply(device_context);
        pass->dispatch(device_context, width, height, 1);

        //// Clear
        ir_map_uav = nullptr;
        env_srv = nullptr;
        auto srv_slot = m_effect_impl->m_effect_helper->map_shader_resource_slot("inputTexture");
        auto uav_slot = m_effect_impl->m_effect_helper->map_unordered_access_slot("outputTexture");
        DX_CORE_INFO("Irradiance map's shader resource view slot: {}; unordered access view slot: {}", srv_slot, uav_slot);
        device_context->CSSetShaderResources(srv_slot, 1, &env_srv);
        device_context->CSSetUnorderedAccessViews(uav_slot, 1, &ir_map_uav, nullptr);
    }

    void PreProcessEffect::compute_brdf_lut(ID3D11Device *device, ID3D11DeviceContext *device_context)
    {
        // Compute Cook-Torrance BRDF 2D LUT for split-sum approximation
        if (!m_effect_impl->sp_brdf_texture)
        {
            m_effect_impl->sp_brdf_texture = std::make_unique<Texture2D>(device, 256, 256, DXGI_FORMAT_R16G16_FLOAT, 1,
                                                                            D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS);
            create_texture_uav(device, m_effect_impl->sp_brdf_texture.get(), 0);
        }

        auto width = m_effect_impl->sp_brdf_texture->get_width();
        auto height = m_effect_impl->sp_brdf_texture->get_height();
        auto brdf_uav = m_effect_impl->sp_brdf_texture->get_unordered_access();
        auto pass = m_effect_impl->m_effect_helper->get_effect_pass(m_effect_impl->brdf_lut_pass);

        m_effect_impl->m_effect_helper->set_unordered_access_by_name("LUT", brdf_uav);
        pass->apply(device_context);
        pass->dispatch(device_context, width, height, 1);

        //// Clear
        brdf_uav = nullptr;
        auto uav_slot = m_effect_impl->m_effect_helper->map_unordered_access_slot("LUT");
        DX_CORE_INFO("BRDF 2D LUT's unordered access view slot: {}", uav_slot);
        device_context->CSSetUnorderedAccessViews(uav_slot, 1, &brdf_uav, nullptr);
    }

    ID3D11ShaderResourceView* PreProcessEffect::get_environment_srv() const
    {
        if (m_effect_impl->env_texture)
        {
            return m_effect_impl->env_texture->get_shader_resource();
        }
        return nullptr;
    }
}
























