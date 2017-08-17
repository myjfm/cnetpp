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
#ifndef CNETPP_BASE_URI_H_
#define CNETPP_BASE_URI_H_

#include <stdint.h>
#include <string>
#include <vector>
#include <regex>

namespace cnetpp {
namespace base {

class Uri {
 public:
  Uri() = default;
  ~Uri() = default;

  // whether it's a valid uri, false by default
  bool Valid() const noexcept {
    return valid_;
  }

  // given a string, parse it into uri
  // e.g. http://www.baidu.com/search/error.html?key=hello#anchor
  // The string is broken down into these parts:
  // scheme: "http",
  // authority: "www.baidu.com",
  // path: "/search/error.html",
  // query "key=hello", and,
  // fragment "anchor"
  // return true if the string is a valid uri, else false
  bool Parse(const std::string& uri);

  // get the uri with string format
  std::string String() const;

  // get the authority: [username:password@]host[:port]
  std::string Authority() const;

  std::string Scheme() const {
    return scheme_;
  }

  std::string Username() const {
    return username_;
  }

  std::string Password() const {
    return password_;
  }

  // get host part of Uri.
  // If host is an IPv6 address, square brackets will be returned,
  // e.g. [::ffff:192.168.89.9]
  // for more information, please refer to rfc3986
  std::string Host() const {
    return host_;
  }

  // This method is the same as Host(), except that when the host is an IPv6
  // address. This method will return raw IPv6 address without square brackets,
  // because some APIs only understands host without square brackets
  std::string Hostname() const;

  uint16_t Port() const noexcept {
    return port_;
  }

  std::string Path() const {
    return path_;
  }

  std::string Query() const {
    return query_;
  }

  std::string Fragment() const {
    return fragment_;
  }

  // get query parameters as key-value pairs
  // e.g. for the uri that contains query string: key1=foo&key2=&key3&=bar&=bar=
  // It returns a vector that contains 3 entries:
  // "key1" => "foo",
  // "key2" => ""
  // "key3" => ""
  // Parts "=bar" and "=bar=" are ignored, as they are not valid query parameters.
  // "=bar" is missing parameter name, while "=bar=" has more than one equal signs,
  // we don't know which one is the delimiter for key and value. It returns query
  // parameter key-value pairs in a vector, each element is a pair of which the
  // first element is parameter name and the second one is parameter value.
  std::vector<std::pair<std::string, std::string>> QueryParams() const {
    return query_params_;
  }

 private:
  static const std::regex kUriRegex;
  static const std::regex kAuthorityAndPathRegex;
  static const std::regex kAuthorityRegex;
  static const std::regex kQueryParamRegex;

  bool valid_ { false };
  std::string scheme_;
  std::string username_;
  std::string password_;
  std::string host_;
  uint16_t port_ { 0 };
  std::string port_str_;
  std::string path_;
  std::string query_;
  std::string fragment_;
  std::vector<std::pair<std::string, std::string>> query_params_;
};

}  // namespace base
}  // namespace cnetpp

#endif  // CNETPP_BASE_URI_H_

