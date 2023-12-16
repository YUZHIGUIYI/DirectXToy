//
// Created by ZZK on 2023/10/16.
//

#include <Toy/Renderer/effects.h>
#include <Toy/Renderer/render_states.h>
#include <Toy/Geometry/vertex.h>
#include <Toy/Model/mesh_data.h>
#include <Toy/Model/texture_manager.h>
#include <Toy/Model/material.h>

namespace toy
{
    static void generate_gaussian_weights(std::array<float, 16> &weights, int32_t kernel_size, float sigma)
    {
        float two_sigma_sq = 2.0f * sigma * sigma;
        float sum = 0.0f;
        int32_t radius = kernel_size / 2;
        for (int32_t i = -radius; i <= radius; ++i)
        {
            auto x = static_cast<float>(i);
            auto temp_weight = std::exp(-x * x / two_sigma_sq);
            sum += temp_weight;
            weights[radius + i] = temp_weight;
        }

        for (int32_t i = 0; i <= kernel_size; ++i)
        {
            weights[i] /= sum;
        }
    }

    struct ShadowEffect::EffectImpl
    {
        EffectImpl() = default;
        ~EffectImpl() = default;

        std::unique_ptr<EffectHelper> effect_helper = nullptr;
        std::shared_ptr<IEffectPass> cur_effect_pass = nullptr;
        com_ptr<ID3D11InputLayout> cur_input_layout = nullptr;

        DirectX::XMFLOAT4X4 world_matrix = {};
        DirectX::XMFLOAT4X4 view_matrix = {};
        DirectX::XMFLOAT4X4 proj_matrix = {};

        std::string_view depth_only_pass = {};
        std::string_view shadow_pass = {};
        std::string_view variance_shadow_pass = {};
        std::string_view exponential_shadow_pass = {};
        std::string_view evsm2_comp_pass = {};
        std::string_view evsm4_comp_pass = {};
        std::string_view debug_pass = {};
        std::string_view gaussian_x_pass = {};
        std::string_view gaussian_y_pass = {};
        std::string_view log_gaussian_pass = {};

        std::array<float, 16> weights = {};
        int32_t blur_size = 5;
        float blur_sigma = 1.0f;

        D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
    };

    ShadowEffect::ShadowEffect()
    {
        m_effect_impl = std::make_unique<EffectImpl>();
    }

    ShadowEffect::~ShadowEffect() = default;

    ShadowEffect::ShadowEffect(toy::ShadowEffect &&other) noexcept
    {
        m_effect_impl.swap(other.m_effect_impl);
    }

    ShadowEffect &ShadowEffect::operator=(toy::ShadowEffect &&other) noexcept
    {
        m_effect_impl.swap(other.m_effect_impl);
        return *this;
    }

    ShadowEffect &ShadowEffect::get()
    {
        static ShadowEffect shadow_effect = {};
        return shadow_effect;
    }

