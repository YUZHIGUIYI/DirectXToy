//
// Created by ZHIKANG on 2023/5/27.
//

#include <Toy/Renderer/effects.h>
#include <Toy/Renderer/effect_helper.h>
#include <Toy/Core/d3d_util.h>
#include <Toy/Geometry/vertex.h>

namespace toy
{
    basic_effect_c::basic_effect_c()
    {
        p_impl_ = std::make_unique<basic_effect_c::impl_s>();
    }

    basic_effect_c::basic_effect_c(toy::basic_effect_c &&other) noexcept
    {
        p_impl_.swap(other.p_impl_);
    }

    basic_effect_c& basic_effect_c::operator=(toy::basic_effect_c &&other) noexcept
    {
        p_impl_.swap(other.p_impl_);
        return *this;
    }

    void basic_effect_c::init(ID3D11Device *device)
    {
        if (!device)
        {
            DX_CORE_CRITICAL("Direct3D device is nullptr, can not initialize base effect");
        }

        if (!p_impl_->cb_objects_.empty())
        {
            DX_CORE_WARN("Basic effect has already initialized");
            return;
        }

        // Initialize render states

        com_ptr<ID3DBlob> blob = nullptr;

        // Create vertex shader 2d
        if (create_shader_from_file(L"../data/shaders/base_2d_vs.cso", L"../data/shaders/base_2d_vs.hlsl", "VS",
                                    "vs_5_0", blob.ReleaseAndGetAddressOf()) != S_OK)
        {
            DX_CRITICAL("Can not compile 2d vertex shader file");
        }
        device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, p_impl_->vertex_shader_2d_.GetAddressOf());
        // Create vertex input layout 2d
        auto&& input_layout_2d = VertexPosTex::get_input_layout();
        device->CreateInputLayout(input_layout_2d.data(), (uint32_t)input_layout_2d.size(),
                                    blob->GetBufferPointer(), blob->GetBufferSize(), p_impl_->vertex_layout_2d_.GetAddressOf());

