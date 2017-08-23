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
#ifndef CNETPP_CONCURRENCY_THREAD_POOL_H_
#define CNETPP_CONCURRENCY_THREAD_POOL_H_

#include <cnetpp/concurrency/thread.h>
#include <cnetpp/concurrency/queue_base.h>
#include <cnetpp/concurrency/priority_queue.h>
#include <cnetpp/concurrency/task.h>
#include <cnetpp/base/log.h>

#include <memory>
#include <mutex>
#include <queue>
#include <condition_variable>

namespace cnetpp {
namespace concurrency {

class ThreadPool final {
 public:
  explicit ThreadPool(const std::string& name);
  ThreadPool(const std::string& name, std::shared_ptr<QueueBase> queue);
  ThreadPool(const std::string& name, bool enable_delay);
  ThreadPool(const std::string& name, std::shared_ptr<QueueBase> queue,
      bool enable_delay);

  ~ThreadPool() {
    Stop(false);
  }

  // disallow copy and move operations
  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;
  
  void set_num_threads(size_t num) {
    assert(status_.load(std::memory_order_acquire) == Status::kInit);
    threads_.resize(num);
  }

  void set_max_num_pending_tasks(size_t num) {
    max_num_pending_tasks_ = num;
  }

  // start all threads in this thread pool
  void Start();

  // stop all threads in this thread pool
  void Stop(bool wait = false);

  // add a task into the queue and one of the threads in the pool will run it
  bool AddTask(std::shared_ptr<Task> task);
  bool AddTask(const std::function<bool()>& closure);

  bool AddDelayTask(std::shared_ptr<Task> task,
      std::chrono::microseconds delay);
  bool AddDelayTask(const std::function<bool()>& closure,
      std::chrono::microseconds delay);

  size_t size() const {
    return threads_.size();
  }

 private:
  enum class Status : int {
    kInit = 0x0,
    kRunning = 0x1,
    kStop = 0x2,
  };

  std::string name_;

  std::atomic<Status> status_ { Status::kInit };

  std::atomic<bool> stopping_ { false };
  std::atomic<bool> force_stop_ { false };

  size_t max_num_pending_tasks_ { 0 };

  std::mutex mutex_;
  std::condition_variable queue_cv_;
  std::shared_ptr<QueueBase> queue_;

  std::vector<std::unique_ptr<Thread>> threads_;

  class DelayTask : public Task {
   public:
    DelayTask(std::shared_ptr<Task> task, std::chrono::microseconds delay)
        : task_(std::move(task)),
          run_time_(std::chrono::system_clock::now() + delay) {
    }

    bool operator()(void* arg = nullptr) override {
      (void) arg;
      assert(task_);
      return (*task_)();
    }

    void Stop() override {
      Task::Stop();
      task_->Stop();
    }

    struct Comparator {
      bool operator() (const std::shared_ptr<Task>& x,
          const std::shared_ptr<Task>& y) const {
        auto xx = std::static_pointer_cast<DelayTask>(x);
        auto yy = std::static_pointer_cast<DelayTask>(y);
        return xx->run_time_ > yy->run_time_;
      }
    };
    std::shared_ptr<Task> task_;
    std::chrono::system_clock::time_point run_time_;
  };

  using DelayQueue = PriorityQueue<DelayTask::Comparator>;

  bool enable_delay_ { false };
  std::unique_ptr<Thread> delay_thread_;
  std::condition_variable delay_queue_cv_;
  std::unique_ptr<DelayQueue> delay_queue_;

  void DoTask();

  void PollDelayTask();
};

}  // namespace concurrency
}  // namespace cnetpp

#endif  // CNETPP_CONCURRENCY_THREAD_POOL_H_

