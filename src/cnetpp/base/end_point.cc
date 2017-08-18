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
#include <cnetpp/base/end_point.h>
#include <cnetpp/base/string_utils.h>

#include <arpa/inet.h>
#include <assert.h>
#include <netinet/in.h>

namespace cnetpp {
namespace base {

bool EndPoint::ToSockAddr(struct sockaddr* address,
                          socklen_t* address_len) const {
  assert(address);
  assert(address_len);

  if (port_ <= 0) {
    return false;
  }

  switch (address_.Family()) {
    case AF_INET: {
      //*address_len = IPAddress::kIPv4AddressSize;
      *address_len = sizeof(struct sockaddr_in);
      auto in_address = reinterpret_cast<struct sockaddr_in*>(address);
      in_address->sin_family = AF_INET;
      in_address->sin_port = htons(port_);
      ::memcpy(&(in_address->sin_addr),
               &((address_.address())[0]),
               IPAddress::kIPv4AddressSize);
      break;
    }
    case AF_INET6: {
      //*address_len = IPAddress::kIPv6AddressSize;
      *address_len = sizeof(struct sockaddr_in6);
      auto in6_address = reinterpret_cast<struct sockaddr_in6*>(address);
      in6_address->sin6_family = AF_INET6;
      in6_address->sin6_port = htons(port_);
      memcpy(&(in6_address->sin6_addr),
             &((address_.address())[0]),
             IPAddress::kIPv6AddressSize);
      break;
    }
    default:
      return false;
  }
  return true;
}

bool EndPoint::FromSockAddr(const struct sockaddr& address,
                            socklen_t address_len) {
  auto error = [this] {
    address_.mutable_address().clear();
    port_ = 0;
    return false;
  };
  switch (address_len) {
//    case IPAddress::kIPv4AddressSize: {
    case sizeof(struct sockaddr_in): {
      if (address_len < static_cast<socklen_t>(sizeof(struct sockaddr_in))) {
        return error();
      }
      auto in_address = reinterpret_cast<const struct sockaddr_in*>(&address);
      if (in_address->sin_family != AF_INET) {
        return error();
      }
      port_ = ntohs(in_address->sin_port);
      address_.mutable_address().resize(IPAddress::kIPv4AddressSize);
      memcpy(&((address_.mutable_address())[0]),
             &(in_address->sin_addr),
             IPAddress::kIPv4AddressSize);
      break;
    }
//    case IPAddress::kIPv6AddressSize: {
    case sizeof(struct sockaddr_in6): {
      if (address_len < static_cast<socklen_t>(sizeof(struct sockaddr_in6))) {
        return error();
      }
      auto in6_address = reinterpret_cast<const struct sockaddr_in6*>(&address);
      if (in6_address->sin6_family != AF_INET6) {
        return error();
      }
      port_ = ntohs(in6_address->sin6_port);
      address_.mutable_address().resize(IPAddress::kIPv6AddressSize);
      memcpy(&((address_.mutable_address())[0]),
             &(in6_address->sin6_addr),
             IPAddress::kIPv6AddressSize);
      break;
    }
    default:
      return error();
  }
  return true;
}

}  // namespace base
}  // namespace cnetpp

namespace std {

void swap(cnetpp::base::EndPoint& a, cnetpp::base::EndPoint& b) {
  a.Swap(b);
}

}  // namespace std

