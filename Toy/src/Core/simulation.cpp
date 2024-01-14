//
// Created by ZZK on 2024/1/14.
//

#include <Toy/Core/simulation.h>

namespace toy
{
    Simulation::Simulation() = default;

    void Simulation::run_one_frame(bool is_active)
    {
        auto elapsed_time = std::chrono::steady_clock::now() - m_last_frame_time_point;
        m_last_frame_time_point = std::chrono::steady_clock::now();
        m_time_step = elapsed_time;
        ++m_frame;
    }

    std::chrono::steady_clock::duration Simulation::get_time_since_launch() const
    {
        return std::chrono::steady_clock::now() - m_launch_time_point;
    }

    float Simulation::get_delta_ime() const
    {
        auto delta_time = std::chrono::duration_cast<std::chrono::duration<float>>(m_time_step).count();
        return delta_time;
    }

    uint32_t Simulation::get_fps() const
    {
        auto delta_time = std::chrono::duration_cast<std::chrono::duration<float, std::milli>>(m_time_step).count();
        return delta_time == 0.0f ? 0 : static_cast<uint32_t>(1000.0f / delta_time);
    }
}