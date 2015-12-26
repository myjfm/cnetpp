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
// This file is private, please do not include this file
//
#ifndef CNETPP_BASE_CSONPP_IMPL_H_
#define CNETPP_BASE_CSONPP_IMPL_H_

#include "csonpp.h"

#include <cstdio>
#include <cstring>
#include <map>
#include <vector>
#include <limits>
#include <memory>

/***************************************
 * the grammer of json:
 * object   -> {}
 *          -> { members }
 *
 * members  -> pair
 *          -> pair, members
 *
 * pair     -> string : value
 *
 * array    -> []
 *          -> [ elements ]
 *
 * elements -> value
 *          -> value, elements
 * 
 * value    -> string
 *          -> number
 *          -> object
 *          -> array
 *          -> true
 *          -> false
 *          -> null
 **************************************/

namespace cnetpp {
namespace base {

struct Token {
  enum class Type {
    kDummy, 
    kLeftBrace,     // {
    kRightBrace,    // }
    kLeftBracket,   // [
    kRightBracket,  // ]
    kComma,         // ,
    kColon,         // :
    kString,        // "xxx"
    kInteger,       // integer
    kDouble,        // -0.99e-5
    kTrue,          // true
    kFalse,         // false
    kNull,          // null
  };

  std::string value;
  Type type;

  Token() : type(Type::kDummy) {}

  bool IsOk() {
    return type != Type::kDummy;
  }
};

class TokenizerImpl {
 public:
  TokenizerImpl(const std::string* str) : str_(str), cur_pos_(0) {
    assert(str_);
  }

  ~TokenizerImpl() {
  }

  void Reset() {
    str_ = nullptr;
    cur_pos_ = 0;
  }

  int GetNextChar() {
    return (*str_)[cur_pos_++];
  }

  void UngetNextChar() {
    cur_pos_--;
  }

  Token GetToken();

 private:
  // it's not responsible for releasing the string
  const std::string* str_;
  size_t cur_pos_;

  int32_t DecodeUnicode();
};

class ParserImpl {
 public:
  ParserImpl() = default;
  ~ParserImpl() = default;

  bool Deserialize(const std::string& str, Value* value);
  void Serialize(const Value& value, std::string* str) const;

 private:
  std::unique_ptr<TokenizerImpl> tokenizer_;

  bool ParseValue(Value* value);
  bool ParseObject(Value* value);
  bool ParseMembers(Value* value);
  bool ParsePair(Value* value);
  bool ParseArray(Value* value);
  bool ParseElements(Value* value);

  std::string SerializeObject(const Value& value) const;
  std::string SerializeArray(const Value& value) const;
  std::string SerializeString(const std::string& utf8_str) const;
};

template<class T>
std::string Number2Str(T iNum) {
  return std::to_string(iNum);
}

std::string to_string(float num) {
  constexpr size_t size = std::numeric_limits<float>::max_exponent10 + 32;
  char buf[size];
  std::snprintf(buf, size, "%#.8g", num);
  char* tail = buf + strlen(buf) - 1;
  if (*tail != '0') {
    return buf;
  }
  while (tail > buf && *tail == '0') {
    --tail;
  }
  char* last_non_zero = tail;
  while (tail >= buf) {
    switch (*tail) {
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      --tail;
      break;
    case '.':
      *(last_non_zero + 2) = '\0';
      return buf;
    }
  }
  return buf;
}

std::string to_string(double num) {
  constexpr size_t size = std::numeric_limits<double>::max_exponent10 + 32;
  char buf[size];
  std::snprintf(buf, size, "%#.16g", num);
  char* tail = buf + strlen(buf) - 1;
  if (*tail != '0') {
    return buf;
  }
  while (tail > buf && *tail == '0') {
    --tail;
  }
  char* last_non_zero = tail;
  while (tail >= buf) {
    switch (*tail) {
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      --tail;
      break;
    case '.':
      *(last_non_zero + 2) = '\0';
      return buf;
    }
  }
  return buf;
}

std::string to_string(long double num) {
  constexpr size_t size = std::numeric_limits<long double>::max_exponent10 + 32;
  char buf[size];
  std::snprintf(buf, size, "%#.32Lg", num);
  char* tail = buf + strlen(buf) - 1;
  if (*tail != '0') {
    return buf;
  }
  while (tail > buf && *tail == '0') {
    --tail;
  }
  char* last_non_zero = tail;
  while (tail >= buf) {
    switch (*tail) {
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      --tail;
      break;
    case '.':
      *(last_non_zero + 2) = '\0';
      return buf;
    }
  }
  return buf;
}

}  // namespace base
}  // namespace cnetpp

#endif  // CNETPP_BASE_CSONPP_IMPL_H_

