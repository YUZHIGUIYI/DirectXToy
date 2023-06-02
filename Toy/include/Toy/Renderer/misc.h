//
// Created by ZZK on 2023/5/30.
//

#pragma once

#include <Toy/Core/base.h>

namespace toy
{
    enum ShaderFlag
    {
        PixelShader = 0x1,
        VertexShader = 0x2,
        GeometryShader = 0x4,
        HullShader = 0x8,
        DomainShader = 0x10,
        ComputeShader = 0x20,
    };

    // Shader resource
    struct ShaderResource
    {
        std::string name;
        D3D11_SRV_DIMENSION dim;
        com_ptr<ID3D11ShaderResourceView> srv;
    };

    // Readable and writable resource
    struct RWResource
    {
        std::string name;
        D3D11_UAV_DIMENSION dim;
        com_ptr<ID3D11UnorderedAccessView> uav;
        uint32_t initial_count;
        bool enable_counter;
        bool first_init;     // prohibit clear again
    };

    // Sampler state
    struct SamplerState
    {
        std::string name;
        com_ptr<ID3D11SamplerState> ss;
    };

    // Constant buffer data
    struct CBufferData
    {
        int32_t is_dirty = 0;
        com_ptr<ID3D11Buffer> cbuffer;
        std::vector<uint8_t> cbuffer_data;
        std::string cbuffer_name;
        uint32_t start_slot = 0;

        CBufferData() = default;
        CBufferData(const std::string& name, uint32_t start_slot, uint32_t byte_width, uint8_t* init_data = nullptr)
        : cbuffer_data(byte_width), cbuffer_name(name), start_slot(start_slot)
        {
            if (init_data)
            {
                memcpy_s(cbuffer_data.data(), byte_width, init_data, byte_width);
            }
        }

        HRESULT create_buffer(ID3D11Device *device)
        {
            if (cbuffer != nullptr)
            {
                return S_OK;
            }

            D3D11_BUFFER_DESC constant_buffer_desc{};
            constant_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
            constant_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            constant_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            constant_buffer_desc.ByteWidth = static_cast<uint32_t>(cbuffer_data.size());
            return device->CreateBuffer(&constant_buffer_desc, nullptr, cbuffer.GetAddressOf());
        }

        void update_buffer(ID3D11DeviceContext *device_context)
        {
            if (is_dirty)
            {
                is_dirty = false;
                D3D11_MAPPED_SUBRESOURCE mapped_data{};
                device_context->Map(cbuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_data);
                memcpy_s(mapped_data.pData, cbuffer_data.size(), cbuffer_data.data(), cbuffer_data.size());
                device_context->Unmap(cbuffer.Get(), 0);
            }
        }

        void bind_vs(ID3D11DeviceContext *device_context)
        {
            device_context->VSSetConstantBuffers(start_slot, 1, cbuffer.GetAddressOf());
        }

        void bind_hs(ID3D11DeviceContext *device_context)
        {
            device_context->HSSetConstantBuffers(start_slot, 1, cbuffer.GetAddressOf());
        }

        void bind_ds(ID3D11DeviceContext *device_context)
        {
            device_context->DSSetConstantBuffers(start_slot, 1, cbuffer.GetAddressOf());
        }

        void bind_gs(ID3D11DeviceContext *device_context)
        {
            device_context->GSSetConstantBuffers(start_slot, 1, cbuffer.GetAddressOf());
        }

        void bind_cs(ID3D11DeviceContext *device_context)
        {
            device_context->CSSetConstantBuffers(start_slot, 1, cbuffer.GetAddressOf());
        }

        void bind_ps(ID3D11DeviceContext *device_context)
        {
            device_context->PSSetConstantBuffers(start_slot, 1, cbuffer.GetAddressOf());
        }
    };

    // Constant buffer variable
    struct effect_constant_buffer_variable_interface_s
    {
        // Set unsigned integer, bool can use
        virtual void set_uint(uint32_t val) = 0;
        // Set signed integer
        virtual void set_sint(int32_t val) = 0;
        // Set float
        virtual void set_float(float val) = 0;

        // Set unsigned integer vector, 1 - 4 components
        // bool can use
        // By setting components to read data
        virtual void set_uint_vector(uint32_t num_components, const uint32_t data[4]) = 0;

        // Set singed integer vector, 1 - 4 components
        virtual void set_sint_vector(uint32_t num_components, const int32_t data[4]) = 0;

        // Set float vector, 1 - 4 components
        virtual void set_float_vector(uint32_t num_components, const float data[4]) = 0;

        // Set unsigned integer matrix, 1 - 4 rows and columns
        virtual void set_uint_matrix(uint32_t rows, uint32_t cols, const uint32_t* no_pad_data) = 0;

        // Set signed integer matrix, 1 - 4 rows and columns
        virtual void set_sint_matrix(uint32_t rows, uint32_t cols, const int32_t* no_pad_data) = 0;

