//
// Created by ZZK on 2024/3/13.
//

#pragma once

#include <Sandbox/Core/base.h>

namespace toy::editor
{
    struct DockingSystem;

    struct Dock
    {
        // Interface
        virtual void on_resize() {}                                         // Called when the viewport size is changing
        virtual void on_update(float delta_time) {}                         // Update one frame
        virtual void on_render(float delta_time) {}                         // For anything to render within a frame
        virtual void on_file_drop(std::string_view filename) {}             // For when a file is dragged on top of the window

        Dock() = default;
        virtual ~Dock() = default;

        Dock(const Dock &) = delete;
        Dock &operator=(const Dock &) = delete;
        Dock(Dock &&) = delete;
        Dock &operator=(Dock &&) = delete;
    };
}
