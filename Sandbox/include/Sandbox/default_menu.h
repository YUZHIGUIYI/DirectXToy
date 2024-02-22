//
// Created by ZZK on 2023/7/1.
//

#pragma once

#include <Toy/toy.h>

namespace toy
{
    struct DefaultMenu final : public ILayer
    {
    public:
        void on_attach(D3DApplication* app) override
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
            auto&& input_controller = InputController::get();
            if (input_controller.is_key_pressed_with_mod(key::Q, key::LeftControl))
            {
                close_d3d_app = true;
            }

            if (close_d3d_app)
            {
                m_d3d_app->on_close(WindowCloseEvent{});
            }
        }

    private:
        D3DApplication* m_d3d_app = nullptr;
    };
}