        // Set float matrix, 1 - 4 rows and columns
        virtual void set_float_matrix(uint32_t rows, uint32_t cols, const float* no_pad_data) = 0;

        // Set other types, allow setting range
        virtual void set_raw(const void* data, uint32_t byte_offset = 0, uint32_t byte_count = 0xFFFFFFFF) = 0;

        // Set property


        // Obtain the most recently setting values, allow specifying reading range
        virtual HRESULT get_raw(void* p_output, uint32_t byte_offset = 0, uint32_t byte_count = 0xFFFFFFFF) = 0;

        virtual ~effect_constant_buffer_variable_interface_s() = default;
    };
    using IEffectConstantBufferVariable = effect_constant_buffer_variable_interface_s;

    struct ConstantBufferVariable : IEffectConstantBufferVariable
    {
        struct PropertyFunctor
        {
            PropertyFunctor(ConstantBufferVariable& _cbv) : cbv(_cbv) {}
            void operator()(int32_t val) { cbv.set_sint(val); }
            void operator()(uint32_t val) { cbv.set_uint(val); }
            void operator()(float val) { cbv.set_float(val); }
            void operator()(const DirectX::XMFLOAT2& val) { cbv.set_float_vector(2, reinterpret_cast<const float *>(&val)); }
            void operator()(const DirectX::XMFLOAT3& val) { cbv.set_float_vector(3, reinterpret_cast<const float*>(&val)); }
            void operator()(const DirectX::XMFLOAT4& val) { cbv.set_float_vector(4, reinterpret_cast<const float*>(&val)); }
            void operator()(const DirectX::XMFLOAT4X4& val) { cbv.set_float_matrix(4, 4, reinterpret_cast<const float*>(&val)); }
            void operator()(const std::vector<float>& val) { cbv.set_raw(val.data()); }
            void operator()(const std::vector<DirectX::XMFLOAT4>& val) { cbv.set_raw(val.data()); }
            void operator()(const std::vector<DirectX::XMFLOAT4X4>& val) { cbv.set_raw(val.data()); }
            void operator()(const std::string& val) {}
            ConstantBufferVariable& cbv;
        };

        std::string name;
        uint32_t start_byte_offset = 0;
        uint32_t byte_width = 0;
        CBufferData* p_CBufferData = nullptr;

        ConstantBufferVariable() = default;
        ~ConstantBufferVariable() override = default;

        ConstantBufferVariable(std::string_view name_, uint32_t offset, uint32_t size, CBufferData* p_data)
                : name(name_), start_byte_offset(offset), byte_width(size), p_CBufferData(p_data)
        {

        }

        void set_matrix_in_bytes(uint32_t rows, uint32_t cols, const uint8_t* no_pad_data)
        {
            // Only allow 1X1 to 4X4
            if (rows == 0 || rows > 4 || cols == 0 || cols > 4)
                return;
            uint32_t remain_bytes = byte_width < 64 ? byte_width : 64;
            uint8_t* pData = p_CBufferData->cbuffer_data.data() + start_byte_offset;
            while (remain_bytes > 0)
            {
                uint32_t rowPitch = sizeof(uint32_t) * cols < remain_bytes ? sizeof(uint32_t) * cols : remain_bytes;
                // Only update when the data is not equal
                if (memcmp(pData, no_pad_data, rowPitch))
                {
                    memcpy_s(pData, rowPitch, no_pad_data, rowPitch);
                    p_CBufferData->is_dirty = true;
                }
                no_pad_data += cols * sizeof(uint32_t);
                pData += 16;
                remain_bytes = remain_bytes < 16 ? 0 : (remain_bytes - 16);
            }
        }

        void set_uint(uint32_t val) override
        {
            set_raw(&val, 0, 4);
        }

        void set_sint(int32_t val) override
        {
            set_raw(&val, 0, 4);
        }

        void set_float(float val) override
        {
            set_raw(&val, 0, 4);
        }

        void set_uint_vector(uint32_t num_components, const uint32_t data[4]) override
        {
            if (num_components > 4)
                num_components = 4;
            uint32_t byte_count = num_components * sizeof(uint32_t);
            if (byte_count > byte_width)
                byte_count = byte_width;
            set_raw(data, 0, byte_count);
        }

        void set_sint_vector(uint32_t num_components, const int32_t data[4]) override
        {
            if (num_components > 4)
                num_components = 4;
            uint32_t byte_count = num_components * sizeof(int32_t);
            if (byte_count > byte_width)
                byte_count = byte_width;
            set_raw(data, 0, byte_count);
        }

        void set_float_vector(uint32_t num_components, const float data[4]) override
        {
            if (num_components > 4)
                num_components = 4;
            uint32_t byte_count = num_components * sizeof(float);
            if (byte_count > byte_width)
                byte_count = byte_width;
            set_raw(data, 0, byte_count);
        }

