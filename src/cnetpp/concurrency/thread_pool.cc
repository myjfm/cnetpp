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
#include <cnetpp/concurrency/thread_factory.h>

#include <algorithm>

namespace cnetpp {
namespace concurrency {

namespace {
  const size_t kDefaultThreadCount = 5;
}

ThreadPool::ThreadPool(size_t thread_count)
    : ThreadPool(CreateDefaultQueue(), thread_count) { }

ThreadPool::ThreadPool(std::shared_ptr<QueueBase> queue, size_t thread_count) {
  queue_ = std::move(queue);

  if (thread_count == 0) {
    thread_count = std::thread::hardware_concurrency();
  }
  if (thread_count == 0) {
    thread_count = kDefaultThreadCount;
  }

  std::vector<std::shared_ptr<Thread>> tmp_threads(thread_count);
  threads_.swap(tmp_threads);
}

void ThreadPool::Start() {
  {
    SpinLock::ScopeGuard guard(status_lock_);
    if (status_ != Status::kInit) {
      return;
    }
    status_ = Status::kRunning;
  }

  for (auto& t: threads_) {
    t = ThreadFactory::Instance()->CreateThread([this] {
      DoTask();
    });
    t->Start();
  }
}

void ThreadPool::Stop() {
  {
    SpinLock::ScopeGuard guard(status_lock_);
    if (status_ != Status::kRunning) {
      return;
    }
    status_ = Status::kStop;
  }

  {
    std::lock_guard<std::mutex> guard(running_tasks_mutex_);
    for_each(running_tasks_.begin(), running_tasks_.end(),
        [] (std::shared_ptr<Task> running_task) {
          running_task->Stop();
        }
    );
    stop_.store(true, std::memory_order_release);
  }

  queue_cond_var_.notify_all();

  for (auto& t : threads_) {
    // blocking stop
    t->Stop();
  }
}

void ThreadPool::AddTask(std::shared_ptr<Task> task) {
  std::unique_lock<std::mutex> guard(queue_mutex_);
  queue_->Push(task);
  queue_cond_var_.notify_one();
}

class InternalTask final : public Task {
 public:
  InternalTask(std::function<bool()> closure)
      : closure_(std::move(closure)) {
  }

  InternalTask(Thread::StartRoutine start_routine)
      : start_routine_(start_routine) {
  }

  bool operator()(void* arg = nullptr) override final {
    if (closure_) {
      return closure_();
    } else if (start_routine_) {
      return start_routine_(nullptr);
    }
  }

  virtual ~InternalTask() { }

 private:
  std::function<bool()> closure_ { nullptr };
  Thread::StartRoutine start_routine_ { nullptr };
};

void ThreadPool::AddTask(std::function<bool()> closure) {
  std::shared_ptr<Task> internal_task(new InternalTask(std::move(closure)));
  std::unique_lock<std::mutex> guard(queue_mutex_);
  queue_->Push(internal_task);
  queue_cond_var_.notify_one();
}

void ThreadPool::AddTask(Thread::StartRoutine start_routine) {
  std::shared_ptr<Task> internal_task(new InternalTask(start_routine));
  std::unique_lock<std::mutex> guard(queue_mutex_);
  queue_->Push(internal_task);
  queue_cond_var_.notify_one();
}

void ThreadPool::DoTask() {
  while (!stop_.load(std::memory_order_acquire)) {
    std::shared_ptr<Task> task;
    {
      std::unique_lock<std::mutex> guard(queue_mutex_);
      queue_cond_var_.wait(guard, [this] {
        return (!queue_->Empty() || stop_.load(std::memory_order_acquire));
      });

      if (stop_.load(std::memory_order_acquire)) {
        return;
      }

      task = queue_->TryPop(); // we just use TryPop()
      if (!task.get()) { // this should not happen
        // TODO(myjfm)
        // log an error
        continue;
      }
    }

    // put the task into the processing task buffer
    {
      std::lock_guard<std::mutex> guard(running_tasks_mutex_);
      running_tasks_.push_back(task);
    }

    if (stop_.load(std::memory_order_acquire)) {
      return;
    }

    // do task
    (*(task.get()))();

    // erase the task from the processing task buffer
    {
      std::lock_guard<std::mutex> guard(running_tasks_mutex_);
      running_tasks_.remove_if([task] (const std::shared_ptr<Task>& that_task) {
        return that_task.get() == task.get();
      });
    }
  }
}

}  // namespace concurrency
}  // namespace cnetpp

