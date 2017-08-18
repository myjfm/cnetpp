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
#ifndef CNETPP_CONCURRENCY_QUEUE_BASE_H_
#define CNETPP_CONCURRENCY_QUEUE_BASE_H_

#include <cnetpp/concurrency/task.h>

#include <memory>
#include <queue>
#include <atomic>
#include <mutex>
#include <condition_variable>

namespace cnetpp {
namespace concurrency {

class QueueBase {
 public:
  QueueBase() = default;
  virtual ~QueueBase() = default;

  // disallow copy and move operations
  QueueBase(const QueueBase&) = delete;
  QueueBase& operator=(const QueueBase&) = delete;
  QueueBase(QueueBase&&) noexcept = delete;
  QueueBase& operator=(QueueBase&&) noexcept = delete;
  
  // push a task into queue
  virtual void Push(std::shared_ptr<Task> task) = 0;
  
  // get a task from queue, if empty, just return nullptr
  virtual std::shared_ptr<Task> TryPop() = 0;
  
  // Get a task from queue.
  // If empty, wait until it becomes not empty(timeout = 0) or
  // timed out(timeout > 0)
  // Return the poped task, nullptr if no task.
  virtual std::shared_ptr<Task> WaitPop(int timeout = 0) = 0;
  
  virtual bool Empty() const = 0;
  
  virtual size_t Size() const = 0;
};

// create a default thread-safe queue
// @return the pointer of the queue
std::shared_ptr<QueueBase> CreateDefaultQueue();

// The default queue is a thread-safe queue,
// use std::queue as the backend entity, 
// so its performance may be not perfect. 
// But it should be enough for most requiements.
class DefaultQueue final : public QueueBase {
 public:
  DefaultQueue() = default;
  virtual ~DefaultQueue();

 protected:
  void Push(std::shared_ptr<Task> task) override final;

  virtual std::shared_ptr<Task> TryPop() override final;

  // may be blocked
  virtual std::shared_ptr<Task> WaitPop(int timeout = 0) override final;

  virtual bool Empty() const override final;

  virtual size_t Size() const override final;

 private:
  std::queue<std::shared_ptr<Task>> queue_;
  mutable std::mutex mutex_;
  std::condition_variable cond_var_;
  std::atomic<bool> stop_ { false };
};


}  // namespace concurrency
}  // namespace cnetpp

#endif  // CNETPP_CONCURRENCY_QUEUE_BASE_H_
