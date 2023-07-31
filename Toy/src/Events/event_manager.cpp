//
// Created by ZHIKANG on 2023/5/21.
//

#include <Toy/Events/event_manager.h>

namespace toy
{
    static std::vector<event_delegate_c<event_t>> event_delegate_factory;

    void event_manager_c::init(GLFWwindow *glfw_window)
    {
        event_delegate_factory.reserve(3);
        event_delegate_c<event_t> window_close_event_delegate{};
        event_delegate_c<event_t> window_resize_event_delegate{};
        event_delegate_c<event_t> key_event_delegate{};
        event_delegate_factory.emplace_back(window_close_event_delegate);
        event_delegate_factory.emplace_back(window_resize_event_delegate);
        event_delegate_factory.emplace_back(key_event_delegate);

        glfwSetWindowCloseCallback(glfw_window, [](GLFWwindow* window)
        {
            event_t event = window_close_event_c{};

            event_delegate_factory[event.index()].fire(event);
        });

        glfwSetFramebufferSizeCallback(glfw_window, [](GLFWwindow* window, int32_t width, int32_t height)
        {
            event_t event = window_resize_event_c{ width, height };

            event_delegate_factory[event.index()].fire(event);
        });

        glfwSetKeyCallback(glfw_window, [](GLFWwindow* window, int32_t key_in, int32_t scan_code, int32_t action, int32_t mods)
        {
            switch (action)
            {
                case GLFW_PRESS:
                {
                    event_t event = key_pressed_event_c{static_cast<key_code>(key_in), false};
                    event_delegate_factory[event.index()].fire(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    event_t event = key_released_event_c{static_cast<key_code>(key_in)};
                    event_delegate_factory[event.index()].fire(event);
                    break;
                }
                case GLFW_REPEAT:
                {
                    event_t event = key_pressed_event_c{static_cast<key_code>(key_in), true};
                    event_delegate_factory[event.index()].fire(event);
                    break;
                }
            }
        });
    }

    void event_manager_c::update()
    {
        // glfwWaitEvents();
        glfwPollEvents();
    }

    uint32_t event_manager_c::subscribe(event_type_e event_type, std::function<void(const event_t &)> &&event_callback)
    {
        auto event_delegate_index = static_cast<size_t>(event_type) - 1;
        return event_delegate_factory[event_delegate_index].subscribe(std::move(event_callback));
    }

    void event_manager_c::unsubscribe(event_type_e event_type, uint32_t subscriber_id)
    {
        auto event_delegate_index = static_cast<size_t>(event_type) - 1;
        event_delegate_factory[event_delegate_index].unsubscribe(subscriber_id);
    }
}