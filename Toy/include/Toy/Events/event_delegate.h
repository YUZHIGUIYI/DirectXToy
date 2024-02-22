//
// Created by ZHIKANG on 2023/5/21.
//

#pragma once

#include <Toy/Core/base.h>
#include <Toy/Events/window_event.h>

namespace toy
{
    // TODO: make thread safe
    // Handles the event flow between subscriber and publisher
    class EventDelegate
    {
    public:
        EventDelegate();

        // Subscribe a method/function to the event delegate which wants to receive the arguments specified in Args
        uint32_t subscribe(std::function<void(const EngineEventVariant &)> &&event_callback);

        // Unsubscribes a method/function from the event delegate
        void unsubscribe(uint32_t subscriber_id);

        // Publishes Args arguments to the subscribers
        void fire(const EngineEventVariant &event_variant);

    private:
        std::unordered_map<uint32_t, std::function<void(const EngineEventVariant &)>> event_callback_map;
        uint32_t count = 0;
    };
}







