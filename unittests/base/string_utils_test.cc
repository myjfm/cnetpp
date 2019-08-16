#include <gtest/gtest.h>

#include <string>

#include "cnetpp/base/string_utils.h"

using namespace std;
using namespace cnetpp::base;

TEST(StringUtilsTest, SplitByCharsTest01) {
  std::string str = "abc defhi gk";
  auto res = StringUtils::SplitByChars(StringPiece(str.data(), 7), " ");
  ASSERT_EQ((size_t)2, res.size());
  res = StringUtils::SplitByChars(str, " ");
  ASSERT_EQ((size_t)3, res.size());
}

TEST(StringUtilsTest, Varint32Test) {
  for (int32_t i = 0; i < 0x7fffffff; ++i) {
    char buf[10];
    auto length = StringUtils::ToVarint32(i, buf);
    uint32_t j;
    StringUtils::ParseVarint32(
        cnetpp::base::StringPiece(buf, length), &j);
    ASSERT_EQ((uint32_t)i, j);
  }
}

TEST(StringUtilsTest, Uint32Test) {
  for (uint32_t i = 0; i < 0xffffffff; ++i) {
    char buf[10];
    StringUtils::PutUint32(i, buf);
    auto j = StringUtils::ToUint32(
        cnetpp::base::StringPiece(buf, sizeof(uint32_t)));
    ASSERT_EQ(i, j);
  }
}
