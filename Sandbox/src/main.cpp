//
// Created by ZHIKANG on 2023/5/17.
//

#include <Sandbox/game_app.h>

int main()
{
    if (!glfwInit())
    {
        throw std::runtime_error("Can not initialize GLFW");
    }

    // Create window
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    auto window = glfwCreateWindow(1280, 720, "DX", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        throw std::runtime_error("Can not create window");
    }

    toy::game_app_c game_app{ window, "DX11Render", 1280, 720 };
    game_app.init();
    game_app.tick();

    glfwTerminate();
}



