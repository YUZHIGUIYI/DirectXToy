//
// Created by ZHIKANG on 2023/5/21.
//

#include <Toy/Events/event_manager.h>

namespace toy
{
    static std::vector<EventDelegate> event_delegate_factory;

    void EventManager::init(GLFWwindow *glfw_window)
    {
        event_delegate_factory.reserve(7);
        event_delegate_factory.emplace_back();  // 0 - Window close event
        event_delegate_factory.emplace_back();  // 1 - Window resize event
        event_delegate_factory.emplace_back();  // 2 - Key typed event
        event_delegate_factory.emplace_back();  // 3 - Mouse moved event
        event_delegate_factory.emplace_back();  // 4 - Mouse scrolled event
        event_delegate_factory.emplace_back();  // 5 - Mouse button event
        event_delegate_factory.emplace_back();  // 6 - Drop event

        // Window close callback - 0
        glfwSetWindowCloseCallback(glfw_window, [](GLFWwindow* window)
        {
            EngineEventVariant event = WindowCloseEvent{};

            event_delegate_factory[event.index()].fire(event);
        });

        // Window resize callback - 1
        glfwSetFramebufferSizeCallback(glfw_window, [](GLFWwindow* window, int32_t width, int32_t height)
        {
            EngineEventVariant event = WindowResizeEvent{ width, height };

            event_delegate_factory[event.index()].fire(event);
        });

        // Key callback - 2 && 3
//        glfwSetKeyCallback(glfw_window, [](GLFWwindow* window, int32_t key_in, int32_t scan_code, int32_t action, int32_t mods)
//        {
//            bool is_pressed = ((action & GLFW_PRESS) || (action & GLFW_REPEAT));
//            bool is_repeat = (action & GLFW_REPEAT) != 0;
//            EngineEventVariant event = KeyTypedEvent{ static_cast<key_code>(key_in), mods, is_pressed, is_repeat };
//            event_delegate_factory[event.index()].fire(event);
//        });

        // Mouse moved callback - 4
//        glfwSetCursorPosCallback(glfw_window, [](GLFWwindow* window, double x_pos, double y_pos)
//        {
//
//        });

        // Mouse scrolled callback - 5
//        glfwSetScrollCallback(glfw_window, [](GLFWwindow* window, double x_offset, double y_offset)
//        {
//
//        });

        // Mouse button callbacks - 6 && 7
//        glfwSetMouseButtonCallback(glfw_window, [](GLFWwindow* window, int32_t button, int32_t action, int32_t mods)
//        {
//            EngineEventVariant event = MouseButtonEvent{ static_cast<mouse_code>(button), mods };
//            event_delegate_factory[event.index()].fire(event);
//        });

        // Drop callback - 8
        glfwSetDropCallback(glfw_window, [](GLFWwindow* window, int32_t path_count, const char** paths)
        {
            for (int32_t i = 0; i < path_count; i++)
            {
                EngineEventVariant event = DropEvent{ paths[i] };
                event_delegate_factory[event.index()].fire(event);
            }
        });
    }

    void EventManager::update()
    {
        glfwPollEvents();
    }

    uint32_t EventManager::subscribe(EventType event_type, std::function<void(const EngineEventVariant &)> &&event_callback)
    {
        auto event_delegate_index = static_cast<size_t>(event_type) - 1;
        return event_delegate_factory[event_delegate_index].subscribe(std::move(event_callback));
    }

    void EventManager::unsubscribe(EventType event_type, uint32_t subscriber_id)
    {
        auto event_delegate_index = static_cast<size_t>(event_type) - 1;
        event_delegate_factory[event_delegate_index].unsubscribe(subscriber_id);
    }
}