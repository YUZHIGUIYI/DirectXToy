//
// Created by ZZK on 2024/3/13.
//

#pragma once

#include <Sandbox/Core/base.h>

namespace toy::editor
{
    struct GuiSystem
    {
        GuiSystem();

        ~GuiSystem();

        GuiSystem(const GuiSystem &) = delete;
        GuiSystem &operator=(const GuiSystem &) = delete;
        GuiSystem(GuiSystem &&) = delete;
        GuiSystem& operator=(GuiSystem &&) = delete;

        void frame_begin();

        void frame_end();

        void set_dark_theme();

        void set_gizmo_style();
    };
}
