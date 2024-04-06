//
// Created by ZZK on 2024/3/12.
//

#pragma once

#include <Toy/Core/base.h>
#include <typeindex>

namespace toy::rtti
{
    struct TypeIndex
    {
    private:
        const std::type_info* info = nullptr;

    public:
        explicit TypeIndex(const std::type_info *input_info) noexcept : info{ input_info }
        {

        }

        bool operator==(const TypeIndex &other) const noexcept
        {
            return hash_code() == other.hash_code();
        }

        bool operator!=(const TypeIndex &other) const noexcept
        {
            return hash_code() != other.hash_code();
        }

        bool operator>(const TypeIndex &other) const noexcept
        {
            return hash_code() > other.hash_code();
        }

        bool operator<(const TypeIndex &other) const noexcept
        {
            return hash_code() < other.hash_code();
        }

        [[nodiscard]] std::size_t hash_code() const noexcept
        {
            return info->hash_code();
        }
    };

    namespace details
    {
        template <typename T>
        const TypeIndex& type_id_impl()
        {
            static TypeIndex id{ &typeid(T) };
            return id;
        }
    }

    template <typename T>
    const TypeIndex& type_id()
    {
        return details::type_id_impl<std::remove_cvref_t<T>>();
    }
}
