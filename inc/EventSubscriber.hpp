#pragma once

#include "EventManager.hpp"

#include <typeindex>
#include <vector>
#include <algorithm>

template <typename T>
class EventSubscriber
{
public:
    explicit EventSubscriber(EventManager<T>& manager)
     : m_manager(&manager)
    {}

    ~EventSubscriber()
    {
        unsubscribeAll();
    }

    EventSubscriber(const EventSubscriber&) = delete;
    EventSubscriber& operator=(const EventSubscriber&) = delete;

    EventSubscriber(EventSubscriber&& other) noexcept
     : m_manager(other.m_manager), m_tokens(std::move(other.m_tokens))
    {
        other.m_manager = nullptr;
    }

    template <typename U>
    void subscribe(T event, EventCallback_t<U> callback)
    {
        m_tokens.push_back(m_manager->subscribe(event, std::move(callback)));
    }

    void unsubscribe(T event)
    {
        m_tokens.erase(
            std::remove_if(m_tokens.begin(), m_tokens.end(),
                [&](const auto& token)
                {
                    if (token.event == event)
                    {
                        m_manager->unsubscribe(token);
                        return true;
                    }
                    return false;
                }),
            m_tokens.end());
    }

    void unsubscribe(T event, std::type_index dataType)
    {
        m_tokens.erase(
            std::remove_if(m_tokens.begin(), m_tokens.end(),
                [&](const auto& token)
                {
                    if (token.event == event && token.dataType == dataType)
                    {
                        m_manager->unsubscribe(token);
                        return true;
                    }
                    return false;
                }),
            m_tokens.end());
    }

    void unsubscribeAll()
    {
        if (m_manager)
        {
            for (auto& token : m_tokens)
            {
                m_manager->unsubscribe(token);
            }
            m_tokens.clear();
        }
    }

private:
    EventManager<T>* m_manager;
    std::vector<typename EventManager<T>::Token> m_tokens;
};
