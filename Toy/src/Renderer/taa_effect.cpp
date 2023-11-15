//
// Created by ZZK on 2023/10/5.
//

#include <Toy/Renderer/effects.h>
#include <Toy/Renderer/render_states.h>
#include <Toy/Model/texture_manager.h>
#include <Toy/Model/material.h>
#include <Toy/Renderer/taa_settings.h>

namespace toy
{
    struct TAAEffect::EffectImpl
    {
        EffectImpl() = default;
        ~EffectImpl() = default;

        std::unique_ptr<EffectHelper> effect_helper = nullptr;
        std::shared_ptr<IEffectPass> cur_effect_pass = nullptr;
        com_ptr<ID3D11InputLayout> cur_vertex_layout = nullptr;
        com_ptr<ID3D11InputLayout> vertex_layout = nullptr;

        std::string_view taa_pass = {};
    };

    TAAEffect::TAAEffect()
    {
        m_effect_impl = std::make_unique<EffectImpl>();
    }

    TAAEffect::~TAAEffect() noexcept = default;

    TAAEffect::TAAEffect(toy::TAAEffect &&other) noexcept
    {
        m_effect_impl.swap(other.m_effect_impl);
    }

    TAAEffect& TAAEffect::operator=(toy::TAAEffect &&other) noexcept
    {
        m_effect_impl.swap(other.m_effect_impl);
        return *this;
    }

    TAAEffect& TAAEffect::get()
    {
        static TAAEffect taa_effect = {};
        return taa_effect;
    }

    void TAAEffect::init(ID3D11Device *device)
    {
        m_effect_impl->effect_helper = std::make_unique<EffectHelper>();
        m_effect_impl->effect_helper->set_binary_cache_directory(DXTOY_HOME L"data/pbr/cache");

        // Set shader name and pass name
        std::string_view screen_triangle_vs = "TAAScreenVS";
        std::string_view taa_ps = "TAAPS";
        m_effect_impl->taa_pass = "TAAPass";

        // Create vertex and pixel shaders and input layout
        m_effect_impl->effect_helper->create_shader_from_file(screen_triangle_vs, DXTOY_HOME L"data/pbr/screen_triangle_vs.hlsl", device,
                                                                "VS", "vs_5_0");
        m_effect_impl->effect_helper->create_shader_from_file(taa_ps, DXTOY_HOME L"data/pbr/taa.hlsl", device,
                                                                "PS", "ps_5_0");

        // Create geometry and deferred lighting passes
        EffectPassDesc pass_desc = {};
        pass_desc.nameVS = screen_triangle_vs;
        pass_desc.namePS = taa_ps;
        m_effect_impl->effect_helper->add_effect_pass(m_effect_impl->taa_pass, device, &pass_desc);

        // Set sampler state
        m_effect_impl->effect_helper->set_sampler_state_by_name("gSamPointClamp", RenderStates::ss_point_clamp.Get());
        m_effect_impl->effect_helper->set_sampler_state_by_name("gSamLinearWrap", RenderStates::ss_linear_wrap.Get());
        m_effect_impl->effect_helper->set_sampler_state_by_name("gSamLinearClamp", RenderStates::ss_linear_clamp.Get());
        m_effect_impl->effect_helper->set_sampler_state_by_name("gSamAnisotropicWrap", RenderStates::ss_anisotropic_wrap_16x.Get());
    }

    void TAAEffect::set_camera_near_far(float nearz, float farz)
    {
        m_effect_impl->effect_helper->get_constant_buffer_variable("gNearZ")->set_float(nearz);
        m_effect_impl->effect_helper->get_constant_buffer_variable("gFarZ")->set_float(farz);
    }

    void TAAEffect::set_viewer_size(int32_t width, int32_t height)
    {
        float render_target_size[2] = { static_cast<float>(width), static_cast<float>(height) };
        float inv_render_target_size[2] = { 1.0f / render_target_size[0], 1.0f / render_target_size[1] };
        m_effect_impl->effect_helper->get_constant_buffer_variable("gRenderTargetSize")->set_float_vector(2, render_target_size);
        m_effect_impl->effect_helper->get_constant_buffer_variable("gInvRenderTargetSize")->set_float_vector(2, inv_render_target_size);
    }

    void TAAEffect::render(ID3D11DeviceContext *device_context, ID3D11ShaderResourceView *history_buffer_srv,
                            ID3D11ShaderResourceView *cur_buffer_srv, ID3D11ShaderResourceView *motion_vector_srv,
                            ID3D11ShaderResourceView *depth_buffer_srv, ID3D11RenderTargetView *lit_buffer_rtv, D3D11_VIEWPORT viewport)
    {
        static uint32_t taa_frame_counter = 0;

        // Jitter
        float jitter_x = taa::s_halton_2[taa_frame_counter] / viewport.Width * taa::s_taa_jitter_distance;
        float jitter_y = taa::s_halton_3[taa_frame_counter] / viewport.Height * taa::s_taa_jitter_distance;
        float jitter[2] = { jitter_x / 2.0f, -jitter_y / 2.0f };

        // Full-screen triangle
        device_context->IASetInputLayout(nullptr);
        device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        device_context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
        device_context->RSSetViewports(1, &viewport);

        m_effect_impl->effect_helper->get_constant_buffer_variable("gJitter")->set_float_vector(2, jitter);

        auto pass = m_effect_impl->effect_helper->get_effect_pass(m_effect_impl->taa_pass);
        m_effect_impl->effect_helper->set_shader_resource_by_name("gHistoryFrameMap", history_buffer_srv);
        m_effect_impl->effect_helper->set_shader_resource_by_name("gCurrentFrameMap", cur_buffer_srv);
        m_effect_impl->effect_helper->set_shader_resource_by_name("gVelocityMap", motion_vector_srv);
        m_effect_impl->effect_helper->set_shader_resource_by_name("gDepthMap", depth_buffer_srv);

        // Apply pass
        pass->apply(device_context);

        // Bind render target view
        device_context->OMSetRenderTargets(1, &lit_buffer_rtv, nullptr);

        // Draw
        device_context->Draw(3, 0);
        device_context->OMSetRenderTargets(0, nullptr, nullptr);
        m_effect_impl->effect_helper->set_shader_resource_by_name("gHistoryFrameMap", nullptr);
        m_effect_impl->effect_helper->set_shader_resource_by_name("gCurrentFrameMap", nullptr);
        m_effect_impl->effect_helper->set_shader_resource_by_name("gVelocityMap", nullptr);
        m_effect_impl->effect_helper->set_shader_resource_by_name("gDepthMap", nullptr);
        pass->apply(device_context);

        taa_frame_counter = (taa_frame_counter + 1) % taa::s_taa_sample;
    }
}