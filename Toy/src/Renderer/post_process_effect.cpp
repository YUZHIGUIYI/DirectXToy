//
// Created by ZZK on 2023/6/8.
//

#include <Toy/Renderer/effects.h>
#include <Toy/Renderer/render_states.h>
#include <Toy/Geometry/vertex.h>
#include <Toy/Model/mesh_data.h>
#include <Toy/Model/texture_manager.h>
#include <Toy/Model/material.h>

namespace toy
{
    static void generate_gaussian_weights(std::vector<float>& weights, int32_t kernel_size, float sigma)
    {
        float two_sigma_sq = 2.0f * sigma * sigma;
        int32_t radius = kernel_size / 2;
        float sum = 0.0f;
        for (int32_t i = -radius; i <= radius; ++i)
        {
            auto x = static_cast<float>(i);
            weights[radius + i] = std::exp(-x * x / two_sigma_sq);
            sum += weights[radius + i];
        }

        // Standardize weights so that the sum of weights is 1.0
        for (size_t i = 0; i <= kernel_size; ++i)
        {
            weights[i] /= sum;
        }
    }

    struct PostProcessEffect::EffectImpl
    {
        EffectImpl() = default;
        ~EffectImpl() = default;

        std::unique_ptr<EffectHelper> m_effect_helper;

        std::vector<float> m_weights;
        int32_t m_blur_radius = 3;
        float m_blur_sigma = 1.0f;
    };

    PostProcessEffect::PostProcessEffect()
    {
        m_effect_impl = std::make_unique<EffectImpl>();

        m_effect_impl->m_weights.resize(32);
    }

    PostProcessEffect::~PostProcessEffect() = default;

    PostProcessEffect::PostProcessEffect(toy::PostProcessEffect &&other) noexcept
    {
        m_effect_impl.swap(other.m_effect_impl);
    }

    PostProcessEffect& PostProcessEffect::operator=(toy::PostProcessEffect &&other) noexcept
    {
        m_effect_impl.swap(other.m_effect_impl);
        return *this;
    }

    PostProcessEffect& PostProcessEffect::get()
    {
        static PostProcessEffect post_process_effect{};
        return post_process_effect;
    }

    void PostProcessEffect::init(ID3D11Device *device)
    {
        m_effect_impl->m_effect_helper = std::make_unique<EffectHelper>();

        // Create compute shader
        m_effect_impl->m_effect_helper->create_shader_from_file("blur_horiz_cs", L"../data/shaders/blur_horiz_cs.cso", device);
        m_effect_impl->m_effect_helper->create_shader_from_file("blur_vert_cs", L"../data/shaders/blur_vert_cs.cso", device);
        m_effect_impl->m_effect_helper->create_shader_from_file("composite_vs", L"../data/shaders/composite_vs.cso", device);
        m_effect_impl->m_effect_helper->create_shader_from_file("composite_ps", L"../data/shaders/composite_ps.cso", device);
        m_effect_impl->m_effect_helper->create_shader_from_file("sobel_cs", L"../data/shaders/sobel_cs.cso", device);

        // Create render pass
        EffectPassDesc pass_desc{};
        pass_desc.nameCS = "blur_horiz_cs";
        m_effect_impl->m_effect_helper->add_effect_pass("blur_horiz_cs", device, &pass_desc);
        pass_desc.nameCS = "blur_vert_cs";
        m_effect_impl->m_effect_helper->add_effect_pass("blur_vert_cs", device, &pass_desc);
        pass_desc.nameCS = "sobel_cs";
        m_effect_impl->m_effect_helper->add_effect_pass("sobel_cs", device, &pass_desc);
        pass_desc.nameVS = "composite_vs";
        pass_desc.namePS = "composite_ps";
        pass_desc.nameCS = "";
        m_effect_impl->m_effect_helper->add_effect_pass("composite", device, &pass_desc);

        m_effect_impl->m_effect_helper->set_sampler_state_by_name("g_SamLinearWrap", RenderStates::ss_linear_wrap.Get());
        m_effect_impl->m_effect_helper->set_sampler_state_by_name("g_SamPointClamp", RenderStates::ss_point_clamp.Get());

        // TODO: set debug object name

    }

    void PostProcessEffect::render_composite(ID3D11DeviceContext *device_context, ID3D11ShaderResourceView *input1,
                                                ID3D11ShaderResourceView *input2, ID3D11RenderTargetView *output,
                                                const D3D11_VIEWPORT &viewport)
    {
        auto&& pass = m_effect_impl->m_effect_helper->get_effect_pass("composite");
        m_effect_impl->m_effect_helper->set_shader_resource_by_name("g_Input", input1);
        m_effect_impl->m_effect_helper->set_shader_resource_by_name("g_EdgeInput",
                                                                    (input2 ? input2 : model::TextureManager::get().get_null_texture()));
        pass->apply(device_context);

        device_context->OMSetRenderTargets(1, &output, nullptr);
        device_context->RSSetViewports(1, &viewport);
        device_context->Draw(3, 0);

        // Clear
        input1 = nullptr;
        device_context->PSSetShaderResources(m_effect_impl->m_effect_helper->map_shader_resource_slot("g_Input"), 1, &input1);
        device_context->PSSetShaderResources(m_effect_impl->m_effect_helper->map_shader_resource_slot("g_EdgeInput"), 1, &input1);
        device_context->OMSetRenderTargets(0, nullptr, nullptr);
    }

