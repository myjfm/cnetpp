// Copyright (c) 2005, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Author: wilsonh@google.com (Wilson Hsieh)
//
#include <cnetpp/base/string_piece.h>
#include <cnetpp/base/string_utils.h>

#include <limits.h>

#include <algorithm>

namespace cnetpp {
namespace base {

// defined in implementation only for shorter typing
typedef StringPiece::size_type size_type;

const size_type StringPiece::npos;

bool operator==(const StringPiece& x, const StringPiece& y) {
  if ((!x.data() && !y.data()) || (x.length() == 0 && y.length() == 0)) {
    return true;
  }
  if (!x.data() || !y.data()) {
    return false;
  }
  return ((x.size() == y.size()) && (x.data() == y.data() || !::memcmp(x.data(), y.data(), x.size())));
}

int StringPiece::compare(const StringPiece& x) const {
  if (&x == this) {
    return 0;
  }
  if (!x.data() && !this->data()) {
    return 0;
  }
  if (!x.data()) {
    return -1;
  }
  if(!this->data()) {
    return 1;
  }
  int equal = ::memcmp(ptr_, x.ptr_, len_ < x.len_ ? len_ : x.len_);
  if (equal != 0) {
    return equal;
  }
  if (len_ < x.len_) {
    return -1;
  } else if (len_ > x.len_) {
    return 1;
  }
  return 0;
}

int StringPiece::ignore_case_compare(const StringPiece& x) const {
  if (&x == this) {
    return 0;
  }
  if (!x.data() && !this->data()) {
    return 0;
  }
  if (!x.data()) {
    return -1;
  }
  if(!this->data()) {
    return 1;
  }
  size_t index = 0;
  while (index < x.size() && index < len_) {
    int diff = toupper(x[index]) - toupper(ptr_[index]);
    if (diff) {
      return diff;
    }
  }
  if (index == x.size() && index == len_) {
    return 0;
  }
  if (index == x.size()) {
    return -1;
  }
  return 1;
}

bool StringPiece::ignore_case_equal(const StringPiece& other) const {
  if (&other == this) {
    return true;
  }
  if (!other.data() && !this->data()) {
    return true;
  }
  if (!other.data() || !this->data() || this->size() != other.size()) {
    return false;
  }
  size_t index = 0;
  while (index < len_) {
    if (::toupper(other[index]) != ::toupper(ptr_[index])) {
      return false;
    }
    index++;
  }
  return true;
}

// Does "this" start with "x"
bool StringPiece::starts_with(const StringPiece& x) const {
  return ((len_ >= x.len_) && !::memcmp(ptr_, x.ptr_, x.len_));
}

// Does "this" end with "x"
bool StringPiece::ends_with(const StringPiece& x) const {
  return (len_ >= x.len_ &&
    !::memcmp(ptr_ + len_ - x.len_, x.ptr_, x.len_));
}

size_type StringPiece::copy(char* buf, size_type n, size_type pos) const {
  assert(buf);
  size_type ret = std::min(len_ - pos, n);
  ::memcpy(buf, ptr_ + pos, ret);
  return ret;
}

size_type StringPiece::find(const StringPiece& s, size_type pos) const {
  if (pos > len_) {
    return npos;
  }

  const char* res = std::search(ptr_ + pos,
                                ptr_ + len_,
                                s.ptr_,
                                s.ptr_ + s.len_);
  pos = res - ptr_;
  return pos + s.len_ <= len_ ? pos : npos;
}

size_type StringPiece::find(char c, size_type pos) const {
  if (pos >= len_) {
    return npos;
  }

  const char* res = std::find(ptr_ + pos, ptr_ + len_, c);
  return res != ptr_ + len_ ? static_cast<size_t>(res - ptr_) : npos;
}

size_type StringPiece::rfind(const StringPiece& s, size_type pos) const {
  if (len_ < s.len_) {
    return npos;
  }

  if (s.empty()) {
    return std::min(len_, pos);
  }

  const char* last = ptr_ + std::min(len_ - s.len_, pos) + s.len_;
  const char* res = std::find_end(ptr_, last, s.ptr_, s.ptr_ + s.len_);
  return res != last ? static_cast<size_t>(res - ptr_) : npos;
}

size_type StringPiece::rfind(char c, size_type pos) const {
  if (len_ == 0) {
    return npos;
  }

  for (size_type i = std::min(pos, len_ - 1);; --i) {
    if (ptr_[i] == c) {
      return i;
    }
    if (i == 0) {
      break;
    }
  }
  return npos;
}

// For each character in characters_wanted, sets the index corresponding
// to the ASCII code of that character to 1 in table.  This is used by
// the m_find.*_of methods below to tell whether or not a character is in
// the lookup table in constant time.
// The argument `table' must be an array that is large enough to hold all
// the possible values of an unsigned char.  Thus it should be be declared
// as follows:
//   bool table[UCHAR_MAX + 1]
static inline void BuildLookupTable(const StringPiece& characters_wanted,
  bool* table) {
  const size_type len = characters_wanted.length();
  const char* const data = characters_wanted.data();
  for (size_type i = 0; i < len; ++i) {
    table[static_cast<unsigned char>(data[i])] = true;
  }
}

size_type StringPiece::find_first_of(const StringPiece& s,
  size_type pos) const {
  if (len_ == 0 || s.len_ == 0) {
    return npos;
  }

  // Avoid the cost of BuildLookupTable() for a single-character search.
  if (s.len_ == 1) {
    return find_first_of(s.ptr_[0], pos);
  }

  bool lookup[UCHAR_MAX + 1] { false };
  BuildLookupTable(s, lookup);
  for (size_type i = pos; i < len_; ++i) {
    if (lookup[static_cast<unsigned char>(ptr_[i])]) {
      return i;
    }
  }
  return npos;
}

size_type StringPiece::find_first_not_of(const StringPiece& s,
  size_type pos) const {
  if (len_ == 0) {
    return npos;
  }

  if (s.len_ == 0) {
    return 0;
  }

  // Avoid the cost of BuildLookupTable() for a single-character search.
  if (s.len_ == 1) {
    return find_first_not_of(s.ptr_[0], pos);
  }

  bool lookup[UCHAR_MAX + 1] { false };
  BuildLookupTable(s, lookup);
  for (size_type i = pos; i < len_; ++i) {
    if (!lookup[static_cast<unsigned char>(ptr_[i])]) {
      return i;
    }
  }
  return npos;
}

size_type StringPiece::find_first_not_of(char c, size_type pos) const {
  if (len_ == 0) {
    return npos;
  }

  for (; pos < len_; ++pos) {
    if (ptr_[pos] != c) {
      return pos;
    }
  }
  return npos;
}

size_type StringPiece::find_last_of(const StringPiece& s, size_type pos) const {
  if (len_ == 0 || s.len_ == 0) {
    return npos;
  }

  // Avoid the cost of BuildLookupTable() for a single-character search.
  if (s.len_ == 1) {
    return find_last_of(s.ptr_[0], pos);
  }

  bool lookup[UCHAR_MAX + 1] { false };
  BuildLookupTable(s, lookup);
  for (size_type i = std::min(pos, len_ - 1); ; --i) {
    if (lookup[static_cast<unsigned char>(ptr_[i])]) {
      return i;
    }
    if (i == 0) {
      break;
    }
  }
  return npos;
}

size_type StringPiece::find_last_not_of(const StringPiece& s, size_type pos) const {
  if (len_ == 0) {
    return npos;
  }

  size_type i = std::min(pos, len_ - 1);
  if (s.len_ == 0) {
    return i;
  }

  // Avoid the cost of BuildLookupTable() for a single-character search.
  if (s.len_ == 1) {
    return find_last_not_of(s.ptr_[0], pos);
  }

  bool lookup[UCHAR_MAX + 1] { false };
  BuildLookupTable(s, lookup);
  for (; ; --i) {
    if (!lookup[static_cast<unsigned char>(ptr_[i])]) {
      return i;
    }
    if (i == 0) {
      break;
    }
  }
  return npos;
}

size_type StringPiece::find_last_not_of(char c, size_type pos) const {
  if (len_ == 0) {
    return npos;
  }

  for (size_type i = std::min(pos, len_ - 1); ; --i) {
    if (ptr_[i] != c) {
      return i;
    }
    if (i == 0) {
      break;
    }
  }
  return npos;
}

StringPiece StringPiece::substr(size_type pos, size_type n) const {
  if (pos > len_) {
    pos = len_;
  }
  if (n > len_ - pos) {
    n = len_ - pos;
  }
  return StringPiece(ptr_ + pos, n);
}

}  // namespace base
}  // namespace cnetpp

