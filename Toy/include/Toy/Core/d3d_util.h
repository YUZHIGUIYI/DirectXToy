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

        // DirectX::XMFLOAT3 Operator * float
        inline DirectX::XMFLOAT3 operator*(const float scale_factor, const DirectX::XMFLOAT3 &input)
        {
            return DirectX::XMFLOAT3{ scale_factor * input.x, scale_factor * input.y, scale_factor * input.z };
        }

        inline DirectX::XMFLOAT3 operator*(const DirectX::XMFLOAT3 &input, const float scale_factor)
        {
            return operator*(scale_factor, input);
        }

        // DirectX::XMFLOAT3 Operator / float
        inline DirectX::XMFLOAT3 operator/(const DirectX::XMFLOAT3 &input, const float divisor)
        {
            return DirectX::XMFLOAT3{ input.x / divisor, input.y / divisor, input.z / divisor };
        }

        // DirectX::XMFLOAT3 Operator + float
        inline DirectX::XMFLOAT3 operator+(const DirectX::XMFLOAT3 &input, const float addend)
        {
            return DirectX::XMFLOAT3{ input.x + addend, input.y + addend, input.z + addend };
        }

        inline DirectX::XMFLOAT3 operator+(const float addend, const DirectX::XMFLOAT3 &input)
        {
            return operator+(input, addend);
        }

        // DirectX::XMFLOAT3 Operator - float
        inline DirectX::XMFLOAT3 operator-(const DirectX::XMFLOAT3 &input, const float subtrahend)
        {
            return DirectX::XMFLOAT3{ input.x - subtrahend, input.y - subtrahend, input.z - subtrahend };
        }
    }

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
        static std::hash<std::string_view> hash{};
        return hash(str);
    }
}
