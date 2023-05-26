//
// Created by ZZK on 2023/5/25.
//

#include <Toy/Core/input.h>

namespace toy
{
    bool input_c::is_key_pressed(GLFWwindow *glfw_window, toy::key_code key_in)
    {
        auto state = glfwGetKey(glfw_window, static_cast<int32_t>(key_in));
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    bool input_c::is_mouse_button_pressed(GLFWwindow *glfw_window, toy::mouse_code button)
    {
        auto state = glfwGetMouseButton(glfw_window, static_cast<int32_t>(button));
        return state == GLFW_PRESS;
    }

    std::pair<float, float> input_c::get_mouse_position(GLFWwindow *glfw_window)
    {
        double x_pos = 0.0, y_pos = 0.0;
        glfwGetCursorPos(glfw_window, &x_pos, &y_pos);

        return std::pair{ static_cast<float>(x_pos), static_cast<float>(y_pos) };
    }

    float input_c::get_mouse_x(GLFWwindow* glfw_window)
    {
        return get_mouse_position(glfw_window).first;
    }

    float input_c::get_mouse_y(GLFWwindow *glfw_window)
    {
        return get_mouse_position(glfw_window).second;
    }
}


























