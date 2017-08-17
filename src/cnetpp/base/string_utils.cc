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
#include "string_utils.h"

#include <arpa/inet.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>

namespace cnetpp {
namespace base {

void StringUtils::LTrim(char* str) {
  assert(str != nullptr);

  char* p = str;
  while (::isspace(*p)) {
    p++;
  }

  if (p == str) {
    return;
  }

  while (*str++ = *p++) {}
}

void StringUtils::LTrim(std::string* str) {
  assert(str);
  *str = LTrim(*str);
}

std::string StringUtils::LTrim(StringPiece str) {
  const char* p = str.data();
  while (::isspace(*p)) {
    p++;
  }

  return std::move(
      str.substr(p - str.data(),
                 str.length() - (p - str.data())).as_string());
}

void StringUtils::LTrim(StringPiece str, std::string* result) {
  assert(result);
  *result = LTrim(str);
}

void StringUtils::RTrim(char* str) {
  assert(str != nullptr);

  char* p = str + ::strlen(str) - 1;

  while (p >= str && ::isspace(*p)) {
    *p = '\0';
    --p;
  }
}

void StringUtils::RTrim(std::string* str) {
  assert(str);
  *str = RTrim(*str);
}

std::string StringUtils::RTrim(StringPiece str) {
  if (str.length() == 0) {
    return "";
  }

  const char* head = str.data();

  const char* tail = head + str.length() - 1;
  while (tail >= head && ::isspace(*tail)) {
    tail--;
  }

  return std::string(head, tail - head + 1);
}

void StringUtils::RTrim(StringPiece str, std::string* result) {
  assert(result);
  *result = std::move(RTrim(str));
}

void StringUtils::Trim(char* str) {
  LTrim(str);
  RTrim(str);
}

void StringUtils::Trim(std::string* str) {
  assert(str);
  *str = std::move(Trim(*str));
}

std::string StringUtils::Trim(StringPiece str) {
  const char* head = str.data();
  const char* tail = head + str.length() - 1;

  while (*head && ::isspace(*head)) {
    ++head;
  }

  while (tail >= head && ::isspace(*tail)) {
    --tail;
  }

  return std::string(head, tail - head + 1);
}

void StringUtils::Trim(StringPiece str, std::string* result) {
  assert(result);
  *result = std::move(Trim(str));
}

void StringUtils::ToUpper(char* str) {
  assert(str);
  while (*str) {
    if (*str >= 'a' && *str <= 'z') {
      *str += 'A' - 'a';
    }
    ++str;
  }
}

void StringUtils::ToUpper(std::string* str) {
  for (auto& c : *str) {
    if (c >= 'a' && c <= 'z') {
      c += 'A' - 'a';
    }
  }
}

std::string StringUtils::ToUpper(StringPiece str) {
  std::string dst_str = std::move(str.as_string());
  ToUpper(&dst_str);
  return std::move(dst_str);
}

void StringUtils::ToUpper(StringPiece str, std::string* result) {
  assert(result);
  *result = std::move(ToUpper(str));
}

void StringUtils::ToLower(char* str) {
  assert(str);
  while (*str) {
    if (*str >= 'A' && *str <= 'Z') {
      *str += 'a' - 'A';
    }
    ++str;
  }
}

void StringUtils::ToLower(std::string* str) {
  assert(str);
  for (auto& c : *str) {
    if (c >= 'A' && c <= 'Z') {
      c += 'a' - 'A';
    }
  }
}

std::string StringUtils::ToLower(StringPiece str) {
  std::string dst_str = std::move(str.as_string());
  ToLower(&dst_str);
  return std::move(dst_str);
}

void StringUtils::ToLower(StringPiece str, std::string* result) {
  assert(result);
  *result = std::move(ToLower(str));
}

std::vector<std::string> StringUtils::SplitByChars(StringPiece str,
                                                   StringPiece separators) {
  std::vector<std::string> sub_strs;
  SplitByChars(str, separators, &sub_strs);
  return std::move(sub_strs);
}

void StringUtils::SplitByChars(StringPiece str,
                               StringPiece separators,
                               std::vector<std::string>* result) {
  assert(result);
  result->clear();
  if (str.empty() || separators.empty()) {
    result->push_back(std::move(str.as_string()));
    return;
  }

  int map[256] = { 0 };
  for (auto itr = separators.begin(); itr != separators.end(); ++itr) {
    map[static_cast<size_t>(*itr)] = true;
  }

  const char* head = str.data();
  const char* tail = head;
  while (*head) {
    while (*head && map[static_cast<size_t>(*head)]) {
      head++;
    }

    if (!*head) {
      return;
    }

    tail = head + 1;
    while (*tail && !map[static_cast<size_t>(*tail)]) {
      tail++;
    }

    std::string tmp_str(head, tail - head);
    result->push_back(std::move(tmp_str));
    head = tail;
  }
}

std::vector<std::string> StringUtils::SplitByString(
    StringPiece str,
    StringPiece separator) {
  std::vector<std::string> sub_strs;
  SplitByString(str, separator, &sub_strs);
  return std::move(sub_strs);
}

void StringUtils::SplitByString(StringPiece str,
                                StringPiece separator,
                                std::vector<std::string>* result) {
  assert(result);
  if (str.empty() || separator.empty()) {
    result->push_back(std::move(str.as_string()));
    return;
  }

  StringPiece::size_type pre_pos = 0;
  StringPiece::size_type cur_pos;

  while (true) {
    cur_pos = str.find(separator, pre_pos);
    if (cur_pos == StringPiece::npos) {
      cur_pos = str.length();
      if (cur_pos != pre_pos) {
        std::string tmp(str.data() + pre_pos, cur_pos - pre_pos);
        result->push_back(std::move(tmp));
      }
      break;
    } else {
      if (cur_pos != pre_pos) {
        std::string tmp(str.data() + pre_pos, cur_pos - pre_pos);
        result->push_back(std::move(tmp));
      }
      pre_pos = cur_pos + separator.length();
    }
  }
}

uint32_t StringUtils::ToUint32(StringPiece str) {
  assert(str.size() >= sizeof(uint32_t));
  return ntohl(*reinterpret_cast<const uint32_t*>(str.data()));
}

char* StringUtils::PutUint32(uint32_t value, char* buf) {
  assert(buf);
  value = htonl(value);
  memcpy(buf, reinterpret_cast<const void*>(&value), sizeof(uint32_t));
  return buf + sizeof(uint32_t);
}

int StringUtils::ParseVarint32(StringPiece str, uint32_t* value) {
  assert(value);

  if (str.size() < 1) {
    return 0;
  }
  char tmp = str[0];
  if (tmp >= 0) {
    *value = tmp;
    return 1;
  }

  if (str.size() < 2) {
    return 0;
  }
  *value = tmp & 0x7f;
  if ((tmp = str[1]) >= 0) {
    *value |= tmp << 7;
    return 2;
  } else {
    *value |= (tmp & 0x7f) << 7;
    if (str.size() < 3) {
      return 0;
    }
    if ((tmp = str[2]) >= 0) {
      *value |= tmp << 14;
      return 3;
    } else {
      *value |= (tmp & 0x7f) << 14;
      if (str.size() < 4) {
        return 0;
      }
      if ((tmp = str[3]) >= 0) {
        *value |= tmp << 21;
        return 4;
      } else {
        *value |= (tmp & 0x7f) << 21;
        if (str.size() < 5) {
          return 0;
        }
        *value |= (tmp = str[4]) << 28;
        if (tmp < 0) {
          for (int i = 0; i < 5; i++) {
            if (str.size() < i + 6) {
              return 0;
            }
            if (str[i + 5] >= 0) {
              return i + 6;
            }
          }
          // invalid data, should not happen
          return -1;
        }
        return 5;
      }
    }
  }
}

int StringUtils::ToVarint32(uint32_t value, char* buf) {
  assert(buf);
  size_t i = 0;
  while (value > 0x7f) {
    buf[i++] = 0x80 | (value & 0x7f);
    value >>= 7;
  }
  buf[i++] = value;
  return i;
}

bool StringUtils::IsHexDigit(char c) {
  if ((c >= '0' && c <= '9') || 
      (c >= 'a' && c <= 'f') || 
      (c >= 'A' && c <= 'F')) {
    return true;
  }
  return false;
}

int StringUtils::HexCharToInt(char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  } else if (c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  } else if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  }
  return -1;
}

