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
#include <cnetpp/concurrency/queue_base.h>

#include <chrono>

namespace cnetpp {
namespace concurrency {

std::shared_ptr<QueueBase> CreateDefaultQueue() {
	return std::make_shared<DefaultQueue>();
}

DefaultQueue::~DefaultQueue() {
	stop_.store(true, std::memory_order_release);
	cond_var_.notify_all();
}

void DefaultQueue::Push(std::shared_ptr<Task> task) {
  std::unique_lock<std::mutex> guard(mutex_);
	queue_.push(std::move(task));
	cond_var_.notify_one();
}

std::shared_ptr<Task> DefaultQueue::TryPop() {
  std::unique_lock<std::mutex> guard(mutex_);
	if (queue_.empty()) {
		return nullptr;
  }
  std::shared_ptr<Task> result(std::move(queue_.front()));
	queue_.pop();
	return result;
}
	
std::shared_ptr<Task> DefaultQueue::WaitPop(int timeout) {
  std::unique_lock<std::mutex> guard(mutex_);
	if (timeout <= 0) {
		cond_var_.wait(guard, [this] {
        return (!queue_.empty() || stop_.load(std::memory_order_acquire));
    });
	} else {
    if (!cond_var_.wait_for(guard,
                            std::chrono::milliseconds(timeout),
                            [this] {
                              return (!queue_.empty() ||
                                      stop_.load(std::memory_order_acquire));
                            })) {
      return nullptr; // timed out
    }
  }
  
  if (stop_) {
    return nullptr;
  }
  
  std::shared_ptr<Task> task(std::move(queue_.front()));
  queue_.pop();
  return task;
}

bool DefaultQueue::Empty() const {
  std::unique_lock<std::mutex> lGuard(mutex_);
  return queue_.empty();
}

size_t DefaultQueue::Size() const {
  std::unique_lock<std::mutex> lGuard(mutex_);
  return queue_.size();
}

} // end of namespace concurrency
} // end of namespace cnetpp

