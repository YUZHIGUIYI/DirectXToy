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
        std::vector<spdlog::sink_ptr> logSinks;
        logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
        logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("DXToy.log", true));

        logSinks[0]->set_pattern("%^[%T] %n: %v%$");
        logSinks[1]->set_pattern("[%T] [%l] %n: %v");

        class_core_logger_ = std::make_shared<spdlog::logger>("DXToy", std::begin(logSinks), std::end(logSinks));
        spdlog::register_logger(class_core_logger_);
        class_core_logger_->set_level(spdlog::level::trace);
        class_core_logger_->flush_on(spdlog::level::trace);

        class_client_logger_ = std::make_shared<spdlog::logger>("Sandbox", std::begin(logSinks), std::end(logSinks));
        spdlog::register_logger(class_client_logger_);
        class_client_logger_->set_level(spdlog::level::trace);
        class_client_logger_->flush_on(spdlog::level::trace);
    }
}