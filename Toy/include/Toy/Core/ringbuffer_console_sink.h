//
// Created by ZZK on 2024/1/21.
//

#pragma once

#include <Toy/Core/circular_queue.h>

namespace toy::logger
{
    // Sink for ImGui console
    template <typename Mutex>
    class RingbufferConsoleSink final : public spdlog::sinks::base_sink<Mutex>
    {
    public:
        explicit RingbufferConsoleSink(size_t max_items)
        : m_queue{ max_items }, m_dirty{ true }, m_has_new_entries{ false }
        {

        }

        std::vector<spdlog::details::log_msg_buffer> last_raw_data(size_t limit = 0)
        {
            std::lock_guard<Mutex> lock_guard(spdlog::sinks::base_sink<Mutex>::mutex_);
            auto items_available = m_queue.size();
            auto items = limit > 0 ? std::min(limit, items_available) : items_available;
            std::vector<spdlog::details::log_msg_buffer> results;
            results.reserve(items);
            for (size_t i = (items_available - items); i < items_available; ++i)
            {
                results.push_back(m_queue.at(i));
            }
            return results;
        }

        std::vector<std::pair<spdlog::level::level_enum, std::string>> last_formatted_data(size_t limit = 0)
        {
            std::lock_guard<Mutex> lock_guard(spdlog::sinks::base_sink<Mutex>::mutex_);
            auto item_available = m_queue.size();
            auto items = limit > 0 ? std::min(limit, item_available) : item_available;
            std::vector<std::pair<spdlog::level::level_enum, std::string>> results;
            results.reserve(items);
            for (size_t i = (item_available - items); i < item_available; ++i)
            {
                spdlog::memory_buf_t formatted;
                auto&& temp_msg_buffer = m_queue.at(i);
                spdlog::sinks::base_sink<Mutex>::formatter_->format(temp_msg_buffer, formatted);
                results.emplace_back(temp_msg_buffer.level, SPDLOG_BUF_TO_STRING(formatted));
            }
            return results;
        }

        void clear_log()
        {
            std::lock_guard<Mutex> lock_guard(spdlog::sinks::base_sink<Mutex>::mutex_);
            m_queue.clear();
            m_cache_log.clear();
            m_dirty.store(true, std::memory_order_release);
            m_has_new_entries.store(false, std::memory_order_release);
        }

        std::vector<std::pair<spdlog::level::level_enum, std::string>> &get_cache_log()
        {
            if (m_dirty.load(std::memory_order_acquire))
            {
                m_cache_log = std::move(last_formatted_data());
            }
            m_dirty.store(false, std::memory_order_release);
            return m_cache_log;
        }

        const std::array<float, 4> &get_level_colorization(spdlog::level::level_enum level)
        {
            static std::map<spdlog::level::level_enum, std::array<float, 4>> colorization_mappings{
                { spdlog::level::trace, std::array<float, 4>{1.0f, 1.0f, 1.0f, 1.0f} },
                { spdlog::level::debug, std::array<float, 4>{1.0f, 1.0f, 1.0f, 1.0f} },
                { spdlog::level::info, std::array<float, 4>{1.0f, 1.0f, 1.0f, 1.0f} },
                { spdlog::level::warn, std::array<float, 4>{1.0f, 0.494f, 0.0f, 1.0f} },
                { spdlog::level::err, std::array<float, 4>{1.0f, 0.0f, 0.0f, 1.0f} },
                { spdlog::level::critical, std::array<float, 4>{1.0f, 1.0f, 1.0f, 1.0f} },
                { spdlog::level::off, std::array<float, 4>{1.0f, 1.0f, 1.0f, 1.0f} },
            };

            return colorization_mappings[level];
        }

        bool has_new_entries() const
        {
            return m_has_new_entries.load(std::memory_order_acquire);
        }

    protected:
        void sink_it_(const spdlog::details::log_msg &msg) override
        {
            m_queue.push_back(spdlog::details::log_msg_buffer{ msg });
            m_dirty.store(true, std::memory_order_release);
            m_has_new_entries.store(true, std::memory_order_release);
        }

        void flush_() override
        {

        }

    private:
        details::CircularQueue<spdlog::details::log_msg_buffer> m_queue;
        std::vector<std::pair<spdlog::level::level_enum, std::string>> m_cache_log;
        std::atomic<bool> m_dirty;
        std::atomic<bool> m_has_new_entries;
    };

    using RingbufferConsoleSinkMt = RingbufferConsoleSink<std::mutex>;
    using RingbufferConsoleSinkSt = RingbufferConsoleSink<spdlog::details::null_mutex>;
}





























