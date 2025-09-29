#pragma once

#include <typeindex>

class EventCallbackIf
{
    public:
        virtual ~EventCallbackIf() = default;
        virtual std::type_index GetDataType() const = 0;
};