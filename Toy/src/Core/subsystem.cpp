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
        m_systems.clear();
        is_disposed = true;
    }
}