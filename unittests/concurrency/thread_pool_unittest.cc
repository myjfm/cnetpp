#include <cnetpp/concurrency/thread_pool.h>

#include <gtest/gtest.h>

#include <string.h>
#include <pthread.h>

#include <atomic>
#include <memory>

TEST(ThreadPool, Test01) {
  auto tp = std::make_shared<cnetpp::concurrency::ThreadPool>("Test01", true);
  tp->set_num_threads(200);
  tp->Start();
  ASSERT_EQ(tp->size(), static_cast<size_t>(200));
  std::atomic<int> i { 0 };
  auto closure = [&i] () -> bool { i++; return true; };
  for (int j = 0; j < 400; ++j) {
    tp->AddTask(closure);
  }
  for (int j = 0; j < 400; ++j) {
    tp->AddDelayTask(closure, std::chrono::seconds(1));
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  EXPECT_EQ(i, 400);
  std::this_thread::sleep_for(std::chrono::seconds(1));
  EXPECT_EQ(i, 800);
  tp->Stop();
}

TEST(ThreadPool, TestMaxPendingTasks) {
  auto tp = std::make_shared<cnetpp::concurrency::ThreadPool>("Test01", true);
  tp->set_num_threads(2);
  tp->set_max_num_pending_tasks(4);
  tp->Start();
  ASSERT_EQ(tp->size(), static_cast<size_t>(2));
  std::atomic<int> i { 0 };
  auto closure = [&i] () -> bool {
    i++;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return true;
  };
  for (int j = 0; j < 2; ++j) {
    auto r = tp->AddDelayTask(closure, std::chrono::seconds(1));
    EXPECT_TRUE(r);
  }
  for (int j = 0; j < 2; ++j) {
    auto r = tp->AddTask(closure);
    EXPECT_TRUE(r);
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  for (int j = 0; j < 2; ++j) {
    auto r = tp->AddTask(closure);
    EXPECT_TRUE(r);
  }
  for (int j = 0; j < 2; ++j) {
    auto r = tp->AddTask(closure);
    EXPECT_FALSE(r);
  }
  for (int j = 0; j < 2; ++j) {
    auto r = tp->AddDelayTask(closure, std::chrono::seconds(1));
    EXPECT_FALSE(r);
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  EXPECT_EQ(i, 4);
  std::this_thread::sleep_for(std::chrono::seconds(1));
  EXPECT_EQ(i, 6);
  tp->Stop();
}

TEST(ThreadPool, TestStopImmediately) {
  auto tp = std::make_shared<cnetpp::concurrency::ThreadPool>("Test01", true);
  tp->set_num_threads(2);
  tp->Start();
  ASSERT_EQ(tp->size(), static_cast<size_t>(2));
  std::atomic<int> i { 0 };
  auto closure = [&i] () -> bool {
    i++;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return true;
  };
  for (int j = 0; j < 2; ++j) {
    auto r = tp->AddDelayTask(closure, std::chrono::seconds(1));
    EXPECT_TRUE(r);
  }
  for (int j = 0; j < 2; ++j) {
    auto r = tp->AddTask(closure);
    EXPECT_TRUE(r);
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  tp->Stop(false);
  EXPECT_EQ(i, 2);
}

TEST(ThreadPool, TestDelayTask) {
  auto tp = std::make_shared<cnetpp::concurrency::ThreadPool>("Test01", true);
  tp->set_num_threads(2);
  tp->Start();
  ASSERT_EQ(tp->size(), static_cast<size_t>(2));
  std::atomic<int> i { 0 };
  std::atomic<int> j { 0 };
  auto closure1 = [&i] () -> bool {
    i++;
    return true;
  };
  auto closure2 = [&j] () -> bool {
    j++;
    return true;
  };
  ASSERT_TRUE(tp->AddDelayTask(closure1, std::chrono::milliseconds(10)));
  ASSERT_TRUE(tp->AddDelayTask(closure2, std::chrono::milliseconds(20)));
  std::this_thread::sleep_for(std::chrono::milliseconds(5));
  EXPECT_EQ(i, 0);
  EXPECT_EQ(j, 0);
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  EXPECT_EQ(i, 1);
  EXPECT_EQ(j, 0);
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  EXPECT_EQ(i, 1);
  EXPECT_EQ(j, 1);
  //tp->Stop();
}

TEST(ThreadPool, TestOnlyDelayTask) {
  auto tp = std::make_shared<cnetpp::concurrency::ThreadPool>("Test01", true);
  tp->set_num_threads(0);
  tp->Start();
  // at least one thread num
  ASSERT_EQ(tp->size(), static_cast<size_t>(1));
  std::atomic<int> i { 0 };
  std::atomic<int> j { 0 };
  auto closure1 = [&i] () -> bool {
    i++;
    return true;
  };
  auto closure2 = [&j] () -> bool {
    j++;
    return true;
  };
  ASSERT_TRUE(tp->AddDelayTask(closure1, std::chrono::milliseconds(10)));
  ASSERT_TRUE(tp->AddDelayTask(closure2, std::chrono::milliseconds(20)));
  std::this_thread::sleep_for(std::chrono::milliseconds(5));
  EXPECT_EQ(i, 0);
  EXPECT_EQ(j, 0);
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  EXPECT_EQ(i, 1);
  EXPECT_EQ(j, 0);
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  EXPECT_EQ(i, 1);
  EXPECT_EQ(j, 1);
  //tp->Stop();
}
