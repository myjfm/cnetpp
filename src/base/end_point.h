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
#ifndef CNETPP_BASE_END_POINT_H_
#define CNETPP_BASE_END_POINT_H_

#include <sys/socket.h>

#include <string>

#include "ip_address.h"

namespace cnetpp {
namespace base {

class EndPoint final {
 public:
  EndPoint() = default;
  ~EndPoint() = default;

  EndPoint(const IPAddress& address, int port)
      : address_(address),
        port_(port) {
  }

  EndPoint(IPAddress&& address, int port)
      : address_(std::move(address)),
        port_(port) {
  }

  EndPoint(const StringPiece& address, int port)
      : address_(address), port_(port) {
  }

  EndPoint(const struct sockaddr& address, socklen_t address_length) {
    bool res = FromSockAddr(address, address_length);
    assert(res);
  }

  EndPoint(const EndPoint&) = default;
  EndPoint& operator=(const EndPoint&) = default;

  EndPoint(EndPoint&&) = default;
  EndPoint& operator=(EndPoint&&) = default;

  int Family() const {
    return address_.Family();
  }

  int Port() const {
    return port_;
  }

  IPAddress& mutable_address() {
    return address_;
  }

  const IPAddress& address() const {
    return address_;
  }

  bool ToSockAddr(struct sockaddr* address,
                  socklen_t* address_len) const;

  bool FromSockAddr(const struct sockaddr& address,
                    socklen_t address_len);

  std::string ToString() const {
    std::string res;
    res.reserve(64);
    res.append(address_.ToString());
    res.append(1, ',');
    res.append(std::to_string(static_cast<unsigned>(port_)));
    return std::move(res);
  }

  std::string ToStringWithoutPort() const {
    return address_.ToString();
  }

  void Swap(EndPoint& end_point) {
    std::swap(address_, end_point.address_);
    std::swap(port_, end_point.port_);
  }

 private:
  IPAddress address_;
  int port_ { 0 };
};

}  // namespace base
}  // namespace cnetpp

namespace std {

void swap(cnetpp::base::EndPoint& a, cnetpp::base::EndPoint& b);

}  // namespace std

#endif  // CNETPP_BASE_END_POINT_H_

