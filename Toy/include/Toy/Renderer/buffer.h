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
        com_ptr<ID3D11Buffer> m_buffer = nullptr;
        com_ptr<ID3D11ShaderResourceView> m_shader_resource = nullptr;
        com_ptr<ID3D11UnorderedAccessView> m_unordered_access = nullptr;
        uint32_t m_byte_width = 0;
    };

    // Ensure the size and layout of T are the same as those of structure in shader
    template<typename T>
    class StructureBuffer : public Buffer
    {
    public:
        StructureBuffer(ID3D11Device* device, uint32_t elements,
                        uint32_t bind_flags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE,
                        bool enable_counter = false,
                        bool dynamic = false);
        ~StructureBuffer() = default;

        StructureBuffer(const StructureBuffer&) = delete;
        StructureBuffer& operator=(const StructureBuffer&) = delete;
        StructureBuffer(StructureBuffer&&) = default;
        StructureBuffer& operator=(StructureBuffer&&) = default;

        // Only support dynamic buffer
        T* map_discard(ID3D11DeviceContext* device_context);

        [[nodiscard]] uint32_t get_num_elements() const { return m_elements; }

    private:
        uint32_t m_elements;
    };

    template<typename T>
    StructureBuffer<T>::StructureBuffer(ID3D11Device *device, uint32_t elements, uint32_t bind_flags,
                                        bool enable_counter, bool dynamic)
    : m_elements(elements),
    Buffer(device, CD3D11_BUFFER_DESC{ sizeof(T) * elements, bind_flags,
                                    dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT,
                                    dynamic ? D3D11_CPU_ACCESS_WRITE : uint32_t(0),
                                    D3D11_RESOURCE_MISC_BUFFER_STRUCTURED,
                                    sizeof(T) },
        CD3D11_SHADER_RESOURCE_VIEW_DESC{ D3D11_SRV_DIMENSION_BUFFER, DXGI_FORMAT_UNKNOWN, 0, elements },
        CD3D11_UNORDERED_ACCESS_VIEW_DESC{ D3D11_UAV_DIMENSION_BUFFER, DXGI_FORMAT_UNKNOWN, 0, elements, 0, D3D11_BUFFER_UAV_FLAG_COUNTER })
    {

    }

    template<typename T>
    T* StructureBuffer<T>::map_discard(ID3D11DeviceContext *device_context)
    {
        D3D11_MAPPED_SUBRESOURCE mapped_resource{};
        device_context->Map(m_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
        return static_cast<T*>(mapped_resource.pData);
    }

}














