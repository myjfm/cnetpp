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
//   * Neither the name of Shuo Chen nor the names of other contributors may be
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
#ifndef CNETPP_BASE_CSONPP_H_
#define CNETPP_BASE_CSONPP_H_

#include <assert.h>

#include <stdint.h>
#include <stddef.h>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

namespace cnetpp {
namespace base {

class Value;

class Object {
 public:
  typedef std::map<std::string, Value> MapType;
  typedef MapType::const_iterator ConstIterator;
  typedef MapType::iterator Iterator;
  
  friend bool operator==(const Object& left, const Object& right);
  friend bool operator!=(const Object& left, const Object& right);
  friend bool operator>(const Object& left, const Object& right);
  friend bool operator<(const Object& left, const Object& right);
  friend bool operator<=(const Object& left, const Object& right);
  friend bool operator>=(const Object& left, const Object& right);

  Object() = default;
  Object(const Object& other) ;
  Object(Object&& other);

  Object& operator=(const Object& other);
  Object& operator=(Object&& other);
  
  // find element matching key, or insert with default Value
  Value& operator[](const std::string& key);
  // find element matching key, or insert with default Value
  Value& operator[](std::string&& key);

  ConstIterator CBegin() const;
  ConstIterator CEnd() const;

  ConstIterator Begin() const;
  ConstIterator End() const;

  Iterator Begin();
  Iterator End();
  
  Iterator Find(const std::string& key);
  ConstIterator Find(const std::string& key) const;
  
  void Clear();

  size_t Size() const;
  
 private:
  MapType value_;
};

class Array {
 public:
  typedef std::vector<Value> VectorType;
  typedef VectorType::const_iterator ConstIterator;
  typedef VectorType::iterator Iterator;
  
  friend bool operator==(const Array& left, const Array& right);
  friend bool operator!=(const Array& left, const Array& right);
  friend bool operator>(const Array& left, const Array& right);
  friend bool operator<(const Array& left, const Array& right);
  friend bool operator>=(const Array& left, const Array& right);
  friend bool operator<=(const Array& left, const Array& right);
  
  Array() = default;

  Array(const Array& array) ;
  Array(Array&& array);

  Array& operator=(const Array& array);
  Array& operator=(Array&& array);
  
  // the element must exist
  Value& operator[](size_t index);
  // the element must exist
  const Value& operator[](size_t index) const;

  void Append(const Value& value);
  void Append(Value&& value);

  ConstIterator CBegin() const;
  ConstIterator CEnd() const;

  ConstIterator Begin() const;
  ConstIterator End() const;

  Iterator Begin();
  Iterator End();

  Iterator Find(const Value& value);
  ConstIterator Find(const Value& value) const;

  void Clear();

  size_t Size() const;
  
 private:
  VectorType value_;
};

class Value {
 public:
  enum class ValueType {
    kDummy   = 0,  /* invalid type */
    kNull    = 1,  /* e.g. null */
    kBool    = 2,  /* e.g. true or false */
    kInteger = 3,  /* e.g. -100 */
    kDouble  = 4,  /* e.g. -1.2e-10 */
    kString  = 5,  /* e.g. "abcd" */
    kObject  = 6,  /* e.g. {"a":true} */
    kArray   = 7,  /* e.g. ["a", "b"] */
  };

  friend bool operator==(const Value& left, const Value& right);
  friend bool operator!=(const Value& left, const Value& right);
  friend bool operator<(const Value& left, const Value& right);
  friend bool operator>(const Value& left, const Value& right);
  friend bool operator<=(const Value& left, const Value& right);
  friend bool operator>=(const Value& left, const Value& right);

  explicit Value(ValueType iType = ValueType::kDummy);
  explicit Value(std::nullptr_t null);
	explicit Value(bool value);
	explicit Value(int8_t value);
	explicit Value(uint8_t value);
	explicit Value(int16_t value);
	explicit Value(uint16_t value);
	explicit Value(int32_t value);
	explicit Value(uint32_t value);
	explicit Value(int64_t value);
	explicit Value(uint64_t value);
	explicit Value(float value);
	explicit Value(double value);
  explicit Value(const char* value);
	explicit Value(const std::string& value);
	explicit Value(std::string&& value);
	explicit Value(const Object& value);
	explicit Value(Object&& value);
	explicit Value(const Array& value);
	explicit Value(Array&& value);

