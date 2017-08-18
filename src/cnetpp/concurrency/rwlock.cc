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
#include <cnetpp/concurrency/rwlock.h>

namespace cnetpp {
namespace concurrency {

#if _POSIX_VERSION >= 199506L && defined(_POSIX_THREADS)
#define ASSERT_FUNC(func) do { assert(!func(&lock_)); } while (0)

RWLock::RWLock() {
  pthread_rwlock_init(&lock_, nullptr);
}

RWLock::~RWLock() {
  ASSERT_FUNC(pthread_rwlock_destroy);
}

void RWLock::RDLock() {
  ASSERT_FUNC(pthread_rwlock_rdlock);
}

void RWLock::WRLock() {
  ASSERT_FUNC(pthread_rwlock_wrlock);
}

void RWLock::RDUnlock() {
  ASSERT_FUNC(pthread_rwlock_unlock);
}
void RWLock::WRUnlock() {
  ASSERT_FUNC(pthread_rwlock_unlock);
}

#undef ASSERT_FUNC
#else
RWLock::RWLock() : reader_count_(0) {
}

RWLock::~RWLock() {
}

void RWLock::RDLock() {
  std::unique_lock<std::mutex> reader_count_guard(reader_count_mutex_);
  if (++reader_count_ == 1) {
    mutex_.lock();
  }
}

void RWLock::RDUnlock() {
  std::unique_lock<std::mutex> reader_count_guard(reader_count_mutex_);
  if (--reader_count_ == 0) {
    mutex_.unlock();
  }
}

void RWLock::WRLock() {
  mutex_.lock();
}

void RWLock::WRUnlock() {
  mutex_.unlock();
}
#endif  // _POSIX_VERSION >= 199506L && defined(_POSIX_THREADS)

}  // namespace concurrency
}  // namespace cnetpp

