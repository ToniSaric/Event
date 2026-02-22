#pragma once

#include <functional>

enum class EventType
{
    EVENT_1,
    EVENT_2,
    EVENT_3,
};

namespace std
{
    template <>
    struct hash<EventType>
    {
        std::size_t operator()(const EventType& eventType) const
        {
            return std::hash<int>{}(static_cast<int>(eventType));
        }
    };
}