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

    EngineEventVariant TaskSystem::try_pop()
    {
        if (!m_pending_events.empty())
        {
            EngineEventVariant engine_event = std::move(m_pending_events.front());
            m_pending_events.pop_front();
            return engine_event;
        }
        return NoneEvent{};
    }

    bool TaskSystem::try_get(toy::EngineEventVariant &engine_event)
    {
        if (!m_pending_events.empty())
        {
            engine_event = std::move(m_pending_events.front());
            m_pending_events.pop_front();
            return true;
        }
        return false;
    }

    void TaskSystem::assign(std::initializer_list<EngineEventVariant> engine_event_list)
    {
        assign(engine_event_list.begin(), engine_event_list.end());
    }
}