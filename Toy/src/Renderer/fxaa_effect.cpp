//
// Created by ZZK on 2023/7/29.
//

#include <Toy/Renderer/effects.h>
#include <Toy/Renderer/render_states.h>
#include <Toy/Geometry/vertex.h>
#include <Toy/Model/mesh_data.h>
#include <Toy/Model/texture_manager.h>
#include <Toy/Model/material.h>

namespace toy
{
    struct FXAAEffect::EffectImpl
    {
        EffectImpl() = default;
        ~EffectImpl() = default;

        int32_t m_major = 2;
        int32_t m_minor = 9;
        int32_t m_enable_debug = 0;
        float m_quality_sub_pix = 0.75f;
        float m_quality_edge_threshold = 0.166f;
        float m_quality_edge_threshold_min = 0.0833f;

        std::unique_ptr<EffectHelper> m_effect_helper = nullptr;
    };


    FXAAEffect::FXAAEffect()
    {
        m_effect_impl = std::make_unique<EffectImpl>();
    }

    FXAAEffect::~FXAAEffect() = default;

    FXAAEffect::FXAAEffect(toy::FXAAEffect &&other) noexcept
    {
        m_effect_impl.swap(other.m_effect_impl);
    }

    FXAAEffect& FXAAEffect::operator=(toy::FXAAEffect &&other) noexcept
    {
        m_effect_impl.swap(other.m_effect_impl);
        return *this;
    }

    void FXAAEffect::init(ID3D11Device *device)
    {
        m_effect_impl->m_effect_helper = std::make_unique<EffectHelper>();
        m_effect_impl->m_effect_helper->set_binary_cache_directory(DXTOY_HOME L"data/defer/cache");

        std::array<std::string_view, 17> strs{
            "10", "11", "12", "13", "14", "15",
            "20", "21", "22", "23", "24", "25",
            "26", "27", "28", "29", "39" };
        std::array<D3D_SHADER_MACRO, 3> defines{
            D3D_SHADER_MACRO{ "FXAA_QUALITY__PRESET", "39" },
            D3D_SHADER_MACRO{ nullptr, nullptr },
            D3D_SHADER_MACRO{ nullptr, nullptr } };

        // Create vertex shader
        m_effect_impl->m_effect_helper->create_shader_from_file("FullScreenTriangleTexcoordVS", DXTOY_HOME L"data/defer/fxaa.hlsl", device,
                                                                "FullScreenTriangleTexcoordVS", "vs_5_0");

        // major quality, minor quality, debug mode
        std::string ps_name{ "000_PS" };
        std::string pass_name{ "000_FXAA" };
        EffectPassDesc pass_desc{};

        // Create pass
        pass_desc.nameVS = "FullScreenTriangleTexcoordVS";

        for (auto str : strs)
        {
            ps_name[0] = pass_name[0] = str[0];
            ps_name[1] = pass_name[1] = str[1];
            ps_name[2] = pass_name[2] = '1';
            defines[1].Name = "DEBUG_OUTPUT";
            defines[1].Definition = "";
            // Create pixel shader - for Debug
            m_effect_impl->m_effect_helper->create_shader_from_file(ps_name, DXTOY_HOME L"data/defer/fxaa.hlsl", device,
                                                            "PS", "ps_5_0", defines.data());
            // Create pixel pass
            pass_desc.namePS = ps_name;
            m_effect_impl->m_effect_helper->add_effect_pass(pass_name, device, &pass_desc);

            ps_name[2] = pass_name[2] = '0';
            defines[1].Name = nullptr;
            defines[1].Definition = nullptr;
            // Create pixel shader
            m_effect_impl->m_effect_helper->create_shader_from_file(ps_name, DXTOY_HOME L"data/defer/fxaa.hlsl", device,
                                                                "PS", "ps_5_0", defines.data());
            // Create pixel pass
            pass_desc.namePS = ps_name;
            m_effect_impl->m_effect_helper->add_effect_pass(pass_name, device, &pass_desc);
        }

        m_effect_impl->m_effect_helper->set_sampler_state_by_name("g_SamplerLinearClamp", RenderStates::ss_linear_clamp.Get());

        // TODO: set debug object name
    }

    void FXAAEffect::set_quality(int32_t major, int32_t minor)
    {
        m_effect_impl->m_major = std::clamp(major, 1, 3);
        switch (major)
        {
            case 1: m_effect_impl->m_minor = std::clamp(minor, 0, 5); break;
            case 2: m_effect_impl->m_minor = std::clamp(minor, 0, 9); break;
            case 3: m_effect_impl->m_minor = 9; break;
            default: break;
        }
    }

    void FXAAEffect::set_quality_sub_pix(float value)
    {
        m_effect_impl->m_effect_helper->get_constant_buffer_variable("g_QualitySubPix")->set_float(value);
    }

    void FXAAEffect::set_quality_edge_threshold(float threshold)
    {
        m_effect_impl->m_effect_helper->get_constant_buffer_variable("g_QualityEdgeThreshold")->set_float(threshold);
    }

    void FXAAEffect::set_quality_edge_threshold_min(float min_threshold)
    {
        m_effect_impl->m_effect_helper->get_constant_buffer_variable("g_QualityEdgeThresholdMin")->set_float(min_threshold);
    }

    void FXAAEffect::enable_debug(bool enabled)
    {
        m_effect_impl->m_enable_debug = enabled;
    }

    void FXAAEffect::render_fxaa(ID3D11DeviceContext *device_context, ID3D11ShaderResourceView *input_srv,
                                    ID3D11RenderTargetView *output_rtv, const D3D11_VIEWPORT &viewport)
    {
        using namespace std::literals;
        device_context->IASetInputLayout(nullptr);
        device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        device_context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
        std::string pass_name = "000_FXAA"s;
        pass_name[0] = '0' + m_effect_impl->m_major;
        pass_name[1] = '0' + m_effect_impl->m_minor;
        pass_name[2] = '0' + m_effect_impl->m_enable_debug;
        auto pass = m_effect_impl->m_effect_helper->get_effect_pass(pass_name);
        m_effect_impl->m_effect_helper->set_shader_resource_by_name("g_TextureInput", input_srv);
        std::array<float, 2> texel_sizes{ 1.0f / viewport.Width, 1.0f / viewport.Height };
        m_effect_impl->m_effect_helper->get_constant_buffer_variable("g_TexelSize")->set_float_vector(texel_sizes.size(), texel_sizes.data());
        pass->apply(device_context);
        device_context->OMSetRenderTargets(1, &output_rtv, nullptr);
        device_context->RSSetViewports(1, &viewport);
        device_context->Draw(3, 0);

        auto slot = m_effect_impl->m_effect_helper->map_shader_resource_slot("g_TextureInput");
        input_srv = nullptr;
        device_context->PSSetShaderResources(slot, 1, &input_srv);
        device_context->OMSetRenderTargets(0, nullptr, nullptr);
    }
}




























