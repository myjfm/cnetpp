#include <cnetpp/concurrency/thread.h>

#include <gtest/gtest.h>

#include <string.h>
#include <pthread.h>
#include <assert.h>

#include <atomic>
#include <memory>

TEST(Thread, Test01) {
  int base_thread_index = cnetpp::concurrency::Thread::MaxThreadIndex();
  ASSERT_TRUE(cnetpp::concurrency::Thread::ThisThread() == nullptr);
  {
    std::atomic<int> i { 0 };
    char name[20];
    char* p = name;
    auto t = std::make_shared<cnetpp::concurrency::Thread>([base_thread_index,
        &i, p] () -> bool {
        if (cnetpp::concurrency::Thread
            ::ThisThread()->ThreadIndex() != base_thread_index) {
          abort();
        }
        i++;
        pthread_getname_np(pthread_self(), p, 20);
        return true;
        }, "Test01-1");
    ASSERT_TRUE(cnetpp::concurrency::Thread::ThisThread() == nullptr);
    ASSERT_EQ(t->ThreadIndex(), base_thread_index);
    ASSERT_EQ(t->MaxThreadIndex(), base_thread_index + 1);
    ASSERT_EQ(t->ThreadPoolIndex(), -1);
    ASSERT_EQ(t->GetStatus(), cnetpp::concurrency::Thread::Status::kInit);
    t->Start();
    ASSERT_EQ(t->GetStatus(), cnetpp::concurrency::Thread::Status::kRunning);
    ASSERT_TRUE(t->IsJoinable());
    t->Stop();
    ASSERT_EQ(t->GetStatus(), cnetpp::concurrency::Thread::Status::kStop);
    ASSERT_EQ(i, 1);
    ASSERT_EQ("Test01-1", t->name());
    ASSERT_EQ(strcmp("Test01-1", name), 0);
  }
  {
    std::atomic<int> i { 0 };
    char name[20];
    char* p = name;
    auto t = std::make_shared<cnetpp::concurrency::Thread>([base_thread_index,
        &i, p] () -> bool {
        if (cnetpp::concurrency::Thread
            ::ThisThread()->ThreadIndex() != base_thread_index + 1) {
          abort();
        }
        i++;
        pthread_getname_np(pthread_self(), p, 20);
        return true;
        }, "Test01-2");
    ASSERT_TRUE(cnetpp::concurrency::Thread::ThisThread() == nullptr);
    ASSERT_EQ(t->ThreadIndex(), base_thread_index + 1);
    ASSERT_EQ(t->MaxThreadIndex(), base_thread_index + 2);
    ASSERT_EQ(t->ThreadPoolIndex(), -1);
    ASSERT_EQ(t->GetStatus(), cnetpp::concurrency::Thread::Status::kInit);
    t->Start();
    ASSERT_EQ(t->GetStatus(), cnetpp::concurrency::Thread::Status::kRunning);
    ASSERT_TRUE(t->IsJoinable());
    t->Stop();
    ASSERT_EQ(t->GetStatus(), cnetpp::concurrency::Thread::Status::kStop);
    ASSERT_EQ(i, 1);
    ASSERT_EQ("Test01-2", t->name());
    ASSERT_EQ(strcmp("Test01-2", name), 0);
  }
}

