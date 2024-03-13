//
// Created by ZZK on 2024/3/12.
//

#include <Toy/Runtime/application.h>
#include <Toy/Runtime/events.h>
#include <Toy/Core/subsystem.h>

namespace toy::runtime
{
    void Application::start()
    {
        // TODO: add subsystems
    }

    void Application::setup()
    {

    }

    void Application::stop()
    {

    }

    void Application::tick()
    {
        // TODO: update subsystems

        float dt = 0.005f;
        on_frame_begin(dt);
        on_frame_update(dt);
        on_frame_render(dt);
        on_frame_render(dt);
        on_frame_ui_render(dt);
        on_frame_end(dt);
    }

    void Application::run()
    {
        core::details::initialize();

        setup();

        start();

        while (m_is_running)
        {
            tick();
        }

        stop();

        core::details::dispose();
    }

    void Application::quit()
    {
        m_is_running = false;
    }

}