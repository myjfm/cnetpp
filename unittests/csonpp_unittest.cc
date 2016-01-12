#include <gtest/gtest.h>
#include <base/csonpp.h>
#include <string>
using namespace std;
using namespace cnetpp::base;

TEST(Value, ConstructTest) {
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

TEST(Value, AssignTest) {
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

TEST(Parser, DeSerializeToObject) {
  std::string str1("{}");
  Value value1 = Parser::Deserialize(str1);
  ASSERT_TRUE(value1.Type() == Value::ValueType::kObject);
  ASSERT_EQ(value1.Size(), 0);

  std::string str2("{\"abc\":1}");
  Value value2 = Parser::Deserialize(str2);
  ASSERT_TRUE(value2.Type() == Value::ValueType::kObject);
  ASSERT_EQ(value2.Size(), 1);
  ASSERT_EQ(value2["abc"].AsInteger(), 1);

  std::string str3("{\"abc\":1,\"def\":\"abc\"}");
  Value value3 = Parser::Deserialize(str3);
  ASSERT_TRUE(value3.Type() == Value::ValueType::kObject);
  ASSERT_EQ(value3.Size(), 2);
  ASSERT_EQ(value3["abc"].AsInteger(), 1);
  ASSERT_EQ(value3["def"].AsString(), "abc");

  std::string str4(" { \"hello\" : \"world\", \"t\" : true , \"f\" : false, \"n\": null, \"i\":123, \"pi\": 3.1416, \"a\":[1, 2, 3, 4] } ");
  Value value4 = Parser::Deserialize(str4);
  ASSERT_TRUE(value4.Type() == Value::ValueType::kObject);
  ASSERT_EQ(value4.Size(), 7);
  ASSERT_EQ(value4["hello"].AsString(), "world");
  ASSERT_TRUE(value4["t"].AsBool());
  ASSERT_FALSE(value4["f"].AsBool());
  ASSERT_TRUE(value4["n"].Type() == Value::ValueType::kNull);
  ASSERT_EQ(value4["i"].AsInteger(), 123);
  ASSERT_DOUBLE_EQ(value4["pi"].AsDouble(), 3.1416);
  ASSERT_EQ(value4["a"].Type(), Value::ValueType::kArray);
  ASSERT_EQ(value4["a"].Size(), 4);
  ASSERT_EQ(value4["a"][0].AsInteger(), 1);
  ASSERT_EQ(value4["a"][1].AsInteger(), 2);
  ASSERT_EQ(value4["a"][2].AsInteger(), 3);
  ASSERT_EQ(value4["a"][3].AsInteger(), 4);

  std::string str5("{");
  Value value5 = Parser::Deserialize(str5);
  ASSERT_EQ(value5.Type(), Value::ValueType::kDummy);

  std::string str6("{12");
  Value value6 = Parser::Deserialize(str6);
  ASSERT_EQ(value6.Type(), Value::ValueType::kDummy);

  std::string str7("");
  Value value7 = Parser::Deserialize(str7);
  ASSERT_EQ(value7.Type(), Value::ValueType::kDummy);

  std::string str8("{\"abc\":1,#this is first member\r\n\"def\":\"abc\\r\"}");
  Value value8 = Parser::Deserialize(str8);
  ASSERT_EQ(value8.Type(), Value::ValueType::kObject);
  ASSERT_EQ(value8["abc"].AsInteger(), 1);
  ASSERT_EQ(value8["def"].AsString(), "abc\r");

  std::string str9("{\"abc\":1#this is first member\r,\"def\":\"abc#\"#this second member\n}");
  Value value9 = Parser::Deserialize(str9);
  ASSERT_EQ(value9.Type(), Value::ValueType::kObject);
  ASSERT_EQ(value9["abc"].AsInteger(), 1);
  ASSERT_EQ(value9["def"].AsString(), "abc#");
}

TEST(CsonppTest, DeSerializeToArray) {
  std::string str1("[]");
  Value value1 = Parser::Deserialize(str1);
  ASSERT_TRUE(value1.Type() == Value::ValueType::kArray);
  ASSERT_EQ(value1.Size(), 0);

  std::string str2("[3.1456, 1., 2, \"ab\", { \"aaa\" : { \"123\" :   \t1.2e-5}\t}, true, false, null]");
  Value value2 = Parser::Deserialize(str2);
  ASSERT_TRUE(value2.Type() == Value::ValueType::kArray);
  ASSERT_EQ(value2.Size(), 8);
  ASSERT_DOUBLE_EQ(value2[0].AsDouble(), 3.1456);
  ASSERT_DOUBLE_EQ(value2[1].AsDouble(), 1.);
  ASSERT_EQ(value2[2].AsInteger(), 2);
  ASSERT_EQ(value2[3].AsString(), "ab");
  ASSERT_TRUE(value2[4].Type() == Value::ValueType::kObject);
  ASSERT_EQ(value2[4].Size(), 1);
  ASSERT_TRUE(value2[4]["aaa"].Type() == Value::ValueType::kObject);
  ASSERT_DOUBLE_EQ(value2[4]["aaa"]["123"].AsDouble(), 0.000012);
  ASSERT_TRUE(value2[5].AsBool());
  ASSERT_FALSE(value2[6].AsBool());
  ASSERT_TRUE(value2[7].Type() == Value::ValueType::kNull);

  std::string str3("[12,false, false  , null , [12e4,32, [], \"12\"]]");
  Value value3 = Parser::Deserialize(str3);
  ASSERT_TRUE(value3.Type() == Value::ValueType::kArray);
  ASSERT_EQ(value3.Size(), 5);
  ASSERT_EQ(value3[0].AsInteger(), 12);
  ASSERT_FALSE(value3[1].AsBool());
  ASSERT_FALSE(value3[2].AsBool());
  ASSERT_EQ(value3[3].Type(), Value::ValueType::kNull);
  ASSERT_EQ(value3[4].Type(), Value::ValueType::kArray);
  ASSERT_EQ(value3[4].Size(), 4);
  ASSERT_DOUBLE_EQ(value3[4][0].AsDouble(), 120000);
  ASSERT_EQ(value3[4][1].AsInteger(), 32);
  ASSERT_EQ(value3[4][2].Type(), Value::ValueType::kArray);
  ASSERT_EQ(value3[4][2].Size(), 0);
  ASSERT_EQ(value3[4][3].AsString(), "12");
}

TEST(CsonppTest, DeserializeUnicodeString) {
  std::string str1("[3.1456, 1., 2, \"a\\u0062\", { \"a\\u5066a\" : { \"12\\uD800\\uDC00\" :   \t1.2e-2}\t}, true, false, null]");
  Value value1 = Parser::Deserialize(str1);
  ASSERT_TRUE(value1.Type() == Value::ValueType::kArray);
  ASSERT_EQ(value1.Size(), 8);
  ASSERT_DOUBLE_EQ(value1[0].AsDouble(), 3.1456);
  ASSERT_DOUBLE_EQ(value1[1].AsDouble(), 1.);
  ASSERT_EQ(value1[2].AsInteger(), 2);
  ASSERT_EQ(value1[3].AsString(), "ab");
  ASSERT_EQ(value1[4].Type(), Value::ValueType::kObject);
  ASSERT_EQ(value1[4].Size(), 1);
  ASSERT_EQ(value1[4]["a\xE5\x81\xA6\x61"].Type(), Value::ValueType::kObject);
  ASSERT_EQ(value1[4]["a\xE5\x81\xA6\x61"]["12\xF0\x90\x80\x80"].AsDouble(), 0.012);
  ASSERT_TRUE(value1[5].AsBool());
  ASSERT_FALSE(value1[6].AsBool());
  ASSERT_EQ(value1[7].Type(), Value::ValueType::kNull);
}

TEST(CsonppTest, Serialize) {
  std::string str1 = "\"I \\uD834\\uDD1E playing with json\"";
  Value value1 = Parser::Deserialize(str1);
  ASSERT_EQ(value1.Type(), Value::ValueType::kString);
  ASSERT_EQ(value1.AsString(), "I \xF0\x9D\x84\x9E playing with json");
  ASSERT_EQ(Parser::Serialize(value1), str1);

  std::string str2 = "-12e-3";
  Value value2 = Parser::Deserialize(str2);
  ASSERT_EQ(value2.Type(), Value::ValueType::kDouble);
  ASSERT_DOUBLE_EQ(value2.AsDouble(), -0.012);
  ASSERT_EQ(Parser::Serialize(value2), "-0.0120");

  std::string str3 = "1234567";
  Value value3 = Parser::Deserialize(str3);
  ASSERT_EQ(value3.Type(), Value::ValueType::kInteger);
  ASSERT_DOUBLE_EQ(value3.AsInteger(), 1234567);
  ASSERT_EQ(Parser::Serialize(value3), "1234567");

  std::string str4 = "-1234567.50";
  Value value4 = Parser::Deserialize(str4);
  ASSERT_EQ(value4.Type(), Value::ValueType::kDouble);
  ASSERT_DOUBLE_EQ(value4.AsDouble(), -1234567.50);
  ASSERT_EQ(Parser::Serialize(value4), "-1234567.50");

  std::string str5 = "{\"a\":   \t-1234567.50}";
  Value value5 = Parser::Deserialize(str5);
  ASSERT_EQ(value5.Type(), Value::ValueType::kObject);
  ASSERT_EQ(Parser::Serialize(value5), "{\"a\":-1234567.50}");

  std::string str6("[3.1456, 1., 2, \"a\\u0062\", { \"a\\u5066a\" : { \"12\\uD800\\uDC00\" :   \t1.2e-2}\t}, true, false, null]");
  Value value6 = Parser::Deserialize(str6);
  ASSERT_EQ(Parser::Serialize(value6), "[3.14560,1.0,2,\"ab\",{\"a\\u5066a\":{\"12\\uD800\\uDC00\":0.0120}},true,false,null]");

  std::string str7("[12,false, false  , null , [12e4,32, [], \"12\"]]");
  Value value7 = Parser::Deserialize(str7);
  std::string aa = Parser::Serialize(value7);
  ASSERT_EQ(Parser::Serialize(value7), "[12,false,false,null,[120000.0,32,[],\"12\"]]");
}

