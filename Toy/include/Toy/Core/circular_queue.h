//
// Created by ZZK on 2024/1/21.
//

#pragma once

#include <Toy/Core/base.h>

namespace toy::details
{
    template <typename T>
    class CircularQueue
    {
    private:
        size_t m_max_items = 0;
        size_t m_head = 0;
        size_t m_tail = 0;
        size_t m_overrun_counter = 0;
        std::vector<T> m_vec_data;

    public:
        explicit CircularQueue(size_t max_items);

        CircularQueue(const CircularQueue &other);
        CircularQueue &operator=(const CircularQueue &other);

        CircularQueue(CircularQueue &&other) noexcept;
        CircularQueue &operator=(CircularQueue &&other) noexcept;

        void push_back(T &&item);

        const T &front() const;

        T &front();

        [[nodiscard]] size_t size() const;

        const T &at(size_t index) const;

        void pop_front();

        void clear();

        [[nodiscard]] bool empty() const;

        [[nodiscard]] bool full() const;

        [[nodiscard]] size_t overrun_counter() const;

        void reset_overrun_counter();

    private:
        void copy_movable(CircularQueue &&other) noexcept;
    };

    template <typename T>
    CircularQueue<T>::CircularQueue(size_t max_items)
    : m_max_items(max_items + 1), m_vec_data(max_items + 1)
    {

    }

    template <typename T>
    CircularQueue<T>::CircularQueue(const CircularQueue &other) = default;

    template <typename T>
    CircularQueue<T> &CircularQueue<T>::operator=(const CircularQueue &other) = default;

    template <typename T>
    CircularQueue<T>::CircularQueue(CircularQueue &&other) noexcept
    {
        copy_movable(std::move(other));
    }

    template <typename T>
    CircularQueue<T> &CircularQueue<T>::operator=(CircularQueue &&other) noexcept
    {
        copy_movable(std::move(other));
        return *this;
    }

    template <typename T>
    void CircularQueue<T>::push_back(T &&item)
    {
        if (m_max_items > 0)
        {
            m_vec_data[m_tail] = std::move(item);
            m_tail = (m_tail + 1) % m_max_items;

            // Overrun last item if full
            if (m_tail == m_head)
            {
                m_head = (m_head + 1) % m_max_items;
                ++m_overrun_counter;
            }
        }
    }

    template <typename T>
    const T &CircularQueue<T>::front() const
    {
        return m_vec_data[m_head];
    }

    template <typename T>
    T &CircularQueue<T>::front()
    {
        return m_vec_data[m_head];
    }

    template <typename T>
    size_t CircularQueue<T>::size() const
    {
        if (m_tail >= m_head)
        {
            return m_tail - m_head;
        } else
        {
            return m_max_items - (m_head - m_tail);
        }
    }

    template <typename T>
    const T &CircularQueue<T>::at(size_t index) const
    {
        DX_CORE_ASSERT(index < size(), "Out of range in CircularQueue");
        return m_vec_data[(m_head + index) % m_max_items];
    }

    template <typename T>
    void CircularQueue<T>::pop_front()
    {
        m_head = (m_head + 1) % m_max_items;
    }

    template <typename T>
    void CircularQueue<T>::clear()
    {
        m_vec_data.clear();
        m_vec_data.resize(m_max_items + 1);
        m_head = 0;
        m_tail = 0;
    }

    template <typename T>
    bool CircularQueue<T>::empty() const
    {
        return m_tail == m_head;
    }

    template <typename T>
    bool CircularQueue<T>::full() const
    {
        if (m_max_items > 0)
        {
            return ((m_tail + 1) % m_max_items) == m_head;
        }
        return false;
    }

    template <typename T>
    size_t CircularQueue<T>::overrun_counter() const
    {
        return m_overrun_counter;
    }

    template <typename T>
    void CircularQueue<T>::reset_overrun_counter()
    {
        m_overrun_counter = 0;
    }

    template <typename T>
    void CircularQueue<T>::copy_movable(CircularQueue &&other) noexcept
    {
        m_max_items = other.m_max_items;
        m_head = other.m_head;
        m_tail = other.m_tail;
        m_overrun_counter = other.m_overrun_counter;
        m_vec_data = std::move(other.m_vec_data);

        other.m_max_items = 0;
        other.m_head = 0;
        other.m_tail = 0;
    }
}


























