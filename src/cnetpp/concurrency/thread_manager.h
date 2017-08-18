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
#ifndef CNETPP_CONCURRENCY_THREAD_MANAGER_H_
#define CNETPP_CONCURRENCY_THREAD_MANAGER_H_

#include <cnetpp/concurrency/thread.h>
#include <cnetpp/concurrency/thread_pool.h>
#include <cnetpp/concurrency/queue_base.h>

#include <memory>
#include <functional>

namespace cnetpp {
namespace concurrency {

class ThreadManager final {
 public:
  static std::shared_ptr<ThreadManager> Instance();

  ~ThreadManager();

  // ThreadManager is a singleton class.
  // disallow copy and move operations
  ThreadManager(const ThreadManager&) = delete;
  ThreadManager& operator=(const ThreadManager&) = delete;
  ThreadManager(ThreadManager&&) noexcept = delete;
  ThreadManager& operator=(ThreadManager&&) noexcept = delete;
  
  // start all threads, including the threads within thread pool.
  // If some threads have already been running, just skip them.
  void StartAll();
  
  // stop all threads.
  // If some threads have already stopped, just skip them.
  void StopAll();
  
  // stop a thread and remove it from ThreadManager
  void DestroyThread(std::shared_ptr<Thread> thread);
  
  // stop a thread pool and remove it from thread manager
  void DestroyThreadPool(std::shared_ptr<ThreadPool> thread_pool);
  
  // create thread using functor object
  std::shared_ptr<Thread> CreateThread(std::shared_ptr<Task> task);
  
  // create thread using function pointer
  std::shared_ptr<Thread> CreateThread(Thread::StartRoutine start_routine,
                                       void* arg = nullptr);
  
  // create thread using std::function introduced by c++11
  std::shared_ptr<Thread> CreateThread(const std::function<void()>& closure);
  std::shared_ptr<Thread> CreateThread(std::function<void()>&& closure);
  
  // create a thread pool
  // thread_count refers to the number of threads that the thread pool has
  std::shared_ptr<ThreadPool> CreateThreadPool(size_t thread_count = 0,
      std::shared_ptr<QueueBase> queue = CreateDefaultQueue());

 private:
  // Singleton class
  ThreadManager() = default;

  static std::shared_ptr<ThreadManager> thread_manager_;
  static std::once_flag once_flag_for_thread_manager_;

  // save all the threads those created by users
  std::vector<std::shared_ptr<Thread>> all_threads_;
  std::mutex all_threads_mutex_;

  // save all thread pools those created by users
  std::mutex all_thread_pools_mutex_;
  std::vector<std::shared_ptr<ThreadPool>> all_thread_pools_;

  void AddThread(std::shared_ptr<Thread> thread);
  bool RemoveThread(std::shared_ptr<Thread> thread);
  void AddThreadPool(std::shared_ptr<ThreadPool> thread_pool);
  bool RemoveThreadPool(std::shared_ptr<ThreadPool> thread_pool);

};

}  // namespace concurrency
}  // namespace cnetpp

#endif  // CNETPP_CONCURRENCY_THREAD_MANAGER_H_

