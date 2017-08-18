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
#include <cnetpp/base/log.h>
#include <cnetpp/concurrency/this_thread.h>

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <memory>

namespace cnetpp {
namespace base {

Log LOG;

void Log::DefaultFunc(Level level, const char* msg) {
  char str_time[22];
  int time_len = FormatTime(str_time, 22);
  if (time_len < 0) {
    return;
  }
  switch (level) {
    case Level::kDebug:
      fprintf(stdout, "D%s %d%s\n", str_time,
          concurrency::ThisThread::GetId(), msg);
      break;
    case Level::kInfo:
      fprintf(stdout, "I%s %d%s\n", str_time,
          concurrency::ThisThread::GetId(), msg);
      break;
    case Level::kWarn:
      fprintf(stdout, "W%s %d%s\n", str_time,
          concurrency::ThisThread::GetId(), msg);
      break;
    case Level::kError:
      fprintf(stdout, "E%s %d%s\n", str_time,
          concurrency::ThisThread::GetId(), msg);
      break;
    case Level::kFatal:
      fprintf(stdout, "F%s %d%s\n", str_time,
          concurrency::ThisThread::GetId(), msg);
      abort();
      break;
    default:
      abort();
      break;
  }
}

int Log::FormatTime(char* buffer, size_t size) {
  uint64_t now = std::chrono::duration_cast<std::chrono::microseconds>(
      std::chrono::system_clock::now().time_since_epoch()).count();
  time_t sec = now / 1000000;
  int usec = now % 1000000;
  struct tm tm;
  localtime_r(&sec, &tm);
  int len = static_cast<int>(strftime(buffer, size, "%m%d %H:%M:%S", &tm));
  if (len == 0) {
    return -1;
  }
  len += snprintf(buffer + len, size - len, ".%06d", usec);
  if (len <= 0) {
    return -1;
  }
  buffer[len] = '\0';
  return len;
}

void Log::VPrintf(Level level, const char* fmt, va_list ap) {
  static const int kStackBufferSize = 256;
  char stack_buf[kStackBufferSize];

  int need = vsnprintf(stack_buf, kStackBufferSize, fmt, ap);
  if (need < kStackBufferSize) {
    func_(level, stack_buf);
  } else {
    std::unique_ptr<char[]> heap_buf(new char[need + 1]);
    int rval = vsnprintf(heap_buf.get(), need + 1, fmt, ap);
    if (rval != -1) {
      func_(level, heap_buf.get());
    }
  }
}

}  // namespace base

}  // namespace cnetpp

