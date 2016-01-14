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
#include "thread_factory.h"

namespace cnetpp {
namespace concurrency {

std::once_flag ThreadFactory::once_flag_for_thread_factory_;
std::shared_ptr<ThreadFactory> ThreadFactory::thread_factory_(nullptr);

std::shared_ptr<ThreadFactory> ThreadFactory::Instance() {
  std::call_once(once_flag_for_thread_factory_, [] {
      thread_factory_.reset(new ThreadFactory());
    assert(thread_factory_.get() != nullptr);
  });
  return thread_factory_;
}

ThreadFactory::~ThreadFactory() {
  thread_factory_ = nullptr;
}

std::shared_ptr<Thread> ThreadFactory::CreateThread(
    std::shared_ptr<Task> task) {
  return std::shared_ptr<Thread>(new Thread(task));
}

std::shared_ptr<Thread> ThreadFactory::CreateThread(
    Thread::StartRoutine start_routine,
    void* arg) {
  return std::shared_ptr<Thread>(new Thread(start_routine, arg));
}

std::shared_ptr<Thread> ThreadFactory::CreateThread(
    const std::function<void()>& func) {
  return std::shared_ptr<Thread>(new Thread(func));
}

std::shared_ptr<Thread> ThreadFactory::CreateThread(
    std::function<void()>&& func) {
  return std::shared_ptr<Thread>(new Thread(std::move(func)));
}

std::shared_ptr<ThreadPool> ThreadFactory::CreateThreadPool(
    size_t thread_count,
    std::shared_ptr<QueueBase> queue) {
  return std::shared_ptr<ThreadPool>(new ThreadPool(std::move(queue),
                                                    thread_count));
}

}  // namespace concurrency
}  // namespace cnetpp

