#pragma once

#include "EventCallback.hpp"
#include "EventType.hpp"

#include <iostream>
#include <vector>
#include <unordered_map>
#include <memory>

class EventSubscriber
{
    using callbackMap_t = std::unordered_map<EventType, std::vector<std::unique_ptr<EventCallbackIf>>>;

    public:
        EventSubscriber()
        {}

        template <typename T>
        void Subscribe(EventType event, EventCallback_t<T> &&callback)
        {
            m_callbacks[event].push_back(std::make_unique<EventCallback<T>>(std::forward<EventCallback_t<T>>(callback)));
        }

        void Unsubscribe(EventType event)
        {
            auto it = m_callbacks.find(event);
            if (it != m_callbacks.end())
            {
                m_callbacks.erase(it);
            }
        }

        void Unsubscribe(EventType event, std::type_index typeIndex)
        {
            auto it = m_callbacks.find(event);
            if (it != m_callbacks.end())
            {
                auto &callbacks = it->second;
                callbacks.erase(std::remove_if(callbacks.begin(), callbacks.end(),
                                [typeIndex](const std::unique_ptr<EventCallbackIf>& callback)
                                {
                                    return callback->GetDataType() == typeIndex;
                                }),
                                callbacks.end());
            }
        }

        template <typename T>
        void Update(EventType event, const T& data)
        {
            auto it = m_callbacks.find(event);
            if (it != m_callbacks.end())
            {
                for (const auto& callback : it->second)
                {
                    if (callback->GetDataType() == typeid(T))
                    {
                        auto *cb = static_cast<EventCallback<T>*>(callback.get());
                        cb->Invoke(data);
                    }
                }
            }
        }

    private:
        callbackMap_t m_callbacks;
};