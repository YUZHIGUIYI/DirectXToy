//
// Created by ZHIKANG on 2023/5/17.
//

#include <Sandbox/sandbox.h>

int main()
{
    if (!glfwInit())
    {
        DX_CRITICAL("Can not initialize GLFW");
    }

    // Create window
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    auto window = glfwCreateWindow(1280, 720, "DX11Render", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        DX_CRITICAL("Can not create window");
    }

    toy::sandbox_c app{ window, "DX11Render", 1280, 720 };
    app.init();
    app.tick();

    glfwTerminate();
}