    void ShadowEffect::init(ID3D11Device *device)
    {
        m_effect_impl->effect_helper = std::make_unique<EffectHelper>();
        m_effect_impl->effect_helper->set_binary_cache_directory(DXTOY_HOME L"data/pbr/cache");

        std::string_view shadow_vs = "ShadowVS";
        std::string_view screen_triangle_vs = "ShadowScreenVS";
        std::string_view shadow_ps = "ShadowPS";
        std::string_view debug_ps = "DebugShadowPS";
        std::string_view exponential_shadow_ps = "ExponentialShadowPS";
        std::string_view evsm2_comp_ps = "EVSM2CompPS";
        std::string_view evsm4_comp_ps = "EVSM4CompPS";
        std::string_view variance_shadow_ps = "VarianceShadowPS_4xMSAA";
        std::string_view gaussian_blurx_ps = "GaussianBlurXPS_9";
        std::string_view gaussian_blury_ps = "GaussianBlurYPS_9";
        std::string_view log_gaussian_blur_ps = "LogGaussianBlurPS_9";

        m_effect_impl->shadow_pass = "ShadowPass";
        m_effect_impl->depth_only_pass = "DepthOnlyPass";
        m_effect_impl->variance_shadow_pass = "VarianceShadowPass";
        m_effect_impl->exponential_shadow_pass = "ExponentialShadowPass";
        m_effect_impl->evsm2_comp_pass = "EVSM2CompPass";
        m_effect_impl->evsm4_comp_pass = "EVSM4CompPass";
        m_effect_impl->debug_pass = "DebugPass";
        m_effect_impl->gaussian_x_pass = "GaussianBlurXPass";
        m_effect_impl->gaussian_y_pass = "GaussianBlurYPass";
        m_effect_impl->log_gaussian_pass = "LogGaussianBlurPass";

        const std::array<D3D_SHADER_MACRO, 2> defines = {
            D3D_SHADER_MACRO{ "BLUR_KERNEL_SIZE", "9" },
            D3D_SHADER_MACRO{ nullptr, nullptr }
        };

        com_ptr<ID3DBlob> blob = nullptr;

        m_effect_impl->effect_helper->create_shader_from_file(shadow_vs, DXTOY_HOME L"data/pbr/shadow_vs.hlsl", device,
                                                                "VS", "vs_5_0", nullptr, blob.GetAddressOf());
        auto&& input_layout = VertexPosNormalTex::get_input_layout();
        device->CreateInputLayout(input_layout.data(), uint32_t(input_layout.size()), blob->GetBufferPointer(),
                                    blob->GetBufferSize(), m_effect_impl->cur_input_layout.GetAddressOf());
        m_effect_impl->effect_helper->create_shader_from_file(screen_triangle_vs, DXTOY_HOME L"data/pbr/screen_triangle_vs.hlsl", device,
                                                                "VS", "vs_5_0");

        m_effect_impl->effect_helper->create_shader_from_file(shadow_ps, DXTOY_HOME L"data/pbr/shadow_ps.hlsl", device,
                                                                "ShadowPS", "ps_5_0");
        m_effect_impl->effect_helper->create_shader_from_file(debug_ps, DXTOY_HOME L"data/pbr/shadow_ps.hlsl", device,
                                                                "DebugShadowPS", "ps_5_0");
        m_effect_impl->effect_helper->create_shader_from_file(exponential_shadow_ps, DXTOY_HOME L"data/pbr/shadow_ps.hlsl", device,
                                                                "ExponentialShadowPS", "ps_5_0");
        m_effect_impl->effect_helper->create_shader_from_file(evsm2_comp_ps, DXTOY_HOME L"data/pbr/shadow_ps.hlsl", device,
                                                                "EVSM2CompPS", "ps_5_0");
        m_effect_impl->effect_helper->create_shader_from_file(evsm4_comp_ps, DXTOY_HOME L"data/pbr/shadow_ps.hlsl", device,
                                                                "EVSM4CompPS", "ps_5_0");
        m_effect_impl->effect_helper->create_shader_from_file(variance_shadow_ps, DXTOY_HOME L"data/pbr/shadow_ps.hlsl", device,
                                                                "VarianceShadowPS", "ps_5_0");
        m_effect_impl->effect_helper->create_shader_from_file(gaussian_blurx_ps, DXTOY_HOME L"data/pbr/shadow_ps.hlsl", device,
                                                                "GaussianBlurXPS", "ps_5_0", defines.data());
        m_effect_impl->effect_helper->create_shader_from_file(gaussian_blury_ps, DXTOY_HOME L"data/pbr/shadow_ps.hlsl", device,
                                                                "GaussianBlurYPS", "ps_5_0", defines.data());
        m_effect_impl->effect_helper->create_shader_from_file(log_gaussian_blur_ps, DXTOY_HOME L"data/pbr/shadow_ps.hlsl", device,
                                                                "LogGaussianBlurPS", "ps_5_0", defines.data());

        EffectPassDesc pass_desc = {};
        pass_desc.nameVS = shadow_vs;
        m_effect_impl->effect_helper->add_effect_pass(m_effect_impl->depth_only_pass, device, &pass_desc);
        auto pass = m_effect_impl->effect_helper->get_effect_pass(m_effect_impl->depth_only_pass);
        pass->set_rasterizer_state(RenderStates::rs_shadow.Get());

        pass_desc.namePS = shadow_ps;
        m_effect_impl->effect_helper->add_effect_pass(m_effect_impl->shadow_pass, device, &pass_desc);
        pass = m_effect_impl->effect_helper->get_effect_pass(m_effect_impl->shadow_pass);
        pass->set_rasterizer_state(RenderStates::rs_shadow.Get());

        pass_desc.nameVS = screen_triangle_vs;
        pass_desc.namePS = variance_shadow_ps;
        m_effect_impl->effect_helper->add_effect_pass(m_effect_impl->variance_shadow_pass, device, &pass_desc);

        pass_desc.nameVS = screen_triangle_vs;
        pass_desc.namePS = exponential_shadow_ps;
        m_effect_impl->effect_helper->add_effect_pass(m_effect_impl->exponential_shadow_pass, device, &pass_desc);

        pass_desc.nameVS = screen_triangle_vs;
        pass_desc.namePS = evsm2_comp_ps;
        m_effect_impl->effect_helper->add_effect_pass(m_effect_impl->evsm2_comp_pass, device, &pass_desc);

        pass_desc.nameVS = screen_triangle_vs;
        pass_desc.namePS = evsm4_comp_ps;
        m_effect_impl->effect_helper->add_effect_pass(m_effect_impl->evsm4_comp_pass, device, &pass_desc);

        pass_desc.nameVS = screen_triangle_vs;
        pass_desc.namePS = debug_ps;
        m_effect_impl->effect_helper->add_effect_pass(m_effect_impl->debug_pass, device, &pass_desc);

        pass_desc.nameVS = screen_triangle_vs;
        pass_desc.namePS = gaussian_blurx_ps;
        m_effect_impl->effect_helper->add_effect_pass(m_effect_impl->gaussian_x_pass, device, &pass_desc);

        pass_desc.nameVS = screen_triangle_vs;
        pass_desc.namePS = gaussian_blury_ps;
        m_effect_impl->effect_helper->add_effect_pass(m_effect_impl->gaussian_y_pass, device, &pass_desc);

        pass_desc.nameVS = screen_triangle_vs;
        pass_desc.namePS = log_gaussian_blur_ps;
        m_effect_impl->effect_helper->add_effect_pass(m_effect_impl->log_gaussian_pass, device, &pass_desc);

        m_effect_impl->effect_helper->set_sampler_state_by_name("gSamLinearWrap", RenderStates::ss_linear_wrap.Get());
        m_effect_impl->effect_helper->set_sampler_state_by_name("gSamPointClamp", RenderStates::ss_point_clamp.Get());
    }

