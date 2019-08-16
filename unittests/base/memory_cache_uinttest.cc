#include <cnetpp/base/memory_cache.h>
#include <gtest/gtest.h>

using namespace cnetpp::base;

void cleanup() { // cleanup
  MemoryCache::Instance()->max_cache_normal(0);
  MemoryCache::Instance()->max_cache_large(0);
  for (int i = 0; i < 8; i++) {
    uint32_t len = 0;
    void* ptr = MemoryCache::Instance()->Allocate(4096 * (1 << i));
    void* ptr2 = MemoryCache::Instance()->Recycle(ptr, &len);
    MemoryCache::Instance()->Deallocate(ptr2);
    ASSERT_EQ(len, 2048u);
  }
  for (int i = 0; i < 16; i++) {
    uint32_t len = 0;
    void* ptr = MemoryCache::Instance()->Allocate((i + 1) * 1024 * 1024);
    void* ptr2 = MemoryCache::Instance()->Recycle(ptr, &len);
    MemoryCache::Instance()->Deallocate(ptr2);
    ASSERT_EQ(len, 2048u);
  }
  MemoryCache::Instance()->max_cache_normal(16);
  MemoryCache::Instance()->max_cache_large(8);
}

TEST(MemoryCache, Test) {
  {
    uint32_t len = 0;
    void* ptr  = MemoryCache::Instance()->Allocate(4095);
    void* ptr2 = MemoryCache::Instance()->Recycle(ptr, &len);
    ASSERT_EQ(ptr, ptr2);
    MemoryCache::Instance()->Deallocate(ptr2);
    ASSERT_EQ(len, 4095u);
  }

  for (int i = 0; i < 8; i++) {
    uint32_t len = 0;
    void* ptr  = MemoryCache::Instance()->Allocate(4096 * (1 << i));
    void* ptr2 = MemoryCache::Instance()->Recycle(ptr, &len);
    ASSERT_FALSE(ptr == ptr2);
    MemoryCache::Instance()->Deallocate(ptr2);
    ASSERT_EQ(len, 2048u);
  }

  for (int i = 0; i < 16; i++) {
    uint32_t len = 0;
    void* ptr  = MemoryCache::Instance()->Allocate((i + 1) * 1024 * 1024);
    void* ptr2 = MemoryCache::Instance()->Recycle(ptr, &len);
    ASSERT_FALSE(ptr == ptr2);
    MemoryCache::Instance()->Deallocate(ptr2);
    ASSERT_EQ(len, 2048u);
  }

  { // 17m
    uint32_t len = 0;
    void* ptr  = MemoryCache::Instance()->Allocate(17 * 1024 * 1024);
    void* ptr2 = MemoryCache::Instance()->Recycle(ptr, &len);
    ASSERT_FALSE(ptr == ptr2);
    MemoryCache::Instance()->Deallocate(ptr2);
    ASSERT_EQ(len, 2048u);
  }

  cleanup();

  { // max-8
    void* ptr[9];
    for (int i = 0; i < 9; i++) {
      ptr[i] = MemoryCache::Instance()->Allocate(1024 * 1024);
    }
    for (int i = 0; i < 9; i++) {
      uint32_t len = 0;
      void* ptr2 = MemoryCache::Instance()->Recycle(ptr[i], &len);
      MemoryCache::Instance()->Deallocate(ptr2);
      ASSERT_EQ(len, 2048u);
    }
    for (int i = 0; i < 9; i++) {
      ptr[i] = MemoryCache::Instance()->Allocate(1024 * 1024);
    }
    for (int i = 0; i < 9; i++) {
      uint32_t len = 0;
      void* ptr2 = MemoryCache::Instance()->Recycle(ptr[i], &len);
      MemoryCache::Instance()->Deallocate(ptr2);
      ASSERT_EQ(len, 2048u);
    }
  }

  cleanup();

  {
    void* ptr[9];
    for (int i = 0; i < 9; i++) {
      ptr[i] = MemoryCache::Instance()->Allocate(1024 * 1024 + i);
    }
    for (int i = 0; i < 9; i++) {
      uint32_t len = 0;
      void* ptr2 = MemoryCache::Instance()->Recycle(ptr[i], &len);
      MemoryCache::Instance()->Deallocate(ptr2);
      ASSERT_EQ(len, 2048u);
    }
    for (int i = 0; i < 9; i++) {
      ptr[i] = MemoryCache::Instance()->Allocate(1024 * 1024 + 9 - i);
    }
    for (int i = 0; i < 9; i++) {
      uint32_t len = 0;
      void* ptr2 = MemoryCache::Instance()->Recycle(ptr[i], &len);
      MemoryCache::Instance()->Deallocate(ptr2);
      ASSERT_EQ(len, 2048u);
    }
  }

  cleanup();

  {
    void* ptr[9];
    for (int i = 0; i < 9; i++) {
      ptr[i] = MemoryCache::Instance()->Allocate(1024 * 1024 + 9 - i);
    }
    for (int i = 0; i < 9; i++) {
      uint32_t len = 0;
      void* ptr2 = MemoryCache::Instance()->Recycle(ptr[i], &len);
      MemoryCache::Instance()->Deallocate(ptr2);
      ASSERT_EQ(len, 2048u);
    }
    for (int i = 0; i < 9; i++) {
      ptr[i] = MemoryCache::Instance()->Allocate(1024 * 1024 + i);
    }
    for (int i = 0; i < 9; i++) {
      uint32_t len = 0;
      void* ptr2 = MemoryCache::Instance()->Recycle(ptr[i], &len);
      MemoryCache::Instance()->Deallocate(ptr2);
      ASSERT_EQ(len, 2048u);
    }
  }

  cleanup();

  {
    void* ptr[9];
    for (int i = 0; i < 9; i+=2) {
      ptr[i] = MemoryCache::Instance()->Allocate(1024 * 1024 + 9 - i);
    }
    for (int i = 1; i < 9; i+=2) {
      ptr[i] = MemoryCache::Instance()->Allocate(1024 * 1024 + 9 - i);
    }
    for (int i = 0; i < 9; i++) {
      uint32_t len = 0;
      void* ptr2 = MemoryCache::Instance()->Recycle(ptr[i], &len);
      MemoryCache::Instance()->Deallocate(ptr2);
      ASSERT_EQ(len, 2048u);
    }
    for (int i = 0; i < 9; i+=2) {
      ptr[i] = MemoryCache::Instance()->Allocate(1024 * 1024 + i);
    }
    for (int i = 1; i < 9; i+=2) {
      ptr[i] = MemoryCache::Instance()->Allocate(1024 * 1024 + i);
    }
    for (int i = 0; i < 9; i++) {
      uint32_t len = 0;
      void* ptr2 = MemoryCache::Instance()->Recycle(ptr[i], &len);
      MemoryCache::Instance()->Deallocate(ptr2);
      ASSERT_EQ(len, 2048u);
    }
  }
}
