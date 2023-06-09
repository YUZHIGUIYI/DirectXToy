//
// Created by ZZK on 2023/6/9.
//

#include <Toy/Renderer/buffer.h>

namespace toy
{
    Buffer::Buffer(ID3D11Device *device, const CD3D11_BUFFER_DESC &buffer_desc)
    : m_byte_width(buffer_desc.ByteWidth)
    {
        device->CreateBuffer(&buffer_desc, nullptr, m_buffer.GetAddressOf());

        if (buffer_desc.BindFlags & D3D11_BIND_UNORDERED_ACCESS)
        {
            device->CreateUnorderedAccessView(m_buffer.Get(), nullptr, m_unordered_access.GetAddressOf());
        }
        if (buffer_desc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
        {
            device->CreateShaderResourceView(m_buffer.Get(), nullptr, m_shader_resource.GetAddressOf());
        }
    }

    Buffer::Buffer(ID3D11Device *device, const CD3D11_BUFFER_DESC &buffer_desc,
                    const CD3D11_SHADER_RESOURCE_VIEW_DESC &srv_desc,
                    const CD3D11_UNORDERED_ACCESS_VIEW_DESC &uav_desc)
    : m_byte_width(buffer_desc.ByteWidth)
    {
        device->CreateBuffer(&buffer_desc, nullptr, m_buffer.GetAddressOf());

        if (buffer_desc.BindFlags & D3D11_BIND_UNORDERED_ACCESS)
        {
            device->CreateUnorderedAccessView(m_buffer.Get(), &uav_desc, m_unordered_access.GetAddressOf());
        }

        if (buffer_desc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
        {
            device->CreateShaderResourceView(m_buffer.Get(), &srv_desc, m_shader_resource.GetAddressOf());
        }
    }

    void* Buffer::map_discard(ID3D11DeviceContext *device_context)
    {
        D3D11_MAPPED_SUBRESOURCE mapped_resource{};
        device_context->Map(m_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
        return mapped_resource.pData;
    }

    void Buffer::unmap(ID3D11DeviceContext *device_context)
    {
        device_context->Unmap(m_buffer.Get(), 0);
    }

    void Buffer::set_debug_object_name(std::string_view name)
    {
        // TODO
    }
}






















