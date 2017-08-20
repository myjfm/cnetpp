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
#include <cnetpp/concurrency/this_thread.h>

#include <errno.h>
#include <string.h>
#include <pthread.h>
#if defined(linux) || defined(__linux) || defined(__linux__)
#include <syscall.h>
#elif defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__)
#include <sys/syscall.h>
#endif
#include <unistd.h>

namespace cnetpp {
namespace concurrency {

int ThisThread::GetId() {
  static thread_local pid_t tid = 0;
  if (tid == 0) {
#if defined(linux) || defined(__linux) || defined(__linux__) || \
    defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__)
  #ifndef __NR_gettid
    #if defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__)
      #define __NR_gettid SYS_gettid
    #else
      #define __NR_gettid 224
    #endif
  #endif
    tid = syscall(__NR_gettid);
#endif
    if (tid <= 0) {
#if defined(linux) || defined(__linux) || defined(__linux__)
      tid = getpid();
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
      tid = GetCurrentThreadId();
#else
      tid = (pid_t)(uintptr_t)pthread_self();
#endif
    }
  }
  return tid;
}

void ThisThread::Sleep(int64_t time_in_milliseconds) {
  usleep(time_in_milliseconds);
}

void ThisThread::Exit() {
  ::pthread_exit(nullptr);  // std::this_thread doesn't support it, use pthread
}

int ThisThread::GetLastError() {
  return errno;  // thraed local variable
}

void ThisThread::SetLastError(int err) {
  errno = err;
}

std::string ThisThread::GetLastErrorString() {
  return strerror(errno);
}

std::string ThisThread::GetErrorString(int err) {
  return strerror(err);
}

}  // namespace concurrency
}  // namespace cnetpp
