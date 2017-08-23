#include <cnetpp/concurrency/priority_queue.h>

#include <gtest/gtest.h>

class MockTask : public cnetpp::concurrency::Task {
 public:
  MockTask(int value, int weight) : value_(value), weight_(weight) {
  }

  int value() const {
    return value_;
  }
  int weight() const {
    return weight_;
  }

  bool operator()(void* arg = nullptr) override {
    (void) arg;
    return true;
  }

 private:
  int value_;
  int weight_;
};

struct MockTaskComparator {
  bool operator()(const std::shared_ptr<cnetpp::concurrency::Task>& x,
      const std::shared_ptr<cnetpp::concurrency::Task>& y) const {
    auto xx = std::static_pointer_cast<MockTask>(x);
    auto yy = std::static_pointer_cast<MockTask>(y);
    return xx->weight() > yy->weight();
  }
};

TEST(PriorityQueue, Test01) {
  auto pq = cnetpp::concurrency::CreatePriorityQueue<MockTaskComparator>();
  ASSERT_TRUE(pq->Push(std::static_pointer_cast<cnetpp::concurrency::Task>(
        std::make_shared<MockTask>(10, 2))));
  ASSERT_TRUE(pq->Push(std::static_pointer_cast<cnetpp::concurrency::Task>(
        std::make_shared<MockTask>(11, 1))));
  auto peek = std::static_pointer_cast<MockTask>(pq->Peek());
  ASSERT_EQ(peek->weight(), 1);
  ASSERT_EQ(peek->value(), 11);
  auto t1 = std::static_pointer_cast<MockTask>(pq->TryPop());
  ASSERT_EQ(peek, t1);
  auto t2 = std::static_pointer_cast<MockTask>(pq->TryPop());
  ASSERT_EQ(t2->weight(), 2);
  ASSERT_EQ(t2->value(), 10);
}

