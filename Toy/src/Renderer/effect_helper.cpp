//
// Created by ZZK on 2023/5/30.
//

#include <Toy/Renderer/effect_helper.h>
#include <Toy/Core/d3d_util.h>

namespace toy
{
#define EFFECTHELPER_CREATE_SHADER(FullShaderType, ShaderType)\
{\
    m_##FullShaderType##s[nameID] = std::make_shared<FullShaderType##Info>();\
    m_##FullShaderType##s[nameID]->name = name;\
    hr = device->Create##FullShaderType(blob->GetBufferPointer(), blob->GetBufferSize(),\
        nullptr, m_##FullShaderType##s[nameID]->p##ShaderType.GetAddressOf());\
    break;\
}

#define EFFECTHELPER_EFFECTPASS_SET_SHADER_AND_PARAM(FullShaderType, ShaderType) \
{\
    if (!p_effect_desc->name##ShaderType.empty())\
    {\
        auto it = p_impl_->m_##FullShaderType##s.find(string_to_id(p_effect_desc->name##ShaderType));\
        if (it != p_impl_->m_##FullShaderType##s.end())\
        {\
            pEffectPass->p##ShaderType##Info = it->second;\
            auto& pCBData = it->second->p_param_data;\
            if (pCBData)\
            {\
                pEffectPass->p##ShaderType##ParamData = std::make_unique<CBufferData>(pCBData->cbuffer_name.c_str(), pCBData->start_slot, (uint32_t)pCBData->cbuffer_data.size()); \
                it->second->p_param_data->create_buffer(device);\
            }\
        }\
        else\
            return E_INVALIDARG;\
    }\
}

#define EFFECTPASS_SET_SHADER(ShaderType)\
{\
    deviceContext->ShaderType##SetShader(p##ShaderType##Info->p##ShaderType.Get(), nullptr, 0);\
}

#define EFFECTPASS_SET_CONSTANTBUFFER(ShaderType)\
{\
    uint32_t slot = 0, mask = p##ShaderType##Info->cb_use_mask;\
    while (mask) {\
        if ((mask & 1) == 0) {\
            ++slot, mask >>= 1;\
            continue;\
        }\
        uint32_t zero_bit = ((mask + 1) | mask) ^ mask;\
        uint32_t count = (zero_bit == 0 ? 32 : (uint32_t)log2((double)zero_bit));\
        if (count == 1) {\
            CBufferData& cbData = cBuffers.at(slot);\
            cbData.update_buffer(deviceContext);\
            deviceContext->ShaderType##SetConstantBuffers(slot, 1, cbData.cbuffer.GetAddressOf());\
            ++slot, mask >>= 1;\
        }\
        else {\
            std::vector<ID3D11Buffer*> constantBuffers(count);\
            for (uint32_t i = 0; i < count; ++i) {\
                CBufferData& cbData = cBuffers.at(slot + i);\
                cbData.update_buffer(deviceContext);\
                constantBuffers[i] = cbData.cbuffer.Get();\
            }\
            deviceContext->ShaderType##SetConstantBuffers(slot, count, constantBuffers.data());\
            slot += count + 1, mask >>= (count + 1);\
        }\
    }\
}                                                \

#define EFFECTPASS_SET_PARAM(ShaderType)\
{\
    if (!p##ShaderType##Info->params.empty())\
    {\
        if (p##ShaderType##ParamData->is_dirty)\
        {\
            p##ShaderType##ParamData->is_dirty = false;\
            p##ShaderType##Info->p_param_data->is_dirty = true;\
            memcpy_s(p##ShaderType##Info->p_param_data->cbuffer_data.data(), p##ShaderType##ParamData->cbuffer_data.size(),\
                p##ShaderType##ParamData->cbuffer_data.data(), p##ShaderType##ParamData->cbuffer_data.size());\
            p##ShaderType##Info->p_param_data->update_buffer(deviceContext);\
        }\
        deviceContext->ShaderType##SetConstantBuffers(p##ShaderType##Info->p_param_data->start_slot,\
            1, p##ShaderType##Info->p_param_data->cbuffer.GetAddressOf());\
    }\
}

#define EFFECTPASS_SET_SAMPLER(ShaderType)\
{\
    uint32_t slot = 0, mask = p##ShaderType##Info->ss_use_mask;\
    while (mask) {\
        if ((mask & 1) == 0) {\
            ++slot, mask >>= 1;\
            continue;\
        }\
        uint32_t zero_bit = ((mask + 1) | mask) ^ mask;\
        uint32_t count = (zero_bit == 0 ? 32 : (uint32_t)log2((double)zero_bit));\
        if (count == 1) {\
            deviceContext->ShaderType##SetSamplers(slot, 1, samplers.at(slot).ss.GetAddressOf());\
            ++slot, mask >>= 1;\
        }\
        else {\
            std::vector<ID3D11SamplerState*> samplerStates(count);\
            for (uint32_t i = 0; i < count; ++i)\
                samplerStates[i] = samplers.at(slot + i).ss.Get();\
            deviceContext->ShaderType##SetSamplers(slot, count, samplerStates.data()); \
            slot += count + 1, mask >>= (count + 1);\
        }\
    }\
}                                         \

#define EFFECTPASS_SET_SHADERRESOURCE(ShaderType)\
{\
    uint32_t slot_shader_res = 0;\
    for (uint32_t i = 0; i < 4; ++i, slot_shader_res = i * 32){\
        uint32_t mask = p##ShaderType##Info->sr_use_masks[i];\
        while (mask) {\
            if ((mask & 1) == 0) {\
                ++slot_shader_res, mask >>= 1; \
                continue; \
            }\
            uint32_t zero_bit = ((mask + 1) | mask) ^ mask; \
            uint32_t count = (zero_bit == 0 ? 32 : (uint32_t)log2((double)zero_bit)); \
            if (count == 1) {\
                deviceContext->ShaderType##SetShaderResources(slot_shader_res, 1, shaderResources.at(slot_shader_res).srv.GetAddressOf()); \
                ++slot_shader_res, mask >>= 1; \
            }\
            else {\
                std::vector<ID3D11ShaderResourceView*> srvs(count); \
                for (uint32_t i = 0; i < count; ++i)\
                    srvs[i] = shaderResources.at(slot_shader_res + i).srv.Get(); \
                deviceContext->ShaderType##SetShaderResources(slot_shader_res, count, srvs.data()); \
                slot_shader_res += count + 1, mask >>= (count + 1); \
            }\
        }\
    }\
}\

    // Effect helper implementation
    HRESULT effect_helper_c::impl_c::update_shader_reflection(std::string_view name, ID3D11Device *device,
                                                                ID3D11ShaderReflection *p_shader_reflection,
                                                                uint32_t shader_flag)
    {
        HRESULT hr;

        D3D11_SHADER_DESC sd;
        hr = p_shader_reflection->GetDesc(&sd);
        if (FAILED(hr))
            return hr;

        size_t nameID = string_to_id(name);

        if (shader_flag == ComputeShader)
        {
            // 获取线程组维度
            p_shader_reflection->GetThreadGroupSize(
                    &m_ComputeShaders[nameID]->thread_group_size_x,
                    &m_ComputeShaders[nameID]->thread_group_size_y,
                    &m_ComputeShaders[nameID]->thread_group_size_z);
        }

        for (uint32_t i = 0; ; ++i)
        {
            D3D11_SHADER_INPUT_BIND_DESC sibDesc;
            hr = p_shader_reflection->GetResourceBindingDesc(i, &sibDesc);
            // 读取完变量后会失败，但这并不是失败的调用
            if (FAILED(hr))
                break;

            // 常量缓冲区
            if (sibDesc.Type == D3D_SIT_CBUFFER)
            {
                ID3D11ShaderReflectionConstantBuffer* pSRCBuffer = p_shader_reflection->GetConstantBufferByName(sibDesc.Name);
                // 获取cbuffer内的变量信息并建立映射
                D3D11_SHADER_BUFFER_DESC cbDesc{};
                hr = pSRCBuffer->GetDesc(&cbDesc);
                if (FAILED(hr))
                    return hr;

                bool isParam = !strcmp(sibDesc.Name, "$Params");

                // 确定常量缓冲区的创建位置
                if (!isParam)
                {
                    auto it = m_CBuffers.find(sibDesc.BindPoint);
                    if (it == m_CBuffers.end())
                    {
                        m_CBuffers.emplace(std::make_pair(sibDesc.BindPoint, CBufferData(sibDesc.Name, sibDesc.BindPoint, cbDesc.Size, nullptr)));
                        m_CBuffers[sibDesc.BindPoint].create_buffer(device);
                    }
                    // 存在不同shader间的cbuffer大小不一致的情况，应当以最大的为准
                    // 例如当前shader通过宏开启了cbuffer最后一个变量导致多一个16 bytes，而另一个shader关闭了该变量
                    else if (it->second.cbuffer_data.size() < cbDesc.Size)
                    {
                        m_CBuffers[sibDesc.BindPoint] = CBufferData(sibDesc.Name, sibDesc.BindPoint, cbDesc.Size, nullptr);
                        m_CBuffers[sibDesc.BindPoint].create_buffer(device);
                    }

                    // 标记该着色器使用了当前常量缓冲区
                    if (cbDesc.Variables > 0)
                    {
                        switch (shader_flag)
                        {
                            case VertexShader: m_VertexShaders[nameID]->cb_use_mask |= (1 << sibDesc.BindPoint); break;
                            case DomainShader: m_DomainShaders[nameID]->cb_use_mask |= (1 << sibDesc.BindPoint); break;
                            case HullShader: m_HullShaders[nameID]->cb_use_mask |= (1 << sibDesc.BindPoint); break;
                            case GeometryShader: m_GeometryShaders[nameID]->cb_use_mask |= (1 << sibDesc.BindPoint); break;
                            case PixelShader: m_PixelShaders[nameID]->cb_use_mask |= (1 << sibDesc.BindPoint); break;
                            case ComputeShader: m_ComputeShaders[nameID]->cb_use_mask |= (1 << sibDesc.BindPoint); break;
                        }
                    }
                }
                else if (cbDesc.Variables > 0)
                {
                    switch (shader_flag)
                    {
                        case VertexShader: m_VertexShaders[nameID]->p_param_data = std::make_unique<CBufferData>(sibDesc.Name, sibDesc.BindPoint, cbDesc.Size, nullptr); break;
                        case DomainShader: m_DomainShaders[nameID]->p_param_data = std::make_unique<CBufferData>(sibDesc.Name, sibDesc.BindPoint, cbDesc.Size, nullptr); break;
                        case HullShader: m_HullShaders[nameID]->p_param_data = std::make_unique<CBufferData>(sibDesc.Name, sibDesc.BindPoint, cbDesc.Size, nullptr); break;
                        case GeometryShader: m_GeometryShaders[nameID]->p_param_data = std::make_unique<CBufferData>(sibDesc.Name, sibDesc.BindPoint, cbDesc.Size, nullptr); break;
                        case PixelShader: m_PixelShaders[nameID]->p_param_data = std::make_unique<CBufferData>(sibDesc.Name, sibDesc.BindPoint, cbDesc.Size, nullptr); break;
                        case ComputeShader: m_ComputeShaders[nameID]->p_param_data = std::make_unique<CBufferData>(sibDesc.Name, sibDesc.BindPoint, cbDesc.Size, nullptr); break;
                    }
                }

                // 记录内部变量
                for (uint32_t j = 0; j < cbDesc.Variables; ++j)
                {
                    ID3D11ShaderReflectionVariable* pSRVar = pSRCBuffer->GetVariableByIndex(j);
                    D3D11_SHADER_VARIABLE_DESC svDesc;
                    hr = pSRVar->GetDesc(&svDesc);
                    if (FAILED(hr))
                        return hr;

                    size_t svNameID = string_to_id(svDesc.Name);
                    // 着色器形参需要特殊对待
                    // 记录着色器的uniform形参
                    // **忽略着色器形参默认值**
                    if (isParam)
                    {
                        switch (shader_flag)
                        {
                            case VertexShader: m_VertexShaders[nameID]->params[svNameID] =
                                                std::make_shared<ConstantBufferVariable>(svDesc.Name, svDesc.StartOffset, svDesc.Size, m_VertexShaders[nameID]->p_param_data.get());
                                break;
                            case DomainShader: m_DomainShaders[nameID]->params[svNameID] =
                                                std::make_shared<ConstantBufferVariable>(svDesc.Name, svDesc.StartOffset, svDesc.Size, m_DomainShaders[nameID]->p_param_data.get());
                                break;
                            case HullShader: m_HullShaders[nameID]->params[svNameID] =
                                                std::make_shared<ConstantBufferVariable>(svDesc.Name, svDesc.StartOffset, svDesc.Size, m_HullShaders[nameID]->p_param_data.get());
                                break;
                            case GeometryShader: m_GeometryShaders[nameID]->params[svNameID] =
                                                std::make_shared<ConstantBufferVariable>(svDesc.Name, svDesc.StartOffset, svDesc.Size, m_GeometryShaders[nameID]->p_param_data.get());
                                break;
                            case PixelShader: m_PixelShaders[nameID]->params[svNameID] =
                                                std::make_shared<ConstantBufferVariable>(svDesc.Name, svDesc.StartOffset, svDesc.Size, m_PixelShaders[nameID]->p_param_data.get());
                                break;
                            case ComputeShader: m_ComputeShaders[nameID]->params[svNameID] =
                                                std::make_shared<ConstantBufferVariable>(svDesc.Name, svDesc.StartOffset, svDesc.Size, m_ComputeShaders[nameID]->p_param_data.get());
                                break;
                        }
                    }
                    // 常量缓冲区的成员
                    else
                    {
                        m_ConstantBufferVariables[svNameID] = std::make_shared<ConstantBufferVariable>(
                                svDesc.Name, svDesc.StartOffset, svDesc.Size, &m_CBuffers[sibDesc.BindPoint]);
                        // 如果有默认值，对其赋初值
                        if (svDesc.DefaultValue)
                            m_ConstantBufferVariables[svNameID]->set_raw(svDesc.DefaultValue);
                    }
                }
            }
            // 着色器资源
            else if (sibDesc.Type == D3D_SIT_TEXTURE || sibDesc.Type == D3D_SIT_STRUCTURED || sibDesc.Type == D3D_SIT_BYTEADDRESS ||
                        sibDesc.Type == D3D_SIT_TBUFFER)
            {
                auto it = m_ShaderResources.find(sibDesc.BindPoint);
                if (it == m_ShaderResources.end())
                {
                    m_ShaderResources.emplace(std::make_pair(sibDesc.BindPoint,
                                                    ShaderResource{ sibDesc.Name, sibDesc.Dimension, nullptr }));
                }

                // 标记该着色器使用了当前着色器资源
                switch (shader_flag)
                {
                    case VertexShader: m_VertexShaders[nameID]->sr_use_masks[sibDesc.BindPoint / 32] |= (1 << (sibDesc.BindPoint % 32)); break;
                    case DomainShader: m_DomainShaders[nameID]->sr_use_masks[sibDesc.BindPoint / 32] |= (1 << (sibDesc.BindPoint % 32)); break;
                    case HullShader: m_HullShaders[nameID]->sr_use_masks[sibDesc.BindPoint / 32] |= (1 << (sibDesc.BindPoint % 32)); break;
                    case GeometryShader: m_GeometryShaders[nameID]->sr_use_masks[sibDesc.BindPoint / 32] |= (1 << (sibDesc.BindPoint % 32)); break;
                    case PixelShader: m_PixelShaders[nameID]->sr_use_masks[sibDesc.BindPoint / 32] |= (1 << (sibDesc.BindPoint % 32)); break;
                    case ComputeShader: m_ComputeShaders[nameID]->sr_use_masks[sibDesc.BindPoint / 32] |= (1 << (sibDesc.BindPoint % 32)); break;
                }

            }
            // 采样器
            else if (sibDesc.Type == D3D_SIT_SAMPLER)
            {
                auto it = m_Samplers.find(sibDesc.BindPoint);
                if (it == m_Samplers.end())
                {
                    m_Samplers.emplace(std::make_pair(sibDesc.BindPoint,
                                            SamplerState{ sibDesc.Name, nullptr }));
                }

                // 标记该着色器使用了当前采样器
                switch (shader_flag)
                {
                    case VertexShader: m_VertexShaders[nameID]->ss_use_mask |= (1 << sibDesc.BindPoint); break;
                    case DomainShader: m_DomainShaders[nameID]->ss_use_mask |= (1 << sibDesc.BindPoint); break;
                    case HullShader: m_HullShaders[nameID]->ss_use_mask |= (1 << sibDesc.BindPoint); break;
                    case GeometryShader: m_GeometryShaders[nameID]->ss_use_mask |= (1 << sibDesc.BindPoint); break;
                    case PixelShader: m_PixelShaders[nameID]->ss_use_mask |= (1 << sibDesc.BindPoint); break;
                    case ComputeShader: m_ComputeShaders[nameID]->ss_use_mask |= (1 << sibDesc.BindPoint); break;
                }

            }
            // 可读写资源
            else if (sibDesc.Type == D3D_SIT_UAV_RWTYPED || sibDesc.Type == D3D_SIT_UAV_RWSTRUCTURED ||
                        sibDesc.Type == D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER || sibDesc.Type == D3D_SIT_UAV_APPEND_STRUCTURED ||
                        sibDesc.Type == D3D_SIT_UAV_CONSUME_STRUCTURED || sibDesc.Type == D3D_SIT_UAV_RWBYTEADDRESS)
            {
                auto it = m_RWResources.find(sibDesc.BindPoint);
                if (it == m_RWResources.end())
                {
                    m_RWResources.emplace(std::make_pair(sibDesc.BindPoint,
                                            RWResource{ sibDesc.Name, static_cast<D3D11_UAV_DIMENSION>(sibDesc.Dimension), nullptr, 0,
                                                sibDesc.Type == D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER, false }));
                }

                // 标记该着色器使用了当前可读写资源
                switch (shader_flag)
                {
                    case PixelShader: m_PixelShaders[nameID]->rw_use_mask |= (1 << sibDesc.BindPoint); break;
                    case ComputeShader: m_ComputeShaders[nameID]->rw_use_mask |= (1 << sibDesc.BindPoint); break;
                }
            }
        }

        return S_OK;
    }

    void effect_helper_c::impl_c::clear()
    {
        m_CBuffers.clear();

        m_ConstantBufferVariables.clear();
        m_ShaderResources.clear();
        m_Samplers.clear();
        m_RWResources.clear();

        m_VertexShaders.clear();
        m_HullShaders.clear();
        m_DomainShaders.clear();
        m_GeometryShaders.clear();
        m_PixelShaders.clear();
        m_ComputeShaders.clear();
    }

    HRESULT effect_helper_c::impl_c::create_shader_from_blob(std::string_view name, ID3D11Device* device, uint32_t shader_flag, ID3DBlob* blob)
    {
        HRESULT hr = 0;
        com_ptr<ID3D11VertexShader> pVS;
        com_ptr<ID3D11DomainShader> pDS;
        com_ptr<ID3D11HullShader> pHS;
        com_ptr<ID3D11GeometryShader> pGS;
        com_ptr<ID3D11PixelShader> pPS;
        com_ptr<ID3D11ComputeShader> pCS;
        // 创建着色器
        size_t nameID = string_to_id(name);
        switch (shader_flag)
        {
            case PixelShader: EFFECTHELPER_CREATE_SHADER(PixelShader, PS);
            case VertexShader: EFFECTHELPER_CREATE_SHADER(VertexShader, VS);
            case GeometryShader: EFFECTHELPER_CREATE_SHADER(GeometryShader, GS);
            case HullShader: EFFECTHELPER_CREATE_SHADER(HullShader, HS);
            case DomainShader: EFFECTHELPER_CREATE_SHADER(DomainShader, DS);
            case ComputeShader: EFFECTHELPER_CREATE_SHADER(ComputeShader, CS);
        }

        return hr;
    }

    // Effect helper
    effect_helper_c::effect_helper_c()
    : p_impl_(std::make_unique<effect_helper_c::impl_c>())
    {

    }

    HRESULT effect_helper_c::add_shader(std::string_view name, ID3D11Device *device, ID3DBlob *blob)
    {
        if (name.empty() || device == nullptr || blob == nullptr)
            return E_INVALIDARG;

        HRESULT hr;
        // 着色器反射
        com_ptr<ID3D11ShaderReflection> pShaderReflection;
        hr = D3DReflect(blob->GetBufferPointer(), blob->GetBufferSize(), __uuidof(ID3D11ShaderReflection),
                        reinterpret_cast<void**>(pShaderReflection.GetAddressOf()));
        if (FAILED(hr))
            return hr;

        // 获取着色器类型
        D3D11_SHADER_DESC sd;
        pShaderReflection->GetDesc(&sd);
        uint32_t shaderFlag = static_cast<ShaderFlag>(1 << D3D11_SHVER_GET_TYPE(sd.Version));

        // 创建着色器
        hr = p_impl_->create_shader_from_blob(name, device, shaderFlag, blob);
        if (FAILED(hr))
            return hr;

        // 建立着色器反射
        return p_impl_->update_shader_reflection(name, device, pShaderReflection.Get(), shaderFlag);
    }

    void effect_helper_c::set_binary_cache_directory(std::wstring_view cache_dir, bool force_write)
    {
        p_impl_->m_cache_dir = cache_dir;
        p_impl_->m_force_write = force_write;
        if (!p_impl_->m_cache_dir.empty())
        {
            CreateDirectoryW(cache_dir.data(), nullptr);
        }
    }

    HRESULT effect_helper_c::create_shader_from_file(std::string_view shader_name, std::wstring_view file_name,
                                                        ID3D11Device *device, const char *entry_point,
                                                        const char *shader_model, const D3D_SHADER_MACRO *p_defines,
                                                        ID3DBlob **pp_shader_byte_code)
    {
        ID3DBlob* pBlobIn = nullptr;
        ID3DBlob* pBlobOut = nullptr;
        // 如果开启着色器字节码文件缓存路径 且 关闭强制覆盖，则优先尝试读取 ${cacheDir}/${shaderName}.cso 并添加
        if (!p_impl_->m_cache_dir.empty() && !p_impl_->m_force_write)
        {
            std::filesystem::path cacheFilename = p_impl_->m_cache_dir / (utf8_to_wstring(shader_name) + L".cso");
            std::wstring wstr = cacheFilename.generic_wstring();
            HRESULT hr = D3DReadFileToBlob(wstr.c_str(), &pBlobOut);
            if (SUCCEEDED(hr))
            {
                hr = add_shader(shader_name, device, pBlobOut);

                if (pp_shader_byte_code)
                    *pp_shader_byte_code = pBlobOut;
                else
                    pBlobOut->Release();

                return hr;
            }
        }

        // 如果没有开启或没有缓存，则读取filename。若为着色器字节码，直接添加
        // 编译好的DXBC文件头
        static char dxbc_header[] = { 'D', 'X', 'B', 'C' };

        HRESULT hr = D3DReadFileToBlob(file_name.data(), &pBlobIn);
        if (FAILED(hr))
            return hr;
        if (memcmp(pBlobIn->GetBufferPointer(), dxbc_header, sizeof(dxbc_header)))
        {
            // 若filename为hlsl源码，则进行编译和添加。开启着色器字节码文件缓存会保存着色器字节码到${cacheDir}/${shaderName}.cso

            uint32_t dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
            // 设置 D3DCOMPILE_DEBUG 标志用于获取着色器调试信息。该标志可以提升调试体验，
            // 但仍然允许着色器进行优化操作
            dwShaderFlags |= D3DCOMPILE_DEBUG;

            // 在Debug环境下禁用优化以避免出现一些不合理的情况
            dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
            ID3DBlob* errorBlob = nullptr;
            std::string filenameu8str = wstring_to_utf8(file_name);
            hr = D3DCompile(pBlobIn->GetBufferPointer(), pBlobIn->GetBufferSize(), filenameu8str.c_str(),
                            p_defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, entry_point, shader_model,
                            dwShaderFlags, 0, &pBlobOut, &errorBlob);
            pBlobIn->Release();

            if (FAILED(hr))
            {
                if (errorBlob != nullptr)
                {
                    OutputDebugStringA(reinterpret_cast<const char*>(errorBlob->GetBufferPointer()));
                    DX_CORE_ERROR("Fail to compile shader file, {0}", reinterpret_cast<const char*>(errorBlob->GetBufferPointer()));
                    errorBlob->Release();
                }
                return hr;
            }

            if (!p_impl_->m_cache_dir.empty())
            {
                std::filesystem::path cacheFilename = p_impl_->m_cache_dir / (utf8_to_wstring(shader_name) + L".cso");
                std::wstring wstr = cacheFilename.generic_wstring();
                D3DWriteBlobToFile(pBlobOut, wstr.c_str(), p_impl_->m_force_write);
            }
        }
        else
        {
            std::swap(pBlobIn, pBlobOut);
        }

        hr = add_shader(shader_name, device, pBlobOut);

        if (pp_shader_byte_code)
            *pp_shader_byte_code = pBlobOut;
        else
            pBlobOut->Release();

        return hr;
    }

    HRESULT effect_helper_c::compile_shader_from_file(std::wstring_view filename, const char *entryPoint,
                                                        const char *shaderModel, ID3DBlob **ppShaderByteCode,
                                                        ID3DBlob **ppErrorBlob, const D3D_SHADER_MACRO *pDefines,
                                                        ID3DInclude *pInclude)
    {
        uint32_t dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
        // 设置 D3DCOMPILE_DEBUG 标志用于获取着色器调试信息。该标志可以提升调试体验，
        // 但仍然允许着色器进行优化操作
        dwShaderFlags |= D3DCOMPILE_DEBUG;

        // 在Debug环境下禁用优化以避免出现一些不合理的情况
        dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
        return D3DCompileFromFile(filename.data(), pDefines, pInclude, entryPoint, shaderModel,
                                    dwShaderFlags, 0, ppShaderByteCode, ppErrorBlob);
    }

    HRESULT effect_helper_c::add_geometry_shader_with_stream_output(std::string_view name, ID3D11Device *device,
                                                                    ID3D11GeometryShader *gs_with_so, ID3DBlob *blob)
    {
        if (name.empty() || !gs_with_so)
            return E_INVALIDARG;

        HRESULT hr;

        // 着色器反射
        com_ptr<ID3D11ShaderReflection> pShaderReflection;
        hr = D3DReflect(blob->GetBufferPointer(), blob->GetBufferSize(), __uuidof(ID3D11ShaderReflection),
                        reinterpret_cast<void**>(pShaderReflection.GetAddressOf()));
        if (FAILED(hr))
            return hr;

        // 获取着色器类型并核验
        D3D11_SHADER_DESC sd;
        pShaderReflection->GetDesc(&sd);
        uint32_t shaderFlag = static_cast<ShaderFlag>(1 << D3D11_SHVER_GET_TYPE(sd.Version));

        if (shaderFlag != GeometryShader)
            return E_INVALIDARG;

        size_t nameID = string_to_id(name);
        p_impl_->m_GeometryShaders[nameID] = std::make_shared<GeometryShaderInfo>();
        p_impl_->m_GeometryShaders[nameID]->pGS = gs_with_so;

        // 建立着色器反射
        return p_impl_->update_shader_reflection(name, device, pShaderReflection.Get(), shaderFlag);
    }

    void effect_helper_c::clear()
    {
        p_impl_->clear();
    }

    HRESULT effect_helper_c::add_effect_pass(std::string_view effect_pass_name, ID3D11Device *device,
                                                const toy::EffectPassDesc *p_effect_desc)
    {
        if (!p_effect_desc || effect_pass_name.empty())
            return E_INVALIDARG;

        size_t effectPassID = string_to_id(effect_pass_name);

        // 不允许重复添加
        auto iter = p_impl_->m_EffectPasses.find(effectPassID);
        if (iter != p_impl_->m_EffectPasses.end())
            return ERROR_OBJECT_NAME_EXISTS;

        auto& pEffectPass = p_impl_->m_EffectPasses[effectPassID] =
                std::make_shared<EffectPass>(this, effect_pass_name, p_impl_->m_CBuffers, p_impl_->m_ShaderResources, p_impl_->m_Samplers, p_impl_->m_RWResources);

        EFFECTHELPER_EFFECTPASS_SET_SHADER_AND_PARAM(VertexShader, VS);
        EFFECTHELPER_EFFECTPASS_SET_SHADER_AND_PARAM(DomainShader, DS);
        EFFECTHELPER_EFFECTPASS_SET_SHADER_AND_PARAM(HullShader, HS);
        EFFECTHELPER_EFFECTPASS_SET_SHADER_AND_PARAM(GeometryShader, GS);
        EFFECTHELPER_EFFECTPASS_SET_SHADER_AND_PARAM(PixelShader, PS);
        EFFECTHELPER_EFFECTPASS_SET_SHADER_AND_PARAM(ComputeShader, CS);

        return S_OK;
    }

    std::shared_ptr<IEffectPass> effect_helper_c::get_effect_pass(std::string_view effect_pass_name)
    {
        auto it = p_impl_->m_EffectPasses.find(string_to_id(effect_pass_name));
        if (it != p_impl_->m_EffectPasses.end())
            return it->second;
        return nullptr;
    }

    std::shared_ptr<IEffectConstantBufferVariable> effect_helper_c::get_constant_buffer_variable(std::string_view name)
    {
        auto it = p_impl_->m_ConstantBufferVariables.find(string_to_id(name));
        if (it != p_impl_->m_ConstantBufferVariables.end())
            return it->second;
        else
            return nullptr;
    }

    void effect_helper_c::set_sampler_state_by_slot(uint32_t slot, ID3D11SamplerState *sampler_state)
    {
        auto it = p_impl_->m_Samplers.find(slot);
        if (it != p_impl_->m_Samplers.end())
            it->second.ss = sampler_state;
    }

    void effect_helper_c::set_sampler_state_by_name(std::string_view name, ID3D11SamplerState *sampler_state)
    {
        auto it = std::find_if(p_impl_->m_Samplers.begin(), p_impl_->m_Samplers.end(),
        [name](const std::pair<uint32_t, SamplerState>& p) {
            return p.second.name == name;
        });
        if (it != p_impl_->m_Samplers.end())
            it->second.ss = sampler_state;
    }

    int32_t effect_helper_c::map_sampler_state_slot(std::string_view name)
    {
        auto it = std::find_if(p_impl_->m_Samplers.begin(), p_impl_->m_Samplers.end(),
        [name](const std::pair<uint32_t, SamplerState>& p) {
            return p.second.name == name;
        });
        if (it != p_impl_->m_Samplers.end())
            return static_cast<int32_t>(it->first);
        return -1;
    }

    void effect_helper_c::set_shader_resource_by_slot(uint32_t slot, ID3D11ShaderResourceView *srv)
    {
        auto it = p_impl_->m_ShaderResources.find(slot);
        if (it != p_impl_->m_ShaderResources.end())
            it->second.srv = srv;
    }

    void effect_helper_c::set_shader_resource_by_name(std::string_view name, ID3D11ShaderResourceView *srv)
    {
        auto it = std::find_if(p_impl_->m_ShaderResources.begin(), p_impl_->m_ShaderResources.end(),
        [name](const std::pair<uint32_t, ShaderResource>& p) {
            return p.second.name == name;
        });
        if (it != p_impl_->m_ShaderResources.end())
            it->second.srv = srv;
    }

    int32_t effect_helper_c::map_shader_resource_slot(std::string_view name)
    {
        auto it = std::find_if(p_impl_->m_ShaderResources.begin(), p_impl_->m_ShaderResources.end(),
        [name](const std::pair<uint32_t, ShaderResource>& p) {
            return p.second.name == name;
        });
        if (it != p_impl_->m_ShaderResources.end())
            return static_cast<int32_t>(it->first);
        return -1;
    }

    void effect_helper_c::set_unordered_access_by_slot(uint32_t slot, ID3D11UnorderedAccessView *uav,
                                                        uint32_t *p_initial_count)
    {
        auto it = p_impl_->m_RWResources.find(slot);
        if (it != p_impl_->m_RWResources.end())
        {
            it->second.uav = uav;
            if (p_initial_count)
            {
                it->second.initial_count = *p_initial_count;
                it->second.first_init = true;
            }
        }
    }

    void effect_helper_c::set_unordered_access_by_name(std::string_view name, ID3D11UnorderedAccessView *uav,
                                                        uint32_t *p_initial_count)
    {
        auto it = std::find_if(p_impl_->m_RWResources.begin(), p_impl_->m_RWResources.end(),
        [name](const std::pair<uint32_t, RWResource>& p) {
            return p.second.name == name;
        });
        if (it != p_impl_->m_RWResources.end())
        {
            it->second.uav = uav;
            if (p_initial_count)
            {
                it->second.initial_count = *p_initial_count;
                it->second.first_init = true;
            }
        }
    }

    int32_t effect_helper_c::map_unordered_access_slot(std::string_view name)
    {
        auto it = std::find_if(p_impl_->m_RWResources.begin(), p_impl_->m_RWResources.end(),
        [name](const std::pair<uint32_t, RWResource>& p) {
            return p.second.name == name;
        });
        if (it != p_impl_->m_RWResources.end())
            return static_cast<int32_t>(it->first);
        return -1;
    }

    // TODO: set debug object name

    // Effect pass
    void EffectPass::set_rasterizer_state(ID3D11RasterizerState *pRS)
    {
        pRasterizerState = pRS;
    }

    void EffectPass::set_blend_state(ID3D11BlendState *pBS, const float *blendFactor, uint32_t sampleMask)
    {
        pBlendState = pBS;
        if (blendFactor)
        {
            memcpy_s(this->blendFactor, sizeof(float[4]), blendFactor, sizeof(float[4]));
        }
        this->sampleMask = sampleMask;
    }

    void EffectPass::set_depth_stencil_state(ID3D11DepthStencilState *pDSS, uint32_t stencilRef)
    {
        pDepthStencilState = pDSS;
        this->stencilRef = stencilRef;
    }

    std::shared_ptr<IEffectConstantBufferVariable> EffectPass::get_vs_param_by_name(std::string_view paramName)
    {
        if (pVSInfo)
        {
            auto it = pVSInfo->params.find(string_to_id(paramName));
            if (it != pVSInfo->params.end())
                return std::make_shared<ConstantBufferVariable>(paramName, it->second->start_byte_offset,
                                                                    it->second->byte_width, pVSParamData.get());
        }
        return nullptr;
    }

    std::shared_ptr<IEffectConstantBufferVariable> EffectPass::get_ds_param_by_name(std::string_view paramName)
    {
        if (pDSInfo)
        {
            auto it = pDSInfo->params.find(string_to_id(paramName));
            if (it != pDSInfo->params.end())
                return std::make_shared<ConstantBufferVariable>(paramName, it->second->start_byte_offset,
                                                                    it->second->byte_width, pDSParamData.get());
        }
        return nullptr;
    }

    std::shared_ptr<IEffectConstantBufferVariable> EffectPass::get_hs_param_by_name(std::string_view paramName)
    {
        if (pHSInfo)
        {
            auto it = pHSInfo->params.find(string_to_id(paramName));
            if (it != pHSInfo->params.end())
                return std::make_shared<ConstantBufferVariable>(paramName, it->second->start_byte_offset,
                                                                    it->second->byte_width, pHSParamData.get());
        }
        return nullptr;
    }

    std::shared_ptr<IEffectConstantBufferVariable> EffectPass::get_gs_param_by_name(std::string_view paramName)
    {
        if (pGSInfo)
        {
            auto it = pGSInfo->params.find(string_to_id(paramName));
            if (it != pGSInfo->params.end())
                return std::make_shared<ConstantBufferVariable>(paramName, it->second->start_byte_offset,
                                                                    it->second->byte_width, pGSParamData.get());
        }
        return nullptr;
    }

    std::shared_ptr<IEffectConstantBufferVariable> EffectPass::get_ps_param_by_name(std::string_view paramName)
    {
        if (pPSInfo)
        {
            auto it = pPSInfo->params.find(string_to_id(paramName));
            if (it != pPSInfo->params.end())
                return std::make_shared<ConstantBufferVariable>(paramName, it->second->start_byte_offset,
                                                                    it->second->byte_width, pPSParamData.get());
        }
        return nullptr;
    }

    std::shared_ptr<IEffectConstantBufferVariable> EffectPass::get_cs_param_by_name(std::string_view paramName)
    {
        if (pCSInfo)
        {
            auto it = pCSInfo->params.find(string_to_id(paramName));
            if (it != pCSInfo->params.end())
                return std::make_shared<ConstantBufferVariable>(paramName, it->second->start_byte_offset,
                                                                    it->second->byte_width, pCSParamData.get());
        }
        return nullptr;
    }

    effect_helper_c* EffectPass::get_effect_helper()
    {
        return pEffectHelper;
    }

    const std::string& EffectPass::get_pass_name()
    {
        return passName;
    }

    void EffectPass::apply(ID3D11DeviceContext *deviceContext)
    {
        // Set shader, constant buffers, sampler, shader resource views, readable and writable resources
        if (pVSInfo)
        {
            EFFECTPASS_SET_SHADER(VS);
            EFFECTPASS_SET_CONSTANTBUFFER(VS);
            EFFECTPASS_SET_PARAM(VS);
            EFFECTPASS_SET_SAMPLER(VS);
            EFFECTPASS_SET_SHADERRESOURCE(VS);
        } else
        {
            deviceContext->VSSetShader(nullptr, nullptr, 0);
        }

        if (pDSInfo)
        {
            EFFECTPASS_SET_SHADER(DS);
            EFFECTPASS_SET_CONSTANTBUFFER(DS);
            EFFECTPASS_SET_PARAM(DS);
            EFFECTPASS_SET_SAMPLER(DS);
            EFFECTPASS_SET_SHADERRESOURCE(DS);
        } else
        {
            deviceContext->DSSetShader(nullptr, nullptr, 0);
        }

        if (pHSInfo)
        {
            EFFECTPASS_SET_SHADER(HS);
            EFFECTPASS_SET_CONSTANTBUFFER(HS);
            EFFECTPASS_SET_PARAM(HS);
            EFFECTPASS_SET_SAMPLER(HS);
            EFFECTPASS_SET_SHADERRESOURCE(HS);
        } else
        {
            deviceContext->HSSetShader(nullptr, nullptr, 0);
        }

        if (pGSInfo)
        {
            EFFECTPASS_SET_SHADER(GS);
            EFFECTPASS_SET_CONSTANTBUFFER(GS);
            EFFECTPASS_SET_PARAM(GS);
            EFFECTPASS_SET_SAMPLER(GS);
            EFFECTPASS_SET_SHADERRESOURCE(GS);
        } else
        {
            deviceContext->GSSetShader(nullptr, nullptr, 0);
        }

        if (pPSInfo)
        {
            EFFECTPASS_SET_SHADER(PS);
            EFFECTPASS_SET_CONSTANTBUFFER(PS);
            EFFECTPASS_SET_PARAM(PS);
            EFFECTPASS_SET_SAMPLER(PS);
            EFFECTPASS_SET_SHADERRESOURCE(PS);
            if (pPSInfo->rw_use_mask)
            {
                std::vector<ID3D11UnorderedAccessView*> pUAVs((size_t)(log2f((float)pPSInfo->rw_use_mask)) + 1);
                std::vector<uint32_t> initCounts((size_t)(log2f((float)pPSInfo->rw_use_mask)) + 1);
                bool needInit = false;
                uint32_t firstSlot = 0;
                for (uint32_t slot = 0, mask = pPSInfo->rw_use_mask; mask; ++slot, mask >>= 1)
                {
                    if (mask & 1)
                    {
                        if (firstSlot == 0)
                            firstSlot = slot;
                        auto& res = rwResources.at(slot);
                        if (res.first_init)
                        {
                            needInit = true;
                            initCounts[slot] = res.initial_count;
                        }

                        res.first_init = false;
                        pUAVs[slot] = res.uav.Get();
                    }
                }
                // 必须一次性设置好，只要有一个需要初始化counter就都会被初始化
                deviceContext->OMSetRenderTargetsAndUnorderedAccessViews(D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL,
                                                    nullptr, nullptr, firstSlot, (uint32_t)pUAVs.size() - firstSlot, &pUAVs[firstSlot],
                                                    (needInit ? initCounts.data() : nullptr));
            }
        } else
        {
            deviceContext->PSSetShader(nullptr, nullptr, 0);
        }

        if (pCSInfo)
        {
            EFFECTPASS_SET_SHADER(CS);
            EFFECTPASS_SET_CONSTANTBUFFER(CS);
            EFFECTPASS_SET_PARAM(CS);
            EFFECTPASS_SET_SAMPLER(CS);
            EFFECTPASS_SET_SHADERRESOURCE(CS);
            for (uint32_t slot = 0, mask = pCSInfo->rw_use_mask; mask; ++slot, mask >>= 1)
            {
                if (mask & 1)
                {
                    auto& res = rwResources.at(slot);
                    deviceContext->CSSetUnorderedAccessViews(slot, 1, res.uav.GetAddressOf(),
                                                                (res.enable_counter && res.first_init ? &res.initial_count : nullptr));
                    res.first_init = false;
                }
            }
        } else
        {
            deviceContext->CSSetShader(nullptr, nullptr, 0);
        }

        // Set render state
        deviceContext->RSSetState(pRasterizerState.Get());
        deviceContext->OMSetBlendState(pBlendState.Get(), blendFactor, sampleMask);
        deviceContext->OMSetDepthStencilState(pDepthStencilState.Get(), stencilRef);
    }

    void EffectPass::dispatch(ID3D11DeviceContext *deviceContext, uint32_t threadX, uint32_t threadY,
                                uint32_t threadZ)
    {
        if (!pCSInfo)
        {
#ifdef _DEBUG
            OutputDebugStringA("[Warning]: No compute shader in current effect pass!");
#endif
            return;
        }

        uint32_t threadGroupCountX = (threadX + pCSInfo->thread_group_size_x - 1) / pCSInfo->thread_group_size_x;
        uint32_t threadGroupCountY = (threadY + pCSInfo->thread_group_size_y - 1) / pCSInfo->thread_group_size_y;
        uint32_t threadGroupCountZ = (threadZ + pCSInfo->thread_group_size_z - 1) / pCSInfo->thread_group_size_z;

        deviceContext->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
    }
}






































