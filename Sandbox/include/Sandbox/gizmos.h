//
// Created by ZZK on 2023/12/4.
//

#pragma once

#include <Sandbox/viewer_specification.h>

namespace toy
{
    struct Gizmos
    {
    public:
        explicit Gizmos(GLFWwindow *glfw_window);

        void on_gizmos_render(Entity &selected_entity, const ViewerSpecification &viewer_specification, std::shared_ptr<Camera> camera);

    private:
        GLFWwindow* m_glfw_window = nullptr;
    };
}
