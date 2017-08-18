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
#include <cnetpp/concurrency/thread.h>

#include <assert.h>

namespace cnetpp {
namespace concurrency {

Thread::Thread(Thread::StartRoutine start_routine, void* arg)
    : start_routine_(start_routine),
      arg_(arg),
      closure_(nullptr),
      task_(nullptr),
      status_(Status::kInit) { }

Thread::Thread(std::shared_ptr<Task> task)
    : start_routine_(nullptr),
      arg_(nullptr),
      closure_(nullptr),
      task_(std::move(task)),
      status_(Status::kInit) { }

Thread::Thread(const std::function<void()>& closure)
    : start_routine_(nullptr),
      arg_(nullptr),
      closure_(closure),
      task_(nullptr),
      status_(Status::kInit) { }

Thread::Thread(std::function<void()>&& closure)
    : start_routine_(nullptr),
      arg_(nullptr),
      closure_(std::move(closure)),
      task_(nullptr),
      status_(Status::kInit) { }

Thread::~Thread() {
  Stop();
}

void Thread::Start() {
  {
    SpinLock::ScopeGuard guard(status_lock_);
    if (status_ != Status::kInit) {
      return;
    }
    status_ = Status::kRunning;
  }

  thread_ = std::thread([this] () {
    DoStart();
  });
}

void Thread::DoStart() {
  if (start_routine_) {
    start_routine_(arg_);
  } else if (task_) {
    (*(task_.get()))();
  } else if (closure_) {
    closure_();
  }
}

void Thread::Stop() {
  {
    SpinLock::ScopeGuard guard(status_lock_);
    if (status_ != Status::kRunning && status_ != Status::kSuspend) {
      return;
    }
    status_ = Status::kStop;
  }

  if (task_) {
    task_->Stop();
  }

  if (thread_.joinable()) {
    thread_.join();
  }
}

}  // namespace concurrency
}  // namespace cnetpp

