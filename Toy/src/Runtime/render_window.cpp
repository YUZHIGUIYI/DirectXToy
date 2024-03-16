//
// Created by ZZK on 2024/3/15.
//

#include <Toy/Runtime/render_window.h>

namespace toy::runtime
{
    RenderWindow::RenderWindow(int32_t width, int32_t height)
    : m_window_width(width), m_window_height(height)
    {
        // Initialize GLFW window
        if (!glfwInit())
        {
            DX_CORE_CRITICAL("Failed to initialize GLFW");
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        m_native_window = glfwCreateWindow(m_window_width, m_window_height, "DX11Renderer", nullptr, nullptr);
        if (!m_native_window)
        {
            glfwTerminate();
            DX_CORE_CRITICAL("Failed to create GLFW window");
        }

        // Register window user
        glfwSetWindowUserPointer(m_native_window, this);

        // Register callbacks
        //// Window close callback
        glfwSetWindowCloseCallback(m_native_window, [](GLFWwindow* window)
        {
            auto render_window = reinterpret_cast<RenderWindow *>(glfwGetWindowUserPointer(window));
            render_window->m_window_close_event = WindowCloseEvent{};
        });

        //// Framebuffer resize callback
        glfwSetFramebufferSizeCallback(m_native_window, [](GLFWwindow* window, int32_t width, int32_t height)
        {
            auto render_window = reinterpret_cast<RenderWindow *>(glfwGetWindowUserPointer(window));
            render_window->m_framebuffer_resize_event = WindowResizeEvent{ width, height };
        });

        //// Drop callback
        glfwSetDropCallback(m_native_window, [](GLFWwindow* window, int32_t path_count, const char** paths)
        {
            auto render_window = reinterpret_cast<RenderWindow *>(glfwGetWindowUserPointer(window));
            for (int32_t i = 0; i < path_count; i++)
            {
                render_window->m_delegate_events.emplace_back(DropEvent{ paths[i] });
            }
        });
    }

    RenderWindow::~RenderWindow()
    {
        glfwDestroyWindow(m_native_window);
        glfwTerminate();
    }

    void RenderWindow::tick()
    {
        m_window_close_event = NoneEvent{};
        m_framebuffer_resize_event = NoneEvent{};
        m_delegate_events.clear();
        glfwPollEvents();
    }

    GLFWwindow *RenderWindow::get_native_window() const
    {
        return m_native_window;
    }

    bool RenderWindow::poll_window_close() const
    {
        return std::holds_alternative<WindowCloseEvent>(m_window_close_event);
    }

    int32_t RenderWindow::get_window_width() const
    {
        return m_window_width;
    }

    int32_t RenderWindow::get_window_height() const
    {
        return m_window_height;
    }

    bool RenderWindow::poll_framebuffer_resize() const
    {
        return std::holds_alternative<WindowResizeEvent>(m_framebuffer_resize_event);
    }

    std::vector<EngineEventVariant> RenderWindow::poll_delegate_events()
    {
        std::vector<EngineEventVariant> poll_window_events;
        poll_window_events.reserve(1 + m_delegate_events.size());

        if (std::holds_alternative<WindowResizeEvent>(m_framebuffer_resize_event))
        {
            poll_window_events.emplace_back(m_framebuffer_resize_event);
        }

        if (poll_window_events.empty() && !m_delegate_events.empty())
        {
            poll_window_events.assign(m_delegate_events.cbegin(), m_delegate_events.cend());
        } else if (!poll_window_events.empty() && !m_delegate_events.empty())
        {
            poll_window_events.insert(poll_window_events.cend(), m_delegate_events.cbegin(), m_delegate_events.cend());
        }

        return poll_window_events;
    }
}