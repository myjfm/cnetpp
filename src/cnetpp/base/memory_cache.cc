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
#include <cnetpp/base/memory_cache.h>

#include <string.h>
#include <unistd.h>
#include <assert.h>

#define CNETPP_PTHREAD_CALL(result) do { \
  if ((result) != 0) abort(); \
} while(0)

namespace cnetpp {
namespace base {

struct MemoryAddress {
  uint32_t              length_;
  struct MemoryAddress* next_;
  char                  data_[];
};

struct MemoryClass {
  uint32_t        length_;
  uint64_t        size_;
  MemoryAddress*  addrs_;
};

class MemoryCacheTLS {
 public:
  using Stats = std::vector<std::tuple<uint32_t, uint32_t, uint64_t>>;

  MemoryCacheTLS() : prev_(nullptr), next_(nullptr) {
    memset(mem_low_, 0, sizeof(MemoryClass) * LOW_CLASS);
    memset(mem_high_, 0, sizeof(MemoryClass) * HIGH_CLASS);
    memset(&mem_normal_, 0, sizeof(MemoryClass));
    tid_ = cnetpp::concurrency::ThisThread::GetId();
  }
  ~MemoryCacheTLS();

  void* Allocate(uint32_t n);
  void* Recycle(void* ptr, uint32_t* len);
  void Deallocate(void* ptr);

 private:
  // 4k 8k 16k 32k 64k 128k 256k 512k
  int GetLowMemoryIndex(uint32_t n) const {
    assert(n >= 4096 && n < 1024 * 1024);
    int x = 0;
    n = n / 4096;
    while (n) {
      n = n >> 1;
      x = x + 1;
    }
    return x - 1;
  }

  // 1m 2m 3m 4m 5m 6m 7m 8m 9m 10m 11m 12m 13m 14m 15m 16m
  int GetHighMemoryIndex(uint32_t n) const {
    assert(n >= 1024 * 1024 && n < 16 * 1024 * 1024);
    return n / (1024 * 1024) - 1;
  }

  void* NewMemory(uint32_t n);
  void* TryRecycle(MemoryClass* memory_class, MemoryAddress* address, uint32_t* len);
  MemoryCacheTLS::Stats GetStats() const;

  friend class MemoryCache;
  constexpr static int LOW_CLASS  = 8;
  constexpr static int HIGH_CLASS = 16;
  constexpr static int MEMORY_ADDRESS_NORMAL = 2048;

