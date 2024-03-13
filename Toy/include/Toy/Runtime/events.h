//
// Created by ZZK on 2024/3/12.
//

#pragma once

#include <Toy/Core/base.h>

namespace toy::runtime
{
    inline std::function<void(float)> on_frame_begin;

    inline std::function<void(float)> on_frame_update;

    inline std::function<void(float)> on_frame_render;

    inline std::function<void(float)> on_frame_ui_render;

    inline std::function<void(float)> on_frame_end;
}
