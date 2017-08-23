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
#include <cnetpp/concurrency/thread_pool.h>
#include <cnetpp/base/log.h>

#include <algorithm>

namespace cnetpp {
namespace concurrency {

namespace {
  const size_t kDefaultThreadCount = 5;
}

ThreadPool::ThreadPool(const std::string& name)
    : ThreadPool(name, CreateDefaultQueue(), false) {
}

ThreadPool::ThreadPool(const std::string& name,
                       std::shared_ptr<QueueBase> queue)
    : ThreadPool(name, queue, false) {
}

ThreadPool::ThreadPool(const std::string& name, bool enable_delay)
    : ThreadPool(name, CreateDefaultQueue(), enable_delay) {
}

ThreadPool::ThreadPool(const std::string& name,
                       std::shared_ptr<QueueBase> queue,
                       bool enable_delay)
    : name_(name), queue_(queue), enable_delay_(enable_delay) {
  size_t num_threads = std::thread::hardware_concurrency();
  if (num_threads == 0) {
    num_threads = kDefaultThreadCount;
  }

  threads_.resize(num_threads);
}

void ThreadPool::Start() {
  Status old = Status::kInit;
  if (!status_.compare_exchange_strong(old, Status::kRunning)) {
    Fatal("Failed to start thread pool, invalid thread status: %d",
        static_cast<int>(old));
  }

  int32_t nr = 0;
  for (auto& t: threads_) {
    t = std::make_unique<Thread>([this] () -> bool { DoTask(); return true; },
        name_ + "-" + std::to_string(nr));
    t->Start();
    Info("Thread %s-%d started.", name_.c_str(), nr);
    nr++;
  }

  if (enable_delay_) {
    delay_queue_ = std::make_unique<DelayQueue>();
    delay_thread_ = std::make_unique<Thread>(
        [this] () -> bool { PollDelayTask(); return true; }, name_ + "-d");
    delay_thread_->Start();
    Info("Delay thread %s-d started.", name_.c_str());
  }
}

void ThreadPool::Stop(bool wait) {
  Status old = Status::kRunning;
  if (!status_.compare_exchange_strong(old, Status::kStop)) {
    return;
  }

  {
    std::unique_lock<std::mutex> guard(mutex_);
    stopping_.store(true, std::memory_order_release);
    force_stop_.store(!wait, std::memory_order_release);
    queue_cv_.notify_all();
    delay_queue_cv_.notify_all();
  }

  delay_thread_->Stop();
  Info("Delay thread %s stopped.", delay_thread_->name().c_str());

  for (auto& t : threads_) {
    t->Stop();
    Info("Thread %s stopped.", t->name().c_str());
  }
}

bool ThreadPool::AddTask(std::shared_ptr<Task> task) {
  if (status_.load(std::memory_order_acquire) != Status::kRunning) {
    Error("Thread pool is not running.");
    return false;
  }

  std::lock_guard<std::mutex> guard(mutex_);
  if (stopping_.load(std::memory_order_acquire)) {
    Error("Adding a task in a stopped thread pool.");
    return false;
  }
  size_t pending_tasks = queue_->size();
  if (enable_delay_) {
    pending_tasks += delay_queue_->size();
  }
  if (max_num_pending_tasks_ > 0 && pending_tasks >= max_num_pending_tasks_) {
    Error("Queue is full.");
    return false;
  }
  auto r = queue_->Push(task);
  assert(r);
  queue_cv_.notify_one();
  return true;
}

class InternalTask final : public Task {
 public:
  InternalTask(std::function<bool()> closure)
      : closure_(std::move(closure)) {
  }

  bool operator()(void* arg = nullptr) override final {
    (void) arg;
    assert(closure_);
    return closure_();
  }

  virtual ~InternalTask() {
  }

 private:
  std::function<bool()> closure_ { nullptr };
};

bool ThreadPool::AddTask(const std::function<bool()>& closure) {
  auto task =
    std::static_pointer_cast<Task>(std::make_shared<InternalTask>(closure));
  return AddTask(task);
}

bool ThreadPool::AddDelayTask(std::shared_ptr<Task> task,
    std::chrono::microseconds delay) {
  if (status_.load(std::memory_order_acquire) != Status::kRunning) {
    Error("Thread pool is not running.");
    return false;
  }

  if (!enable_delay_) {
    Error("The enable_delay option is turned off!");
    return false;
  }
  std::lock_guard<std::mutex> guard(mutex_);
  if (stopping_.load(std::memory_order_acquire)) {
    Error("Adding a task in a stopped thread pool.");
    return false;
  }

  if (max_num_pending_tasks_ > 0 &&
      delay_queue_->size() + queue_->size() >= max_num_pending_tasks_) {
    Error("Delay queue is full.");
    return false;
  }

  auto r = delay_queue_->Push(
      std::static_pointer_cast<Task>(std::make_shared<DelayTask>(task, delay)));
  assert(r);
  delay_queue_cv_.notify_one();
  return true;
}

bool ThreadPool::AddDelayTask(const std::function<bool()>& closure,
    std::chrono::microseconds delay) {
  return AddDelayTask(
      std::static_pointer_cast<Task>(std::make_shared<InternalTask>(closure)),
      delay);
}

void ThreadPool::DoTask() {
  while (true) {
    std::shared_ptr<Task> task;
    {
      std::unique_lock<std::mutex> guard(mutex_);
      queue_cv_.wait(guard, [this] {
        return (!queue_->Empty() || stopping_.load(std::memory_order_acquire));
      });

      if (force_stop_.load(std::memory_order_acquire)) {
        return;
      }

      task = queue_->TryPop();
      if (!task.get()) {
        if (stopping_.load(std::memory_order_acquire)) {
          return;
        }
        continue;
      }
    }

    // do task
    (*(task.get()))();
  }
}

void ThreadPool::PollDelayTask() {
  assert(enable_delay_);
  while (true) {
    if (stopping_ && force_stop_) {
      Info("Forcing stop delay thread: %s-d, exit...", name_.c_str());
      return;
    }

    std::shared_ptr<DelayTask> task;
    {
      std::unique_lock<std::mutex> guard(mutex_);
      task = std::static_pointer_cast<DelayTask>(delay_queue_->Peek());
      if (task) {
        if (task->run_time_ > std::chrono::system_clock::now()) {
          auto run_time = task->run_time_;
          delay_queue_cv_.wait_until(guard, task->run_time_, [this, run_time] {
            auto peek =
                std::static_pointer_cast<DelayTask>(delay_queue_->Peek());
            return stopping_.load(std::memory_order_acquire) ||
                peek->run_time_ != run_time;  // new task arrived
          });
        } else {
          // some task is expired
          auto r = queue_->Push(std::static_pointer_cast<Task>(task));
          assert(r);
          auto t = delay_queue_->TryPop();
          assert(t == std::static_pointer_cast<Task>(task));
          queue_cv_.notify_one();
        }
      } else {
        if (stopping_) {
          Info("Stopping delay thread: %s-d, exit...", name_.c_str());
          return;
        }
        // delay queue is empty
        delay_queue_cv_.wait(guard, [this] {
          return stopping_.load(std::memory_order_acquire) ||
            !delay_queue_->Empty();
        });
      }
    }
  }
}

}  // namespace concurrency
}  // namespace cnetpp