  MemoryCacheTLS* prev_;
  MemoryCacheTLS* next_;
  MemoryClass     mem_low_[LOW_CLASS];
  MemoryClass     mem_high_[HIGH_CLASS];
  MemoryClass     mem_normal_;
  pid_t           tid_;
};

constexpr int MemoryCacheTLS::LOW_CLASS;
constexpr int MemoryCacheTLS::HIGH_CLASS;
constexpr int MemoryCacheTLS::MEMORY_ADDRESS_NORMAL;

MemoryCacheTLS::~MemoryCacheTLS() {
  {
    while (mem_normal_.addrs_ != nullptr) {
      MemoryAddress* ma  = mem_normal_.addrs_;
      mem_normal_.addrs_ = ma->next_;
      free(ma);
    }
  }

  for (int i = 0; i < LOW_CLASS; i++) {
    auto& clazz = mem_low_[i];
    while (clazz.addrs_ != nullptr) {
      MemoryAddress* ma = clazz.addrs_;
      clazz.addrs_      = ma->next_;
      free(ma);
    }
  }

  for (int i = 0; i < HIGH_CLASS; i++) {
    auto& clazz = mem_high_[i];
    while (clazz.addrs_ != nullptr) {
      MemoryAddress* ma = clazz.addrs_;
      clazz.addrs_      = ma->next_;
      free(ma);
    }
  }
}

void* MemoryCacheTLS::NewMemory(uint32_t n) {
  if (n == MEMORY_ADDRESS_NORMAL && mem_normal_.addrs_ != nullptr) {
    MemoryAddress* ma = mem_normal_.addrs_;
    mem_normal_.addrs_    = mem_normal_.addrs_->next_;
    mem_normal_.length_  -= 1;
    mem_normal_.size_    -= MEMORY_ADDRESS_NORMAL;
    return ma->data_;
  }

  auto ptr =
    reinterpret_cast<MemoryAddress*>(malloc(sizeof(MemoryAddress) + n));
  ptr->length_ = n;
  ptr->next_   = nullptr;
  return ptr->data_;
}

void* MemoryCacheTLS::Allocate(uint32_t n) {
  if (n >= 4096 && n < 1024 * 1024) {
    auto& memory_class  = mem_low_[GetLowMemoryIndex(n)];
    MemoryAddress** ptr = &memory_class.addrs_;
    while (*ptr != nullptr) {
      MemoryAddress* addr = *ptr;
      if (addr->length_ >= n) {
        *ptr = addr->next_;
        memory_class.length_ -= 1;
        memory_class.size_   -= addr->length_;
        return addr->data_;
      }
      ptr = &addr->next_;
    }
  } else if (n >= 1024 * 1024) {
    int index = HIGH_CLASS - 1;
    if (n < 16 * 1024 * 1024) {
      index = GetHighMemoryIndex(n);
    }
    auto& memory_class = mem_high_[index];
    MemoryAddress** ptr = &memory_class.addrs_;
    while (*ptr != nullptr) {
      MemoryAddress* addr = *ptr;
      if (addr->length_ >= n) {
        *ptr = addr->next_;
        memory_class.length_ -= 1;
        memory_class.size_   -= addr->length_;
        return addr->data_;
      }
      ptr = &addr->next_;
    }
  }

  return NewMemory(n);
}

void MemoryCacheTLS::Deallocate(void* ptr) {
  if (ptr == nullptr) return;
  auto addr =
    reinterpret_cast<MemoryAddress*>((char *)ptr - sizeof(MemoryAddress));
  if (addr->length_ == MEMORY_ADDRESS_NORMAL
      && mem_normal_.length_ < MemoryCache::Instance()->max_cache_normal()) {
    addr->next_    = mem_normal_.addrs_;
    mem_normal_.addrs_ = addr;
    mem_normal_.length_ += 1;
    mem_normal_.size_   += MEMORY_ADDRESS_NORMAL;
  } else {
    free(addr);
  }

  while (mem_normal_.length_ > MemoryCache::Instance()->max_cache_normal()) {
    MemoryAddress* ma = mem_normal_.addrs_;
    mem_normal_.addrs_    = mem_normal_.addrs_->next_;
    mem_normal_.length_  -= 1;
    mem_normal_.size_    -= MEMORY_ADDRESS_NORMAL;
    free(ma);
  }
}

void* MemoryCacheTLS::TryRecycle(MemoryClass* memory_class,
    MemoryAddress* address, uint32_t* len) {
  MemoryAddress** next = &memory_class->addrs_;

  while (*next != nullptr) {
    if ((*next)->length_ <= address->length_) break;
    next = &(*next)->next_;
  }

  address->next_ = *next;
  *next          = address;
  memory_class->length_ += 1;
  memory_class->size_   += address->length_;

  while (memory_class->length_ > MemoryCache::Instance()->max_cache_large()) {
    auto addr = memory_class->addrs_;
    memory_class->addrs_ = memory_class->addrs_->next_;
    memory_class->length_ -= 1;
    memory_class->size_   -= addr->length_;
    free(addr);
  }

  *len = MEMORY_ADDRESS_NORMAL;
  return NewMemory(MEMORY_ADDRESS_NORMAL);
}

void* MemoryCacheTLS::Recycle(void* ptr, uint32_t* len) {
  auto address =
    reinterpret_cast<MemoryAddress*>((char *)ptr - sizeof(MemoryAddress));
  if (address->length_ < 4096) {
    *len = address->length_;
    return ptr;
  }

  MemoryClass* memory_class;
  if (address->length_ < 1024 * 1024) {
    memory_class  = &mem_low_[GetLowMemoryIndex(address->length_)];
  } else {
    int index = HIGH_CLASS - 1;
    if (address->length_ < 16 * 1024 * 1024) {
      index = GetHighMemoryIndex(address->length_);
    }
    memory_class = &mem_high_[index];
  }
  return TryRecycle(memory_class, address, len);
}

MemoryCacheTLS::Stats MemoryCacheTLS::GetStats() const {
  MemoryCacheTLS::Stats stats(LOW_CLASS + HIGH_CLASS + 1);
  stats[0] = std::make_tuple(MEMORY_ADDRESS_NORMAL,
        mem_normal_.length_, mem_normal_.size_);
  for (int i = 0; i < LOW_CLASS; i++) {
    stats[i + 1] = std::make_tuple(4096 * (1 << i),
          mem_low_[i].length_, mem_low_[i].size_);
  }
  for (int i = 0; i < HIGH_CLASS; i++) {
    stats[i + 1 + LOW_CLASS] = std::make_tuple((i + 1) * 1024 * 1024,
          mem_high_[i].length_, mem_high_[i].size_);
  }
  return stats;
}

uint32_t MemoryCache::max_cache_normal_ = 1024;
uint32_t MemoryCache::max_cache_large_  = 8;
pthread_key_t            MemoryCache::tls_key_;
MemoryCacheHead          MemoryCache::h_;
__thread MemoryCacheTLS* MemoryCache::pool_ = nullptr;

MemoryCache* MemoryCache::Instance() {
  static MemoryCache cache;
  return &cache;
}

MemoryCache::MemoryCache() {
  CNETPP_PTHREAD_CALL(atexit(MemoryCache::CleanupAtMainExit));
  CNETPP_PTHREAD_CALL(pthread_key_create(&tls_key_, CleanupAtThreadExit));
}

MemoryCache::~MemoryCache() {}

void* MemoryCache::Allocate(uint32_t n) {
  EnsureHasMemoryCacheTLS();
  return pool_->Allocate(n);
}

void MemoryCache::Deallocate(void* ptr) {
  EnsureHasMemoryCacheTLS();
  pool_->Deallocate(ptr);
}

void* MemoryCache::Recycle(void* ptr, uint32_t* len) {
  EnsureHasMemoryCacheTLS();
  return ptr != nullptr ? pool_->Recycle(ptr, len) : ptr;
}

void MemoryCache::PrepareCleanupTLSAtThreadExit() {
  if (pthread_getspecific(tls_key_) == nullptr) {
    void* flag = (void *)(uintptr_t)0x1;
    CNETPP_PTHREAD_CALL(pthread_setspecific(tls_key_, flag));
  }
}

void MemoryCache::CleanupMemoryCacheTLS(MemoryCacheTLS* pool) {
  if (pool->next_ != nullptr) {
    pool->next_->prev_ = pool->prev_;
  }
  if (pool->prev_ != nullptr) {
    pool->prev_->next_ = pool->next_;
  }
  if (h_.head_ == pool) {
    h_.head_ = pool->next_;
  }
}

void MemoryCache::CleanupAtThreadExit(void* argv __attribute__((unused))) {
  if (pool_ != nullptr) {
    {
      std::lock_guard<std::mutex> guard(h_.mtx_);
      CleanupMemoryCacheTLS(pool_);
    }
    delete pool_;
    pool_ = nullptr;
  }
}

void MemoryCache::CleanupAtMainExit() {
  pid_t pid = getpid();
  MemoryCacheTLS* ptr = nullptr;
  {
    std::lock_guard<std::mutex> guard(h_.mtx_);
    ptr = h_.head_;
    while (ptr != nullptr) {
      if (ptr->tid_ == pid) break;
      ptr = ptr->next_;
    }
    if (ptr != nullptr) {
      CleanupMemoryCacheTLS(ptr);
    }
  }
  delete ptr;
}

void MemoryCache::EnsureHasMemoryCacheTLS() {
  if (pool_ == nullptr) {
    pool_ = new MemoryCacheTLS();
    std::lock_guard<std::mutex> guard(h_.mtx_);
    if (h_.head_ != nullptr) {
      pool_->next_ = h_.head_;
      h_.head_->prev_ = pool_;
    }
    h_.head_ = pool_;
    PrepareCleanupTLSAtThreadExit();
  }
}

MemoryCache::Stats MemoryCache::GetStats() const {
  MemoryCache::Stats stats;
  std::lock_guard<std::mutex> guard(h_.mtx_);
  auto ptr = h_.head_;
  while (ptr != nullptr) {
    stats.emplace_back(ptr->GetStats());
    ptr = ptr->next_;
  }
  return stats;
}

}  // namespace base
}  // namespace cnetpp
