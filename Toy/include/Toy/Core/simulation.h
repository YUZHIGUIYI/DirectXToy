//
// Created by ZZK on 2024/1/14.
//

#pragma once

#include <Toy/Core/base.h>

namespace toy
{
    class Simulation
    {
    public:
        Simulation();

        void run_one_frame(bool is_active = true);
        [[nodiscard]] std::chrono::steady_clock::duration get_time_since_launch() const;
        [[nodiscard]] float get_delta_ime() const;
        [[nodiscard]] uint32_t get_fps() const;

    private:
        std::chrono::steady_clock::duration m_time_step = std::chrono::steady_clock::duration::zero();
        std::chrono::steady_clock::time_point m_last_frame_time_point = std::chrono::steady_clock::now();
        std::chrono::steady_clock::time_point m_launch_time_point = std::chrono::steady_clock::now();
        uint64_t m_frame = 0;
    };
}
