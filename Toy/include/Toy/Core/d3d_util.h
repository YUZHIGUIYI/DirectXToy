//
// Created by ZZK on 2023/5/18.
//

#pragma once

#include <Toy/Core/base.h>

namespace toy
{
    namespace math
    {
        constexpr double pi_div_180 = 0.01745329251994329576923690768489;
        constexpr double inv_pi_div_180 = 57.295779513082320876798154814105;

        // Convert degree to radian
        template <typename T>
        requires std::is_floating_point_v<T>
        T radians(T degrees)
        {
            return degrees * static_cast<T>(pi_div_180);
        }

        // Convert float3 degree to float3 radian
        inline DirectX::XMFLOAT3 float3_radians(const DirectX::XMFLOAT3 &degrees)
        {
            return { degrees.x * static_cast<float>(pi_div_180), degrees.y * static_cast<float>(pi_div_180),
                        degrees.z * static_cast<float>(pi_div_180) };
        }

        // Convert radian to degree
        template <typename T>
        requires std::is_floating_point_v<T>
        T degrees(T radians)
        {
            return radians * static_cast<T>(inv_pi_div_180);
        }

        // Convert float3 radian to float3 degree
        inline DirectX::XMFLOAT3 float3_degrees(const DirectX::XMFLOAT3 &radians)
        {
            return { radians.x * static_cast<float>(inv_pi_div_180), radians.y * static_cast<float>(inv_pi_div_180),
                        radians.z * static_cast<float>(inv_pi_div_180) };
        }
    }

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
