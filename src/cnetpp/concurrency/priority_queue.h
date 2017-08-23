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
//   * Neither the name of myjfm nor the names of other contributors may be
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

#include <cnetpp/concurrency/queue_base.h>

#include <queue>

namespace cnetpp {
namespace concurrency {

template<class Compare>
class PriorityQueue final : public QueueBase {
 public:
  PriorityQueue(size_t capacity = 0) : capacity_(capacity) {
  }
  virtual ~PriorityQueue() = default;

  bool Push(std::shared_ptr<Task> task) override {
    if (capacity_ > 0 && queue_.size() >= capacity_) {
      return false;
    }
    queue_.push(std::move(task));
    return true;
  }

  std::shared_ptr<Task> TryPop() override {
    if (queue_.empty()) {
      return nullptr;
    }
    std::shared_ptr<Task> result(std::move(queue_.top()));
    queue_.pop();
    return result;
  }

  std::shared_ptr<Task> Peek() override {
    if (queue_.empty()) {
      return nullptr;
    }
    return queue_.top();
  }

  bool Empty() const override {
    return queue_.empty();
  }

  bool Full() const override {
    if (capacity_ > 0) {
      return queue_.size() == capacity_;
    } else {
      return false;
    }
  }

  size_t size() const override {
    return queue_.size();
  }

  size_t capacity() const override {
    return capacity_;
  }

 private:
  std::priority_queue<std::shared_ptr<Task>,
                      std::deque<std::shared_ptr<Task>>,
                      Compare> queue_;
  size_t capacity_ { 0 };
};

template<class Compare>
inline std::shared_ptr<QueueBase> CreatePriorityQueue(size_t capacity = 0) {
  return std::static_pointer_cast<QueueBase>(
      std::make_shared<PriorityQueue<Compare>>(capacity));
}

}  // namespace concurrency
}  // namespace cnetpp

#endif // CNETPP_CONCURRENCY_PRIORITY_QUEUE_H_

