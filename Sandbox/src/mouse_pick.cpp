//
// Created by ZZK on 2023/11/30.
//

#include <Sandbox/mouse_pick.h>

namespace toy
{
    uint32_t MousePickHelper::pick_entity(ID3D11Device *device, ID3D11DeviceContext *device_context, ID3D11Texture2D *entity_id_buffer, int32_t mouse_pos_x, int32_t mouse_pos_y)
    {
        if (mouse_pos_x < 0 || mouse_pos_y < 0)
        {
            return 0;
        }

        D3D11_TEXTURE2D_DESC staging_desc = {};
        entity_id_buffer->GetDesc(&staging_desc);
        if (staging_desc.Width != staging_width || staging_desc.Height != staging_height)
        {
            staging_texture.Reset();
            staging_width = staging_desc.Width;
            staging_height = staging_desc.Height;
            staging_desc.Usage = D3D11_USAGE_STAGING;
            staging_desc.BindFlags = 0;
            staging_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

            device->CreateTexture2D(&staging_desc, nullptr, staging_texture.GetAddressOf());
        }

        device_context->CopyResource(staging_texture.Get(), entity_id_buffer);

        return get_entity_id(device_context, static_cast<uint32_t>(mouse_pos_x), static_cast<uint32_t>(mouse_pos_y));
    }

    uint32_t MousePickHelper::get_entity_id(ID3D11DeviceContext *device_context, uint32_t mouse_pos_x, uint32_t mouse_pos_y)
    {
        // TODO: check return value
        // Note: Avoid cast mapped data from void pointer to uint32_t pointer, consider uint8_t pointer firstly
        // Note: Should cast staging data of vector from uint32_t pointer to uint8_t pointer

        if (mouse_pos_x >= staging_width || mouse_pos_y >= staging_height) return 0;

        staging_data.clear();
        staging_data.resize(staging_width * staging_height, 0);

        D3D11_MAPPED_SUBRESOURCE mapped_subresource = {};
        device_context->Map(staging_texture.Get(), 0, D3D11_MAP_READ, 0, &mapped_subresource);
        auto texture_data = reinterpret_cast<uint8_t *>(mapped_subresource.pData);
        auto reinterpret_staging_data = reinterpret_cast<uint8_t *>(staging_data.data());
        auto stride = mapped_subresource.RowPitch;
        auto multiple = sizeof(std::decay_t<decltype(staging_data)>::value_type) / sizeof(uint8_t);
        for (uint32_t i = 0; i < staging_height; ++i)
        {
            memcpy_s(reinterpret_staging_data + i * staging_width * multiple, staging_width * multiple, texture_data, staging_width * multiple);
            texture_data += stride;
        }
        device_context->Unmap(staging_texture.Get(), 0);

        auto index = static_cast<size_t>(mouse_pos_y * staging_width + mouse_pos_x - 1);
        return staging_data[index];
    }
}