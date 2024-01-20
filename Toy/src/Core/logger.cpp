//
// Created by ZHIKANG on 2023/5/19.
//

#include <Toy/Core/logger.h>


namespace toy
{
    Logger& Logger::get()
    {
        static Logger logger{};
        return logger;
    }

    Logger::Logger()
    {
        // Log messages are additionally sent to Hazel.log file
        // Just one console sink
        // One sink and one log file only, different patterns
        std::vector<spdlog::sink_ptr> log_sinks;
        log_sinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
        log_sinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("DXToy.log", true));

        log_sinks[0]->set_pattern("%^[%T] %n: %v%$");
        log_sinks[1]->set_pattern("[%T] [%l] %n: %v");

        m_core_logger = std::make_shared<spdlog::logger>("DXToy", std::begin(log_sinks), std::end(log_sinks));
        spdlog::register_logger(m_core_logger);
        m_core_logger->set_level(spdlog::level::debug);
        m_core_logger->flush_on(spdlog::level::debug);

        m_client_logger = std::make_shared<spdlog::logger>("Sandbox", std::begin(log_sinks), std::end(log_sinks));
        spdlog::register_logger(m_client_logger);
        m_client_logger->set_level(spdlog::level::trace);
        m_client_logger->flush_on(spdlog::level::trace);
    }
}