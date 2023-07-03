//
// Created by ZZK on 2023/6/25.
//

#pragma once

#include <Toy/toy.h>

namespace toy
{
    class FileDialog
    {
    public:
        static std::string window_open_file_dialog(GLFWwindow* glfw_window, const char* title, const char* exts);

        static std::string window_save_file_dialog(GLFWwindow* glfw_window, const char* title, const char* exts);
    };
}
