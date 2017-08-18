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
#ifndef CNETPP_CONCURRENCY_THREAD_FACTORY_H_
#define CNETPP_CONCURRENCY_THREAD_FACTORY_H_

#include <cnetpp/concurrency/task.h>
#include <cnetpp/concurrency/queue_base.h>
#include <cnetpp/concurrency/thread.h>
#include <cnetpp/concurrency/thread_pool.h>

#include <memory>
#include <mutex>

namespace cnetpp {
namespace concurrency {

// Only ThreadManager can use this class.
class ThreadFactory final {
 public:
  static std::shared_ptr<ThreadFactory> Instance();

  /* virtual */~ThreadFactory();

  // disallow copy and move operations
  ThreadFactory(const ThreadFactory&) = delete;
  ThreadFactory& operator=(const ThreadFactory&) = delete;
  ThreadFactory(ThreadFactory&&) = delete;
  ThreadFactory& operator=(ThreadFactory&&) = delete;

  /* protected: */
  // create a thread with a task instance
  // NOTE:
  // after calling this method, the thread will start running automatically, 
  // you should not call Thread::Start() manually.
  /* virtual */
  std::shared_ptr<Thread> CreateThread(std::shared_ptr<Task> task);
  
  // create a thread with a function pointer
  // NOTE:
  // after calling this method, the thread will start running automatically, 
  // you should not call Thread::Start() manually.
  /* virtual */
  std::shared_ptr<Thread> CreateThread(Thread::StartRoutine start_routine,
                                       void* arg);
  
  // create a thread with a lambda
  // NOTE:
  // after calling this method,  the thread will start running automatically, 
  // you should not call Thread::Start() manually.
  /* virtual */
  std::shared_ptr<Thread> CreateThread(const std::function<void()>& closure);
  /* virtual */
  std::shared_ptr<Thread> CreateThread(std::function<void()>&& closure);
  
  // create a thread pool which has a fixed number of threads
  // NOTE:
  // after calling this method, all the threads within the pool will starting 
  // running automatically, you should not call ThreadPool::Start() manually.
  // thread_count: 0 by default, means using the number of logical processors
  /* virtual */
  std::shared_ptr<ThreadPool> CreateThreadPool(size_t thread_count = 0,
      std::shared_ptr<QueueBase> queue = CreateDefaultQueue());

 private:
  ThreadFactory() = default;

  static std::shared_ptr<ThreadFactory> thread_factory_;
  static std::once_flag once_flag_for_thread_factory_;
};

}  // namespace concurrency
}  // namespace cnetpp

#endif  // CNETPP_CONCURRENCY_THREAD_FACTORY_H_

