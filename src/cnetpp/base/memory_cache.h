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
#ifndef CNETPP_BASE_MEMORY_CACHE_H_
#define CNETPP_BASE_MEMORY_CACHE_H_

#include <stdint.h>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <deque>
#include <vector>
#include <pthread.h>

namespace cnetpp {
namespace base {

class MemoryCacheTLS;

typedef struct MemoryCacheHead {
  MemoryCacheHead() : head_(nullptr) {}
  std::mutex      mtx_;
  MemoryCacheTLS* head_;
} MemoryCacheHead;

class MemoryCache {
 public:
  using Stats =
    std::deque<std::vector<std::tuple<uint32_t, uint32_t, uint64_t>>>;

  static MemoryCache* Instance();

  ~MemoryCache();

  void* Allocate(uint32_t n);
  void Deallocate(void* ptr);
  void* Recycle(void* ptr, uint32_t* len);

  void max_cache_normal(uint32_t n) {
    max_cache_normal_ = n;
  }
  uint32_t max_cache_normal() const {
    return max_cache_normal_;
  }

  void max_cache_large(uint32_t n) {
    max_cache_large_ = n;
  }
  uint32_t max_cache_large() const {
    return max_cache_large_;
  }

  void EnsureHasMemoryCacheTLS();
  MemoryCache::Stats GetStats() const;

 private:
  MemoryCache();

  static void PrepareCleanupTLSAtThreadExit();
  static void CleanupAtThreadExit(void* argv __attribute__((unused)));
  static void CleanupAtMainExit();
  static void CleanupMemoryCacheTLS(MemoryCacheTLS* pool);

  static uint32_t  max_cache_normal_;
  static uint32_t  max_cache_large_;
  static pthread_key_t            tls_key_;
  static MemoryCacheHead          h_;
  static __thread MemoryCacheTLS* pool_;
};

}  // namespace base
}  // namespace cnetpp

#endif  // CNETPP_BASE_MEMORY_CACHE_H_
