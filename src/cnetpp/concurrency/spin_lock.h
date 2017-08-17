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
#ifndef CNETPP_CONCURRENCY_SPIN_LOCK_H_
#define CNETPP_CONCURRENCY_SPIN_LOCK_H_

#include <assert.h>

#include <thread>
#include <atomic>

namespace cnetpp {
namespace concurrency {

class SpinLock final {
 public:
  class ScopeGuard final {
   public:
    explicit ScopeGuard(SpinLock& lock) : lock_(lock) {
      lock_.Lock();
    }
    
    ~ScopeGuard() {
      lock_.Unlock();
    }
    
    // disallow copy and move operations
    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;
    ScopeGuard(ScopeGuard&&) noexcept = delete;
    ScopeGuard& operator=(ScopeGuard&&) noexcept = delete;
    
   private:
    SpinLock& lock_;
  };
  
  SpinLock() {
    flag_.clear(std::memory_order_release);
  }

  ~SpinLock() = default;

  void Lock() {
    auto i = 0;
    auto dummy = 0;
    while (!TryLock()) {
      i++;
      if (i < 16) {
        continue;
      } else if (i < 32) {
        dummy = i;
        dummy++;
      } else {
        std::this_thread::yield(); // schedule this thread again
      }
    }
  }

  bool TryLock() {
    return !flag_.test_and_set(std::memory_order_acquire);
  }

  void Unlock() {
    flag_.clear(std::memory_order_release);
  }

  // disallow copy and move operations
  SpinLock(const SpinLock&) = delete;
  SpinLock& operator=(const SpinLock&) = delete;
  SpinLock(SpinLock&&) noexcept = delete;
  SpinLock& operator=(SpinLock&&) noexcept = delete;

 private:
  std::atomic_flag flag_;
};

}  // namespace concurrency
}  // namespace cnetpp

#endif  // CNETPP_CONCURRENCY_SPIN_LOCK_H_
