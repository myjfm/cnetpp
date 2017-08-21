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
#ifndef CNETPP_BASE_LOG_H_
#define CNETPP_BASE_LOG_H_

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>

namespace cnetpp {
namespace base {

class Log {
 public:
  enum class Level : uint8_t {
    kDebug,
    kInfo,
    kWarn,
    kError,
    kFatal,
  };

  Log() : func_(&DefaultFunc) {
  }

  inline void set_func(void (*func)(Level level, const char*)) {
    func_ = func;
  }

  inline void Debug(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    VPrintf(Level::kDebug, fmt, ap);
    va_end(ap);
  }
  inline void Info(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    VPrintf(Level::kInfo, fmt, ap);
    va_end(ap);
  }
  inline void Warn(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    VPrintf(Level::kWarn, fmt, ap);
    va_end(ap);
  }
  inline void Error(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    VPrintf(Level::kError, fmt, ap);
    va_end(ap);
  }
  inline void Fatal(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    VPrintf(Level::kFatal, fmt, ap);
    va_end(ap);
  }

  static void DefaultFunc(Level level, const char* msg);

 private:
  void VPrintf(Level level, const char* fmt, va_list ap);

  static int FormatTime(char* buffer, size_t size);

  void (*func_)(Level level, const char*);
};

extern Log LOG;

#define Debug(fmt, ...) \
  cnetpp::base::LOG.Debug(" %s:%d] " fmt, __FILE__, __LINE__, ##__VA_ARGS__);

#define Info(fmt, ...) \
  cnetpp::base::LOG.Info(" %s:%d] " fmt, __FILE__, __LINE__, ##__VA_ARGS__);

#define Warn(fmt, ...) \
  cnetpp::base::LOG.Warn(" %s:%d] " fmt, __FILE__, __LINE__, ##__VA_ARGS__);

#define Error(fmt, ...) \
  cnetpp::base::LOG.Error(" %s:%d] " fmt, __FILE__, __LINE__, ##__VA_ARGS__);

#define Fatal(fmt, ...) \
  cnetpp::base::LOG.Fatal(" %s:%d] " fmt, __FILE__, __LINE__, ##__VA_ARGS__);

}  // namespace base

}  // namespace cnetpp

#endif  // CNETPP_BASE_LOG_H_

