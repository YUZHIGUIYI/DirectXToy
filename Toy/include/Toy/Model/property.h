//
// Created by ZZK on 2023/5/30.
//

#pragma once

#include "Toy/Core/base.h"

namespace toy
{
    template<typename T, typename V>
    struct is_variant_member;

    template<typename T, typename ... Args>
    struct is_variant_member<T, std::variant<Args ... >> : public std::disjunction<std::is_same<T, Args>...>
    {

    };

    template<typename T, typename ... Args>
    constexpr bool is_variant_member_v = is_variant_member<T, Args ...>::value;

    using Property = std::variant<
            int32_t, uint32_t, float, DirectX::XMFLOAT2, DirectX::XMFLOAT3, DirectX::XMFLOAT4, DirectX::XMFLOAT4X4,
            std::vector<float>, std::vector<DirectX::XMFLOAT4>, std::vector<DirectX::XMFLOAT4X4>,
            std::string>;
}
