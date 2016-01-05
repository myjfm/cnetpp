#include <gtest/gtest.h>
#include <base/csonpp.h>
#include <string>
using namespace std;
using namespace cnetpp::base;

TEST(Value, ConstructTest)
{
  {
    Value v(1);
    ASSERT_EQ(v.GetInteger(), 1);
  }
  {
    string str= "hello";
    Value v(str);
    ASSERT_EQ(v.GetString(), "hello");
  }
  {
    Value v("hello");
    ASSERT_EQ(v.GetString(), "hello");
  }
  {
    Value v(1.23);
    ASSERT_EQ(v.GetDouble(), 1.23);
  }
  {
    Value v(false);
    ASSERT_EQ(v.GetBool(), false);
  }
  {
    Value v(true);
    ASSERT_EQ(v.GetBool(), true);
  }
}

TEST(Value, AssignTest)
{
  Value v;
  v = 12;
  ASSERT_EQ(v.Type(), Value::ValueType::kInteger);
  ASSERT_EQ(v.GetInteger(), 12);
  v = 12.3;
  ASSERT_EQ(v.Type(), Value::ValueType::kDouble);
  ASSERT_EQ(v.GetDouble(), 12.3);
  v = false;
  ASSERT_EQ(v.Type(), Value::ValueType::kBool);
  ASSERT_EQ(v.GetBool(), false);
  v = "hello";
  ASSERT_EQ(v.Type(), Value::ValueType::kString);
  ASSERT_EQ(v.GetString(), "hello");
}
