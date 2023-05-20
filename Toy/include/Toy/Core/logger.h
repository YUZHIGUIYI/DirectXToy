//
// Created by ZHIKANG on 2023/5/19.
//

#pragma once

#include <Toy/Core/base.h>

namespace toy
{
    class logger_c : public disable_copyable_c
    {
    public:
        logger_c();

        std::shared_ptr<spdlog::logger> class_core_logger_;
        std::shared_ptr<spdlog::logger> class_client_logger_;
    };

    using logger_system = singleton_c<logger_c>;
}

// Core log macros
#define DX_CORE_TRACE(...)		::toy::logger_system::get().class_core_logger_->trace(__VA_ARGS__)
#define DX_CORE_INFO(...)		::toy::logger_system::get().class_core_logger_->info(__VA_ARGS__)
#define DX_CORE_WARN(...)		::toy::logger_system::get().class_core_logger_->warn(__VA_ARGS__)
#define DX_CORE_ERROR(...)		::toy::logger_system::get().class_core_logger_->error(__VA_ARGS__)
#define DX_CORE_CRITICAL(...)	::toy::logger_system::get().class_core_logger_->critical(__VA_ARGS__); throw std::runtime_error("DXToy fatal error!")

// Client log macros
#define DX_TRACE(...)			::toy::logger_system::get().class_client_logger_->trace(__VA_ARGS__)
#define DX_INFO(...)			::toy::logger_system::get().class_client_logger_->info(__VA_ARGS__)
#define DX_WARN(...)			::toy::logger_system::get().class_client_logger_->warn(__VA_ARGS__)
#define DX_ERROR(...)			::toy::logger_system::get().class_client_logger_->error(__VA_ARGS__)
#define DX_CRITICAL(...)		::toy::logger_system::get().class_client_logger_->critical(__VA_ARGS__); throw std::runtime_error("Sandbox fatal error!")
