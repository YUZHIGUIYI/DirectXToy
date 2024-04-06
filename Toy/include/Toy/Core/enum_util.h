//
// Created by ZZK on 2024/2/27.
//

#pragma once

#include <Toy/Core/base.h>

namespace toy
{
    template <typename T>
    concept enum_concept = std::is_enum_v<T>;

    template <enum_concept T>
    std::underlying_type_t<T> operator|(const T lhs, const T rhs) noexcept
    {
        using underlying_type = std::underlying_type_t<T>;
        return static_cast<underlying_type>(static_cast<underlying_type>(lhs) | static_cast<underlying_type>(rhs));
    }

    template <enum_concept T>
    std::underlying_type_t<T> operator&(const T lhs, const T rhs) noexcept
    {
        using underlying_type = std::underlying_type_t<T>;
        return static_cast<underlying_type>(static_cast<underlying_type>(lhs) & static_cast<underlying_type>(rhs));
    }

    template <enum_concept T>
    std::underlying_type_t<T> operator^(const T lhs, const T rhs) noexcept
    {
        using underlying_type = std::underlying_type_t<T>;
        return static_cast<underlying_type>(static_cast<underlying_type>(lhs) ^ static_cast<underlying_type>(rhs));
    }

    template <enum_concept T>
    bool operator!(const T value) noexcept
    {
        using underlying_type = std::underlying_type_t<T>;
        return !static_cast<underlying_type>(value);
    }

    template <enum_concept T>
    struct EnumWrapper
    {
    private:
        T enum_value = {};

    public:
        constexpr EnumWrapper() : enum_value{} {}
        explicit constexpr EnumWrapper(T enum_value_in) : enum_value{ enum_value_in } {}

        constexpr operator std::underlying_type_t<T>() const
        {
            return static_cast<std::underlying_type_t<T>>(enum_value);
        }
    };
}
