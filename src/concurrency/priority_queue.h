// Copyright (c) 2015, myjfm(mwxjmmyjfm@gmail.com).  All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//   * Neither the name of Shuo Chen nor the names of other contributors may be
// used to endorse or promote products derived from this software without
// specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
#ifndef CNETPP_CONCURRENCY_PRIORITY_QUEUE_H_
#define CNETPP_CONCURRENCY_PRIORITY_QUEUE_H_

#include "queue_base.h"

namespace cnetpp {
namespace concurrency {

template<class Compare>
class PriorityQueue final : public QueueBase {
 public:
  PriorityQueue() : stop_(false) {
  }

  virtual ~PriorityQueue() = default;

  void Push(std::shared_ptr<Task> task) override final {
    std::unique_lock<std::mutex> guard(mutex_);
    queue_.push(std::move(task));
    cond_var_.notify_one();
  }

  std::shared_ptr<Task> TryPop() override final {
    std::unique_lock<std::mutex> guard(mutex_);
    if (queue_.empty()) {
      return nullptr;
    }
    std::shared_ptr<Task> result(std::move(queue_.top()));
    queue_.pop();
    return result;
  }

  std::shared_ptr<Task> WaitPop(int timeout = 0) override final {
    std::unique_lock<std::mutex> guard(mutex_);
    if (timeout <= 0) {
      cond_var_.wait(guard, [this] {
          return (!queue_.empty() || stop_.load(std::memory_order_acquire));
      });
    } else {
      if (!cond_var_.wait_for(guard_, 
                          std::chrono::milliseconds(timeout), 
                          [this] {
                            return (!queue_.empty() || 
                              stop_.load(std::memory_order_acquire));
                          })) {
        return nullptr; // timed out
      }
    }

    if (stop_) {
      return nullptr;
    }

    std::shared_ptr<Task> result(std::move(queue_.top()));
    queue_.pop();
    return result;
  }

  bool Empty() const override final {
    std::unique_lock<std::mutex> guard(mutex_);
    return queue_.empty();
  }

  size_t Size() const override final {
    std::unique_lock<std::mutex> guard(mutex_);
    return queue_.size();
  }

 private:
  std::priority_queue<std::shared_ptr<Task>, 
                      std::vector<std::shared_ptr<Task>>, 
                      Compare> queue_;
  mutable std::mutex mutex_;
  std::condition_variable cond_var_;

  std::atomic<bool> stop_;
};

// Create a thread-safe priority queue.
// user should provide the comparison functor or function
// @return the pointer of the queue
template<class Compare>
std::shared_ptr<QueueBase> CreatePriorityQueue() {
  return std::make_shared<PriorityQueue<Compare>>();
}

}  // namespace concurrency
}  // namespace cnetpp

#endif // CNETPP_CONCURRENCY_PRIORITY_QUEUE_H_

