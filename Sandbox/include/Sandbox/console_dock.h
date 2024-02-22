//
// Created by ZZK on 2024/1/22.
//

#pragma once

#include <Toy/toy.h>
#include <Toy/Core/ringbuffer_console_sink.h>

namespace toy
{
    struct ConsoleDock
    {
    public:
        explicit ConsoleDock();

        void on_console_render();

    private:
        std::shared_ptr<logger::RingbufferConsoleSinkMt> m_console_sink;
    };
}
