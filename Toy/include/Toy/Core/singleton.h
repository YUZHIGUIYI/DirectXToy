//
// Created by ZHIKANG on 2023/5/19.
//

#pragma once

namespace toy
{
    // Note: currently deprecated
    template<typename T, typename = std::enable_if_t<std::is_default_constructible_v<T>>>
    class singleton_c
    {
    private:
        singleton_c() = default;

    public:
        static_assert(std::is_default_constructible_v<T>, "Ensure type T has a default constructor");

        static T& get()
        {
            static T singleton{};
            return singleton;
        }
    };

    template<typename T>
    T singleton_handle = singleton_c<T>::get();
}
