//
// Created by ZHIKANG on 2023/5/27.
//

#pragma once

#include <Toy/Core/base.h>

namespace toy
{
    // A class requires memory alignment should derive from the following class
    template<typename DerivedType>
    struct aligned_type_s
    {
        static void* operator new(size_t size)
        {
            const size_t aligned_size = __alignof(DerivedType);

            static_assert(aligned_size > 8, "Aligned new is only useful for types with > 8 byte alignment! Ensure add __declspec(align) on DerivedType");

            void* ptr = _aligned_malloc(size, aligned_size);
            if (!ptr)
            {
                DX_CORE_ERROR("Derived type bad alloc");
                throw std::bad_alloc();
            }

            return ptr;
        }

        static void operator delete(void* ptr)
        {
            _aligned_free(ptr);
        }
    };

    template<typename DerivedType>
    using AlignedType = aligned_type_s<DerivedType>;

    struct constant_buffer_base_s
    {
        int32_t is_dirty;
        com_ptr<ID3D11Buffer> c_buffer;

        constant_buffer_base_s() : is_dirty(false), c_buffer(nullptr) {}

        virtual HRESULT create_buffer(ID3D11Device *device) = 0;
        virtual void update_buffer(ID3D11DeviceContext *device_context) = 0;
        virtual void bind_vs(ID3D11DeviceContext *device_context) = 0;
        virtual void bind_hs(ID3D11DeviceContext *device_context) = 0;
        virtual void bind_ds(ID3D11DeviceContext *device_context) = 0;
        virtual void bind_gs(ID3D11DeviceContext *device_context) = 0;
        virtual void bind_cs(ID3D11DeviceContext *device_context) = 0;
        virtual void bind_ps(ID3D11DeviceContext *device_context) = 0;
    };

    using CBufferBase = constant_buffer_base_s;

    template<typename DataType, uint32_t StartSlot>
    struct constant_buffer_object_s : constant_buffer_base_s
    {
        DataType data;

        constant_buffer_object_s() : constant_buffer_base_s{}, data{} {}

        HRESULT create_buffer(ID3D11Device *device) override
        {
            if (c_buffer != nullptr)
            {
                return S_OK;
            }

            D3D11_BUFFER_DESC constant_buffer_desc{};
            constant_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
            constant_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            constant_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            constant_buffer_desc.ByteWidth = sizeof(DataType);
            return device->CreateBuffer(&constant_buffer_desc, nullptr, c_buffer.GetAddressOf());
        }

        void update_buffer(ID3D11DeviceContext *device_context) override
        {
            if (is_dirty)
            {
                is_dirty = false;
                D3D11_MAPPED_SUBRESOURCE mapped_data{};
                device_context->Map(c_buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_data);
                memcpy_s(mapped_data.pData, sizeof(DataType), &data, sizeof(DataType));
                device_context->Unmap(c_buffer.Get(), 0);
            }
        }

        void bind_vs(ID3D11DeviceContext *device_context) override
        {
            device_context->VSSetConstantBuffers(StartSlot, 1, c_buffer.GetAddressOf());
        }
        void bind_hs(ID3D11DeviceContext *device_context) override
        {
            device_context->HSSetConstantBuffers(StartSlot, 1, c_buffer.GetAddressOf());
        }
        void bind_ds(ID3D11DeviceContext *device_context) override
        {
            device_context->DSSetConstantBuffers(StartSlot, 1, c_buffer.GetAddressOf());
        }
        void bind_gs(ID3D11DeviceContext *device_context) override
        {
            device_context->GSSetConstantBuffers(StartSlot, 1, c_buffer.GetAddressOf());
        }
        void bind_cs(ID3D11DeviceContext *device_context) override
        {
            device_context->CSSetConstantBuffers(StartSlot, 1, c_buffer.GetAddressOf());
        }
        void bind_ps(ID3D11DeviceContext *device_context) override
        {
            device_context->PSSetConstantBuffers(StartSlot, 1, c_buffer.GetAddressOf());
        }
    };

    template<typename DataType, uint32_t StartSlot>
    using CBufferObject = constant_buffer_object_s<DataType, StartSlot>;
}











