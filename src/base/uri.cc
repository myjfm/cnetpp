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
#include "uri.h"

#include <cstdlib>
#include <string>
#include <unordered_map>

#include "string_utils.h"

namespace cnetpp {
namespace base {

const std::regex Uri::kUriRegex("([a-zA-Z][a-zA-Z0-9+.-]*):" // scheme:
                                 "([^?#]*)" // authority and path
                                 "(?:\\?([^#]*))?" // ?query
                                 "(?:#(.*))?"); // #fragment

const std::regex Uri::kAuthorityAndPathRegex("//([^/]*)(/.*)?");

const std::regex Uri::kAuthorityRegex(
    "(?:([^@:]*)(?::([^@]*))?@)?" // username, password
    "(\\[[^\\]]*\\]|[^\\[:]*)"    // host (IP-literal (e.g. '['+IPv6+']',
                                  // dotted-IPv4, or named host)
    "(?::(\\d*))?"); // port

const std::regex Uri::kQueryParamRegex("(^|&)([^=&]*)=?([^=&]*)(?=(&|$))");

static const std::unordered_map<std::string, uint16_t> kSchemeToPortMap = {{"http", 80}, {"https", 443}};
static uint16_t get_scheme_default_port(const std::string& schema) {
  auto i = kSchemeToPortMap.find(schema);
  if (i == kSchemeToPortMap.end()) {
    return 0;
  }
  return i->second;
}

bool Uri::Parse(const std::string& str_uri) {
  port_ = 0;

  std::smatch match;
  if (!std::regex_match(str_uri.begin(), str_uri.end(), match, kUriRegex)) {
    return false;
  }

  auto &sub_match = match[1];
  scheme_ = std::move(std::string(sub_match.first, sub_match.second));
  scheme_ = StringUtils::ToLower(scheme_);

  auto &authority_and_path = match[2];
  std::smatch authority_and_path_match;
  if (!std::regex_match(authority_and_path.first,
                        authority_and_path.second,
                        authority_and_path_match,
                        kAuthorityAndPathRegex)) {
    // Does not start with //, doesn't have authority 
    path_ = authority_and_path;
  } else {
    auto &authority = authority_and_path_match[1];
    std::smatch authority_match;
    if (!std::regex_match(authority.first,
                          authority.second,
                          authority_match,
                          kAuthorityRegex)) {
      return false;
    }

    port_str_ = authority_match[4].str();
    if (!port_str_.empty()) {
      port_ = static_cast<uint16_t>(std::strtoul(port_str_.c_str(), nullptr, 10));
    } else {
      port_ = get_scheme_default_port(scheme_);
    }

    username_ = std::string(authority_match[1].first,
                            authority_match[1].second);
    password_ = std::string(authority_match[2].first,
                            authority_match[2].second);
    host_ = std::string(authority_match[3].first,
                        authority_match[3].second);
    path_ = std::string(authority_and_path_match[2].first,
                        authority_and_path_match[2].second);
  }

  query_ = std::string(match[3].first, match[3].second);
  if (!query_.empty()) {
    // Parse query string 
    std::sregex_iterator param_begin_itr(match[3].first,
                                        match[3].second,
                                        kQueryParamRegex);
    std::sregex_iterator param_end_itr;
    for (auto itr = param_begin_itr; itr != param_end_itr; itr++) {
      if (itr->length(2) == 0) {
        // key is empty, ignore it 
        continue;
      }
      query_params_.emplace_back(
          std::string((*itr)[2].first, (*itr)[2].second), // parameter name
          std::string((*itr)[3].first, (*itr)[3].second)  // parameter value
      );
    }
  }

  fragment_ = std::move(std::string(match[4].first, match[4].second));
  valid_ = true;
  return true;
}

std::string Uri::String() const {
  std::string str_uri;
  str_uri.append(scheme_);
  str_uri.append("://");
  if (!password_.empty()) {
    str_uri.append(username_);
    str_uri.append(":");
    str_uri.append(password_);
    str_uri.append("@");
  } else if (!username_.empty()) {
    str_uri.append(username_);
    str_uri.append("@");
  }

  str_uri.append(host_);
  if (!port_str_.empty()) {
    str_uri.append(":");
#if 0
    str_uri.append(StringUtils::NumberToStr(port_));
#endif
    str_uri.append(port_str_);
  }
  str_uri.append(path_);
  if (!query_.empty()) {
    str_uri.append("?");
    str_uri.append(query_);
  }
  if (!fragment_.empty()) {
    str_uri.append("#");
    str_uri.append(fragment_);
  }

  return std::move(str_uri);
}

std::string Uri::Authority() const {
  std::string res;

  // Port is 5 characters max and we have up to 3 delimiters. 
  res.reserve(host_.size() + username_.size() + password_.size() + 8);

  if (!username_.empty() || !password_.empty()) {
    res.append(username_);

    if (!password_.empty()) {
      res.push_back(':');
      res.append(password_);
    }

    res.push_back('@');
  }

  res.append(host_);

  if (!port_str_.empty()) {
    res.push_back(':');
#if 0
    res.append(StringUtils::NumberToStr(port_));
#endif
    res.append(port_str_);
  }

  return std::move(res);
}

std::string Uri::Hostname() const {
  if (host_.size() > 0 && host_[0] == '[') {
    // If it starts with '[', then it should end with ']',
    // this is ensured by regex 
    return std::move(host_.substr(1, host_.size() - 2));
  }
  return host_;
}

}  // namespace base
}  // namespace cnetpp

