//
// Created by ZHIKANG on 2023/6/4.
//

#pragma once

#include <Toy/Core/base.h>

namespace toy
{
    class ImGuiPass
    {
    public:
        static void init(GLFWwindow* glfw_window, ID3D11Device* device, ID3D11DeviceContext* device_context);

        static void begin();

        static void end();

        static void release();

        static void set_dark_theme();

        static void set_gizmo_style();
    };
}
