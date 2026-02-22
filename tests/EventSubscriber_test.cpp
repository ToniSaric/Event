#include <gtest/gtest.h>
#include "EventSubscriber.hpp"

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

// ── basic subscribe ───────────────────────────────────────────────────────────

TEST(EventSubscriberTest, SubscribeDelegatesToManager)
{
    EventManager<TestEvent> manager;
    EventSubscriber<TestEvent> sub(manager);
    bool called = false;
    sub.subscribe<int>(TestEvent::A, [&](const int&) { called = true; });
    manager.publish(TestEvent::A, 42);
    EXPECT_TRUE(called);
}

TEST(EventSubscriberTest, SubscribeSameEventMultipleTimesAllFire)
{
    EventManager<TestEvent> manager;
    EventSubscriber<TestEvent> sub(manager);
    int count = 0;
    sub.subscribe<int>(TestEvent::A, [&](const int&) { count++; });
    sub.subscribe<int>(TestEvent::A, [&](const int&) { count++; });
    manager.publish(TestEvent::A, 1);
    EXPECT_EQ(count, 2);
}

TEST(EventSubscriberTest, SubscribeDifferentEventsAllFire)
{
    EventManager<TestEvent> manager;
    EventSubscriber<TestEvent> sub(manager);
    int countA = 0, countB = 0;
    sub.subscribe<int>(TestEvent::A, [&](const int&) { countA++; });
    sub.subscribe<int>(TestEvent::B, [&](const int&) { countB++; });
    manager.publish(TestEvent::A, 1);
    manager.publish(TestEvent::B, 1);
    EXPECT_EQ(countA, 1);
    EXPECT_EQ(countB, 1);
}

TEST(EventSubscriberTest, MultipleSubscribersToSameEventBothFire)
{
    EventManager<TestEvent> manager;
    int count1 = 0, count2 = 0;
    EventSubscriber<TestEvent> sub1(manager);
    EventSubscriber<TestEvent> sub2(manager);
    sub1.subscribe<int>(TestEvent::A, [&](const int&) { count1++; });
    sub2.subscribe<int>(TestEvent::A, [&](const int&) { count2++; });
    manager.publish(TestEvent::A, 1);
    EXPECT_EQ(count1, 1);
    EXPECT_EQ(count2, 1);
}

// ── RAII ──────────────────────────────────────────────────────────────────────

TEST(EventSubscriberTest, DestructorAutoUnsubscribes)
{
    EventManager<TestEvent> manager;
    int count = 0;
    {
        EventSubscriber<TestEvent> sub(manager);
        sub.subscribe<int>(TestEvent::A, [&](const int&) { count++; });
        manager.publish(TestEvent::A, 1);
        EXPECT_EQ(count, 1);
    }  // sub destroyed — auto-unsubscribes
    manager.publish(TestEvent::A, 1);
    EXPECT_EQ(count, 1);  // not called again
}

TEST(EventSubscriberTest, DestructorWithNoSubscriptionsDoesNotCrash)
{
    EventManager<TestEvent> manager;
    EXPECT_NO_THROW(
    {
        EventSubscriber<TestEvent> sub(manager);
    });
}

TEST(EventSubscriberTest, OneSubscriberDestroyedOtherStillWorks)
{
    EventManager<TestEvent> manager;
    int count1 = 0, count2 = 0;
    EventSubscriber<TestEvent> sub2(manager);
    sub2.subscribe<int>(TestEvent::A, [&](const int&) { count2++; });
    {
        EventSubscriber<TestEvent> sub1(manager);
        sub1.subscribe<int>(TestEvent::A, [&](const int&) { count1++; });
        manager.publish(TestEvent::A, 1);
        EXPECT_EQ(count1, 1);
        EXPECT_EQ(count2, 1);
    }  // sub1 destroyed
    manager.publish(TestEvent::A, 1);
    EXPECT_EQ(count1, 1);  // sub1 unsubscribed
    EXPECT_EQ(count2, 2);  // sub2 still works
}

// ── unsubscribe — by event ────────────────────────────────────────────────────

TEST(EventSubscriberTest, UnsubscribeByEventRemovesAllForThatEvent)
{
    EventManager<TestEvent> manager;
    EventSubscriber<TestEvent> sub(manager);
    int count = 0;
    sub.subscribe<int>(TestEvent::A, [&](const int&) { count++; });
    sub.subscribe<double>(TestEvent::A, [&](const double&) { count++; });
    sub.subscribe<int>(TestEvent::B, [&](const int&) { count++; });

    manager.publish(TestEvent::A, 1);
    manager.publish(TestEvent::A, 1.0);
    manager.publish(TestEvent::B, 1);
    EXPECT_EQ(count, 3);

    sub.unsubscribe(TestEvent::A);
    manager.publish(TestEvent::A, 1);
    manager.publish(TestEvent::A, 1.0);
    manager.publish(TestEvent::B, 1);
    EXPECT_EQ(count, 4);  // only EVENT_B fires
}

TEST(EventSubscriberTest, UnsubscribeByEventDoesNotAffectOtherSubscribers)
{
    EventManager<TestEvent> manager;
    EventSubscriber<TestEvent> sub1(manager);
    EventSubscriber<TestEvent> sub2(manager);
    int count1 = 0, count2 = 0;
    sub1.subscribe<int>(TestEvent::A, [&](const int&) { count1++; });
    sub2.subscribe<int>(TestEvent::A, [&](const int&) { count2++; });

    sub1.unsubscribe(TestEvent::A);
    manager.publish(TestEvent::A, 1);
    EXPECT_EQ(count1, 0);
    EXPECT_EQ(count2, 1);  // sub2 unaffected
}