    void ShadowEffect::set_depth_only_render()
    {
        m_effect_impl->cur_effect_pass = m_effect_impl->effect_helper->get_effect_pass(m_effect_impl->depth_only_pass);
        m_effect_impl->topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    }

    void ShadowEffect::set_default_render()
    {
        m_effect_impl->cur_effect_pass = m_effect_impl->effect_helper->get_effect_pass(m_effect_impl->shadow_pass);
        m_effect_impl->topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    }

    void ShadowEffect::render_variance_shadow(ID3D11DeviceContext *device_context, ID3D11ShaderResourceView *input_srv,
                                                ID3D11RenderTargetView *output_rtv, const D3D11_VIEWPORT &viewport)
    {
        // TODO: select MSAA from texture sample count
        m_effect_impl->cur_effect_pass = m_effect_impl->effect_helper->get_effect_pass(m_effect_impl->variance_shadow_pass);

        device_context->IASetInputLayout(nullptr);
        device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_effect_impl->effect_helper->set_shader_resource_by_name("gShadowMap", input_srv);
        m_effect_impl->cur_effect_pass->apply(device_context);
        device_context->OMSetRenderTargets(1, &output_rtv, nullptr);
        device_context->RSSetViewports(1, &viewport);
        device_context->Draw(3, 0);

        auto srv_slot = m_effect_impl->effect_helper->map_shader_resource_slot("gShadowMap");
        input_srv = nullptr;
        device_context->PSSetShaderResources(static_cast<uint32_t>(srv_slot), 1, &input_srv);
        device_context->OMSetRenderTargets(0, nullptr, nullptr);
    }

    void ShadowEffect::render_exponential_shadow(ID3D11DeviceContext *device_context, ID3D11ShaderResourceView *input_srv,
                                                    ID3D11RenderTargetView *output_rtv, const D3D11_VIEWPORT &viewport, float magic_power)
    {
        m_effect_impl->cur_effect_pass = m_effect_impl->effect_helper->get_effect_pass(m_effect_impl->exponential_shadow_pass);

        device_context->IASetInputLayout(nullptr);
        device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_effect_impl->effect_helper->set_shader_resource_by_name("gShadowMap", input_srv);
        m_effect_impl->cur_effect_pass->get_ps_param_by_name("c")->set_float(magic_power);
        m_effect_impl->cur_effect_pass->apply(device_context);
        device_context->OMSetRenderTargets(1, &output_rtv, nullptr);
        device_context->RSSetViewports(1, &viewport);
        device_context->Draw(3, 0);

        auto srv_slot = m_effect_impl->effect_helper->map_shader_resource_slot("gShadowMap");
        input_srv = nullptr;
        device_context->PSSetShaderResources(static_cast<uint32_t>(srv_slot), 1, &input_srv);
        device_context->OMSetRenderTargets(0, nullptr, nullptr);
    }