char StringUtils::IntToHexChar(int i) {
  if (i >= 0 && i < 10) {
    return i + '0';
  } else if (i >= 10 && i < 16) {
    return i - 10 + 'A';
  } else {
    return 0;
  }
}

bool StringUtils::IsUriChar(char c) {
  switch (c) {
  // unreserved
  case '0': case '1': case '2': case '3': case '4': case '5': case '6': 
  case '7': case '8': case '9':
  case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': 
  case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n': 
  case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u': 
  case 'v': case 'w': case 'x': case 'y': case 'z':
  case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': 
  case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N': 
  case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U': 
  case 'V': case 'W': case 'X': case 'Y': case 'Z':
  case '-': case '.': case '_': case '~': 
  // reserved
  case '!': case '*': case '\'': case '(': case ')': case ';': case ':': 
  case '@': case '&': case '=': case '+': case '$': case ',': case '/': 
  case '?': case '#': case '[': case ']':
    return true;
  default:
    return false;
  }
}

int64_t StringUtils::HexStrToInteger(StringPiece str, bool* success) {
  const char* index = str.data();
  int64_t i = 0;
  bool succ = false;

  if (str.length() > 2 && str[0] == '0' && 
      (str[1] == 'x' || str[1] == 'X')) {
    index = str.data() + 2;
  }

  while (true) {
    if (*index == '\0')
      break;

    if (*index >= '0' && *index <= '9') {
      succ = true;
      i = i * 16 + *index - '0';
    } else if (*index >= 'a' && *index <= 'f') {
      succ = true;
      i = i * 16 + (*index - 'a' + 10);
    } else if (*index >= 'A' && *index <= 'F') {
      succ = true;
      i = i * 16 + (*index - 'A' + 10);
    } else {
      succ = false;
      break;
    }

    index++;
  }

  if (success)
    *success = succ;
  return i;
}

