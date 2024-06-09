//
// Created by ZZK on 2023/5/31.
//

#pragma once

#include <Toy/Core/d3d_util.h>
#include <Toy/Model/property.h>

namespace toy::model
{
#define FOREACH_MATERIAL_SEMANTICS(func) \
    func(DiffuseMap)                     \
    func(SpecularMap)                    \
    func(AlbedoMap)                      \
    func(NormalMap)                      \
    func(MetalnessMap)                   \
    func(RoughnessMap)                   \
    func(NormalCameraMap)                \
    func(AmbientOcclusionMap)            \
    func(DiffuseColor)                   \
    func(AmbientColor)                   \
    func(SpecularColor)                  \
    func(SpecularFactor)                 \
    func(Opacity)                        \
    func(Metalness)                      \
    func(Roughness)                      \
    func(IrradianceMap)                  \
    func(PrefilteredSpecularMap)         \
    func(BRDFLUT)

    enum class MaterialSemantics : uint8_t
    {
#define MATERIAL_SEMANTICS_ENUM_FUNCTION(name) name,
        FOREACH_MATERIAL_SEMANTICS(MATERIAL_SEMANTICS_ENUM_FUNCTION)
#undef MATERIAL_SEMANTICS_ENUM_FUNCTION
    };

    inline constexpr std::string_view material_semantics_name(MaterialSemantics material_semantics)
    {
        switch (material_semantics)
        {
#define MATERIAL_SEMANTICS_ENUM_STRING(name) case MaterialSemantics::name: return "$"#name;
            FOREACH_MATERIAL_SEMANTICS(MATERIAL_SEMANTICS_ENUM_STRING)
#undef MATERIAL_SEMANTICS_ENUM_STRING
            default: DX_CORE_CRITICAL("Unsupported material semantics enum");
        }
    }

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
            auto it = m_Properties.find(string_to_id(name));
            return std::get<T>(it->second);
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


























