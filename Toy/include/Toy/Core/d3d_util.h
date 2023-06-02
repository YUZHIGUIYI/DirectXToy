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

#pragma warning(push)
#pragma warning(disable: 28251)
    extern "C" __declspec(dllimport) int __stdcall MultiByteToWideChar(unsigned int cp, unsigned long flags, const char* str, int cbmb, wchar_t* widestr, int cchwide);
    extern "C" __declspec(dllimport) int __stdcall WideCharToMultiByte(unsigned int cp, unsigned long flags, const wchar_t* widestr, int cchwide, char* str, int cbmb, const char* defchar, int* used_default);
#pragma warning(pop)

    inline std::wstring utf8_to_wstring(std::string_view utf8str)
    {
        if (utf8str.empty()) return std::wstring();
        int cbMultiByte = static_cast<int>(utf8str.size());
        int req = MultiByteToWideChar(65001, 0, utf8str.data(), cbMultiByte, nullptr, 0);
        std::wstring res(req, 0);
        MultiByteToWideChar(65001, 0, utf8str.data(), cbMultiByte, &res[0], req);
        return res;
    }

    inline std::string wstring_to_utf8(std::wstring_view wstr)
    {
        if (wstr.empty()) return std::string();
        int cbMultiByte = static_cast<int>(wstr.size());
        int req = WideCharToMultiByte(65001, 0, wstr.data(), cbMultiByte, nullptr, 0, nullptr, nullptr);
        std::string res(req, 0);
        WideCharToMultiByte(65001, 0, wstr.data(), cbMultiByte, &res[0], req, nullptr, nullptr);
        return res;
    }

    // string convert to hash ID
    using XID = size_t;
    inline XID string_to_id(std::string_view str)
    {
        static std::hash<std::string_view> hash;
        return hash(str);
    }

    namespace XMath
    {
        inline DirectX::XMMATRIX XM_CALLCONV inverse_transpose(const DirectX::FXMMATRIX& M)
        {
            using namespace DirectX;
            // The transposition of the inverse of the world matrix only applies to the normal vector
            // Do not need displacement component
            // Must remove
            XMMATRIX A = M;
            A.r[3] = g_XMIdentityR3;

            return XMMatrixTranspose(XMMatrixInverse(nullptr, A));
        }

        inline float lerp(float a, float b, float t)
        {
            return (1.0f - t) * a + t * b;
        }
    }
}