    void ShadowEffect::render_exponential_variance_shadow(ID3D11DeviceContext *device_context, ID3D11ShaderResourceView *input_srv,
                                                            ID3D11RenderTargetView *output_rtv, const D3D11_VIEWPORT &viewport,
                                                            float pos_exp, float *opt_neg_exp)
    {
        std::array<float, 2> exps = { pos_exp, 0.0f };
        if (opt_neg_exp)
        {
            m_effect_impl->cur_effect_pass = m_effect_impl->effect_helper->get_effect_pass(m_effect_impl->evsm4_comp_pass);
            exps[1] = *opt_neg_exp;
        } else
        {
            m_effect_impl->cur_effect_pass = m_effect_impl->effect_helper->get_effect_pass(m_effect_impl->evsm2_comp_pass);
        }

        device_context->IASetInputLayout(nullptr);
        device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_effect_impl->effect_helper->get_constant_buffer_variable("gEvsmExponents")->set_float_vector(2, exps.data());
        m_effect_impl->effect_helper->set_shader_resource_by_name("gShadowMap", input_srv);
        m_effect_impl->cur_effect_pass->apply(device_context);
        device_context->OMSetRenderTargets(1, &output_rtv, nullptr);
        device_context->RSSetViewports(1, &viewport);
        device_context->Draw(3, 0);

        auto srv_slot = m_effect_impl->effect_helper->map_shader_resource_slot("gShadowMap");
        input_srv = nullptr;
        device_context->PSSetShaderResources(static_cast<uint32_t>(srv_slot), 1, &input_srv);
        device_context->OMSetRenderTargets(0, nullptr, nullptr);
    }

    void ShadowEffect::render_depth_to_texture(ID3D11DeviceContext *device_context, ID3D11ShaderResourceView *input_srv,
                                                ID3D11RenderTargetView *output_rtv, const D3D11_VIEWPORT &viewport)
    {
        device_context->IASetInputLayout(nullptr);
        device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_effect_impl->cur_effect_pass = m_effect_impl->effect_helper->get_effect_pass(m_effect_impl->debug_pass);
        m_effect_impl->effect_helper->set_shader_resource_by_name("gShadowMap", input_srv);
        m_effect_impl->cur_effect_pass->apply(device_context);
        device_context->OMSetRenderTargets(1, &output_rtv, nullptr);
        device_context->RSSetViewports(1, &viewport);
        device_context->Draw(3, 0);

        // Clear
        auto slot = m_effect_impl->effect_helper->map_shader_resource_slot("gShadowMap");
        input_srv = nullptr;
        device_context->PSSetShaderResources(static_cast<uint32_t>(slot), 1, &input_srv);
        device_context->OMSetRenderTargets(0, nullptr, nullptr);
    }

    void ShadowEffect::set_16bit_format_shadow(bool enable)
    {
        m_effect_impl->effect_helper->get_constant_buffer_variable("g16BitShadow")->set_sint(enable);
    }

    void ShadowEffect::set_blur_kernel_size(int32_t size)
    {
        if (size % 2 == 0 || size > 15) return;

        m_effect_impl->blur_size = size;
        generate_gaussian_weights(m_effect_impl->weights, m_effect_impl->blur_size, m_effect_impl->blur_sigma);
    }

    void ShadowEffect::set_blur_sigma(float sigma)
    {
        if (sigma < 0.0f) return;

        m_effect_impl->blur_sigma = sigma;
        generate_gaussian_weights(m_effect_impl->weights, m_effect_impl->blur_size, m_effect_impl->blur_sigma);
    }

    void ShadowEffect::gaussian_blur_x(ID3D11DeviceContext *device_context, ID3D11ShaderResourceView *input_srv,
                                        ID3D11RenderTargetView *output_rtv, const D3D11_VIEWPORT &viewport)
    {
        auto pass = m_effect_impl->effect_helper->get_effect_pass(m_effect_impl->gaussian_x_pass);

        device_context->IASetInputLayout(nullptr);
        device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_effect_impl->effect_helper->set_shader_resource_by_name("gShadowMap", input_srv);
        m_effect_impl->effect_helper->get_constant_buffer_variable("gBlurWeightsArray")->set_raw(m_effect_impl->weights.data());
        pass->apply(device_context);
        device_context->OMSetRenderTargets(1, &output_rtv, nullptr);
        device_context->RSSetViewports(1, &viewport);
        device_context->Draw(3, 0);

        // Clear
        auto slot = m_effect_impl->effect_helper->map_shader_resource_slot("gShadowMap");
        input_srv = nullptr;
        device_context->PSSetShaderResources(static_cast<uint32_t>(slot), 1, &input_srv);
        device_context->OMSetRenderTargets(0, nullptr, nullptr);
    }

