//
// Created by ZZK on 2023/5/31.
//

#pragma once

#include <Toy/Core/d3d_util.h>
#include <Toy/Model/property.h>

namespace toy::model
{
    class Material
    {
    public:
        Material() = default;

        void clear()
        {
            m_Properties.clear();
        }

        template<typename T>
        void set(std::string_view name, const T& value)
        {
            static_assert(is_variant_member_v<T, Property>, "Type T is not one of the Property types");
            m_Properties[string_to_id(name)] = value;
        }

        template<typename T>
        const T& get(std::string_view name) const
        {
            auto it = m_Properties.find(string_to_id(name));
            return std::get<T>(it->second);
        }

        template<typename T>
        T& get(std::string_view name)
        {
            return const_cast<T &>(this->get<T>(name));
        }

        template<typename T>
        [[nodiscard]] bool has(std::string_view name) const
        {
            auto it = m_Properties.find(string_to_id(name));
            if (it == m_Properties.end() || !std::holds_alternative<T>(it->second))
            {
                return false;
            }
            return true;
        }

        template<typename T>
        const T* try_get(std::string_view name) const
        {
            auto it = m_Properties.find(string_to_id(name));
            if (it != m_Properties.end())
            {
                return &std::get<T>(it->second);
            } else
            {
                return nullptr;
            }
        }

        template<typename T>
        T* try_get(std::string_view name)
        {
            return const_cast<T *>(this->try_get<T>(name));
        }

        [[nodiscard]] bool has_property(std::string_view name) const
        {
            return m_Properties.find(string_to_id(name)) != m_Properties.end();
        }

    private:
        std::unordered_map<XID, Property> m_Properties;
    };
}


























