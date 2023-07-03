//
// Created by ZZK on 2023/6/25.
//

#pragma once

#include <Toy/Core/base.h>

namespace toy
{
    class d3d_application_c;

    struct ILayer
    {
        // Interface
        virtual void on_attach(d3d_application_c* app) {}                   // Called once at start
        virtual void on_detach() {}                                         // Called before destroying the application
        virtual void on_resize() {}                                         // Called when the viewport size is changing
        virtual void on_ui_render() {}                                      // Called for anything related to UI
        virtual void on_ui_menu() {}                                        // This is the menubar to create
        virtual void on_render(float dt) {}                                 // For anything to render within a frame
        virtual void on_file_drop(std::string_view filename) {}             // For when a file is dragged on top of the window

        virtual ~ILayer() = default;
    };
}