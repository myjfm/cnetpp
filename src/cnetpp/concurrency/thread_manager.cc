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
#include <cnetpp/concurrency/thread_manager.h>
#include <cnetpp/concurrency/thread_factory.h>

#include <assert.h>

namespace cnetpp {
namespace concurrency {

std::shared_ptr<ThreadManager> ThreadManager::thread_manager_(nullptr);
std::once_flag ThreadManager::once_flag_for_thread_manager_;

std::shared_ptr<ThreadManager> ThreadManager::Instance() {
  std::call_once(once_flag_for_thread_manager_, [] {
    thread_manager_.reset(new ThreadManager());
    assert(thread_manager_.get() != nullptr);
  });
  return thread_manager_;
}

ThreadManager::~ThreadManager() {
  StopAll();
  thread_manager_ = nullptr;
}

void ThreadManager::StartAll() {
  // First start all Threads.
  {
    std::lock_guard<std::mutex> guard(all_threads_mutex_);
    for (auto &thread : all_threads_) {
      thread->Start();
    }
  }

  // Then start all ThreadPools.
  {
    std::lock_guard<std::mutex> guard(all_thread_pools_mutex_);
    for (auto &thread_pool : all_thread_pools_) {
      thread_pool->Start();
    }
  }
}

void ThreadManager::StopAll() {
  // First start all Threads.
  {
    std::lock_guard<std::mutex> guard(all_threads_mutex_);
    for (auto &thread : all_threads_) {
      thread->Stop();
    }
  }

  // Then start all ThreadPools.
  {
    std::lock_guard<std::mutex> guard(all_thread_pools_mutex_);
    for (auto &thread_pool : all_thread_pools_) {
      thread_pool->Stop();
    }
  }
}

void ThreadManager::DestroyThread(std::shared_ptr<Thread> thread) {
  if (!thread)
    return;

  if (RemoveThread(thread)) {
    thread->Stop();
  }
}

void ThreadManager::DestroyThreadPool(std::shared_ptr<ThreadPool> thread_pool) {
  if (!thread_pool.get())
    return;

  if (RemoveThreadPool(thread_pool)) {
    thread_pool->Stop();
  }
}

std::shared_ptr<Thread> ThreadManager::CreateThread(std::shared_ptr<Task> task)
{
  auto new_thread = ThreadFactory::Instance()->CreateThread(task);
  AddThread(new_thread);
  // start the thread automatically
  new_thread->Start();
  return std::move(new_thread);
}

std::shared_ptr<Thread> ThreadManager::CreateThread(
    Thread::StartRoutine start_routine,
    void* arg) {
  auto new_thread = ThreadFactory::Instance()->CreateThread(start_routine, arg);
  AddThread(new_thread);
  // start the thread automatically
  new_thread->Start();
  return std::move(new_thread);
}

std::shared_ptr<Thread> ThreadManager::CreateThread(
    const std::function<void()>& closure) {
  auto new_thread = ThreadFactory::Instance()->CreateThread(closure);
  AddThread(new_thread);
  // start the thread automatically
  new_thread->Start();
  return std::move(new_thread);
}

std::shared_ptr<Thread> ThreadManager::CreateThread(
    std::function<void()>&& closure) {
  auto new_thread = ThreadFactory::Instance()->CreateThread(std::move(closure));
  AddThread(new_thread);
  // start the thread automatically
  new_thread->Start();
  return std::move(new_thread);
}

std::shared_ptr<ThreadPool> ThreadManager::CreateThreadPool(
    size_t thread_count,
    std::shared_ptr<QueueBase> queue) {
  auto new_thread_pool = 
    ThreadFactory::Instance()->CreateThreadPool(thread_count, std::move(queue));
  AddThreadPool(new_thread_pool);
  // start the thread pool automatically
  new_thread_pool->Start();
  return std::move(new_thread_pool);
}

// just add the specific thread into buffer to easy to manage
void ThreadManager::AddThread(std::shared_ptr<Thread> thread) {
  std::lock_guard<std::mutex> guard(all_threads_mutex_);
  decltype(all_threads_) tmp_all_threads;
  tmp_all_threads.swap(all_threads_);
  for (auto& t : tmp_all_threads) {
    if (t && t->GetStatus() != Thread::Status::kStop) {
      all_threads_.push_back(std::move(t));
    }
  }

  all_threads_.push_back(std::move(thread));
}

// just remove the specific thread from buffer
bool ThreadManager::RemoveThread(std::shared_ptr<Thread> thread) {
  std::lock_guard<std::mutex> guard(all_threads_mutex_);
  auto result = false;
  decltype(all_threads_) tmp_all_threads;
  tmp_all_threads.swap(all_threads_);
  for (auto& t : tmp_all_threads) {
    if (!t) {
      continue;
    }

    if (t == thread) {
      result = true;
    }

    if (t->GetStatus() != Thread::Status::kStop && t != thread) {
      all_threads_.push_back(std::move(t));
    }
  }

  return result;
}

// just add the specific thread pool into buffer to easy to manage
void ThreadManager::AddThreadPool(std::shared_ptr<ThreadPool> thread_pool) {
  std::lock_guard<std::mutex> guard(all_thread_pools_mutex_);
  all_thread_pools_.push_back(std::move(thread_pool));
}

// just remove the specific thread pool from buffer
bool ThreadManager::RemoveThreadPool(std::shared_ptr<ThreadPool> thread_pool) {
  std::lock_guard<std::mutex> guard(all_thread_pools_mutex_);
  for (auto itr = all_thread_pools_.begin(); 
       itr != all_thread_pools_.end(); 
       ++itr) {
    if (*itr == thread_pool) {
      all_thread_pools_.erase(itr);
      return true;
    }
  }
  return false;
}

}  // namespace concurrency
}  // namespace cnetpp

