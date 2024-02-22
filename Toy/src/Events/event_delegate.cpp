//
// Created by ZZK on 2024/1/13.
//

#include <Toy/Events/event_delegate.h>

namespace toy
{
    EventDelegate::EventDelegate() = default;

    uint32_t EventDelegate::subscribe(std::function<void(const EngineEventVariant &)> &&event_callback)
    {
        auto subscribe_id = count++;
        auto pair = event_callback_map.try_emplace(subscribe_id, event_callback);
        return (pair.second ? subscribe_id : pair.first->first);
    }

    void EventDelegate::unsubscribe(uint32_t subscriber_id)
    {
        event_callback_map.erase(subscriber_id);
    }

    void  EventDelegate::fire(const EngineEventVariant &event_variant)
    {
        for (auto&& event_callback : event_callback_map)
        {
            event_callback.second(event_variant);
        }
    }
}