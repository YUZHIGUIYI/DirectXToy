//
// Created by ZZK on 2024/3/20.
//

#include <Toy/Runtime/task_system.h>

namespace toy::runtime
{
    void TaskSystem::push(EngineEventVariant &&engine_event)
    {
        m_pending_events.emplace_back(std::move(engine_event));
    }

    void TaskSystem::push(const EngineEventVariant &engine_event)
    {
        m_pending_events.emplace_back(engine_event);
    }

    void TaskSystem::clear()
    {
        m_pending_events.clear();
    }

    bool TaskSystem::empty() const
    {
        return m_pending_events.empty();
    }

    const std::vector<EngineEventVariant> &TaskSystem::get_pending_events() const
    {
        return m_pending_events;
    }
}