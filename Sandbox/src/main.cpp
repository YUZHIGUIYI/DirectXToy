//
// Created by ZHIKANG on 2023/5/17.
//

#include <Sandbox/pbr_viewer.h>
#include <Sandbox/default_menu.h>

int main()
{
    if (!glfwInit())
    {
        DX_CRITICAL("Fail to initialize GLFW");
    }

    // Create window
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    std::pair<int32_t, int32_t> window_size{ 1600, 900 };
    auto window = glfwCreateWindow(window_size.first, window_size.second, "DX11Renderer", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        DX_CRITICAL("Failed to create GLFW window");
    }

    // Create and initialize d3d application
    auto d3d_app = std::make_unique<toy::D3DApplication>(window, window_size.first, window_size.second);
    d3d_app->init();

    // Create layers of d3d application
    d3d_app->add_layer(std::make_shared<toy::viewer::PBRViewer>("PBRViewer"));
    d3d_app->add_layer(std::make_shared<toy::DefaultMenu>());

    // Tick d3d application
    d3d_app->tick();

    glfwDestroyWindow(window);
    glfwTerminate();
}



