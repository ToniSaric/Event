#include <gtest/gtest.h>
#include "EventManager.hpp"

enum class TestEvent { A, B, C };

namespace std
{
    template <>
    struct hash<TestEvent>
    {
        size_t operator()(const TestEvent& e) const
        {
            return std::hash<int>{}(static_cast<int>(e));
        }
    };
}

// ── publish — basic routing ───────────────────────────────────────────────────

TEST(EventManagerTest, PublishCallsMatchingCallback)
{
    EventManager<TestEvent> manager;
    bool called = false;
    manager.subscribe<int>(TestEvent::A, [&](const int&) { called = true; });
    manager.publish(TestEvent::A, 42);
    EXPECT_TRUE(called);
}

TEST(EventManagerTest, PublishDoesNotCallWrongEventCallback)
{
    EventManager<TestEvent> manager;
    bool called = false;
    manager.subscribe<int>(TestEvent::A, [&](const int&) { called = true; });
    manager.publish(TestEvent::B, 42);
    EXPECT_FALSE(called);
}

TEST(EventManagerTest, PublishDoesNotCallWrongTypeCallback)
{
    EventManager<TestEvent> manager;
    bool called = false;
    manager.subscribe<int>(TestEvent::A, [&](const int&) { called = true; });
    manager.publish(TestEvent::A, 3.14);  // double, not int
    EXPECT_FALSE(called);
}

TEST(EventManagerTest, CallbackReceivesCorrectData)
{
    EventManager<TestEvent> manager;
    int received = 0;
    manager.subscribe<int>(TestEvent::A, [&](const int& val) { received = val; });
    manager.publish(TestEvent::A, 99);
    EXPECT_EQ(received, 99);
}

TEST(EventManagerTest, CallbackFiredOnEachPublish)
{
    EventManager<TestEvent> manager;
    int count = 0;
    manager.subscribe<int>(TestEvent::A, [&](const int&) { count++; });
    manager.publish(TestEvent::A, 1);
    manager.publish(TestEvent::A, 2);
    manager.publish(TestEvent::A, 3);
    EXPECT_EQ(count, 3);
}

// ── publish — multiple callbacks ──────────────────────────────────────────────

TEST(EventManagerTest, MultipleCallbacksSameEventSameType)
{
    EventManager<TestEvent> manager;
    int count = 0;
    manager.subscribe<int>(TestEvent::A, [&](const int&) { count++; });
    manager.subscribe<int>(TestEvent::A, [&](const int&) { count++; });
    manager.publish(TestEvent::A, 42);
    EXPECT_EQ(count, 2);
}

TEST(EventManagerTest, MultipleCallbacksSameEventDifferentTypes)
{
    EventManager<TestEvent> manager;
    int intCount = 0, doubleCount = 0;
    manager.subscribe<int>(TestEvent::A, [&](const int&) { intCount++; });
    manager.subscribe<double>(TestEvent::A, [&](const double&) { doubleCount++; });
    manager.publish(TestEvent::A, 42);
    manager.publish(TestEvent::A, 3.14);
    EXPECT_EQ(intCount, 1);
    EXPECT_EQ(doubleCount, 1);
}

TEST(EventManagerTest, MultipleEventsAreIndependent)
{
    EventManager<TestEvent> manager;
    int countA = 0, countB = 0;
    manager.subscribe<int>(TestEvent::A, [&](const int&) { countA++; });
    manager.subscribe<int>(TestEvent::B, [&](const int&) { countB++; });
    manager.publish(TestEvent::A, 1);
    EXPECT_EQ(countA, 1);
    EXPECT_EQ(countB, 0);
    manager.publish(TestEvent::B, 1);
    EXPECT_EQ(countA, 1);
    EXPECT_EQ(countB, 1);
}

TEST(EventManagerTest, CallbacksForSameEventFiredInSubscriptionOrder)
{
    EventManager<TestEvent> manager;
    std::vector<int> order;
    manager.subscribe<int>(TestEvent::A, [&](const int&) { order.push_back(1); });
    manager.subscribe<int>(TestEvent::A, [&](const int&) { order.push_back(2); });
    manager.subscribe<int>(TestEvent::A, [&](const int&) { order.push_back(3); });
    manager.publish(TestEvent::A, 0);
    EXPECT_EQ(order, (std::vector<int>{1, 2, 3}));
}

// ── publish — nested dispatch ─────────────────────────────────────────────────

TEST(EventManagerTest, PublishFromWithinCallbackFiresOtherEvent)
{
    EventManager<TestEvent> manager;
    int countB = 0;
    manager.subscribe<int>(TestEvent::B, [&](const int&) { countB++; });
    manager.subscribe<int>(TestEvent::A, [&](const int& val)
    {
        manager.publish(TestEvent::B, val);  // publish a different event
    });
    manager.publish(TestEvent::A, 1);
    EXPECT_EQ(countB, 1);
}

