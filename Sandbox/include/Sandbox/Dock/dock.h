//
// Created by ZZK on 2024/3/13.
//

#pragma once

#include <Toy/toy.h>

namespace toy::editor
{
    struct DockingSystem;

    struct Dock
    {
        // Interface
        virtual void on_attach(DockingSystem *docking_system) {}            // Called once at start
        virtual void on_detach() {}                                         // Called before destroying the application
        virtual void on_resize() {}                                         // Called when the viewport size is changing
        virtual void on_ui_render() {}                                      // Called for anything related to UI
        virtual void on_ui_menu() {}                                        // This is the menubar to create
        virtual void on_render(float dt) {}                                 // For anything to render within a frame
        virtual void on_file_drop(std::string_view filename) {}             // For when a file is dragged on top of the window

        virtual ~Dock() = default;
    };
}
