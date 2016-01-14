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
#include "http_request.h"
#include "../base/string_utils.h"

namespace cnetpp {
namespace http {

// NOTE: The order must be consistent with enum values because GetMethodName
// access this table by method_type enum as index
static const std::map<HttpRequest::MethodType, const char*> kValidMethodNames =
{
  { HttpRequest::MethodType::kHead, "HEAD" },
  { HttpRequest::MethodType::kGet, "GET" },
  { HttpRequest::MethodType::kPost, "POST" },
  { HttpRequest::MethodType::kPut, "PUT" },
  { HttpRequest::MethodType::kDelete, "DELETE" },
  { HttpRequest::MethodType::kOptions, "OPTIONS" },
  { HttpRequest::MethodType::kTrace, "TRACE" },
  { HttpRequest::MethodType::kConnect, "CONNECT" },
  { HttpRequest::MethodType::kUnknown, NULL },
};

void HttpRequest::Reset() {
    HttpPacket::Reset();
    method_ = MethodType::kUnknown;
    uri_ = "/";
}

HttpRequest::MethodType HttpRequest::GetMethodByName(const char* method_name) {
  for (auto itr = kValidMethodNames.begin();
       itr != kValidMethodNames.end();
       ++itr) {
    // Method is case sensitive.
    if (itr->second && strcmp(method_name, itr->second) == 0) {
      return itr->first;
    }
  }
  return MethodType::kUnknown;
}

const char* HttpRequest::GetMethodName(HttpRequest::MethodType method) {
  auto itr = kValidMethodNames.find(method);
  if (itr != kValidMethodNames.end()) {
    return itr->second;
  }
  return nullptr;
}

bool HttpRequest::ParseStartLine(base::StringPiece data, ErrorType* error) {
  ErrorType error_placeholder;
  if (!error) {
    error = &error_placeholder;
  }

  std::vector<std::string> fields;
  base::StringUtils::SplitByString(data, " ", &fields);
  if (fields.size() != 2 && fields.size() != 3) {
    *error = ErrorType::kStartLineNotComplete;
    return false;
  }

  method_ = GetMethodByName(fields[0].c_str());
  if (method_ == MethodType::kUnknown) {
    *error = ErrorType::kMethodNotFound;
    return false;
  }
  uri_ = fields[1];

  if (fields.size() == 3) {
    Version http_version = GetVersionNumber(fields[2]);
    if (http_version == Version::kVersionUnknown) {
      *error = ErrorType::kVersionUnsupported;
      return false;
    }
    set_http_version(http_version);
  }

  return true;
}

void HttpRequest::AppendStartLineToString(std::string* result) const {
  assert(method_ != MethodType::kUnknown);
  assert(result);
  result->append(GetMethodName(method_));
  result->append(" ");
  result->append(uri_);
  result->append(" ");
  result->append(GetVersionString(http_version()));
}

}  // namespace http
}  // namespace cnetpp