    void PostProcessEffect::compute_sobel(ID3D11DeviceContext *device_context, ID3D11ShaderResourceView *input,
                                            ID3D11UnorderedAccessView *output, uint32_t width, uint32_t height)
    {
        auto&& pass = m_effect_impl->m_effect_helper->get_effect_pass("sobel_cs");
        m_effect_impl->m_effect_helper->set_shader_resource_by_name("g_Input", input);
        m_effect_impl->m_effect_helper->set_unordered_access_by_name("g_Output", output);
        pass->apply(device_context);
        pass->dispatch(device_context, width, height, 1);

        // Clear
        input = nullptr;
        output = nullptr;
        device_context->CSSetShaderResources(m_effect_impl->m_effect_helper->map_shader_resource_slot("g_Input"), 1, &input);
        device_context->CSSetUnorderedAccessViews(m_effect_impl->m_effect_helper->map_unordered_access_slot("g_Output"), 1, &output, nullptr);
    }

    void PostProcessEffect::set_blur_kernel_size(int32_t size)
    {
        if (size % 2 == 0 || size > m_effect_impl->m_weights.size())
        {
            return;
        }

        m_effect_impl->m_blur_radius = size / 2;
        generate_gaussian_weights(m_effect_impl->m_weights, size, m_effect_impl->m_blur_sigma);
    }

    void PostProcessEffect::set_blur_sigma(float sigma)
    {
        if (sigma < 0.0f)
        {
            return;
        }

        m_effect_impl->m_blur_sigma = sigma;
        generate_gaussian_weights(m_effect_impl->m_weights, m_effect_impl->m_blur_radius * 2 + 1, m_effect_impl->m_blur_sigma);
    }

    void PostProcessEffect::compute_gaussian_blur_x(ID3D11DeviceContext *device_context,
                                                    ID3D11ShaderResourceView *input, ID3D11UnorderedAccessView *output,
                                                    uint32_t width, uint32_t height)
    {
        device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        auto pass = m_effect_impl->m_effect_helper->get_effect_pass("blur_horiz_cs");
        m_effect_impl->m_effect_helper->get_constant_buffer_variable("g_Weights")->set_raw(m_effect_impl->m_weights.data());
        m_effect_impl->m_effect_helper->get_constant_buffer_variable("g_BlurRadius")->set_sint(m_effect_impl->m_blur_radius);
        m_effect_impl->m_effect_helper->set_shader_resource_by_name("g_Input", input);
        m_effect_impl->m_effect_helper->set_unordered_access_by_name("g_Output", output);
        pass->apply(device_context);
        pass->dispatch(device_context, width, height, 1);

        // Clear
        input = nullptr;
        output = nullptr;
        device_context->CSSetShaderResources(m_effect_impl->m_effect_helper->map_shader_resource_slot("g_Input"), 1, &input);
        device_context->CSSetUnorderedAccessViews(m_effect_impl->m_effect_helper->map_unordered_access_slot("g_Output"), 1, &output,nullptr);
    }

    void PostProcessEffect::compute_gaussian_blur_y(ID3D11DeviceContext *device_context,
                                                    ID3D11ShaderResourceView *input, ID3D11UnorderedAccessView *output,
                                                    uint32_t width, uint32_t height)
    {
        device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        auto&& pass = m_effect_impl->m_effect_helper->get_effect_pass("blur_vert_cs");
        m_effect_impl->m_effect_helper->get_constant_buffer_variable("g_Weights")->set_raw(m_effect_impl->m_weights.data());
        m_effect_impl->m_effect_helper->get_constant_buffer_variable("g_BlurRadius")->set_sint(m_effect_impl->m_blur_radius);
        m_effect_impl->m_effect_helper->set_shader_resource_by_name("g_Input", input);
        m_effect_impl->m_effect_helper->set_unordered_access_by_name("g_Output", output);
        pass->apply(device_context);
        pass->dispatch(device_context, width, height, 1);

        // Clear
        input = nullptr;
        output = nullptr;
        device_context->CSSetShaderResources(m_effect_impl->m_effect_helper->map_shader_resource_slot("g_Input"), 1, &input);
        device_context->CSSetUnorderedAccessViews(m_effect_impl->m_effect_helper->map_unordered_access_slot("g_Output"), 1, &output, nullptr);
    }
}






























