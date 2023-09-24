//
// Created by ZHIKANG on 2023/5/19.
//

#pragma once

#include <Toy/Core/base.h>

namespace toy
{
    class Logger : public disable_copyable_c
    {
    public:
        Logger();

        static Logger &get();

    public:
        std::shared_ptr<spdlog::logger> class_core_logger_;
        std::shared_ptr<spdlog::logger> class_client_logger_;
    };
}

// Core log macros
#define DX_CORE_TRACE(...)		::toy::Logger::get().class_core_logger_->trace(__VA_ARGS__)
#define DX_CORE_INFO(...)		::toy::Logger::get().class_core_logger_->info(__VA_ARGS__)
#define DX_CORE_WARN(...)		::toy::Logger::get().class_core_logger_->warn(__VA_ARGS__)
#define DX_CORE_ERROR(...)		::toy::Logger::get().class_core_logger_->error(__VA_ARGS__)
#define DX_CORE_CRITICAL(...)	::toy::Logger::get().class_core_logger_->critical(__VA_ARGS__); throw std::runtime_error("DXToy fatal error!")

// Client log macros
#define DX_TRACE(...)			::toy::Logger::get().class_client_logger_->trace(__VA_ARGS__)
#define DX_INFO(...)			::toy::Logger::get().class_client_logger_->info(__VA_ARGS__)
#define DX_WARN(...)			::toy::Logger::get().class_client_logger_->warn(__VA_ARGS__)
#define DX_ERROR(...)			::toy::Logger::get().class_client_logger_->error(__VA_ARGS__)
#define DX_CRITICAL(...)		::toy::Logger::get().class_client_logger_->critical(__VA_ARGS__); throw std::runtime_error("Sandbox fatal error!")
