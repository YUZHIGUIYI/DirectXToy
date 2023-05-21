//
// Created by ZHIKANG on 2023/5/21.
//

#pragma once

#include <Toy/Events/event_delegate.h>
#include <Toy/Events/window_event.h>

namespace toy
{
    class event_manager_c
    {
    public:
        // GLFW registers callbacks
        static void init(GLFWwindow* glfw_window);

        /*
         * Event processing
         * It puts the thread to sleep until at least one event has been received and then processes all received events
         */
        static void update();

        static uint32_t subscribe(event_type_e event_type, std::function<void(const event_t &)> &&event_callback);

        static void unsubscribe(event_type_e event_type, uint32_t subscriber_id);
    };
}