#include <cnetpp/concurrency/thread.h>

#include <gtest/gtest.h>

#include <string.h>
#include <pthread.h>

#include <atomic>
#include <memory>

TEST(Thread, Test01) {
  std::atomic<int> i { 0 };
  char name[20];
  char* p = name;
  auto t = std::make_shared<cnetpp::concurrency::Thread>([&i, p] () -> bool {
    i++;
    pthread_getname_np(pthread_self(), p, 20);
    return true;
  }, "Test01-1");
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

