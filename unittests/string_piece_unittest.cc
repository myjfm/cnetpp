#include <gtest/gtest.h>
#include <base/string_piece.h>
#include <string>
using namespace std;
using namespace cnetpp::base;

TEST(StringPiece, MiscTest) {
  StringPiece str((const char*)NULL, 0);
  ASSERT_TRUE(str.empty());
  string s;
  str.copy_to_string(&s);
  ASSERT_EQ(s, "");
}

TEST(StringPiece, CompareTest) {
  StringPiece str((const char*)NULL, 0);
  StringPiece str2("", 0);
  ASSERT_EQ(str, str2);
  ASSERT_EQ(str2, str2);
  ASSERT_EQ(str, str);
  ASSERT_TRUE(str == str2);
}

TEST(StringPiece, FindTest) {
  StringPiece str("This is a test string");
  ASSERT_EQ(str.find("is", 0), 2);
  ASSERT_EQ(str.find("is", 4), 5);
  ASSERT_EQ(str.find('i', 0), 2);
  str.set((const char*)NULL, 0);
  ASSERT_EQ(str.find('c', 9), StringPiece::npos);
}