        // Create pixel shader 2d
        if (create_shader_from_file(L"../data/shaders/base_2d_ps.cso", L"../data/shaders/base_2d_ps.hlsl", "PS",
                                    "ps_5_0", blob.ReleaseAndGetAddressOf()) != S_OK)
        {
            DX_CRITICAL("Can not compile 2d pixel shader file");
        }
        device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, p_impl_->pixel_shader_2d_.GetAddressOf());

        // Create vertex shader 3d
        if (create_shader_from_file(L"../data/shaders/base_3d_vs.cso", L"../data/shaders/base_3d_vs.hlsl", "VS",
                                    "vs_5_0", blob.ReleaseAndGetAddressOf()) != S_OK)
        {
            DX_CRITICAL("Can not compile 3d vertex shader file");
        }
        device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, p_impl_->vertex_shader_3d_.GetAddressOf());
        // Create vertex input layout 3d
        auto&& input_layout_3d = VertexPosNormalTex::get_input_layout();
        device->CreateInputLayout(input_layout_3d.data(), (uint32_t)input_layout_3d.size(), blob->GetBufferPointer(),
                                    blob->GetBufferSize(), p_impl_->vertex_layout_3d_.GetAddressOf());

        // Create pixel shader 3d
        if (create_shader_from_file(L"../data/shaders/base_3d_ps.cso", L"../data/shaders/base_3d_ps.hlsl", "PS",
                                    "ps_5_0", blob.ReleaseAndGetAddressOf()) != S_OK)
        {
            DX_CRITICAL("Can not compile 3d pixel shader file");
        }
        device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, p_impl_->pixel_shader_3d_.GetAddressOf());

        // Manage all constant buffer objects
        p_impl_->cb_objects_.assign({
            &p_impl_->cb_drawing_,
            &p_impl_->cb_frame_,
            &p_impl_->cb_states_,
            &p_impl_->cb_on_resize_,
            &p_impl_->cb_rarely_
        });

        // Create constant buffers
        for (auto&& buffer_object : p_impl_->cb_objects_)
        {
            buffer_object->create_buffer(device);
        }

        // Set debug object name

        // Finish
    }

    void basic_effect_c::set_default_render(ID3D11DeviceContext *device_context)
    {
        device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        device_context->IASetInputLayout(p_impl_->vertex_layout_3d_.Get());
        device_context->VSSetShader(p_impl_->vertex_shader_3d_.Get(), nullptr, 0);
        device_context->RSSetState(nullptr);
        device_context->PSSetShader(p_impl_->pixel_shader_3d_.Get(), nullptr, 0);
        device_context->PSSetSamplers(0, 1, RenderStates::ss_linear_wrap.GetAddressOf());
        device_context->OMSetDepthStencilState(nullptr, 0);
        device_context->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
    }

    void basic_effect_c::set_alpha_blend_render(ID3D11DeviceContext *device_context)
    {
        device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        device_context->IASetInputLayout(p_impl_->vertex_layout_3d_.Get());
        device_context->VSSetShader(p_impl_->vertex_shader_3d_.Get(), nullptr, 0);
        device_context->RSSetState(RenderStates::rs_no_cull.Get());
        device_context->PSSetShader(p_impl_->pixel_shader_3d_.Get(), nullptr, 0);
        device_context->PSSetSamplers(0, 1, RenderStates::ss_linear_wrap.GetAddressOf());
        device_context->OMSetDepthStencilState(nullptr, 0);
        device_context->OMSetBlendState(RenderStates::bs_transparent.Get(), nullptr, 0xFFFFFFFF);
    }

    void basic_effect_c::set_no_depth_test_render(ID3D11DeviceContext *device_context)
    {
        device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        device_context->IASetInputLayout(p_impl_->vertex_layout_3d_.Get());
        device_context->VSSetShader(p_impl_->vertex_shader_3d_.Get(), nullptr, 0);
        device_context->RSSetState(RenderStates::rs_no_cull.Get());
        device_context->PSSetShader(p_impl_->pixel_shader_3d_.Get(), nullptr, 0);
        device_context->PSSetSamplers(0, 1, RenderStates::ss_linear_wrap.GetAddressOf());
        device_context->OMSetDepthStencilState(RenderStates::ds_no_depth_test.Get(), 0);
        device_context->OMSetBlendState(RenderStates::bs_additive.Get(), nullptr, 0xFFFFFFFF);
    }

    void basic_effect_c::set_no_depth_write_render(ID3D11DeviceContext *device_context)
    {
        device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        device_context->IASetInputLayout(p_impl_->vertex_layout_3d_.Get());
        device_context->VSSetShader(p_impl_->vertex_shader_3d_.Get(), nullptr, 0);
        device_context->RSSetState(RenderStates::rs_no_cull.Get());
        device_context->PSSetShader(p_impl_->pixel_shader_3d_.Get(), nullptr, 0);
        device_context->PSSetSamplers(0, 1, RenderStates::ss_linear_wrap.GetAddressOf());
        device_context->OMSetDepthStencilState(RenderStates::ds_no_depth_write.Get(), 0);
        device_context->OMSetBlendState(RenderStates::bs_additive.Get(), nullptr, 0xFFFFFFFF);
    }

    void basic_effect_c::set_no_double_blend_render(ID3D11DeviceContext *device_context, uint32_t stencil_ref)
    {
        device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        device_context->IASetInputLayout(p_impl_->vertex_layout_3d_.Get());
        device_context->VSSetShader(p_impl_->vertex_shader_3d_.Get(), nullptr, 0);
        device_context->RSSetState(RenderStates::rs_no_cull.Get());
        device_context->PSSetShader(p_impl_->pixel_shader_3d_.Get(), nullptr, 0);
        device_context->PSSetSamplers(0, 1, RenderStates::ss_linear_wrap.GetAddressOf());
        device_context->OMSetDepthStencilState(RenderStates::ds_no_double_blend.Get(), stencil_ref);
        device_context->OMSetBlendState(RenderStates::bs_transparent.Get(), nullptr, 0xFFFFFFFF);
    }

    void basic_effect_c::set_write_stencil_only_render(ID3D11DeviceContext *device_context, uint32_t stencil_ref)
    {
        device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        device_context->IASetInputLayout(p_impl_->vertex_layout_3d_.Get());
        device_context->VSSetShader(p_impl_->vertex_shader_3d_.Get(), nullptr, 0);
        device_context->RSSetState(nullptr);
        device_context->PSSetShader(p_impl_->pixel_shader_3d_.Get(), nullptr, 0);
        device_context->PSSetSamplers(0, 1, RenderStates::ss_linear_wrap.GetAddressOf());
        device_context->OMSetDepthStencilState(RenderStates::ds_write_stencil.Get(), stencil_ref);
        device_context->OMSetBlendState(RenderStates::bs_no_color_write.Get(), nullptr, 0xFFFFFFFF);
    }

    void basic_effect_c::set_default_with_stencil_render(ID3D11DeviceContext *device_context, uint32_t stencil_ref)
    {
        device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        device_context->IASetInputLayout(p_impl_->vertex_layout_3d_.Get());
        device_context->VSSetShader(p_impl_->vertex_shader_3d_.Get(), nullptr, 0);
        device_context->RSSetState(RenderStates::rs_cull_clock_wise.Get());
        device_context->PSSetShader(p_impl_->pixel_shader_3d_.Get(), nullptr, 0);
        device_context->PSSetSamplers(0, 1, RenderStates::ss_linear_wrap.GetAddressOf());
        device_context->OMSetDepthStencilState(RenderStates::ds_draw_with_stencil.Get(), stencil_ref);
        device_context->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
    }

    void basic_effect_c::set_alpha_blend_with_stencil_render(ID3D11DeviceContext *device_context, uint32_t stencil_ref)
    {
        device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        device_context->IASetInputLayout(p_impl_->vertex_layout_3d_.Get());
        device_context->VSSetShader(p_impl_->vertex_shader_3d_.Get(), nullptr, 0);
        device_context->RSSetState(RenderStates::rs_no_cull.Get());
        device_context->PSSetShader(p_impl_->pixel_shader_3d_.Get(), nullptr, 0);
        device_context->PSSetSamplers(0, 1, RenderStates::ss_linear_wrap.GetAddressOf());
        device_context->OMSetDepthStencilState(RenderStates::ds_draw_with_stencil.Get(), stencil_ref);
        device_context->OMSetBlendState(RenderStates::bs_transparent.Get(), nullptr, 0xFFFFFFFF);
    }

    void basic_effect_c::set_no_depth_test_with_stencil_render(ID3D11DeviceContext *device_context, uint32_t stencil_ref)
    {
        device_context->IASetInputLayout(p_impl_->vertex_layout_3d_.Get());
        device_context->VSSetShader(p_impl_->vertex_shader_3d_.Get(), nullptr, 0);
        device_context->RSSetState(RenderStates::rs_no_cull.Get());
        device_context->PSSetShader(p_impl_->pixel_shader_3d_.Get(), nullptr, 0);
        device_context->PSSetSamplers(0, 1, RenderStates::ss_linear_wrap.GetAddressOf());
        device_context->OMSetDepthStencilState(RenderStates::ds_no_depth_test_with_stencil.Get(), stencil_ref);
        device_context->OMSetBlendState(RenderStates::bs_additive.Get(), nullptr, 0xFFFFFFFF);
    }

    void basic_effect_c::set_no_depth_write_with_stencil_render(ID3D11DeviceContext *device_context, uint32_t stencil_ref)
    {
        device_context->IASetInputLayout(p_impl_->vertex_layout_3d_.Get());
        device_context->VSSetShader(p_impl_->vertex_shader_3d_.Get(), nullptr, 0);
        device_context->RSSetState(RenderStates::rs_no_cull.Get());
        device_context->PSSetShader(p_impl_->pixel_shader_3d_.Get(), nullptr, 0);
        device_context->PSSetSamplers(0, 1, RenderStates::ss_linear_wrap.GetAddressOf());
        device_context->OMSetDepthStencilState(RenderStates::ds_no_depth_write_with_stencil.Get(), stencil_ref);
        device_context->OMSetBlendState(RenderStates::bs_additive.Get(), nullptr, 0xFFFFFFFF);
    }

    void basic_effect_c::set_2d_default_render(ID3D11DeviceContext *device_context)
    {
        device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        device_context->IASetInputLayout(p_impl_->vertex_layout_2d_.Get());
        device_context->VSSetShader(p_impl_->vertex_shader_2d_.Get(), nullptr, 0);
        device_context->RSSetState(nullptr);
        device_context->PSSetShader(p_impl_->pixel_shader_2d_.Get(), nullptr, 0);
        device_context->PSSetSamplers(0, 1, RenderStates::ss_linear_wrap.GetAddressOf());
        device_context->OMSetDepthStencilState(nullptr, 0);
        device_context->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
    }

    void basic_effect_c::set_2d_alpha_blend_render(ID3D11DeviceContext *device_context)
    {
        device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        device_context->IASetInputLayout(p_impl_->vertex_layout_2d_.Get());
        device_context->VSSetShader(p_impl_->vertex_shader_2d_.Get(), nullptr, 0);
        device_context->RSSetState(RenderStates::rs_no_cull.Get());
        device_context->PSSetShader(p_impl_->pixel_shader_2d_.Get(), nullptr, 0);
        device_context->PSSetSamplers(0, 1, RenderStates::ss_linear_wrap.GetAddressOf());
        device_context->OMSetDepthStencilState(nullptr, 0);
        device_context->OMSetBlendState(RenderStates::bs_transparent.Get(), nullptr, 0xFFFFFFFF);
    }

    void XM_CALLCONV basic_effect_c::set_world_matrix(DirectX::FXMMATRIX world)
    {
        auto& buffer_object = p_impl_->cb_drawing_;
        buffer_object.data.world = DirectX::XMMatrixTranspose(world);
        buffer_object.data.world_inv_transpose = DirectX::XMMatrixTranspose(inverse_transpose(world));
        p_impl_->is_dirty_ = buffer_object.is_dirty = true;
    }

    void XM_CALLCONV basic_effect_c::set_view_matrix(DirectX::FXMMATRIX view)
    {
        auto& buffer_object = p_impl_->cb_frame_;
        buffer_object.data.view = DirectX::XMMatrixTranspose(view);
        p_impl_->is_dirty_ = buffer_object.is_dirty = true;
    }

    void XM_CALLCONV basic_effect_c::set_proj_matrix(DirectX::FXMMATRIX proj)
    {
        auto& buffer_object = p_impl_->cb_on_resize_;
        buffer_object.data.proj = DirectX::XMMatrixTranspose(proj);
        p_impl_->is_dirty_ = buffer_object.is_dirty = true;
    }

    void XM_CALLCONV basic_effect_c::set_reflection_matrix(DirectX::FXMMATRIX reflection)
    {
        auto& buffer_object = p_impl_->cb_rarely_;
        buffer_object.data.reflection = DirectX::XMMatrixTranspose(reflection);
        p_impl_->is_dirty_ = buffer_object.is_dirty = true;
    }

    void XM_CALLCONV basic_effect_c::set_shadow_matrix(DirectX::FXMMATRIX shadow)
    {
        auto& buffer_object = p_impl_->cb_rarely_;
        buffer_object.data.shadow = DirectX::XMMatrixTranspose(shadow);
        p_impl_->is_dirty_ = buffer_object.is_dirty = true;
    }

    void XM_CALLCONV basic_effect_c::set_ref_shadow_matrix(DirectX::FXMMATRIX ref_shadow)
    {
        auto& buffer_object = p_impl_->cb_rarely_;
        buffer_object.data.ref_shadow = DirectX::XMMatrixTranspose(ref_shadow);
        p_impl_->is_dirty_ = buffer_object.is_dirty = true;
    }

    void basic_effect_c::set_dir_light(size_t pos, const toy::DirectionalLight &dir_light)
    {
        assert(pos < max_lights);
        auto& buffer_object = p_impl_->cb_rarely_;
        buffer_object.data.dir_light[pos] = dir_light;
        p_impl_->is_dirty_ = buffer_object.is_dirty = true;
    }

    void basic_effect_c::set_point_light(size_t pos, const toy::PointLight &point_light)
    {
        assert(pos < max_lights);
        auto& buffer_object = p_impl_->cb_rarely_;
        buffer_object.data.point_light[pos] = point_light;
        p_impl_->is_dirty_ = buffer_object.is_dirty = true;
    }

    void basic_effect_c::set_material(const toy::Material &material)
    {
        auto& buffer_object = p_impl_->cb_drawing_;
        buffer_object.data.material = material;
        p_impl_->is_dirty_ = buffer_object.is_dirty = true;
    }

    void basic_effect_c::set_texture(ID3D11ShaderResourceView *texture)
    {
        p_impl_->texture_ = texture;
    }

    void basic_effect_c::set_eye_pos(const DirectX::XMFLOAT3 &eye_pos)
    {
        auto& buffer_object = p_impl_->cb_frame_;
        buffer_object.data.eye_pos = eye_pos;
        p_impl_->is_dirty_ = buffer_object.is_dirty = true;
    }

    void basic_effect_c::set_reflection_state(bool state)
    {
        auto& buffer_object = p_impl_->cb_states_;
        buffer_object.data.is_reflection = state;
        p_impl_->is_dirty_ = buffer_object.is_dirty = true;
    }

    void basic_effect_c::set_shadow_state(bool state)
    {
        auto& buffer_object = p_impl_->cb_states_;
        buffer_object.data.is_shadow = state;
        p_impl_->is_dirty_ = buffer_object.is_dirty = true;
    }

    void basic_effect_c::apply(ID3D11DeviceContext *device_context)
    {
        auto&& buffer_objects = p_impl_->cb_objects_;
        // Bind
        buffer_objects[0]->bind_vs(device_context);
        buffer_objects[1]->bind_vs(device_context);
        buffer_objects[2]->bind_vs(device_context);
        buffer_objects[3]->bind_vs(device_context);
        buffer_objects[4]->bind_vs(device_context);

        buffer_objects[0]->bind_ps(device_context);
        buffer_objects[1]->bind_ps(device_context);
        buffer_objects[2]->bind_ps(device_context);
        buffer_objects[4]->bind_ps(device_context);

        // Set texture
        device_context->PSSetShaderResources(0, 1, p_impl_->texture_.GetAddressOf());

        // Update constant buffer
        if (p_impl_->is_dirty_)
        {
            p_impl_->is_dirty_ = false;
            for (auto& buffer_object : p_impl_->cb_objects_)
            {
                buffer_object->update_buffer(device_context);
            }
        }
    }
}









































