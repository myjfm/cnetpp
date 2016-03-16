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
#include "csonpp_impl.h"

#include <assert.h>

namespace cnetpp {
namespace base {

Object::Object(const Object& other) 
  : value_(other.value_) {
}

Object::Object(Object&& other) 
: value_(std::move(other.value_)) {
}

Object& Object::operator=(const Object& other) {
  if (this != &other)
    value_ = other.value_;
  return *this;
}

Object& Object::operator=(Object&& other) {
  if (this != &other)
    value_ = std::move(other.value_);
  return *this;
}

Value& Object::operator[](const std::string& key) {
  return value_[key];
}

Value& Object::operator[](std::string&& key) {
  return value_[std::move(key)];
}

Object::ConstIterator Object::CBegin() const {
  return value_.cbegin();
}

Object::ConstIterator Object::CEnd() const {
  return value_.cend();
}

Object::ConstIterator Object::Begin() const {
  return value_.begin();
}

Object::ConstIterator Object::End() const {
  return value_.end();
}

Object::Iterator Object::Begin() {
  return value_.begin();
}

Object::Iterator Object::End() {
  return value_.end();
}

Object::Iterator Object::Find(const std::string& key) {
  return value_.find(key);
}

Object::ConstIterator Object::Find(const std::string& key) const {
  return value_.find(key);
}

void Object::Clear() {
  value_.clear();
}
size_t Object::Size() const {
  return value_.size();
}

Array::Array(const Array& array) : value_(array.value_) {
}

Array::Array(Array&& array) : value_(std::move(array.value_)) {
}

Array& Array::operator=(const Array& array) {
  if (this != &array)
    value_ = array.value_;
  return *this;
}

Array& Array::operator=(Array&& array) {
  if (this != &array)
    value_ = std::move(array.value_);
  return *this;
}

// the element must exist
Value& Array::operator[](size_t index) {
  assert(index < value_.size());
  return value_[index];
}

// the element must exist
const Value& Array::operator[](size_t index) const {
  assert(index < value_.size());
  return value_[index];
}

void Array::Append(const Value& value) {
  value_.push_back(value);
}

void Array::Append(Value&& value) {
  value_.push_back(std::move(value));
}

Array::ConstIterator Array::CBegin() const {
  return value_.cbegin();
}

Array::ConstIterator Array::CEnd() const {
  return value_.cend();
}

Array::ConstIterator Array::Begin() const {
  return value_.begin();
}

Array::ConstIterator Array::End() const {
  return value_.end();
}

Array::Iterator Array::Begin() {
  return value_.begin();
}

Array::Iterator Array::End() {
  return value_.end();
}

Array::Iterator Array::Find(const Value& value) {
  return std::find(Begin(), End(), value);
}

Array::ConstIterator Array::Find(const Value& value) const {
  return std::find(Begin(), End(), value);
}

void Array::Clear() {
  value_.clear();
}

size_t Array::Size() const {
  return value_.size();
}

Value::Value(ValueType type) : type_(type) {
}

Value::Value(std::nullptr_t null) : type_(ValueType::kNull) {
}

Value::Value(bool value) : type_(ValueType::kBool), bool_(value) {
}

Value::Value(int8_t value)
    : type_(ValueType::kInteger),
      integer_(static_cast<int64_t>(value)) {
}

Value::Value(uint8_t value)
    : type_(ValueType::kInteger),
      integer_(static_cast<int64_t>(value)) {
}

Value::Value(int16_t value)
    : type_(ValueType::kInteger),
      integer_(static_cast<int64_t>(value)) {
}

Value::Value(uint16_t value)
    : type_(ValueType::kInteger),
      integer_(static_cast<int64_t>(value)) {
}

Value::Value(int32_t value)
    : type_(ValueType::kInteger),
      integer_(static_cast<int64_t>(value)) {
}

Value::Value(uint32_t value)
    : type_(ValueType::kInteger),
      integer_(static_cast<int64_t>(value)) {
}

Value::Value(int64_t value)
    : type_(ValueType::kInteger),
      integer_(value) {
}

Value::Value(uint64_t value)
    : type_(ValueType::kInteger),
      integer_(value) {
}

Value::Value(float value)
    : type_(ValueType::kDouble),
      double_(static_cast<double>(value)) {
}

Value::Value(double value)
    : type_(ValueType::kDouble),
      double_(value) {
}

Value::Value(const char* value)
    : type_(ValueType::kString),
      string_(value) {
}

Value::Value(const std::string& value)
    : type_(ValueType::kString),
      string_(value) {
}

Value::Value(std::string&& value)
    : type_(ValueType::kString),
      string_(std::move(value)) {
}

Value::Value(const Object& value)
    : type_(ValueType::kObject),
      object_(value) {
}

Value::Value(Object&& value)
    : type_(ValueType::kObject),
      object_(std::move(value)) {
}

Value::Value(const Array& value)
    : type_(ValueType::kArray),
      array_(value) {
}

Value::Value(Array&& value)
    : type_(ValueType::kArray),
      array_(std::move(value)) {
}

Value::Value(const Value& value) : type_(value.type_) {
  switch(type_) {
  case ValueType::kBool:
    bool_ = value.bool_;
    break;
  case ValueType::kInteger:
    integer_ = value.integer_;
    break;
  case ValueType::kDouble:
    double_ = value.double_;
    break;
  case ValueType::kString:
    string_ = value.string_;
    break;
  case ValueType::kObject:
    object_ = value.object_;
    break;
  case ValueType::kArray:
    array_ = value.array_;
    break;
  default:
    break;
  }
}

Value::Value(Value&& value) 
: type_(value.type_) {
  switch(type_) {
  case ValueType::kBool:
    bool_ = value.bool_;
    break;
  case ValueType::kInteger:
    integer_ = value.integer_;
    break;
  case ValueType::kDouble:
    double_ = value.double_;
    break;
  case ValueType::kString:
    string_ = std::move(value.string_);
    break;
  case ValueType::kObject:
    object_ = std::move(value.object_);
    break;
  case ValueType::kArray:
    array_ = std::move(value.array_);
    break;
  default:
    break;
  }
}

Value& Value::operator=(const Value& value) {
  if (&value != this) {
    type_ = value.type_;
    switch(type_) {
    case ValueType::kBool:
      bool_ = value.bool_;
      break;
    case ValueType::kInteger:
      integer_ = value.integer_;
      break;
    case ValueType::kDouble:
      double_ = value.double_;
      break;
    case ValueType::kString:
      string_ = value.string_;
      break;
    case ValueType::kObject:
      object_ = value.object_;
      break;
    case ValueType::kArray:
      array_ = value.array_;
      break;
    default:
      break;
    }
  }

  return *this;
}

Value& Value::operator=(Value&& value) {
  if (&value != this) {
    type_ = value.type_;
    switch(type_) {
    case ValueType::kBool:
      bool_ = value.bool_;
      break;
    case ValueType::kInteger:
      integer_ = value.integer_;
      break;
    case ValueType::kDouble:
      double_ = value.double_;
      break;
    case ValueType::kString:
      string_ = std::move(value.string_);
      break;
    case ValueType::kObject:
      object_ = std::move(value.object_);
      break;
    case ValueType::kArray:
      array_ = std::move(value.array_);
      break;
    default:
      break;
    }
  }

  return *this;
}

Value& Value::operator=(bool value) {
  type_ = ValueType::kBool;
  bool_ = value;
  return *this;
}

Value& Value::operator=(int8_t value) {
  type_ = ValueType::kInteger;
  integer_ = static_cast<int64_t>(value);
  return *this;
}

Value& Value::operator=(uint8_t value) {
  type_ = ValueType::kInteger;
  integer_ = static_cast<int64_t>(value);
  return *this;
}

Value& Value::operator=(int16_t value) {
  type_ = ValueType::kInteger;
  integer_ = static_cast<int64_t>(value);
  return *this;
}

Value& Value::operator=(uint16_t value) {
  type_ = ValueType::kInteger;
  integer_ = static_cast<int64_t>(value);
  return *this;
}

Value& Value::operator=(int32_t value) {
  type_ = ValueType::kInteger;
  integer_ = static_cast<int64_t>(value);
  return *this;
}

Value& Value::operator=(uint32_t value) {
  type_ = ValueType::kInteger;
  integer_ = static_cast<int64_t>(value);
  return *this;
}

Value& Value::operator=(int64_t value) {
  type_ = ValueType::kInteger;
  integer_ = value;
  return *this;
}

Value& Value::operator=(uint64_t value) {
  type_ = ValueType::kInteger;
  integer_ = value;
  return *this;
}

Value& Value::operator=(float value) {
  type_ = ValueType::kDouble;
  double_ = static_cast<double>(value);
  return *this;
}

Value& Value::operator=(double value) {
  type_ = ValueType::kDouble;
  double_ = value;
  return *this;
}

Value& Value::operator=(const char* value) {
  type_ = ValueType::kString;
  string_ = value;
  return *this;
}

Value& Value::operator=(const std::string& value) {
  type_ = ValueType::kString;
  string_ = value;
  return *this;
}

Value& Value::operator=(std::string&& value) {
  type_ = ValueType::kString;
  string_ = std::move(value);
  return *this;
}

Value& Value::operator=(const Object& value) {
  type_ = ValueType::kObject;
  object_ = value;
  return *this;
}

Value& Value::operator=(Object&& value) {
  type_ = ValueType::kObject;
  object_ = std::move(value);
  return *this;
}

Value& Value::operator=(const Array& value) {
  type_ = ValueType::kArray;
  array_ = value;
  return *this;
}

Value& Value::operator=(Array&& value) {
  type_ = ValueType::kArray;
  array_ = std::move(value);
  return *this;
}

void Value::Append(const Value& value) {
  assert(type_ == ValueType::kArray);
  array_.Append(value);
}

void Value::Append(Value&& value) {
  assert(type_ == ValueType::kArray);
  array_.Append(std::move(value));
}

void Value::Append(const std::string& key, bool value) {
  assert(type_ == ValueType::kObject);
  object_[key] = value;
}

void Value::Append(std::string&& key, bool value) {
  assert(type_ == ValueType::kObject);
  object_[std::move(key)] = value;
}

void Value::Append(const std::string& key, int8_t value) {
  assert(type_ == ValueType::kObject);
  object_[key] = value;
}

void Value::Append(std::string&& key, int8_t value) {
  assert(type_ == ValueType::kObject);
  object_[std::move(key)] = value;
}

void Value::Append(const std::string& key, uint8_t value) {
  assert(type_ == ValueType::kObject);
  object_[key] = value;
}

void Value::Append(std::string&& key, uint8_t value) {
  assert(type_ == ValueType::kObject);
  object_[std::move(key)] = value;
}

void Value::Append(const std::string& key, int16_t value) {
  assert(type_ == ValueType::kObject);
  object_[key] = value;
}

void Value::Append(std::string&& key, uint16_t value) {
  assert(type_ == ValueType::kObject);
  object_[std::move(key)] = value;
}

void Value::Append(const std::string& key, int32_t value) {
  assert(type_ == ValueType::kObject);
  object_[key] = value;
}

void Value::Append(std::string&& key, int32_t value) {
  assert(type_ == ValueType::kObject);
  object_[std::move(key)] = value;
}

void Value::Append(const std::string& key, uint32_t value) {
  assert(type_ == ValueType::kObject);
  object_[key] = value;
}

void Value::Append(std::string&& key, uint32_t value) {
  assert(type_ == ValueType::kObject);
  object_[std::move(key)] = value;
}

void Value::Append(const std::string& key, int64_t value) {
  assert(type_ == ValueType::kObject);
  object_[key] = value;
}

void Value::Append(std::string&& key, int64_t value) {
  assert(type_ == ValueType::kObject);
  object_[std::move(key)] = value;
}

void Value::Append(const std::string& key, uint64_t value) {
  assert(type_ == ValueType::kObject);
  object_[key] = value;
}

void Value::Append(std::string&& key, uint64_t value) {
  assert(type_ == ValueType::kObject);
  object_[std::move(key)] = value;
}

void Value::Append(const std::string& key, float value) {
  assert(type_ == ValueType::kObject);
  object_[key] = value;
}

void Value::Append(std::string&& key, float value) {
  assert(type_ == ValueType::kObject);
  object_[std::move(key)] = value;
}

void Value::Append(const std::string& key, double value) {
  assert(type_ == ValueType::kObject);
  object_[key] = value;
}

void Value::Append(std::string&& key, double value) {
  assert(type_ == ValueType::kObject);
  object_[std::move(key)] = value;
}

void Value::Append(const std::string& key, const char* value) {
  assert(type_ == ValueType::kObject);
  object_[key] = value;
}

void Value::Append(std::string&& key, const char* value) {
  assert(type_ == ValueType::kObject);
  object_[std::move(key)] = value;
}

void Value::Append(const std::string& key, const std::string& value) {
  assert(type_ == ValueType::kObject);
  object_[key] = value;
}

void Value::Append(std::string&& key, const std::string& value) {
  assert(type_ == ValueType::kObject);
  object_[std::move(key)] = value;
}

void Value::Append(const std::string& key, std::string&& value) {
  assert(type_ == ValueType::kObject);
  object_[key] = std::move(value);
}

void Value::Append(std::string&& key, std::string&& value) {
  assert(type_ == ValueType::kObject);
  object_[std::move(key)] = std::move(value);
}

void Value::Append(const std::string& key, const Object& value) {
  assert(type_ == ValueType::kObject);
  object_[key] = value;
}

void Value::Append(std::string&& key, const Object& value) {
  assert(type_ == ValueType::kObject);
  object_[std::move(key)] = value;
}

void Value::Append(const std::string& key, Object&& value) {
  assert(type_ == ValueType::kObject);
  object_[key] = std::move(value);
}

void Value::Append(std::string&& key, Object&& value) {
  assert(type_ == ValueType::kObject);
  object_[std::move(key)] = std::move(value);
}

void Value::Append(const std::string& key, const Array& value) {
  assert(type_ == ValueType::kObject);
  object_[key] = value;
}

void Value::Append(std::string&& key, const Array& value) {
  assert(type_ == ValueType::kObject);
  object_[std::move(key)] = value;
}

void Value::Append(const std::string& key, Array&& value) {
  assert(type_ == ValueType::kObject);
  object_[key] = std::move(value);
}

void Value::Append(std::string&& key, Array&& value) {
  assert(type_ == ValueType::kObject);
  object_[std::move(key)] = std::move(value);
}

Value::ValueType Value::Type() const {
  return type_;
}

size_t Value::Size() const {
  assert(type_ == ValueType::kObject || type_ == ValueType::kArray);
  return (type_ == ValueType::kObject) ? object_.Size() : array_.Size();
}

void Value::Clear() {
  switch (type_) {
  case ValueType::kArray:
    array_.Clear();
    return;
  case ValueType::kObject:
    object_.Clear();
    return;
  default:
    assert(false);
    return;
  }
}

bool Value::IsNumeric() const {
  return (type_ == ValueType::kDouble || type_ == ValueType::kInteger);
}

bool Value::IsIntegral() const {
  return type_ == ValueType::kInteger;
}

bool Value::IsDouble() const {
  return type_ == ValueType::kDouble;
}

bool Value::IsBool() const {
  return type_ == ValueType::kBool;
}

bool Value::IsString() const {
  return type_ == ValueType::kString;
}

bool Value::IsObject() const {
  return type_ == ValueType::kObject;
}

bool Value::IsArray() const {
  return type_ == ValueType::kArray;
}

int64_t Value::AsInteger() const {
  if (type_ == ValueType::kInteger) {
    return integer_;
  } else if (type_ == ValueType::kDouble) {
    return static_cast<int64_t>(double_);
  } else {
    assert(false);
  }
}

double Value::AsDouble() const {
  if (type_ == ValueType::kInteger) {
    return static_cast<double>(integer_);
  } else if (type_ == ValueType::kDouble) {
    return double_;
  } else {
    assert(false);
  }
}

bool Value::AsBool() const {
  assert(type_ == ValueType::kBool);
  return bool_;
}

std::string Value::AsString() const {
  assert(type_ == ValueType::kString);
  return string_;
}

Object Value::AsObject() const {
  assert(type_ == ValueType::kObject);
  return object_;
}

Array Value::AsArray() const {
  assert(type_ == ValueType::kArray);
  return array_;
}

const int64_t& Value::GetInteger() const {
  assert(type_ == ValueType::kInteger);
  return integer_;
}

const double& Value::GetDouble() const {
  assert(type_ == ValueType::kDouble);
  return double_;
}

const bool& Value::GetBool() const {
  assert(type_ == ValueType::kBool);
  return bool_;
}

const std::string& Value::GetString() const {
  assert(type_ == ValueType::kString);
  return string_;
}

const Object& Value::GetObject() const {
  assert(type_ == ValueType::kObject);
  return object_;
}

const Array& Value::GetArray() const {
  assert(type_ == ValueType::kArray);
  return array_;
}

Value& Value::operator[](size_t index) {
  assert(type_ == ValueType::kArray);
  assert(index < array_.Size());
  return array_[index];
}

const Value& Value::operator[](size_t index) const {
  assert(type_ == ValueType::kArray);
  assert(index < array_.Size());
  return array_[index];
}

Value& Value::operator[](const std::string& key) {
  assert(type_ == ValueType::kObject);
  return object_[key];
}

Value& Value::operator[](std::string&& key) {
  assert(type_ == ValueType::kObject);
  return object_[std::move(key)];
}
  
bool operator==(const Object& left, const Object& right) {
  return left.value_ == right.value_;
}

bool operator!=(const Object& left, const Object& right) {
  return !(left.value_ == right.value_);
}

bool operator>(const Object& left, const Object& right) {
  return left.value_ > right.value_;
}

bool operator<(const Object& left, const Object& right) {
  return left.value_ < right.value_;
}

bool operator>=(const Object& left, const Object& right) {
  return !(left.value_ < right.value_);
}

bool operator<=(const Object& left, const Object& right) {
  return !(left.value_ > right.value_);
}

bool operator==(const Array& left, const Array& right) {
  return left.value_ == right.value_;
}

bool operator!=(const Array& left, const Array& right) {
  return !(left == right);
}

bool operator>(const Array& left, const Array& right) {
  return left > right;
}

bool operator<(const Array& left, const Array& right) {
  return left < right;
}

bool operator>=(const Array& left, const Array& right) {
  return !(left < right);
}

bool operator<=(const Array& left, const Array& right) {
  return !(left > right);
}

bool operator==(const Value& left, const Value& right) {
  if (left.type_ != right.type_ && (!left.IsNumeric() || !right.IsNumeric()))
    return false;

  switch (left.type_) {
  case Value::ValueType::kBool:
    return left.bool_ == right.bool_;
  case Value::ValueType::kInteger:
    if (right.type_ == Value::ValueType::kDouble) {
      return (static_cast<double>(left.integer_) - right.double_) < 1e-8;
    } else if (right.type_ == Value::ValueType::kInteger) {
      return left.integer_ == right.integer_;
    } else {
      return false;
    }
  case Value::ValueType::kDouble:
    if (right.type_ == Value::ValueType::kDouble) {
      return (left.double_ - right.double_) < 1e-8;
    } else if (right.type_ == Value::ValueType::kInteger) {
      return (left.double_ - static_cast<double>(right.integer_)) < 1e-8;
    } else {
      return false;
    }
  case Value::ValueType::kString:
    return left.string_ == right.string_;
  case Value::ValueType::kObject:
    return left.object_ == right.object_;
  case Value::ValueType::kArray:
    return left.array_ == right.array_;
  default: // Null type or Dummy type
    return true;
  }
}

bool operator!=(const Value& left, const Value& right) {
  return !(left == right);
}

bool operator<(const Value& left, const Value& right) {
  if (left.IsIntegral() && right.IsIntegral()) {
    return left.integer_ < right.integer_;
  } else if (left.IsIntegral() && right.IsDouble()) {
    return static_cast<double>(left.integer_) < right.double_;
  } else if (left.IsDouble() && right.IsIntegral()) {
    return left.double_ < static_cast<double>(right.integer_);
  } else if (left.IsDouble() && right.IsDouble()) {
    return left.double_ < right.double_;
  } else if (left.IsString() && right.IsString()) {
    return left.string_ < right.string_;
  } else if (left.IsObject() && right.IsObject()) {
    return left.object_ < right.object_;
  } else if (left.IsArray() && right.IsArray()) {
    return left.array_ < right.array_;
  } else {
    assert(false);
  }
  return false;
}

bool operator>(const Value& left, const Value& right) {
  return right < left;
}

bool operator<=(const Value& left, const Value& right) {
  return !(left > right);
}

bool operator>=(const Value& left, const Value& right) {
  return !(left < right);
}
/**
 * convert a unicode code point to a utf-8 string
 * unicode code point ranges from [U+000000, U+10FFFF],
 * utf-8 has the format:
 * 1. 0xxxxxxx    1 byte, or
 * 2. 110xxxxx 10xxxxxx 2 byte, or
 * 3. 1110xxxx 10xxxxxx 10xxxxxx 3 byte, or
 * 4. 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx 4 byte
 * for more details, please refer to http://en.wikipedia.org/wiki/UTF-8
 * @param unicode_code_point the unicode code point
 * @return "" if the unicode_code_point is not a valid codepoint
 */
static std::string CodePoint2Utf8(int32_t unicode_code_point) {
  std::string result;
  if (unicode_code_point < 0x0 || unicode_code_point > 0x10FFFF) // invalid
    return result;
  
  if (unicode_code_point <= 0x7F) {
    result.reserve(2);
    result.append(1, static_cast<char>(unicode_code_point));
  } else if (unicode_code_point <= 0x7FF) {
    result.reserve(3);
    result.append(1, static_cast<char>(0xC0 | (unicode_code_point >> 6)));
    result.append(1, static_cast<char>(0x80 | (unicode_code_point & 0x3F)));
  } else if (unicode_code_point <= 0xFFFF) {
    result.reserve(4);
    result.append(1, static_cast<char>(0xE0 | (unicode_code_point >> 12)));
    result.append(1, static_cast<char>(0x80 |
                                       ((unicode_code_point >> 6) & 0x3F)));
    result.append(1, static_cast<char>(0x80 |
                                       (unicode_code_point & 0x3F)));
  } else if (unicode_code_point <= 0x10FFFF) {
    result.reserve(5);
    result.append(1, static_cast<char>(0xF0 | (unicode_code_point >> 18)));
    result.append(1, static_cast<char>(0x80 |
                                       ((unicode_code_point >> 12) & 0x3F)));
    result.append(1, static_cast<char>(0x80 |
                                       ((unicode_code_point >> 6) & 0x3F)));
    result.append(1, static_cast<char>(0x80 | (unicode_code_point & 0x3F)));
  }
  return result;
}

/**
 * convert a utf-8 string to unicode code point
 * If the first charactor in utf8_string <= 0x7f then it is an ascii charactor,
 * else the utf-8 string occupies more than 1 bytes.
 * @param utf8_string  the string
 * @return the converted unicode code point, -1 if error occured
 * NOTE: after the convertion, 
 * utf8_string will point to the next position right after the utf-8 string
 */
static int32_t Utf82CodePoint(const char*& utf8_string) {
  assert(utf8_string);
  if (static_cast<uint32_t>(*utf8_string) <= 0x7F) // ascii charactor
    return *utf8_string++;

  auto l1 = static_cast<uint32_t>(*utf8_string) & 0xFF;
  auto l2 = static_cast<uint32_t>(*(utf8_string + 1)) & 0xFF;
  auto l2_error = [&utf8_string] (uint32_t i2) {
    if (!i2) utf8_string++;
    else utf8_string += 2;
    return -1;
  };
  auto l3_error = [&utf8_string] (uint32_t i3) {
    if (!i3) utf8_string += 2;
    else utf8_string += 3;
    return -1;
  };
  auto l4_error = [&utf8_string] (uint32_t i4) {
    if (!i4) utf8_string += 3;
    else utf8_string += 4;
    return -1;
  };

  if ((l1 >> 5) == 0x06) { // first byte is 110xxxxx
    if ((l2 >> 6) == 0x02) { // second byte is 10xxxxxx
      utf8_string += 2;
      return ((l1 & 0x1F) << 6) | (l2 & 0x3F);
    }
    return l2_error(l2);
  }
  
  if ((l1 >> 4) == 0x0E) { // first is 1110xxxx
    if ((l2 >> 6) == 0x02) { // second byte is 10xxxxxx
      auto l3 = static_cast<uint32_t>(*(utf8_string + 2)) & 0xFF;
      if ((l3 >> 6) == 0x02) { // third byte is also 10xxxxxx
        utf8_string += 3;
        return ((l1 & 0x0F) << 12) | ((l2 & 0x3F) << 6) | (l3 & 0x3F);
      }
      return l3_error(l3);
    }
    return l2_error(l2);
  }
  
  if ((l1 >> 3) == 0x01E) { // first is 11110xxx
    if ((l2 >> 6) == 0x02) { // second byte is 10xxxxxx
      auto l3 = static_cast<uint32_t>(*(utf8_string + 2)) & 0xFF;
      if ((l3 >> 6) == 0x02) { // third byte is also 10xxxxxx
        auto l4 = static_cast<uint32_t>(*(utf8_string + 3)) & 0xFF;
        if ((l4 >> 6) == 0x02) {
          utf8_string += 4;
          return (((l1 & 0x07) << 18) | 
                  ((l2 & 0x3F) << 12) | 
                  ((l3 & 0x3F) << 6) | 
                  (l4 & 0x3F));
        }
        return l4_error(l4);
      }
      return l3_error(l3);
    }
    return l2_error(l2);
  }
  return -1;
}

bool Parser::Deserialize(const std::string& csonpp_string, Value* value) {
  ParserImpl impl;
  return impl.Deserialize(csonpp_string, value);
}

void Parser::Serialize(const Value& value, std::string* csonpp_string) {
  ParserImpl impl;
  impl.Serialize(value, csonpp_string);
}

Token TokenizerImpl::GetToken() {
  /**
   * The DFA
   * kStart + 'n'(ull)   -> END (null)
   * kStart + 'f'(alse)  -> END (false)
   * kStart + 't'(rue)   -> END (true)
   * kStart + ':'        -> END (:)
   * kStart + ','        -> END (,)
   * kStart + '['        -> END ([)
   * kStart + ']'        -> END (])
   * kStart + '{'        -> END ({)
   * kStart + '}'        -> END (})
   * kStart + '\"'       -> kString
   * kStart + '-'        -> kNumber1
   * kStart + '[1-9]'    -> kNumber2
   * kString + '[^"]'    -> kString
   * kString + '\"'      -> DONE (String)
   * kNumber1 + '[1-9]'  -> kNumber2
   * kNumber1 + '0'      -> kNumber3
   * kNumber2 + '[0-9]'  -> kNumber2
   * kNumber2 + '.'      -> kNumber4
   * kNumber2 + '[eE]'   -> kNumber5
   * kNumber2 + '[,}\]#]'-> DONE (Integer)
   * kNumber3 + '.'      -> kNumber4
   * kNumber3 + '[,}\]#]'-> DONE (Integer)
   * kNumber4 + '[0-9]'  -> kNumber4
   * kNumber4 + '[eE]'   -> kNumber5
   * kNumber4 + '[,}\]#]'-> DONE (Double)
   * kNumber5 + '[+-]'   -> kNumber6
   * kNumber5 + '[0-9]'  -> kNumber7
   * kNumber6 + '[0-9]'  -> kNumber7
   * kNumber7 + '[0-9]'  -> kNumber7
   * kNumber7 + '[,}\]#]'-> DONE (Double)
   */
  enum class DFAState {
    kStart,
    kString,
    kNumber1,
    kNumber2,
    kNumber3,
    kNumber4,
    kNumber5,
    kNumber6,
    kNumber7,
  };

  Token token;
  DFAState state = DFAState::kStart;

  auto error_occured = [&token] {
    token.value = "";
    token.type = Token::Type::kDummy;
    return token;
  };

  while (true) {
    int c = GetNextChar();
    switch (state) {
    case DFAState::kStart:
      while (isspace(c)) {
        c = GetNextChar();
      }
      switch (c) {
      case ',':
        token.value.append(1, static_cast<char>(c));
        token.type = Token::Type::kComma;
        return token;
      case ':':
        token.value.append(1, static_cast<char>(c));
        token.type = Token::Type::kColon;
        return token;
      case '{':
        token.value.append(1, static_cast<char>(c));
        token.type = Token::Type::kLeftBrace;
        return token;
      case '}':
        token.value.append(1, static_cast<char>(c));
        token.type = Token::Type::kRightBrace;
        return token;
      case '[':
        token.value.append(1, static_cast<char>(c));
        token.type = Token::Type::kLeftBracket;
        return token;
      case ']':
        token.value.append(1, static_cast<char>(c));
        token.type = Token::Type::kRightBracket;
        return token;
      case 't':
        if (GetNextChar() == 'r' &&
            GetNextChar() == 'u' &&
            GetNextChar() == 'e') {
          token.value.append("true");
          token.type = Token::Type::kTrue;
          return token;
        } else {
          return error_occured();
        }
      case 'f':
        if (GetNextChar() == 'a' &&
            GetNextChar() == 'l' &&
            GetNextChar() == 's' &&
            GetNextChar() == 'e') {
          token.value.append("false");
          token.type = Token::Type::kFalse;
          return token;
        } else {
          return error_occured();
        }
      case 'n':
        if (GetNextChar() == 'u' &&
            GetNextChar() == 'l' &&
            GetNextChar() == 'l') {
          token.value.append("null");
          token.type = Token::Type::kNull;
          return token;
        } else {
          return error_occured();
        }
      case '\"':
        state = DFAState::kString;
        token.type = Token::Type::kString;
        break;
      case '-':
        state = DFAState::kNumber1;
        token.value.append(1, static_cast<char>(c));
        token.type = Token::Type::kInteger;
        break;
      case '0':
        state = DFAState::kNumber3;
        token.value.append(1, static_cast<char>(c));
        token.type = Token::Type::kInteger;
        break;
      case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
        state = DFAState::kNumber2;
        token.value.append(1, static_cast<char>(c));
        token.type = Token::Type::kInteger;
        break;
      case '#':
        do {                        // for windows  \r\n
          c = GetNextChar();        // for linux    \n
        } while (c != '\n' && c != '\0');
        if (c == '\0') {
          return error_occured();
        }
        break;
      default:
        return error_occured();
      }
      break;
    case DFAState::kString:
      switch (c) {
      case '\"': return token;
      case '\\': {
        int cc = GetNextChar();
        switch (cc) {
        case '\"': token.value.append(1, '\"'); break;
        case '\\': token.value.append(1, '\\'); break;
        case 'r':  token.value.append(1, '\r'); break;
        case 'n':  token.value.append(1, '\n'); break;
        case 't':  token.value.append(1, '\t'); break;
        case 'b':  token.value.append(1, '\b'); break;
        case 'f':  token.value.append(1, '\f'); break;
        case 'u': {
          // convert unicode escapse charactor
          char32_t code_point = DecodeUnicode();
          if (code_point == 0)
            return error_occured();
          token.value += CodePoint2Utf8(code_point);
          break;
        }
        default: return error_occured();
        }
        break;
      }
      case '\0': return error_occured();
      default: token.value.append(1, static_cast<char>(c)); break;
      }
      break;
    case DFAState::kNumber1:
      if (c == '0') {
        state = DFAState::kNumber3;
        token.value.append(1, static_cast<char>(c));
      } else if (c >= '1' && c <= '9') {
        state = DFAState::kNumber2;
        token.value.append(1, static_cast<char>(c));
      } else {
        return error_occured();
      }
      break;
    case DFAState::kNumber2:
      while (isdigit(c)) {
        token.value.append(1, static_cast<char>(c));
        c = GetNextChar();
      }
      if (c == '\0' || c == ',' || c == '}' || c == ']' || c == '#') {
        UngetNextChar();
        return token;
      } else if (c == 'e' || c == 'E') {
        state = DFAState::kNumber5;
        token.value.append(1, static_cast<char>(c));
        token.type = Token::Type::kDouble;
      } else if (c == '.') {
        state = DFAState::kNumber4;
        token.value.append(1, static_cast<char>(c));
        token.type = Token::Type::kDouble;
      } else {
        return error_occured();
      }
      break;
    case DFAState::kNumber3:
      if (c == '\0' || c == ',' || c == '}' || c == ']' || c == '#') {
        UngetNextChar();
        return token;
      } else if (c == 'e' || c == 'E') {
        state = DFAState::kNumber5;
        token.value.append(1, static_cast<char>(c));
        token.type = Token::Type::kDouble;
      } else if (c == '.') {
        state = DFAState::kNumber4;
        token.value.append(1, static_cast<char>(c));
        token.type = Token::Type::kDouble;
      } else {
        return error_occured();
      }
      break;
    case DFAState::kNumber4:
      while (isdigit(c)) {
        token.value.append(1, static_cast<char>(c));
        c = GetNextChar();
      }
      if (c == '\0' || c == ',' || c == '}' || c == ']' || c == '#') {
        UngetNextChar();
        return token;
      } else if (c == 'e' || c == 'E') {
        token.value.append(1, static_cast<char>(c));
        state = DFAState::kNumber5;
      } else {
        return error_occured();
      }
      break;
    case DFAState::kNumber5:
      if (isdigit(c)) {
        token.value.append(1, static_cast<char>(c));
        state = DFAState::kNumber7;
      } else if (c == '+' || c == '-') {
        token.value.append(1, static_cast<char>(c));
        state = DFAState::kNumber6;
      } else {
        return error_occured();
      }
      break;
    case DFAState::kNumber6:
      if (isdigit(c)) {
        token.value.append(1, static_cast<char>(c));
        state = DFAState::kNumber7;
      } else {
        return error_occured();
      }
      break;
    case DFAState::kNumber7:
      while (isdigit(c)) {
        token.value.append(1, static_cast<char>(c));
        c = GetNextChar();
      }
      if (c == '\0' || c == ',' || c == '}' || c == ']' || c == '#') {
        UngetNextChar();
        return token;
      } else {
        return error_occured();
      }
      break;
    }
  }
  return error_occured();
}

int32_t TokenizerImpl::DecodeUnicode() {
  auto hex_char_to_int = [] (char ch) -> int {
    if (ch >= '0' && ch <= '9') return ch - '0';
    else if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
    else if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
    return -1;
  };
  auto get_four_hex_digit = [this, hex_char_to_int] (int& code1,
                                                     int& code2,
                                                     int& code3,
                                                     int& code4) -> bool {
    code1 = hex_char_to_int(static_cast<char>(GetNextChar()));
    if (code1 < 0) {
      return false;
    }
    code2 = hex_char_to_int(static_cast<char>(GetNextChar()));
    if (code2 < 0) {
      return false;
    }
    code3 = hex_char_to_int(static_cast<char>(GetNextChar()));
    if (code3 < 0) {
      return false;
    }
    code4 = hex_char_to_int(static_cast<char>(GetNextChar()));
    if (code4 < 0) {
      return false;
    }
    return true;
  };

  int code1, code2, code3, code4;
  if (!get_four_hex_digit(code1, code2, code3, code4))
    return 0;
  auto code_point1 = (code1 << 12) + (code2 << 8) + (code3 << 4) + code4;
  if (code_point1 >= 0xD800 && code_point1 <= 0xDBFF) {
    // surrogate pair
    if (GetNextChar() != '\\' || GetNextChar() != 'u')
      return 0;

    int code5, code6, code7, code8;
    if (!get_four_hex_digit(code5, code6, code7, code8))
      return 0;
    auto code_point2 = (code5 << 12) + (code6 << 8) + (code7 << 4) + code8;
    if (code_point2 < 0xDC00 && code_point2 > 0xDFFF)
      return 0;
    code_point1 =
        0x10000 + ((code_point1 & 0x3ff) << 10) + (code_point2 & 0x3ff);
  } else if (code_point1 >= 0xDC00 && code_point1 <= 0xDFFF) {
    return 0;
  }
  return code_point1;
}

void ParserImpl::Serialize(const Value& value, 
                           std::string* csonpp_string) const {
  assert(csonpp_string);
  switch (value.Type()) {
  case Value::ValueType::kNull:
    *csonpp_string = "null";
    break;
  case Value::ValueType::kBool:
    *csonpp_string = value.AsBool() ? "true" : "false";
    break;
  case Value::ValueType::kInteger:
    *csonpp_string = std::to_string(value.GetInteger());
    break;
  case Value::ValueType::kDouble:
    *csonpp_string = to_string(value.GetDouble());
    break;
  case Value::ValueType::kString:
    csonpp_string->append(SerializeString(value.GetString()));
    break;
  case Value::ValueType::kObject:
    *csonpp_string = SerializeObject(value);
    break;
  case Value::ValueType::kArray:
    *csonpp_string = SerializeArray(value);
    break;
  default:
    csonpp_string->clear();
    break;
  }
}

std::string ParserImpl::SerializeObject(const Value& value) const {
  std::string result("{");
  const auto& object = value.GetObject();
  int size = value.Size();
  auto cur = 0;
  for (auto const_itr = object.Begin(); 
       const_itr != object.End(); 
       ++const_itr, ++cur) {
    result.append(SerializeString(const_itr->first));
    result.append(1, ':');
    std::string sub_str;
    Serialize(const_itr->second, &sub_str);
    result.append(std::move(sub_str));
    if (cur != size - 1)
      result.append(1, ',');
  }
  result.append(1, '}');
  return result;
}

std::string ParserImpl::SerializeArray(const Value& value) const {
  std::string result("[");
  const auto& array = value.AsArray();
  int size = array.Size();
  for (auto i = 0; i < size; ++i) {
    std::string sub_str;
    Serialize(array[i], &sub_str);
    result.append(std::move(sub_str));
    if (i != size - 1)
      result.append(1, ',');
  }
  result.append(1, ']');
  return result;
}

std::string ParserImpl::SerializeString(const std::string& utf8_string) const {
  auto int_to_hex_char = [] (int i) -> char {
    if (i >= 0 && i < 10) return i + '0';
    else if (i >= 10 && i < 16) return i - 10 + 'A';
    return 0;                                                                   
  };

  std::string result("\"");
  result.reserve(utf8_string.size() * 2);
  const char* ch = utf8_string.c_str();
  while (*ch) {
    int32_t code_point = Utf82CodePoint(ch);
    if (code_point < 0) {
      result.clear();
      return result;
    }

    if (code_point > 0x7F) {
      if (code_point <= 0xFFFF) { // in Basic Multilingual Plane
        result.append("\\u");
        result.append(1, int_to_hex_char(code_point >> 12));
        result.append(1, int_to_hex_char((code_point >> 8) & 0xF));
        result.append(1, int_to_hex_char((code_point >> 4) & 0xF));
        result.append(1, int_to_hex_char(code_point & 0xF));
      } else { // in Supplementary Planes
        code_point -= 0x10000;
        assert(code_point <= 0xFFFFF);
        int32_t lead_surrogate = 0xD800 + ((code_point >> 10) & 0x3FF);
        int32_t trail_surrogate = 0xDC00 + (code_point & 0x3FF);
        result.append("\\u");
        result.append(1, int_to_hex_char(lead_surrogate >> 12));
        result.append(1, int_to_hex_char((lead_surrogate >> 8) & 0xF));
        result.append(1, int_to_hex_char((lead_surrogate >> 4) & 0xF));
        result.append(1, int_to_hex_char(lead_surrogate & 0xF));
        result.append("\\u");
        result.append(1, int_to_hex_char(trail_surrogate >> 12));
        result.append(1, int_to_hex_char((trail_surrogate >> 8) & 0xF));
        result.append(1, int_to_hex_char((trail_surrogate >> 4) & 0xF));
        result.append(1, int_to_hex_char(trail_surrogate & 0xF));
      }
    } else if (code_point <= 0x1F) { // control charactor
      switch (code_point) {
      case '\b': result.append("\\b"); break;
      case '\f': result.append("\\f"); break;
      case '\n': result.append("\\n"); break;
      case '\r': result.append("\\r"); break;
      case '\t': result.append("\\t"); break;
      default:
        result.append("\\u00");
        result.append(1, int_to_hex_char((code_point >> 4) & 0xf));
        result.append(1, int_to_hex_char(code_point & 0xf));
        break;
      }
    } else if (code_point == '\\' || code_point == '\"') {
      result.append(1, '\\');
      result.append(1, code_point);
    } else {
      result.append(1, code_point);
    }
  }
  result.append(1, '\"');
  return result;
}

bool ParserImpl::Deserialize(const std::string& csonpp_string, Value* value) {
  tokenizer_ = std::unique_ptr<TokenizerImpl>(new TokenizerImpl(&csonpp_string));

  auto error_occured = [value, this] {
    *value = Value();
    tokenizer_->Reset();
    return false;
  };

  if (csonpp_string.empty()) {
    return error_occured();
  }

  if (!ParseValue(value)) {
    return error_occured();
  }
  return true;
}

bool ParserImpl::ParseValue(Value* value) {
  auto error_occured = [&value] {
    *value = Value();
    return false;
  };

  Token token = tokenizer_->GetToken();
  switch (token.type) {
  case Token::Type::kLeftBrace:
    return ParseObject(value);
  case Token::Type::kLeftBracket:
    return ParseArray(value);
  case Token::Type::kString:
    *value = Value(token.value);
    return true;
  case Token::Type::kInteger: {
    *value = Value(static_cast<int64_t>(strtoll(token.value.c_str(),
                                                nullptr,
                                                10)));
    return true;
  }
  case Token::Type::kDouble: {
    *value = Value(strtod(token.value.c_str(), nullptr));
    return true;
  }
  case Token::Type::kTrue:
    *value = Value(true);
    return true;
  case Token::Type::kFalse:
    *value = Value(false);
    return true;
  case Token::Type::kNull:
    *value = Value(nullptr);
    return true;
  default:
    return error_occured();
  }
}

bool ParserImpl::ParseObject(Value* value) {
  auto error_occured = [value] {
    *value = Value();
    return false;
  };
  *value = Value(Value::ValueType::kObject);
  if (!ParseMembers(value)) {
    tokenizer_->UngetNextChar();
    auto token = tokenizer_->GetToken();
    if (token.type != Token::Type::kRightBrace) {
      return error_occured();
    }
    *value = Value(Value::ValueType::kObject);
  }
  return true;
}

bool ParserImpl::ParseMembers(Value* value) {
  auto error_occured = [value] {
    *value = Value();
    return false;
  };
  if (!ParsePair(value)) {
    return error_occured();
  }
  auto token = tokenizer_->GetToken();
  if (token.type == Token::Type::kComma) {
    return ParseMembers(value);
  } else if (token.type == Token::Type::kRightBrace) {
    return true;
  } else {
    return error_occured();
  }
}

bool ParserImpl::ParsePair(Value* value) {
  auto error_occured = [&value] {
    *value = Value();
    return false;
  };
  auto token = tokenizer_->GetToken();
  if (token.type != Token::Type::kString) {
    return error_occured();
  }

  auto next_token = tokenizer_->GetToken();
  if (next_token.type != Token::Type::kColon) {
    return error_occured();
  }

  Value sub_value;
  if (!ParseValue(&sub_value)) {
    return error_occured();
  }
  (*value)[token.value] = sub_value;
  return true;
}

bool ParserImpl::ParseArray(Value* value) {
  auto error_occured = [value] {
    *value = Value();
    return false;
  };
  *value = Value(Value::ValueType::kArray);
  if (!ParseElements(value)) {
    tokenizer_->UngetNextChar();
    auto next_token = tokenizer_->GetToken();
    if (next_token.type != Token::Type::kRightBracket) {
      return error_occured();
    }
    *value = Value(Value::ValueType::kArray);
  }
  return true;
}

bool ParserImpl::ParseElements(Value* value) {
  auto error_occured = [value] {
    *value = Value();
    return false;
  };
  Value sub_value;
  if (!ParseValue(&sub_value)) {
    return error_occured();
  }
  value->Append(std::move(sub_value));
  auto next_token = tokenizer_->GetToken();
  if (next_token.type == Token::Type::kRightBracket) {
    return true;
  } else if (next_token.type == Token::Type::kComma) {
    return ParseElements(value);
  }
  return error_occured();
}

}  // namespace base
}  // namespace cnetpp

