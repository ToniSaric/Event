#pragma once

#include <functional>

using EventCallback_t = std::function<void(const T&)>;

template <typename T>
class EventCallback
{
    public:
        EventCallback(EventCallback_t callback) : m_callback {callback} {}

        void invoke(const T& event)
        {
            m_callback(event);
        }
    private:
        EventCallback_t m_callback;
}