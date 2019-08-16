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
#include <cnetpp/base/log.h>

#include <pthread.h>

#include <cassert>

namespace cnetpp {
namespace concurrency {

namespace {
  thread_local Thread* cnetpp_current_thread = nullptr;
}

std::atomic<int> Thread::cnetpp_max_thread_index_{0};

Thread::Thread(std::shared_ptr<Task> task, const std::string& name)
    : name_(name), task_(std::move(task)) {
  if (name_.length() > kMaxNameLength) {
    CnetppFatal("Thread name is too long: %s", name_.c_str());
  }
  thread_index_ = cnetpp_max_thread_index_.fetch_add(1);
  thread_pool_index_ = -1;
}

Thread::Thread(const std::function<bool()>& closure, const std::string& name)
    : Thread(std::static_pointer_cast<Task>(
          std::make_shared<InternalTask>(closure)), name) {
}

Thread::~Thread() {
  Stop();
}

void Thread::Start() {
  if (!task_.get()) {
    CnetppFatal("task_ is null, please pass a valid task!");
  }

  Status old = Status::kInit;
  if (!status_.compare_exchange_strong(old, Status::kRunning)) {
    CnetppFatal("Failed to start thread, invalid thread status: %d",
        static_cast<int>(old));
  }

  if (thread_.get()) {
    CnetppFatal("thread_ is not null, there must be something wrong.");
  }

  thread_ = std::make_unique<std::thread>([this] () {
#if defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__)
    pthread_setname_np(name_.c_str());
#else
    pthread_setname_np(pthread_self(), name_.c_str());
#endif
    cnetpp_current_thread = this;
    (*task_)();
  });
}

void Thread::Stop() {
  Status old = Status::kRunning;
  if (!status_.compare_exchange_strong(old, Status::kStop)) {
    return;
  }

  task_->Stop();

  if (thread_->joinable()) {
    thread_->join();
  }
  thread_.reset();
}

void Thread::Join() {
  if (!thread_.get()) {
    return;
  }

  if (!IsJoinable()) {
    CnetppFatal("The thread is not joinable!");
  }
  thread_->join();
  thread_.reset();
  status_.store(Status::kStop, std::memory_order_release);
}

void Thread::SetThreadPoolIndex(int index) {
  thread_pool_index_ = index;
}

int Thread::ThreadPoolIndex() const {
  return thread_pool_index_;
}

int Thread::ThreadIndex() const {
  return thread_index_;
}

const Thread* Thread::ThisThread() {
  return cnetpp_current_thread;
}

int Thread::MaxThreadIndex() {
  return cnetpp_max_thread_index_;
}

}  // namespace concurrency
}  // namespace cnetpp

