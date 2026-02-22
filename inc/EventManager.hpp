#pragma once

#include "EventCallback.hpp"

#include <typeindex>
#include <vector>
#include <unordered_map>
#include <memory>
#include <algorithm>

template <typename T>
class EventManager
{
public:
    struct Token
    {
        T event;
        std::type_index dataType;
        size_t id;
    };

    EventManager() = default;
    EventManager(const EventManager&) = delete;
    EventManager& operator=(const EventManager&) = delete;

    template <typename U>
    Token subscribe(T event, EventCallback_t<U> callback)
    {
        size_t id = m_nextId++;
        m_callbacks[event].push_back({id, std::make_unique<EventCallback<U>>(std::move(callback))});
        return {event, typeid(U), id};
    }

    void unsubscribe(const Token& token)
    {
        auto it = m_callbacks.find(token.event);
        if (it != m_callbacks.end())
        {
            auto& vec = it->second;
            vec.erase(std::remove_if(vec.begin(), vec.end(),
                      [&token](const Entry& e) { return e.id == token.id; }),
                      vec.end());
        }
    }

    void unsubscribe(T event)
    {
        m_callbacks.erase(event);
    }

    template <typename U>
    void publish(T event, const U& data)
    {
        auto it = m_callbacks.find(event);
        if (it != m_callbacks.end())
        {
            for (const auto& entry : it->second)
            {
                if (entry.callback->getDataType() == typeid(U))
                {
                    auto* cb = static_cast<EventCallback<U>*>(entry.callback.get());
                    cb->invoke(data);
                }
            }
        }
    }

private:
    struct Entry
    {
        size_t id;
        std::unique_ptr<EventCallbackIf> callback;
    };

    using callbackMap_t = std::unordered_map<T, std::vector<Entry>>;
    callbackMap_t m_callbacks;
    size_t m_nextId{0};
};
