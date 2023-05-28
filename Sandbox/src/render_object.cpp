//
// Created by ZZK on 2023/5/24.
//

#include <Sandbox/render_object.h>

namespace toy
{
    render_object_c::render_object_c()
    : class_vertex_stride(0), class_index_count(0)
    {}

    transform_c& render_object_c::get_transform()
    {
        return class_transform;
    }

    const transform_c& render_object_c::get_transform() const
    {
        return class_transform;
    }

    void render_object_c::set_texture(ID3D11ShaderResourceView *texture)
    {
        class_texture = texture;
    }

    void render_object_c::set_material(const toy::Material &material)
    {
        class_material = material;
    }

    void render_object_c::draw(ID3D11DeviceContext *device_context, basic_effect_c& effect)
    {
        // Set vertex and index buffer
        uint32_t strides = class_vertex_stride;
        uint32_t offsets = 0;
        device_context->IASetVertexBuffers(0, 1, class_vertex_buffer.GetAddressOf(), &strides, &offsets);
        device_context->IASetIndexBuffer(class_index_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

        // Update constant buffer data and then apply
        effect.set_world_matrix(class_transform.get_local_to_world_matrix_xm());
        effect.set_texture(class_texture.Get());
        effect.set_material(class_material);

        effect.apply(device_context);

        // Draw
        device_context->DrawIndexed(class_index_count, 0, 0);
    }

    void render_object_c::set_debug_object_name(const std::string &name)
    {

    }
}