//
// Created by ZZK on 2023/5/18.
//

#pragma once

#include <Toy/Core/base.h>

namespace toy
{
    // ------------------------------
    // create_shader_from_file function
    // ------------------------------
    // [In]cso_file_name   Compiled binary cso-shader file, if specified, prioritize finding and reading the file
    // [In]hlsl_file_name  HLSL shader code, if not specified cso file, try to compile this hlsl shader code
    // [In]entry_point     Entry point of HLSL shader code
    // [In]shader_model    Shader model, "*s_5_0" format, may be one of c, d, g, h, p, v
    // [Out]blob_out_pp    Output shader binary information
    HRESULT create_shader_from_file(const wchar_t* cso_file_name, const wchar_t* hlsl_file_name, const char* entry_point,
                                    const char* shader_model, ID3DBlob** blob_out_pp);

    inline DirectX::XMMATRIX XM_CALLCONV inverse_transpose(const DirectX::FXMMATRIX& M)
    {
        using namespace DirectX;
        // Do not need displacement component
        // Must remove
        XMMATRIX A = M;
        A.r[3] = g_XMIdentityR3;

        return XMMatrixTranspose(XMMatrixInverse(nullptr, A));
    }
}
