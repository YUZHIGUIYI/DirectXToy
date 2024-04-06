//
// Created by ZZK on 2024/3/13.
//

#include <Toy/Core/subsystem.h>

namespace toy::core::details
{
    bool initialize()
    {
        if (!context().initialize())
        {
            return false;
        }

        status() = InternalStatus::running;
        return true;
    }

    InternalStatus& status()
    {
        static InternalStatus internal_status = InternalStatus::idle;
        return internal_status;
    }

    void dispose()
    {
        context().dispose();
        status() = InternalStatus::disposed;
    }

    SubsystemContext& context()
    {
        static SubsystemContext subsystem_context = {};
        return subsystem_context;
    }

    bool SubsystemContext::initialize()
    {
        return is_disposed ? false : true;
    }

    void SubsystemContext::dispose()
    {
        for (auto it = m_system_orders.rbegin(); it != m_system_orders.rend(); ++it)
        {
            auto found = m_systems.find(*it);
            if (found == m_systems.end())
            {
                DX_CORE_CRITICAL("Failed to find subsystem");
            }
            found->second.reset();
            m_systems.erase(found);
        }
        m_system_orders.clear();
        m_systems.clear();
        is_disposed = true;
    }
}