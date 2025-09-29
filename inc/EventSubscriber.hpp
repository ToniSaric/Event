#pragma once

#include "EventCallback.hpp"
#include "EventType.hpp"

#include <iostream>
#include <vector>
#include <unordered_map>
#include <memory>


template <typename T>
class EventSubscriber
{
    using callbackMap_t = std::unordered_map<T, std::vector<std::unique_ptr<EventCallbackIf>>>;

    public:
        EventSubscriber()
        {}

        template <typename U>
        void Subscribe(EventType event, EventCallback_t<U> &&callback)
        {
            m_callbacks[event].push_back(std::make_unique<EventCallback<U>>(std::forward<EventCallback_t<U>>(callback)));
        }

        void Unsubscribe(T event)
        {
            auto it = m_callbacks.find(event);
            if (it != m_callbacks.end())
            {
                m_callbacks.erase(it);
            }
        }

        void Unsubscribe(T event, std::type_index typeIndex)
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

        template <typename U>
        void Update(EventType event, const U& data)
        {
            auto it = m_callbacks.find(event);
            if (it != m_callbacks.end())
            {
                for (const auto& callback : it->second)
                {
                    if (callback->GetDataType() == typeid(U))
                    {
                        auto *cb = static_cast<EventCallback<U>*>(callback.get());
                        cb->Invoke(data);
                    }
                }
            }
        }

    private:
        callbackMap_t m_callbacks;
};