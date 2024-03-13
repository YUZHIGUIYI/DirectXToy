//
// Created by ZHIKANG on 2023/5/19.
//

#pragma once

#include <Toy/Core/base.h>

namespace toy
{
    class Logger
    {
    private:
        Logger();

    public:
        Logger(const Logger &) = delete;
        Logger &operator=(const Logger &) = delete;
        Logger(Logger &&) = delete;
        Logger &operator=(Logger &&) = delete;

        static Logger &get();

        void register_console_sink(const spdlog::sink_ptr &console_sink);
    public:
        std::shared_ptr<spdlog::logger> m_core_logger;
        std::shared_ptr<spdlog::logger> m_client_logger;
    };
}

// Core log macros
#define DX_CORE_TRACE(...)		::toy::Logger::get().m_core_logger->trace(__VA_ARGS__)
#define DX_CORE_INFO(...)		::toy::Logger::get().m_core_logger->info(__VA_ARGS__)
#define DX_CORE_WARN(...)		::toy::Logger::get().m_core_logger->warn(__VA_ARGS__)
#define DX_CORE_ERROR(...)		::toy::Logger::get().m_core_logger->error(__VA_ARGS__)
#define DX_CORE_CRITICAL(...)	::toy::Logger::get().m_core_logger->critical(__VA_ARGS__); throw std::runtime_error("DXToy fatal error!")
#define DX_CORE_ASSERT(expression, ...)	\
if (!(expression)) { ::toy::Logger::get().m_core_logger->error(__VA_ARGS__); } assert(expression)

// Client log macros
#define DX_TRACE(...)			::toy::Logger::get().m_client_logger->trace(__VA_ARGS__)
#define DX_INFO(...)			::toy::Logger::get().m_client_logger->info(__VA_ARGS__)
#define DX_WARN(...)			::toy::Logger::get().m_client_logger->warn(__VA_ARGS__)
#define DX_ERROR(...)			::toy::Logger::get().m_client_logger->error(__VA_ARGS__)
#define DX_CRITICAL(...)		::toy::Logger::get().m_client_logger->critical(__VA_ARGS__); throw std::runtime_error("Sandbox fatal error!")
#define DX_ASSERT(expression, ...)	\
if (!(expression)) { ::toy::Logger::get().m_client_logger->error(__VA_ARGS__); } assert(expression)
