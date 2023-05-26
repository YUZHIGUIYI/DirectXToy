//
// Created by ZZK on 2023/5/25.
//

#pragma once

#include <Toy/Core/key_code.h>
#include <Toy/Core/mouse_code.h>

namespace toy
{
    // TODO: will be reconstructed
    class input_c
    {
    public:
        static bool is_key_pressed(GLFWwindow* glfw_window, key_code key_in);

        static bool is_mouse_button_pressed(GLFWwindow* glfw_window, mouse_code button);

        static std::pair<float, float> get_mouse_position(GLFWwindow* glfw_window);

        static float get_mouse_x(GLFWwindow* glfw_window);

        static float get_mouse_y(GLFWwindow* glfw_window);
    };

    using DX_INPUT = input_c;
}















