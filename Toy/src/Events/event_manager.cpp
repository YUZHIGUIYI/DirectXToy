//
// Created by ZHIKANG on 2023/5/21.
//

#include <Toy/Events/event_manager.h>

namespace toy
{
    static std::vector<EventDelegate> event_delegate_factory;

    void EventManager::init(GLFWwindow *glfw_window)
    {
        event_delegate_factory.reserve(9);
        event_delegate_factory.emplace_back();  // 0 - Window close event
        event_delegate_factory.emplace_back();  // 1 - Window resize event
        event_delegate_factory.emplace_back();  // 2 - Key pressed event
        event_delegate_factory.emplace_back();  // 3 - Key released event
        event_delegate_factory.emplace_back();  // 4 - Mouse moved event
        event_delegate_factory.emplace_back();  // 5 - Mouse scrolled event
        event_delegate_factory.emplace_back();  // 6 - Mouse button pressed event
        event_delegate_factory.emplace_back();  // 7 - Mouse button released event
        event_delegate_factory.emplace_back();  // 8 - Drop event

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
//            switch (action)
//            {
//                case GLFW_PRESS:
//                {
//                    EngineEventVariant event = KeyPressedEvent{static_cast<key_code>(key_in), mods, false};
//                    event_delegate_factory[event.index()].fire(event);
//                    break;
//                }
//                case GLFW_RELEASE:
//                {
//                    EngineEventVariant event = KeyReleasedEvent{static_cast<key_code>(key_in), mods};
//                    event_delegate_factory[event.index()].fire(event);
//                    break;
//                }
//                case GLFW_REPEAT:
//                {
//                    EngineEventVariant event = KeyPressedEvent{static_cast<key_code>(key_in), mods, true};
//                    event_delegate_factory[event.index()].fire(event);
//                    break;
//                }
//                default:
//                    break;
//            }
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
//            switch (action)
//            {
//                case GLFW_PRESS:
//                case GLFW_RELEASE:
//                default:
//                    break;
//            }
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