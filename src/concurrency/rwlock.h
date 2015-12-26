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
//   * Neither the name of Shuo Chen nor the names of other contributors may be
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
#ifndef CNETPP_CONCURRENCY_RWLOCK_H_
#define CNETPP_CONCURRENCY_RWLOCK_H_

#include <assert.h>

#if _POSIX_VERSION >= 199506L &&  defined(_POSIX_THREADS)
#include <pthread.h> // wrap up the pthread_rwlock_t
#else
#include <mutex> // use std::mutex to simulate the rwlock
#endif

namespace cnetpp {
namespace concurrency {

class RWLock final {
 public:
  class ReadScopeGuard final {
   public:
    explicit ReadScopeGuard(RWLock& lock) : lock_(lock) {
      lock_.RDLock();
    }
    
    ~ReadScopeGuard() {
      lock_.RDUnlock();
    }
  
    // disallow copy and move operations
    ReadScopeGuard(const ReadScopeGuard&) = delete;
    ReadScopeGuard& operator=(const ReadScopeGuard&) = delete;
    ReadScopeGuard(ReadScopeGuard&&) noexcept = delete;
    ReadScopeGuard& operator=(ReadScopeGuard&&) noexcept = delete;

   private:
    RWLock& lock_;
  };

  class WriteScopeGuard final {
   public:
    explicit WriteScopeGuard(RWLock& lock) : lock_(lock) {
      lock_.WRLock();
    }
    
    ~WriteScopeGuard() {
      lock_.WRUnlock();
    }
  
    // disallow copy and move operations
    WriteScopeGuard(const WriteScopeGuard&) = delete;
    WriteScopeGuard& operator=(const WriteScopeGuard&) = delete;
    WriteScopeGuard(WriteScopeGuard&&) noexcept = delete;
    WriteScopeGuard& operator=(WriteScopeGuard&&) noexcept = delete;

   private:
    RWLock& lock_;
  };
  
  RWLock();
  ~RWLock();
  
  // disallow copy and move operations
  RWLock(const RWLock&) = delete;
  RWLock& operator=(const RWLock&) = delete;
  RWLock(RWLock&&) noexcept = delete;
  RWLock& operator=(RWLock&&) noexcept = delete;

  void RDLock();
  void RDUnlock();

  void WRLock();
  void WRUnlock();

 private:
#if _POSIX_VERSION >= 199506L && defined(_POSIX_THREADS)
  pthread_rwlock_t lock_;
#else
  std::mutex mutex_;
  int reader_count_;
  std::mutex reader_count_mutex_;
#endif
};

}  // namespace concurrency
}  // namespace cnetpp

#endif  // CNETPP_CONCURRENCY_RWLOCK_H_
