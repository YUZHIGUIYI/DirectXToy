//
// Created by ZZK on 2023/6/9.
//

#pragma once

#include <Toy/Core/base.h>

namespace toy
{
    class Buffer
    {
    public:
        Buffer(ID3D11Device* device, const CD3D11_BUFFER_DESC& buffer_desc);
        Buffer(ID3D11Device* device, const CD3D11_BUFFER_DESC& buffer_desc,
                const CD3D11_SHADER_RESOURCE_VIEW_DESC& srv_desc,
                const CD3D11_UNORDERED_ACCESS_VIEW_DESC& uav_desc);

        virtual ~Buffer() = default;

        Buffer(const Buffer&) = delete;
        Buffer& operator=(const Buffer&) = delete;
        Buffer(Buffer&&) = default;
        Buffer& operator=(Buffer&&) = default;

        ID3D11Buffer* get_buffer() { return m_buffer.Get(); }
        ID3D11UnorderedAccessView* get_unordered_access() { return m_unordered_access.Get(); }
        ID3D11ShaderResourceView* get_shader_resource() { return m_shader_resource.Get(); }

        void* map_discard(ID3D11DeviceContext* device_context);
        void unmap(ID3D11DeviceContext* device_context);
        [[nodiscard]] uint32_t get_byte_width() const { return m_byte_width; }

        // Set debug object name
        void set_debug_object_name(std::string_view name);

    protected:
        com_ptr<ID3D11Buffer> m_buffer;
        com_ptr<ID3D11ShaderResourceView> m_shader_resource;
        com_ptr<ID3D11UnorderedAccessView> m_unordered_access;
        uint32_t m_byte_width = 0;
    };
}














