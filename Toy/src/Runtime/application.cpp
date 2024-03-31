//
// Created by ZZK on 2024/3/12.
//

#include <Toy/Runtime/application.h>
#include <Toy/Runtime/events.h>
#include <Toy/Core/subsystem.h>
#include <Toy/Runtime/renderer.h>
#include <Toy/Runtime/simulation.h>
#include <Toy/Runtime/input_controller.h>
#include <Toy/Runtime/render_window.h>
#include <Toy/Runtime/scene_graph.h>
#include <Toy/Runtime/task_system.h>

namespace toy::runtime
{
    void Application::start()
    {
        // TODO: add subsystems
        auto&& scene_graph = core::add_subsystem<SceneGraph>();
        auto&& task_system = core::add_subsystem<TaskSystem>();
        auto&& render_window = core::add_subsystem<RenderWindow>(1600, 900);
        auto&& renderer = core::add_subsystem<Renderer>(render_window.get_native_window(), render_window.get_window_width(), render_window.get_window_height());
        auto&& simulation = core::add_subsystem<Simulation>();
        auto&& input_controller = core::add_subsystem<InputController>();
        input_controller.register_event(render_window.get_native_window());
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
        auto&& simulation = core::get_subsystem<Simulation>();
        auto&& render_window = core::get_subsystem<RenderWindow>();
        auto&& input_controller = core::get_subsystem<InputController>();
        auto&& renderer = core::get_subsystem<Renderer>();

        simulation.tick();
        render_window.tick();
        input_controller.update_state();

        process_events();

        if (!m_is_running || renderer.is_renderer_minimized()) return;

        float delta_time = simulation.get_delta_ime();
        renderer.tick();

        on_frame_begin(delta_time);
        on_frame_update(delta_time);
        on_frame_render(delta_time);
        on_frame_end(delta_time);

        renderer.present();
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

    void Application::process_events()
    {
        auto&& render_window = core::get_subsystem<RenderWindow>();
        if (render_window.poll_window_close())
        {
            quit();
            return;
        }

        auto&& renderer = core::get_subsystem<Renderer>();
        renderer.process_pending_events();
    }

}