TEST(EventSubscriberTest, UnsubscribeNonSubscribedEventDoesNotCrash)
{
    EventManager<TestEvent> manager;
    EventSubscriber<TestEvent> sub(manager);
    EXPECT_NO_THROW(sub.unsubscribe(TestEvent::A));
}

// ── unsubscribe — by event + type ─────────────────────────────────────────────

TEST(EventSubscriberTest, UnsubscribeByEventAndTypeRemovesOnlyMatching)
{
    EventManager<TestEvent> manager;
    EventSubscriber<TestEvent> sub(manager);
    int intCount = 0, doubleCount = 0;
    sub.subscribe<int>(TestEvent::A, [&](const int&) { intCount++; });
    sub.subscribe<double>(TestEvent::A, [&](const double&) { doubleCount++; });

    manager.publish(TestEvent::A, 42);
    manager.publish(TestEvent::A, 3.14);
    EXPECT_EQ(intCount, 1);
    EXPECT_EQ(doubleCount, 1);

    sub.unsubscribe(TestEvent::A, typeid(int));
    manager.publish(TestEvent::A, 42);
    manager.publish(TestEvent::A, 3.14);
    EXPECT_EQ(intCount, 1);    // not called
    EXPECT_EQ(doubleCount, 2); // still called
}

TEST(EventSubscriberTest, UnsubscribeByTypeNotSubscribedDoesNotCrash)
{
    EventManager<TestEvent> manager;
    EventSubscriber<TestEvent> sub(manager);
    sub.subscribe<int>(TestEvent::A, [](const int&) {});
    EXPECT_NO_THROW(sub.unsubscribe(TestEvent::A, typeid(double)));
}

TEST(EventSubscriberTest, UnsubscribeByTypeOnNonSubscribedEventDoesNotCrash)
{
    EventManager<TestEvent> manager;
    EventSubscriber<TestEvent> sub(manager);
    EXPECT_NO_THROW(sub.unsubscribe(TestEvent::A, typeid(int)));
}

// ── unsubscribeAll ────────────────────────────────────────────────────────────

TEST(EventSubscriberTest, UnsubscribeAllClearsAllSubscriptions)
{
    EventManager<TestEvent> manager;
    EventSubscriber<TestEvent> sub(manager);
    int count = 0;
    sub.subscribe<int>(TestEvent::A, [&](const int&) { count++; });
    sub.subscribe<int>(TestEvent::B, [&](const int&) { count++; });

    manager.publish(TestEvent::A, 1);
    manager.publish(TestEvent::B, 1);
    EXPECT_EQ(count, 2);

    sub.unsubscribeAll();
    manager.publish(TestEvent::A, 1);
    manager.publish(TestEvent::B, 1);
    EXPECT_EQ(count, 2);
}

TEST(EventSubscriberTest, UnsubscribeAllWhenEmptyDoesNotCrash)
{
    EventManager<TestEvent> manager;
    EventSubscriber<TestEvent> sub(manager);
    EXPECT_NO_THROW(sub.unsubscribeAll());
}

TEST(EventSubscriberTest, MultipleUnsubscribeAllIsIdempotent)
{
    EventManager<TestEvent> manager;
    EventSubscriber<TestEvent> sub(manager);
    sub.subscribe<int>(TestEvent::A, [](const int&) {});
    sub.unsubscribeAll();
    EXPECT_NO_THROW(sub.unsubscribeAll());
}

TEST(EventSubscriberTest, SubscribeAfterUnsubscribeAllWorks)
{
    EventManager<TestEvent> manager;
    EventSubscriber<TestEvent> sub(manager);
    int count = 0;
    sub.subscribe<int>(TestEvent::A, [&](const int&) { count++; });
    sub.unsubscribeAll();

    sub.subscribe<int>(TestEvent::A, [&](const int&) { count++; });
    manager.publish(TestEvent::A, 1);
    EXPECT_EQ(count, 1);
}

// ── move semantics ────────────────────────────────────────────────────────────

TEST(EventSubscriberTest, MoveTransfersSubscriptions)
{
    EventManager<TestEvent> manager;
    int count = 0;
    EventSubscriber<TestEvent> sub1(manager);
    sub1.subscribe<int>(TestEvent::A, [&](const int&) { count++; });

    EventSubscriber<TestEvent> sub2(std::move(sub1));
    manager.publish(TestEvent::A, 1);
    EXPECT_EQ(count, 1);  // sub2 holds the token and fires
}

TEST(EventSubscriberTest, MovedFromSubscriberDoesNotUnsubscribeOnDestruction)
{
    EventManager<TestEvent> manager;
    int count = 0;
    {
        EventSubscriber<TestEvent> sub1(manager);
        sub1.subscribe<int>(TestEvent::A, [&](const int&) { count++; });
        EventSubscriber<TestEvent> sub2(std::move(sub1));
        // sub1 goes out of scope here — must NOT unsubscribe
        manager.publish(TestEvent::A, 1);
        EXPECT_EQ(count, 1);
    }  // sub2 goes out of scope here — unsubscribes
    manager.publish(TestEvent::A, 1);
    EXPECT_EQ(count, 1);  // both gone, nothing fires
}
