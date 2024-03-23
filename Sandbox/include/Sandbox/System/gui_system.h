//
// Created by ZZK on 2024/3/13.
//

#pragma once

#include <Toy/toy.h>

namespace toy::editor
{
    struct GuiSystem
    {
        GuiSystem();

        void frame_begin();

        void frame_end();

        void release();

        void set_dark_theme();

        void set_gizmo_style();

    private:
        bool has_released = false;
    };
}
