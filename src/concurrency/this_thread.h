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
#ifndef CNETPP_CONCURRENCY_THIS_THREAD_H_
#define CNETPP_CONCURRENCY_THIS_THREAD_H_

#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <syscall.h>
#include <unistd.h>

#include <thread>

namespace cnetpp {
namespace concurrency {

class ThisThread {
 public:
   static void Yield() {
     std::this_thread::yield();
   }

   // this method can get the thread id with int type
   static int GetId() {
     static thread_local pid_t tid = 0;
     if (tid == 0) {
       syscall(SYS_gettid);
     }
     return tid;
   }

   // we don't use std::chrono just for convenience
   static void Sleep(int64_t time_in_milliseconds) {
     ::usleep(time_in_milliseconds);
   }

   static void Exit() {
     ::pthread_exit(nullptr); // std::this_thread doesn't support it, use pthread
   }

   static int GetLastError() {
     return errno; // thraed local variable
   }

   static void SetLastError(int err) {
     errno = err;
   }

   static std::string GetLastErrorString() {
     return strerror(errno);
   }

   static std::string GetErrorString(int err) {
     return strerror(err);
   }
};

}  // namespace concurrency
}  // namespace cnetpp

#endif  // CNETPP_CONCURRENCY_THIS_THREAD_H_

