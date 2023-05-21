//
// Created by ZHIKANG on 2023/5/21.
//

#pragma once

#include <Toy/Core/base.h>

namespace toy
{
    // TODO: make thread safe
    // Handles the event flow between subscriber and publisher
    template<typename T>
    class event_delegate_c
    {
    public:
        event_delegate_c();

        // Subscribe a method/function to the event delegate which wants to receive the arguments specified in Args
        uint32_t subscribe(std::function<void(const T &)> &&event_callback);

        // Unsubscribes a method/function from the event delegate
        void unsubscribe(uint32_t subscriber_id);

        // Publishes Args arguments to the subscribers
        void fire(const T &arg);

    private:
        std::unordered_map<uint32_t, std::function<void(const T &)>> event_callback_map;
        uint32_t count;
    };

    template<typename T>
    event_delegate_c<T>::event_delegate_c()
    : count(0)
    { }

    template<typename T>
    uint32_t event_delegate_c<T>::subscribe(std::function<void(const T &)> &&event_callback)
    {
        auto subscribe_id = count++;
        auto pair = event_callback_map.try_emplace(subscribe_id, event_callback);
        return (pair.second ? subscribe_id : pair.first->first);
    }

    template<typename T>
    void event_delegate_c<T>::unsubscribe(uint32_t subscriber_id)
    {
        event_callback_map.erase(subscriber_id);
    }

    template<typename T>
    void  event_delegate_c<T>::fire(const T &arg)
    {
        for (auto&& event_callback : event_callback_map)
        {
            event_callback.second(arg);
        }
    }
}







