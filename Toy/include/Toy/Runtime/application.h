//
// Created by ZZK on 2024/3/12.
//

#pragma once

#include <Toy/Core/base.h>

namespace toy::runtime
{
    struct Application
    {
    public:
        Application() = default;

        virtual ~Application() = default;

        Application(const Application &) = delete;
        Application& operator=(const Application &) = delete;
        Application(Application &&) = delete;
        Application& operator=(Application &&) = delete;

        virtual void start();

        virtual void setup();

        virtual void stop();

        virtual void tick();

        void run();

        void quit();

    private:
        bool m_is_running = true;
    };
}
