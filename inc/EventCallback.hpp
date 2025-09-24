#pragma once

#include "EventCallbackIf.hpp"
#include <functional>

template <typename T>
using EventCallback_t = std::function<void(const T&)>;

template <typename T>
class EventCallback : public EventCallbackIf
{
    public:
        EventCallback(EventCallback_t<T> callback) : m_callback {callback} {}

        void Invoke(const T& event)
        {
            m_callback(event);
        }

    private:
        EventCallback_t<T> m_callback;
};