        void set_uint_matrix(uint32_t rows, uint32_t cols, const uint32_t* no_pad_data) override
        {
            set_matrix_in_bytes(rows, cols, reinterpret_cast<const uint8_t *>(no_pad_data));
        }

        void set_sint_matrix(uint32_t rows, uint32_t cols, const int* no_pad_data) override
        {
            set_matrix_in_bytes(rows, cols, reinterpret_cast<const uint8_t *>(no_pad_data));
        }

        void set_float_matrix(uint32_t rows, uint32_t cols, const float* no_pad_data) override
        {
            set_matrix_in_bytes(rows, cols, reinterpret_cast<const BYTE*>(no_pad_data));
        }

        void set_raw(const void* data, uint32_t byte_offset = 0, uint32_t byte_count = 0xFFFFFFFF) override
        {
            if (!data || byte_offset > byte_width)
                return;
            if (byte_offset + byte_count > byte_width)
                byte_count = byte_width - byte_offset;

            // Only update when the data is not equal
            if (memcmp(p_CBufferData->cbuffer_data.data() + start_byte_offset + byte_offset, data, byte_count))
            {
                memcpy_s(p_CBufferData->cbuffer_data.data() + start_byte_offset + byte_offset, byte_count, data, byte_count);
                p_CBufferData->is_dirty = true;
            }
        }

        HRESULT get_raw(void* p_output, uint32_t byte_offset = 0, uint32_t byte_count = 0xFFFFFFFF) override
        {
            if (byte_offset > byte_width || byte_count > byte_width - byte_offset)
            {
                return E_BOUNDS;
            }
            if (!p_output)
            {
                return E_INVALIDARG;
            }
            memcpy_s(p_output, byte_count, p_CBufferData->cbuffer_data.data() + start_byte_offset + byte_offset, byte_count);
            return S_OK;
        }
    };

    struct VertexShaderInfo
    {
        std::string name;
        com_ptr<ID3D11VertexShader> pVS;
        uint32_t cb_use_mask = 0;
        uint32_t ss_use_mask = 0;
        uint32_t unused = 0;
        uint32_t sr_use_masks[4] = {};
        std::unique_ptr<CBufferData> p_param_data = nullptr;
        std::unordered_map<size_t, std::shared_ptr<ConstantBufferVariable>> params;
    };

    struct DomainShaderInfo
    {
        std::string name;
        com_ptr<ID3D11DomainShader> pDS;
        uint32_t cb_use_mask = 0;
        uint32_t ss_use_mask = 0;
        uint32_t unused = 0;
        uint32_t sr_use_masks[4] = {};
        std::unique_ptr<CBufferData> p_param_data = nullptr;
        std::unordered_map<size_t, std::shared_ptr<ConstantBufferVariable>> params;
    };

    struct HullShaderInfo
    {
        std::string name;
        com_ptr<ID3D11HullShader> pHS;
        uint32_t cb_use_mask = 0;
        uint32_t ss_use_mask = 0;
        uint32_t unused = 0;
        uint32_t sr_use_masks[4] = {};
        std::unique_ptr<CBufferData> p_param_data = nullptr;
        std::unordered_map<size_t, std::shared_ptr<ConstantBufferVariable>> params;
    };

    struct GeometryShaderInfo
    {
        std::string name;
        com_ptr<ID3D11GeometryShader> pGS;
        uint32_t cb_use_mask = 0;
        uint32_t ss_use_mask = 0;
        uint32_t unused = 0;
        uint32_t sr_use_masks[4] = {};
        std::unique_ptr<CBufferData> p_param_data = nullptr;
        std::unordered_map<size_t, std::shared_ptr<ConstantBufferVariable>> params;
    };

    struct PixelShaderInfo
    {
        std::string name;
        com_ptr<ID3D11PixelShader> pPS;
        uint32_t cb_use_mask = 0;
        uint32_t ss_use_mask = 0;
        uint32_t rw_use_mask = 0;
        uint32_t sr_use_masks[4] = {};
        std::unique_ptr<CBufferData> p_param_data = nullptr;
        std::unordered_map<size_t, std::shared_ptr<ConstantBufferVariable>> params;
    };

    struct ComputeShaderInfo
    {
        std::string name;
        com_ptr<ID3D11ComputeShader> pCS;
        uint32_t cb_use_mask = 0;
        uint32_t ss_use_mask = 0;
        uint32_t rw_use_mask = 0;
        uint32_t sr_use_masks[4] = {};
        uint32_t thread_group_size_x = 0;
        uint32_t thread_group_size_y = 0;
        uint32_t thread_group_size_z = 0;
        std::unique_ptr<CBufferData> p_param_data = nullptr;
        std::unordered_map<size_t, std::shared_ptr<ConstantBufferVariable>> params;
    };
}











































