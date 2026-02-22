#include "EventManager.hpp"
#include "EventSubscriber.hpp"
#include "EventType.hpp"

#include <iostream>
#include <cstdint>

struct BatteryData
{
    int    voltage;
    double current;
};

struct ModbusData
{
    uint16_t reg1;
    uint16_t reg2;
};

struct Dummy
{
    int   x;
    float y;
};

int main()
{
    BatteryData batData   = {1, 100};
    ModbusData  modbusData = {15, 16};
    Dummy       dummy     = {1, 100.45f};

    EventManager<EventType>   manager;
    EventSubscriber<EventType> sub(manager);

    sub.subscribe<BatteryData>(EventType::EVENT_1, [](const BatteryData&)
    {
        std::cout << "Calling BatteryData event" << std::endl;
    });

    sub.subscribe<ModbusData>(EventType::EVENT_2, [](const ModbusData&)
    {
        std::cout << "Calling ModbusData event" << std::endl;
    });

    sub.subscribe<Dummy>(EventType::EVENT_3, [](const Dummy&)
    {
        std::cout << "Calling Dummy event" << std::endl;
    });

    sub.subscribe<Dummy>(EventType::EVENT_1, [](const Dummy&)
    {
        std::cout << "Calling Dummy event on EVENT_1" << std::endl;
    });

    // Dispatch — both BatteryData and Dummy callbacks fire for EVENT_1
    manager.publish(EventType::EVENT_1, batData);
    manager.publish(EventType::EVENT_2, modbusData);
    manager.publish(EventType::EVENT_3, dummy);
    manager.publish(EventType::EVENT_1, dummy);

    // Unsubscribe only BatteryData callbacks from EVENT_1
    sub.unsubscribe(EventType::EVENT_1, typeid(BatteryData));

    // Only Dummy callback fires for EVENT_1 now
    manager.publish(EventType::EVENT_1, batData);
    manager.publish(EventType::EVENT_2, modbusData);
    manager.publish(EventType::EVENT_3, dummy);
    manager.publish(EventType::EVENT_1, dummy);

    return 0;
}
