//
// Created by ZZK on 2024/3/13.
//

#pragma once

#include <Sandbox/Core/base.h>
#include <Toy/Runtime/application.h>

namespace toy::editor
{
    struct EditorApplication final : public runtime::Application
    {
    public:
        EditorApplication() = default;

        ~EditorApplication() override = default;

        void start() override;

        void setup() override;

        void stop() override;

        void on_frame_render(float delta_time);

    private:
        void on_docks_render(float delta_time);

        void on_menu_render();

        void on_start_page_render();

    private:
        bool m_show_start_page = true;
    };
}
