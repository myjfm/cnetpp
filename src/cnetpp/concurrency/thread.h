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
#include <atomic>
#include <thread>
#include <string>

namespace cnetpp {
namespace concurrency {

// DO NOT inherit it, instead, inherit the Task interface
class Thread final {
public:
  using Id = std::thread::id;
  static const size_t kMaxNameLength = 16;

  enum class Status : int {
    kInit = 0x0,
    kRunning = 0x1,
    kStop = 0x3,
  };

  explicit Thread(std::shared_ptr<Task> task,
      const std::string& name = "");
  explicit Thread(const std::function<bool()>& closure,
      const std::string& name = "");

  ~Thread();

  Thread(const Thread&) = delete;
  Thread& operator=(const Thread&) = delete;

  Status GetStatus() const noexcept {
    return status_.load(std::memory_order_acquire);
  }

  void Start();

  void Stop();

  const std::string& name() const {
    return name_;
  }

  Id GetId() const noexcept {
    return thread_->get_id();
  }

  void Join();

  // check whether this thread is joinable
  bool IsJoinable() const noexcept {
    return thread_->joinable();
  }

  void SetThreadPoolIndex(int index);
  // index in thread pool, if not belong to any thread pool,
  // below will return -1
  int ThreadPoolIndex() const;
  int ThreadIndex() const;
  // current allocated max threads
  static int MaxThreadIndex();
  // use below method in thread, otherwise will return nullptr
  static const Thread* ThisThread();
 private:
  class InternalTask : public Task {
   public:
    InternalTask(const std::function<bool()>& closure) : closure_(closure) {
    }

    ~InternalTask() = default;

    bool operator()(void* arg = nullptr) override {
      (void) arg;
      return closure_();
    }

   private:
    std::function<bool()> closure_;
  };

  int thread_index_;
  int thread_pool_index_;

  std::string name_;

  std::shared_ptr<Task> task_;

  std::unique_ptr<std::thread> thread_;

  std::atomic<Status> status_ { Status::kInit };
 private:
  static std::atomic<int> cnetpp_max_thread_index_;
};

}  // namespace concurrency
}  // namespace cnetpp

#endif  // CNETPP_CONCURRENCY_THREAD_H_

