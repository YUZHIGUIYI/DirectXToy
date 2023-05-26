//
// Created by ZZK on 2023/5/24.
//

#include <Sandbox/game_object.h>

namespace toy
{
    game_object_c::game_object_c()
    : class_vertex_stride(0), class_index_count(0)
    {}

    transform_c& game_object_c::get_transform()
    {
        return class_transform;
    }

    const transform_c& game_object_c::get_transform() const
    {
        return class_transform;
    }

    void game_object_c::set_texture(ID3D11ShaderResourceView *texture)
    {
        class_texture = texture;
    }

    void game_object_c::draw(ID3D11DeviceContext *device_context)
    {
        // Set vertex and index buffer
        uint32_t strides = class_vertex_stride;
        uint32_t offsets = 0;
        device_context->IASetVertexBuffers(0, 1, class_vertex_buffer.GetAddressOf(), &strides, &offsets);
        device_context->IASetIndexBuffer(class_index_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

        // Get binding every-draw constant buffer and modify
        com_ptr<ID3D11Buffer> c_buffer = nullptr;
        device_context->VSGetConstantBuffers(0, 1, c_buffer.GetAddressOf());

        CBChangeEveryDraw cb_every_draw{};
        DirectX::XMMATRIX world = class_transform.get_local_to_world_matrix_xm();
        cb_every_draw.world = DirectX::XMMatrixTranspose(world);
        cb_every_draw.world_inv_transpose = DirectX::XMMatrixTranspose(inverse_transpose(world));

        // Update constant buffer
        D3D11_MAPPED_SUBRESOURCE mapped_data{};
        device_context->Map(c_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_data);
        memcpy_s(mapped_data.pData, sizeof(CBChangeEveryDraw), &cb_every_draw, sizeof(CBChangeEveryDraw));
        device_context->Unmap(c_buffer.Get(), 0);

        // Set texture
        device_context->PSSetShaderResources(0, 1, class_texture.GetAddressOf());

        // Draw
        device_context->DrawIndexed(class_index_count, 0, 0);
    }

    void game_object_c::set_debug_object_name(const std::string &name)
    {

    }
}