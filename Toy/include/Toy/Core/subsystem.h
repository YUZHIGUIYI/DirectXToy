//
// Created by ZZK on 2024/3/13.
//

#pragma once

#include <Toy/Core/type_index.h>

namespace toy::core
{
    template <typename S>
    S& get_subsystem();

    template <typename S, typename ... Args>
    S& add_subsystem(Args&& ... args);

    template <typename S>
    void remove_subsystem();

    template <typename ... Args>
    bool has_subsystems();

    namespace details
    {
        struct SubsystemContext;

        enum class InternalStatus : uint8_t
        {
            idle,
            running,
            disposed,
        };

        bool initialize();

        InternalStatus& status();

        void dispose();

        SubsystemContext& context();

        struct SubsystemContext
        {
        public:
            bool initialize();

            void dispose();

            template <typename S, typename ... Args>
            S& add_subsystem(Args&& ... args);

            template <typename S>
            S& get_subsystem();

            template <typename S>
            void remove_subsystem();

            template <typename S>
            [[nodiscard]] bool has_subsystems() const;

            template <typename S1, typename S2, typename ... Args>
            [[nodiscard]] bool has_subsystems() const;

        private:
            std::vector<std::size_t> m_system_orders;
            std::unordered_map<std::size_t, std::shared_ptr<void>> m_systems;
            bool is_disposed = false;
        };

        template <typename S, typename ... Args>
        S& SubsystemContext::add_subsystem(Args&& ... args)
        {
            static_assert(std::is_constructible_v<S, Args...>, "Failed to construct object T from args");
            if (has_subsystems<S>())
            {
                DX_CORE_CRITICAL("Duplicated subsystem");
            }
            auto index = rtti::type_id<S>().hash_code();
            m_system_orders.emplace_back(index);
            auto it = m_systems.try_emplace(index, std::make_unique<S>(std::forward<Args>(args)...));
            return *reinterpret_cast<S *>((*it.first).second.get());
        }

        template <typename S>
        S& SubsystemContext::get_subsystem()
        {
            if (!has_subsystems<S>())
            {
                DX_CORE_CRITICAL("Failed to get subsystem");
            }
            auto index = rtti::type_id<S>().hash_code();
            return *reinterpret_cast<S *>(m_systems[index].get());
        }

        template <typename S>
        void SubsystemContext::remove_subsystem()
        {
            if (!has_subsystems<S>())
            {
                DX_CORE_CRITICAL("Failed to find subsystem");
            }
            auto index = rtti::type_id<S>().hash_code();
            m_systems.erase(index);
            m_system_orders.erase(std::remove_if(m_system_orders.begin(), m_system_orders.end(), [index] (const auto &element) { return index = element; }));
        }

        template <typename S>
        bool SubsystemContext::has_subsystems() const
        {
            auto index = rtti::type_id<S>().hash_code();
            return m_systems.contains(index);
        }

        template <typename S1, typename S2, typename ... Args>
        bool SubsystemContext::has_subsystems() const
        {
            return has_subsystems<S1>() && has_subsystems<S2, Args ...>();
        }
    }

    template <typename S>
    S& get_subsystem()
    {
        if (details::status() != details::InternalStatus::running)
        {
            DX_CORE_CRITICAL("Subsystem context must be initialized");
        }
        return details::context().get_subsystem<S>();
    }

    template <typename S, typename ... Args>
    S& add_subsystem(Args&& ... args)
    {
        if (details::status() != details::InternalStatus::running)
        {
            DX_CORE_CRITICAL("Subsystem context must be initialized");
        }
        return details::context().add_subsystem<S>(std::forward<Args>(args)...);
    }

    template <typename S>
    void remove_subsystem()
    {
        if (details::status() == details::InternalStatus::running)
        {
            details::context().remove_subsystem<S>();
        }
    }

    template <typename ... Args>
    bool has_subsystems()
    {
        if (details::status() != details::InternalStatus::running)
        {
            return false;
        } else
        {
            return details::context().has_subsystems<Args...>();
        }
    }
}
