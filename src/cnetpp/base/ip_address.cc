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
#include <cnetpp/base/ip_address.h>
#include <cnetpp/base/string_utils.h>

#include <sys/socket.h>

#include <cstdlib>
#include <string>

namespace cnetpp {
namespace base {

// const size_t IPAddress::kIPv4AddressSize = 4;
// const size_t IPAddress::kIPv6AddressSize = 16;

bool IPAddress::LiteralToNumber(StringPiece str_addr, IPAddress* number) {
  assert(number);

  if (str_addr.find(":") == StringPiece::npos) {
    return IPv4LiteralToNumber(str_addr, number);
  } else {
    return IPv6LiteralToNumber(str_addr, number);
  }
}

bool IPAddress::NumberToLiteral(const IPAddress& number,
                                std::string* str_addr) {
  assert(str_addr);

  switch (number.Family()) {
    case AF_INET:
      return IPv4NumberToLiteral(number, str_addr);
    case AF_INET6:
      return IPv6NumberToLiteral(number, str_addr);
    default:
      return false;
  }
}

bool IPAddress::IPv4LiteralToNumber(StringPiece str_addr, IPAddress* number) {
  assert(number);

  std::vector<uint8_t>().swap(number->address_);

  if (str_addr.empty()) {
    return false;
  }

  auto stdstr_addr = str_addr.as_string();
  auto pieces = StringUtils::SplitByChars(stdstr_addr, ".");
  if (pieces.size() != 4) {
    return false;
  }
  for (auto& p : pieces) {
    if (p.size() > 3) {
      return false;
    }
    char* pos = nullptr;
    auto int_p = static_cast<uint16_t>(::strtoull(p.c_str(), &pos, 10));
    if (pos == p.data() || int_p > 255) {
      return false;
    }
    (number->address_).push_back(int_p);
  }
  return true;
}

bool IPAddress::IPv6LiteralToNumber(StringPiece str_addr, IPAddress* number) {
  assert(number);

  std::vector<uint8_t>().swap(number->address_);

  if (str_addr.empty()) {
    return false;
  }

  auto convert_piece = [] (const std::string& piece, IPAddress* number) {
    if (piece.length() > 4) {
      return false;
    }
    char* pos = nullptr;
    auto int_p = static_cast<uint16_t>(::strtoull(piece.c_str(), &pos, 16));
    if (pos == piece.data()) {
      return false;
    }
    (number->address_).push_back((int_p & 0xFF00) >> 8);
    (number->address_).push_back(int_p & 0x00FF);
    return true;
  };

  auto stdstr_addr = str_addr.as_string();
  auto pos = stdstr_addr.find("::");
  if (pos == std::string::npos) {
    auto pieces = StringUtils::SplitByChars(stdstr_addr, ":");
    if (pieces.size() != 8) {
      return false;
    }
    for (auto& p : pieces) {
      if (!convert_piece(p, number)) {
        return false;
      }
    }
  } else {
    if (pos == 0) {
      auto pieces = StringUtils::SplitByChars(stdstr_addr, ":");
      if (pieces.size() < 1) {
        return false;
      } else {
        if (pieces[pieces.size() - 1].size() > 4) { // IPv4-mapped address
          for (size_t i = 0; i < 8 - pieces.size() - 1; ++i) {
            (number->address_).push_back(0);
            (number->address_).push_back(0);
          }
          for (size_t i = 0; i < pieces.size() - 1; ++i) {
            if (!convert_piece(pieces[i], number)) {
              return false;
            }
          }
          IPAddress ipv4_number;
          if (!IPv4LiteralToNumber(pieces[pieces.size() - 1], &ipv4_number)) {
            return false;
          }
          auto& ipv4_pieces = ipv4_number.address();
          for (auto& p : ipv4_pieces) {
            (number->address_).push_back(p);
          }
        } else {
          for (size_t i = 0; i < 8 - pieces.size(); ++i) {
            (number->address_).push_back(0);
            (number->address_).push_back(0);
          }
          for (auto& p : pieces) {
            if (!convert_piece(p, number)) {
              return false;
            }
          }
        }
      }
    } else if (pos == stdstr_addr.size() - 2) {
      auto pieces = StringUtils::SplitByChars(stdstr_addr, ":");
      if (pieces.size() < 1) {
        return false;
      }
      for (auto& p : pieces) {
        if (!convert_piece(p, number)) {
          return false;
        }
      }
      for (size_t i = 0; i < 8 - pieces.size(); ++i) {
        (number->address_).push_back(0);
        (number->address_).push_back(0);
      }
    } else {
      auto components = StringUtils::SplitByString(stdstr_addr, "::");
      if (components.size() != 2) {
        return false;
      }
      auto first_pieces = StringUtils::SplitByChars(components[0], ":");
      auto second_pieces = StringUtils::SplitByChars(components[1], ":");
      if (first_pieces.size() < 1 || second_pieces.size() < 1) {
        return false;
      }
      for (auto& p : first_pieces) {
        if (!convert_piece(p, number)) {
          return false;
        }
      }
      for (size_t i = 0;
           i < 8 - first_pieces.size() - second_pieces.size();
           ++i) {
        (number->address_).push_back(0);
        (number->address_).push_back(0);
      }
      for (auto& p : second_pieces) {
        if (!convert_piece(p, number)) {
          return false;
        }
      }
    }
  }
  return true;
}

bool IPAddress::IPv4NumberToLiteral(const IPAddress& number,
                                    std::string* str_addr) {
  assert(str_addr);

  if (number.Size() != kIPv4AddressSize) {
    return false;
  }

  for (size_t i = 0; i < kIPv4AddressSize; ++i) {
    str_addr->append(std::to_string(static_cast<unsigned>(number.address_[i])));
    if (i != kIPv4AddressSize - 1) {
      str_addr->append(1, '.');
    }
  }
  return true;
}

bool IPAddress::IPv6NumberToLiteral(const IPAddress& number,
                                    std::string* str_addr) {
  assert(str_addr);

  if (number.Size() != kIPv6AddressSize) {
    return false;
  }

  auto begin = -1;
  auto end = -1;
  auto tmp_begin = -1;
  for (int i = 0; i < kIPv6AddressSize; i += 2) {
    int tmp = (number.address_[i] << 8) | number.address_[i + 1];
    if (tmp == 0) {
      if (tmp_begin < 0) {
        tmp_begin = i;
      }
    } else {
      if (begin == 0 || tmp_begin < 0) {
        continue;
      } else if (begin < 0) {
        begin = tmp_begin;
        end = i;
      } else {
        if (i - tmp_begin > end - begin) {
          begin = tmp_begin;
          end = i;
        }
      }
      tmp_begin = -1;
    }
  }

  for (int i = 0; i < kIPv6AddressSize;) {
    if (i == begin) { 
      if (i == 0) {
        str_addr->append(1, ':');
      }
      str_addr->append(1, ':');
      i = end;
    } else {
      std::string tmp_str;
      int tmp = (number.address_[i] << 8) | number.address_[i + 1];
      for (auto j = 3; j >= 0; --j) {
        char c = StringUtils::IntToHexChar((tmp >> j) & 0xf);
        if (c == '0' && !tmp_str.empty()) {
          tmp_str.append(1, c);
        }
        tmp_str.append(1, c);
      }
      if (tmp_str.empty()) {
        tmp_str = "0";
      }
      str_addr->append(tmp_str);
      if (i + 2 < kIPv6AddressSize) {
        str_addr->append(1, ':');
      }
      i += 2;
    }
  }
  return true;
}

IPAddress::IPAddress(StringPiece str_addr) {
  if (!LiteralToNumber(str_addr, this)) {
    address_.clear();
  }
}

int IPAddress::Family() const {
  switch (address_.size()) {
    case kIPv4AddressSize:
      return AF_INET;
    case kIPv6AddressSize:
      return AF_INET6;
    default:
      return AF_UNSPEC;
  }
}

std::string IPAddress::ToString() const {
  std::string str_addr;
  if (!NumberToLiteral(*this, &str_addr)) {
    return "";
  }
  return str_addr;
}

}  // namespace base
}  // namespace cnetpp

namespace std {

void swap(cnetpp::base::IPAddress& a, cnetpp::base::IPAddress& b) {
  a.Swap(b);
}

}  // namespace std