    void ShadowEffect::gaussian_blur_y(ID3D11DeviceContext *device_context, ID3D11ShaderResourceView *input_srv,
                                        ID3D11RenderTargetView *output_rtv, const D3D11_VIEWPORT &viewport)
    {
        auto pass = m_effect_impl->effect_helper->get_effect_pass(m_effect_impl->gaussian_y_pass);

        device_context->IASetInputLayout(nullptr);
        device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_effect_impl->effect_helper->set_shader_resource_by_name("gShadowMap", input_srv);
        m_effect_impl->effect_helper->get_constant_buffer_variable("gBlurWeightsArray")->set_raw(m_effect_impl->weights.data());
        pass->apply(device_context);
        device_context->OMSetRenderTargets(1, &output_rtv, nullptr);
        device_context->RSSetViewports(1, &viewport);
        device_context->Draw(3, 0);

        // Clear
        auto slot = m_effect_impl->effect_helper->map_shader_resource_slot("gShadowMap");
        input_srv = nullptr;
        device_context->PSSetShaderResources(static_cast<uint32_t>(slot), 1, &input_srv);
        device_context->OMSetRenderTargets(0, nullptr, nullptr);
    }

    void ShadowEffect::log_gaussian_blur(ID3D11DeviceContext *device_context, ID3D11ShaderResourceView *input_srv,
                                            ID3D11RenderTargetView *output_rtv, const D3D11_VIEWPORT &viewport)
    {
        auto pass = m_effect_impl->effect_helper->get_effect_pass(m_effect_impl->log_gaussian_pass);

        device_context->IASetInputLayout(nullptr);
        device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_effect_impl->effect_helper->set_shader_resource_by_name("gShadowMap", input_srv);
        m_effect_impl->effect_helper->get_constant_buffer_variable("gBlurWeightsArray")->set_raw(m_effect_impl->weights.data());
        pass->apply(device_context);
        device_context->OMSetRenderTargets(1, &output_rtv, nullptr);
        device_context->RSSetViewports(1, &viewport);
        device_context->Draw(3, 0);

        // Clear
        auto slot = m_effect_impl->effect_helper->map_shader_resource_slot("gShadowMap");  // TODO: modify name
        input_srv = nullptr;
        device_context->PSSetShaderResources(static_cast<uint32_t>(slot), 1, &input_srv);
        device_context->OMSetRenderTargets(0, nullptr, nullptr);
    }

    void ShadowEffect::set_material(const model::Material &material)
    {

    }

    MeshDataInput ShadowEffect::get_input_data(const model::MeshData &mesh_data)
    {
        MeshDataInput input;
        input.input_layout = m_effect_impl->cur_input_layout.Get();
        input.topology = m_effect_impl->topology;
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

    void ShadowEffect::apply(ID3D11DeviceContext *device_context)
    {
        using namespace DirectX;
        XMMATRIX world_view_proj = XMLoadFloat4x4(&m_effect_impl->world_matrix) * XMLoadFloat4x4(&m_effect_impl->view_matrix) *
                                    XMLoadFloat4x4(&m_effect_impl->proj_matrix);
        world_view_proj = XMMatrixTranspose(world_view_proj);
        m_effect_impl->effect_helper->get_constant_buffer_variable("gWorldViewProj")->set_float_matrix(4, 4, (const float *)&world_view_proj);

        m_effect_impl->cur_effect_pass->apply(device_context);
    }

    void XM_CALLCONV ShadowEffect::set_world_matrix(DirectX::FXMMATRIX world)
    {
        DirectX::XMStoreFloat4x4(&m_effect_impl->world_matrix, world);
    }

    void XM_CALLCONV ShadowEffect::set_view_matrix(DirectX::FXMMATRIX view)
    {
        DirectX::XMStoreFloat4x4(&m_effect_impl->view_matrix, view);
    }

    void XM_CALLCONV ShadowEffect::set_proj_matrix(DirectX::FXMMATRIX proj)
    {
        DirectX::XMStoreFloat4x4(&m_effect_impl->proj_matrix, proj);
    }
}






















