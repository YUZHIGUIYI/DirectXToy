//
// Created by ZZK on 2023/5/18.
//

#include <Toy/Core/d3d_util.h>

namespace toy
{
    HRESULT create_shader_from_file(const wchar_t* cso_file_name, const wchar_t* hlsl_file_name, const char* entry_point,
                                    const char* shader_model, ID3DBlob** blob_out_pp)
    {
        HRESULT hr = S_OK;

        // Find whether there is a compiled shader
        if (cso_file_name && D3DReadFileToBlob(cso_file_name, blob_out_pp) == S_OK)
        {
            std::wcout << "Has already compiled this shader file: " << hlsl_file_name << std::endl;
            return hr;
        } else
        {
            DWORD dw_shader_flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
            // Set D3DCOMPILE_DEBUG flag to get shader debug information
            dw_shader_flags |= D3DCOMPILE_DEBUG;
            // Prohibit optimization
            dw_shader_flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
            ID3DBlob* error_blob = nullptr;
            hr = D3DCompileFromFile(hlsl_file_name, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entry_point, shader_model,
                                    dw_shader_flags, 0, blob_out_pp, &error_blob);
            if (FAILED(hr))
            {
                if (error_blob)
                {
                    OutputDebugStringA(reinterpret_cast<const char *>(error_blob->GetBufferPointer()));
                }
                if (error_blob)
                {
                    error_blob->Release();
                    error_blob = nullptr;
                }
                return hr;
            }
            // If specific output file name, output binary information to cso output file
            if (cso_file_name)
            {
                return D3DWriteBlobToFile(*blob_out_pp, cso_file_name, FALSE);
            }
        }

        return hr;
    }
}