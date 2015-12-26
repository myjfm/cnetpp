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
#ifndef CNETPP_BASE_STRING_UTILS_H_
#define CNETPP_BASE_STRING_UTILS_H_

#include <assert.h>
#include <stdint.h>
#include <string.h>

#include <string>
#include <vector>
#include <sstream>
#include <limits>

#include "string_piece.h"

namespace cnetpp {
namespace base {

class StringUtils {
 public:
  // remove all the whitespaces from the leftside of the string(c string)
  // NOTE: this operation is inplace
  static void LTrim(char* str);
  static void LTrim(std::string* str);
  // remove all the whitespaces from the leftside of the string(std string)
  // return the new string which has been trimed
  static std::string LTrim(StringPiece str);
  static void LTrim(StringPiece str, std::string* result);

  // remove all the whitespaces from the rightside of the string(c string)
  // NOTE: this operation is inplace
  static void RTrim(char* str);
  static void RTrim(std::string* str);
  // remove all the whitespaces from the leftside of the string(std string)
  // return the new string which has been trimed
  static std::string RTrim(StringPiece str);
  static void RTrim(StringPiece str, std::string* result);

  // remove all the whitespaces from both ends of the string(c string)
  // NOTE: this operation is inplace
  static void Trim(char* str);
  static void Trim(std::string* str);
  // remove all the whitespaces from both ends of the string(std string)
  static std::string Trim(StringPiece str);
  static void Trim(StringPiece str, std::string* result);

  // change all the lowercases to uppercases(c string)
  // NOTE: this operation is inplace
  static void ToUpper(char* str);
  static void ToUpper(std::string* str);
  // change all the lowercases to uppercases(std string)
  static std::string ToUpper(StringPiece str);
  static void ToUpper(StringPiece str, std::string* result);

  // change all the uppercases to lowercases(c string)
  // NOTE: this operation is inplace
  static void ToLower(char* str);
  static void ToLower(std::string* str);
  // change all the uppercases to lowercases(std string)
  static std::string ToLower(StringPiece str);
  static void ToLower(StringPiece str, std::string* result);

  // split the source string by the charactors
  // this method scans the str, once it finds the charactor which it exsits
  // in the separators, it will generate a new substring,
  // and put it into a container, finally returns the container
  static std::vector<std::string> SplitByChars(StringPiece str,
                                               StringPiece separators);
  static void SplitByChars(StringPiece str,
                           StringPiece separators,
                           std::vector<std::string>* result);

  // split the source string by the separator string
  // this method scans the str, once it finds the separator in the str,
  // it will generate a new substring, and put it into a container,
  // and finally, returns the container.
  static std::vector<std::string> SplitByString(StringPiece str,
                                                StringPiece separator);
  static void SplitByString(StringPiece str,
                            StringPiece separator,
                            std::vector<std::string>* result);

  // check if the charactor is the hexadecimal charactor
  static bool IsHexDigit(char c);

  // convert the hexadecimal charactor to integer, e.g. 'a' -> 10, '8' -> 8, etc
  // return the expected integer, -1 if error occured
  static int HexCharToInt(char c);

  // convert an integer to hexadecimal charactor, e.g. 10 -> 'a', 8 -> '8', etc
  // return the expected hexadecimal charactor, 0 if error occured
  static char IntToHexChar(int i);

  // check if the charactor is the uri charactor
  static bool IsUriChar(char c);

  // convert the hexadecimal string to integer
  // the parameter str is c string or std::string
  // the parameter success represents whether the convertion is succeeded,
  // if you don't care, just let it as default
  static int64_t HexStrToInteger(StringPiece str, bool* success = nullptr);

  // change all the charactors that is not the uri charactor to escape format
  // inplace function
  static void Escape(std::string* str);

  // change all the charactors that is not the uri charactor to escape format
  static std::string Escape(StringPiece str);
  static void Escape(StringPiece str, std::string* result);

  // convert a unicode code point to a utf-8 string
  // unicode code point ranges from [U+000000, U+10FFFF],
  // utf-8 has the format:
  // 1. 0xxxxxxx    1 byte, or
  // 2. 110xxxxx 10xxxxxx 2 byte, or
  // 3. 1110xxxx 10xxxxxx 10xxxxxx 3 byte, or
  // 4. 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx 4 byte
  // for more details, please refer to http://en.wikipedia.org/wiki/UTF-8
  // return "" if the unicode_code_point is not a valid codepoint
  static std::string CodePointToUtf8(int32_t unicode_code_point);

  // convert a utf-8 string to unicode code point
  // if the first charactor in utf8_str <= 0x7f, it is an ascii charactor,
  // else the utf-8 string occupies more than 1 bytes
  // return the converted unicode code point, -1 if error occured
  // NOTE: after the convertion,
  // utf8_string will point to the next position right after the utf-8 string
  static int32_t Utf8ToCodePoint(const char*& utf8_str);
};

}  // namespace base
}  // namespace cnetpp

#endif  // CNETPP_BASE_STRING_UTILS_H_

