//
// Created by ZZK on 2023/7/1.
//

#pragma once

#include <Toy/toy.h>

namespace toy
{
    class DefaultMenu final : public ILayer
    {
    public:
        void on_attach(d3d_application_c* app) override
        {
            m_d3d_app = app;
        }

        void on_detach() override
        {

        }

        void on_ui_menu() override
        {
            static bool close_d3d_app = false;
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Exit", "Ctrl+Q"))
                {
                    close_d3d_app = true;
                }
                ImGui::EndMenu();
            }

            // Shortcuts
            GLFWwindow* glfw_window = m_d3d_app->get_glfw_window();
            if (DX_INPUT::is_key_pressed(glfw_window, key::Q) && DX_INPUT::is_key_pressed(glfw_window, key::LeftControl))
            {
                close_d3d_app = true;
            }

            if (close_d3d_app)
            {
                m_d3d_app->on_close(window_close_event_c{});
            }
        }

    private:
        d3d_application_c* m_d3d_app = nullptr;
    };
}
