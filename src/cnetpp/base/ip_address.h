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
#ifndef CNETPP_BASE_IP_ADDRESS_H_
#define CNETPP_BASE_IP_ADDRESS_H_

#include <cnetpp/base/string_piece.h>

#include <stdint.h>

#include <cstdlib>
#include <limits>
#include <vector>

namespace cnetpp {
namespace base {

class IPAddress final {
 public:
  static const size_t kIPv4AddressSize = 4; // 4 bytes
  static const int kIPv6AddressSize = 16; // 16 bytes

  // convert IPv4 address like '192.168.1.1', or
  // IPv6 address like
  // '2001:0DB8:02de:0000:0000:0000:0000:0e13',
  // or '2001:DB8:2de:0000:0000:0000:0000:e13',
  // or '2001:0DB8:02de::0e13',
  // or '::e13:2001',
  // or 'abcd:0001::',
  // or '::ffff:192.168.1.1'
  // to number, in network byte order
  // return false if the str_addr is invalid
  static bool LiteralToNumber(StringPiece str_addr, IPAddress* number);

  // convert IPv4 or IPv6 address with binary format to string format
  static bool NumberToLiteral(const IPAddress& number, std::string* str_addr);

  IPAddress() = default;
  ~IPAddress() = default;

  explicit IPAddress(StringPiece str_addr);

  IPAddress(const IPAddress&) = default;
  IPAddress& operator=(const IPAddress&) = default;
  IPAddress(IPAddress&&) = default;
  IPAddress& operator=(IPAddress&&) = default;

  int Family() const;

  std::vector<uint8_t>& mutable_address() {
    return address_;
  }

  const std::vector<uint8_t>& address() const {
    return address_;
  }

  size_t Size() const {
    return address_.size();
  }

  // onlu ipv4
  // ipv6   -> 255.255.255.255
  // unspec -> 255.255.255.255
  uint32_t ToIPv4ID() const;

  std::string ToString() const;

  void Swap(IPAddress& ip_address) {
    std::swap(address_, ip_address.address_);
  }

 private:
  static bool IPv4LiteralToNumber(StringPiece str_addr,
                                  IPAddress* number);
  static bool IPv6LiteralToNumber(StringPiece str_addr,
                                  IPAddress* number);

  static bool IPv4NumberToLiteral(const IPAddress& number,
                                  std::string* str_addr);
  static bool IPv6NumberToLiteral(const IPAddress& number,
                                  std::string* str_addr);

  std::vector<uint8_t> address_;
};

class IPAddressList {
 public:
  std::vector<IPAddress> addresses_;
};

}  // namespace base
}  // namespace cnetpp

namespace std {

void swap(cnetpp::base::IPAddress& a, cnetpp::base::IPAddress& b);

}  // namespace std

#endif  // CNETPP_BASE_IP_ADDRESS_H_

