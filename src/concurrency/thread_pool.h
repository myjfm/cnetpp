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
#ifndef CNETPP_CONCURRENCY_THREAD_POOL_H_
#define CNETPP_CONCURRENCY_THREAD_POOL_H_

#include <list>
#include <memory>
#include <mutex>
#include <condition_variable>

#include "thread.h"
#include "queue_base.h"
#include "task.h"

namespace cnetpp {
namespace concurrency {

class ThreadPool final {
 public:
  ~ThreadPool() = default;

  // disallow copy and move operations
  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;
  ThreadPool(ThreadPool&&) noexcept = delete;
  ThreadPool& operator=(ThreadPool&&) noexcept = delete;
  
  // start all threads in this thread pool
  void Start();

  // stop all threads in this thread pool
  void Stop();

  // add a task into the queue and one of the threads in the pool will run it
  void AddTask(std::shared_ptr<Task> task);
  void AddTask(Thread::StartRoutine start_routine);
  void AddTask(std::function<bool()> closure);

  size_t Size() const {
    return threads_.size();
  }

 private:
  friend class ThreadFactory;

  enum class Status {
    kInit = 0x0,
    kRunning = 0x1,
    kStop = 0x2,
  };

  SpinLock status_lock_;
  Status status_ { Status::kInit };

  std::atomic<bool> stop_ { false };

  std::mutex queue_mutex_;
  std::condition_variable queue_cond_var_;
  std::shared_ptr<QueueBase> queue_;

  // cache the tasks those is being processed
  // the size of this buffer should less than or equal to
  // the number of threads the thread pool contains
  std::list<std::shared_ptr<Task>> running_tasks_;
  std::mutex running_tasks_mutex_;

  std::vector<std::shared_ptr<Thread>> threads_;

  explicit ThreadPool(size_t thread_count = 0);
  explicit ThreadPool(std::shared_ptr<QueueBase> queue,
                      size_t thread_count = 0);

  void DoTask();
};

}  // namespace concurrency
}  // namespace cnetpp

#endif  // CNETPP_CONCURRENCY_THREAD_POOL_H_

