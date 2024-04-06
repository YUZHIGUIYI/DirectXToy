//
// Created by ZZK on 2024/3/17.
//

#pragma once

#include <Sandbox/Docks/dock.h>
#include <Toy/Core/ringbuffer_console_sink.h>

namespace toy::editor
{
    struct AdminConsoleDock final : Dock
    {
    public:
        explicit AdminConsoleDock(std::string &&dock_name);

        ~AdminConsoleDock() override = default;

        void on_render(float delta_time) override;

    private:
        std::string m_dock_name;
        std::shared_ptr<logger::RingbufferConsoleSinkMt> m_console_sink;
    };
}