void StringUtils::Escape(std::string* str) {
  std::string result;
  result = Escape(*str);
  *str = std::move(result);
}

std::string StringUtils::Escape(StringPiece str) {
  std::string esc_str;
  for (auto itr = str.begin(); itr != str.end(); ++itr) {
    if (IsUriChar(*itr)) {
      esc_str.append(1, *itr);
    } else {
      char buffer[10];
      sprintf(buffer,
              "%%%X%X",
              static_cast<unsigned char>(*itr) >> 4,
              (*itr) & 0x0f);
      esc_str += buffer;
    }
  }
  return std::move(esc_str);
}

void StringUtils::Escape(StringPiece str, std::string* result) {
  assert(result);
  *result = std::move(Escape(str));
}

std::string StringUtils::CodePointToUtf8(int32_t unicode_code_point) {
  std::string res;
  if (unicode_code_point < 0x0 || unicode_code_point > 0x10FFFF)
    return res;
  
  if (unicode_code_point <= 0x7F) {
    res.resize(1);
    res[0] = static_cast<char>(unicode_code_point);
  } else if (unicode_code_point <= 0x7FF) {
    res.resize(2);
    res[0] = static_cast<char>(0xC0 | (unicode_code_point >> 6));
    res[1] = static_cast<char>(0x80 | (unicode_code_point & 0x3F));
  } else if (unicode_code_point <= 0xFFFF) {
    res.resize(3);
    res[0] = static_cast<char>(0xE0 | (unicode_code_point >> 12));
    res[1] = static_cast<char>(0x80 | ((unicode_code_point >> 6) & 0x3F));
    res[2] = static_cast<char>(0x80 | (unicode_code_point & 0x3F));
  } else if (unicode_code_point <= 0x10FFFF) {
    res.resize(4);
    res[0] = static_cast<char>(0xF0 | (unicode_code_point >> 18));
    res[1] = static_cast<char>(0x80 | ((unicode_code_point >> 12) & 0x3F));
    res[2] = static_cast<char>(0x80 | ((unicode_code_point >> 6) & 0x3F));
    res[3] = static_cast<char>(0x80 | (unicode_code_point & 0x3F));
  }
  return res;
}

int32_t StringUtils::Utf8ToCodePoint(const char*& utf8_str) {
  assert(utf8_str);
  if (static_cast<uint32_t>(*utf8_str) <= 0x7F) {
    return *utf8_str++;
  }
  auto l1 = static_cast<uint32_t>(*utf8_str) & 0xFF;
  auto l2 = static_cast<uint32_t>(*(utf8_str + 1)) & 0xFF;
  auto l2_error = [&utf8_str] (uint32_t iL2) {
    if (!iL2) utf8_str++;
    else utf8_str += 2;
    return -1;
  };
  auto l3_error = [&utf8_str] (uint32_t iL3) {
    if (!iL3) utf8_str += 2;
    else utf8_str += 3;
    return -1;
  };
  auto l4_error = [&utf8_str] (uint32_t iL4) {
    if (!iL4) utf8_str += 3;
    else utf8_str += 4;
    return -1;
  };

  if ((l1 >> 5) == 0x06) { // first byte is 110xxxxx
    if ((l2 >> 6) == 0x02) { // second byte is 10xxxxxx
      utf8_str += 2;
      return ((l1 & 0x1F) << 6) | (l2 & 0x3F);
    }
    return l2_error(l2);
  }

  if ((l1 >> 4) == 0x0E) { // first is 1110xxxx
    if ((l2 >> 6) == 0x02) { // second byte is 10xxxxxx
      auto l3 = static_cast<uint32_t>(*(utf8_str + 2)) & 0xFF;
      if ((l3 >> 6) == 0x02) { // third byte is also 10xxxxxx
        utf8_str += 3;
        return ((l1 & 0x0F) << 12) | ((l2 & 0x3F) << 6) | (l3 & 0x3F);
      }
      return l3_error(l3);
    }
    return l2_error(l2);
  }

  if ((l1 >> 3) == 0x01E) { // first is 11110xxx
    if ((l2 >> 6) == 0x02) { // second byte is 10xxxxxx
      auto l3 = static_cast<uint32_t>(*(utf8_str + 2)) & 0xFF;
      if ((l3 >> 6) == 0x02) { // third byte is also 10xxxxxx
        auto l4 = static_cast<uint32_t>(*(utf8_str + 3)) & 0xFF;
        if ((l4 >> 6) == 0x02) {
          utf8_str += 4;
          return ((l1 & 0x07) << 18) | 
                  ((l2 & 0x3F) << 12) | 
                  ((l3 & 0x3F) << 6) | 
                  (l4 & 0x3F);
        }
        return l4_error(l4);
      }
      return l3_error(l3);
    }
    return l2_error(l2);
  }
  return -1;
}

}  // namespace base
}  // namespace cnetpp