// ── unsubscribe — by token ────────────────────────────────────────────────────

TEST(EventManagerTest, UnsubscribeByTokenRemovesOnlyThatCallback)
{
    EventManager<TestEvent> manager;
    int count = 0;
    auto token = manager.subscribe<int>(TestEvent::A, [&](const int&) { count++; });
    manager.subscribe<int>(TestEvent::A, [&](const int&) { count++; });

    manager.publish(TestEvent::A, 1);
    EXPECT_EQ(count, 2);

    manager.unsubscribe(token);
    manager.publish(TestEvent::A, 1);
    EXPECT_EQ(count, 3);  // only second callback fires
}

TEST(EventManagerTest, DoubleUnsubscribeSameTokenDoesNotCrash)
{
    EventManager<TestEvent> manager;
    auto token = manager.subscribe<int>(TestEvent::A, [](const int&) {});
    manager.unsubscribe(token);
    EXPECT_NO_THROW(manager.unsubscribe(token));
}

// ── unsubscribe — by event ────────────────────────────────────────────────────

TEST(EventManagerTest, UnsubscribeByEventRemovesAllCallbacks)
{
    EventManager<TestEvent> manager;
    int count = 0;
    manager.subscribe<int>(TestEvent::A, [&](const int&) { count++; });
    manager.subscribe<int>(TestEvent::A, [&](const int&) { count++; });

    manager.publish(TestEvent::A, 1);
    EXPECT_EQ(count, 2);

    manager.unsubscribe(TestEvent::A);
    manager.publish(TestEvent::A, 1);
    EXPECT_EQ(count, 2);  // neither fires
}

TEST(EventManagerTest, UnsubscribeByEventDoesNotAffectOtherEvents)
{
    EventManager<TestEvent> manager;
    int countA = 0, countB = 0;
    manager.subscribe<int>(TestEvent::A, [&](const int&) { countA++; });
    manager.subscribe<int>(TestEvent::B, [&](const int&) { countB++; });

    manager.unsubscribe(TestEvent::A);
    manager.publish(TestEvent::A, 1);
    manager.publish(TestEvent::B, 1);

    EXPECT_EQ(countA, 0);
    EXPECT_EQ(countB, 1);
}

// ── edge cases ────────────────────────────────────────────────────────────────

TEST(EventManagerTest, PublishToEventWithNoSubscribersDoesNotThrow)
{
    EventManager<TestEvent> manager;
    EXPECT_NO_THROW(manager.publish(TestEvent::A, 42));
}

TEST(EventManagerTest, UnsubscribeNonExistentEventDoesNotThrow)
{
    EventManager<TestEvent> manager;
    EXPECT_NO_THROW(manager.unsubscribe(TestEvent::A));
}

TEST(EventManagerTest, SubscribeAfterPublishDoesNotFireForPastEvents)
{
    EventManager<TestEvent> manager;
    manager.publish(TestEvent::A, 42);  // no subscribers yet
    bool called = false;
    manager.subscribe<int>(TestEvent::A, [&](const int&) { called = true; });
    EXPECT_FALSE(called);  // late subscriber never sees past events
}

TEST(EventManagerTest, PublishAfterUnsubscribeAllDoesNotThrow)
{
    EventManager<TestEvent> manager;
    manager.subscribe<int>(TestEvent::A, [](const int&) {});
    manager.unsubscribe(TestEvent::A);
    EXPECT_NO_THROW(manager.publish(TestEvent::A, 1));
}

TEST(EventManagerTest, ResubscribeAfterUnsubscribeWorks)
{
    EventManager<TestEvent> manager;
    int count = 0;
    auto token = manager.subscribe<int>(TestEvent::A, [&](const int&) { count++; });
    manager.publish(TestEvent::A, 1);
    EXPECT_EQ(count, 1);

    manager.unsubscribe(token);
    manager.publish(TestEvent::A, 1);
    EXPECT_EQ(count, 1);  // unsubscribed

    manager.subscribe<int>(TestEvent::A, [&](const int&) { count++; });
    manager.publish(TestEvent::A, 1);
    EXPECT_EQ(count, 2);  // re-subscribed and fires
}

TEST(EventManagerTest, LargeNumberOfSubscribersAllFire)
{
    EventManager<TestEvent> manager;
    int count = 0;
    const int N = 100;
    for (int i = 0; i < N; ++i)
    {
        manager.subscribe<int>(TestEvent::A, [&](const int&) { count++; });
    }
    manager.publish(TestEvent::A, 0);
    EXPECT_EQ(count, N);
}
