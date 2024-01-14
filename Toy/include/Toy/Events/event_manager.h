//
// Created by ZHIKANG on 2023/5/21.
//

#pragma once

#include <Toy/Events/event_delegate.h>
#include <Toy/Events/window_event.h>

namespace toy
{
    class EventManager
    {
    public:
        // GLFW registers callbacks
        static void init(GLFWwindow* glfw_window);

        /*
         * Event processing
         * It puts the thread to sleep until at least one event has been received and then processes all received events
         */
        static void update();

        static uint32_t subscribe(EventType event_type, std::function<void(const EngineEventVariant &)> &&event_callback);

        static void unsubscribe(EventType event_type, uint32_t subscriber_id);
    };
}