  ~Value() {}

	Value(const Value& value);
  Value(Value&& value);

	Value& operator=(const Value& value);
	Value& operator=(Value&& value);
	Value& operator=(bool value);
	Value& operator=(int8_t value);
	Value& operator=(uint8_t value);
	Value& operator=(int16_t value);
	Value& operator=(uint16_t value);
	Value& operator=(int32_t value);
	Value& operator=(uint32_t value);
	Value& operator=(int64_t value);
	Value& operator=(uint64_t value);
	Value& operator=(float value);
	Value& operator=(double value);
  Value& operator=(const char* value);
	Value& operator=(const std::string& value);
	Value& operator=(std::string&& value);
	Value& operator=(const Object& value);
	Value& operator=(Object&& value);
	Value& operator=(const Array& value);
	Value& operator=(Array&& value);

  void Append(const Value& value);
  void Append(Value&& value);
  void Append(const std::string& key, bool value);
  void Append(std::string&& key, bool value);
  void Append(const std::string& key, int8_t value);
  void Append(std::string&& key, int8_t value);
  void Append(const std::string& key, uint8_t value);
  void Append(std::string&& key, uint8_t value);
  void Append(const std::string& key, int16_t value);
  void Append(std::string&& key, uint16_t value);
  void Append(const std::string& key, int32_t value);
  void Append(std::string&& key, int32_t value);
  void Append(const std::string& key, uint32_t value);
  void Append(std::string&& key, uint32_t value);
  void Append(const std::string& key, int64_t value);
  void Append(std::string&& key, int64_t value);
  void Append(const std::string& key, uint64_t value);
  void Append(std::string&& key, uint64_t value);
  void Append(const std::string& key, float value);
  void Append(std::string&& key, float value);
  void Append(const std::string& key, double value);
  void Append(std::string&& key, double value);
  void Append(const std::string& key, const char* value);
  void Append(std::string&& key, const char* value);
  void Append(const std::string& key, const std::string& value);
  void Append(std::string&& key, const std::string& value);
  void Append(const std::string& key, std::string&& value);
  void Append(std::string&& key, std::string&& value);
  void Append(const std::string& key, const Object& value);
  void Append(std::string&& key, const Object& value);
  void Append(const std::string& key, Object&& value);
  void Append(std::string&& key, Object&& value);
  void Append(const std::string& key, const Array& value);
  void Append(std::string&& key, const Array& value);
  void Append(const std::string& key, Array&& value);
  void Append(std::string&& key, Array&& value);

	ValueType Type() const;

  size_t Size() const;

  void Clear();

  bool IsNumeric() const;
  bool IsIntegral() const;
  bool IsDouble() const;
  bool IsBool() const;
  bool IsString() const;
  bool IsObject() const;
  bool IsArray() const;

  int64_t AsInteger() const;
  double AsDouble() const;
  bool AsBool() const;
  std::string AsString() const;
  Object AsObject() const;
  Array AsArray() const;

  const int64_t& GetInteger() const;
  const double& GetDouble() const;
  const bool& GetBool() const;
  const std::string& GetString() const;
  const Object& GetObject() const;
  const Array& GetArray() const;

  Value& operator[](size_t index);
  const Value& operator[](size_t index) const;
  Value& operator[](const std::string& key);
  Value& operator[](std::string&& key);
  
 private:
  ValueType type_;

  union {
    bool bool_;
    int64_t integer_;
    double double_;
  };
  std::string string_;
  Array array_;
  Object object_;
};

class Parser {
 public:
  static bool Deserialize(const std::string& str, Value* value);

  static Value Deserialize(const std::string& str) {
    Value value;
    Deserialize(str, &value);
    return std::move(value);
  }

  static void Serialize(const Value& value, std::string* str);

  static std::string Serialize(const Value& value) {
    std::string str;
    Serialize(value, &str);
    return std::move(str);
  }
};

}  // namespace base
}  // cnetpp

#endif

