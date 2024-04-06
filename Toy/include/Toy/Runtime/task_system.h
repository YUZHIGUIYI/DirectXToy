//
// Created by ZZK on 2024/3/20.
//

#pragma once

#include <Toy/Core/base.h>
#include <Toy/Events/event.h>
#include <Toy/Events/window_event.h>

namespace toy::runtime
{
    struct TaskSystem
    {
    public:
        void push(EngineEventVariant &&engine_event);

        void push(const EngineEventVariant &engine_event);

        template<std::input_iterator InputIt>
        void assign(InputIt first, InputIt last);

        void assign(std::initializer_list<EngineEventVariant> engine_event_list);

        void clear();

        EngineEventVariant try_pop();

        bool try_get(EngineEventVariant &engine_event);

        [[nodiscard]] bool empty() const;

    private:
        std::deque<EngineEventVariant> m_pending_events;
    };


    template <std::input_iterator InputIt>
    void TaskSystem::assign(InputIt first, InputIt last)
    {
        m_pending_events.assign(first, last);
    }

}
