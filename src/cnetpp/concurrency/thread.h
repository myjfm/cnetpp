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
#ifndef CNETPP_CONCURRENCY_THREAD_H_
#define CNETPP_CONCURRENCY_THREAD_H_

#include <cnetpp/concurrency/task.h>
#include <cnetpp/concurrency/spin_lock.h>

#include <memory>
#include <thread>

namespace cnetpp {
namespace concurrency {

// DO NOT inherit it, instead, inherit the Task interface
class Thread final {
public:
  using StartRoutine = bool(*)(void* arg);
  using Id = std::thread::id;

  enum class Status {
    kInit = 0x0,
    kRunning = 0x1,
    kSuspend = 0x2, // not implemented
    kStop = 0x4,
  };

  ~Thread();

  // Thread is an RAII class, and 
  // disallow copy operations
  Thread(const Thread&) = delete;
  Thread& operator=(const Thread&) = delete;

  // DO NOT move the Thread instance, 
  // because we have a SpinLock inside it.
  Thread(Thread&&) = delete;
  Thread& operator=(Thread&&) = delete;
  
  // Get the current status of the thread
  // return
  //    * kInit     the thread has not started yet, 
  //    * kRunning  the thread is running,
  //    * kSuspend  the thread currently is paused, may support it in the future
  //    * kStop     the thread has already stopped
  Status GetStatus() const noexcept {
    SpinLock::ScopeGuard guard(status_lock_);
    return status_;
  }
  
  // after calling this method, the thread starts running
  void Start();

  void Stop();

  Id GetId() const noexcept {
    return thread_.get_id();
  }
  
  // TODO(myjfm)
  // implement it in the future
  void Suspend() {
    assert(false);
  }
  
  // TODO(myjfm) 
  // implement it in the future
  // detach this thread, so this thread will become unjoinable
  void Detach() {
    assert(false);
#if 0
  if (thread_.joinable()) {
    thread_.detach();
  }
#endif
  }
  
  // TODO(myjfm)
  // implement it in the future
  // join the thread, wait until it finishes
  void Join() {
    assert(false);
#if 0
  if (thread_.joinable()) {
    thread_.join();
  }
#endif
  }
  
  // check whether this thread is joinable
  bool IsJoinable() const noexcept {
    return thread_.joinable();
  }

 private:
  // Please use ThreadManager to create the thread, and 
  // then you can safely forget to call the Stop().
  // Because the ThreadManager manages all the threads intelligently.
  friend class ThreadFactory;
  explicit Thread(StartRoutine start_routine, void* arg = nullptr);
  explicit Thread(std::shared_ptr<Task> task);
  explicit Thread(const std::function<void()>& closure);
  explicit Thread(std::function<void()>&& closure);

  StartRoutine start_routine_;
  void* arg_;
  std::function<void()> closure_;
  std::shared_ptr<Task> task_;

  // We use std::thread introduced from c++11 as the thread entity
  std::thread thread_;

  mutable SpinLock status_lock_;
  Status status_;

  void DoStart();
};

}  // namespace concurrency
}  // namespace cnetpp

#endif  // CNETPP_CONCURRENCY_THREAD_H_

