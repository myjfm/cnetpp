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
#ifndef CNETPP_HTTP_HTTP_REQUEST_H_
#define CNETPP_HTTP_HTTP_REQUEST_H_

#include "http_packet.h"

#include <algorithm>
#include <string>

namespace cnetpp {
namespace http {

// Describes a http request.
class HttpRequest final : public HttpPacket {
 public:
  enum class MethodType {
    kUnknown = -1,
    kHead,
    kGet,
    kPost,
    kPut,
    kDelete,
    kOptions,
    kTrace,
    kConnect,
    kLastField,
  };

  static MethodType GetMethodByName(const char* method_name);
  static const char* GetMethodName(MethodType method);

  HttpRequest() : method_(MethodType::kUnknown), uri_("/") {
  }
  ~HttpRequest() {
  }

  virtual void Reset();
 
  MethodType method() const {
    return method_;
  }
  void set_method(MethodType method) {
    method_ = method;
  }

  const std::string& uri() const {
    return uri_;
  }
  void set_uri(base::StringPiece uri) {
    uri_ = std::move(uri.as_string());
  }

  void Swap(HttpRequest* that) {
    HttpPacket::Swap(that);
    using std::swap;
    swap(method_, that->method_);
    swap(uri_, that->uri_);
  }

 private:
  virtual void AppendStartLineToString(std::string* result) const;
  virtual bool ParseStartLine(base::StringPiece data, ErrorType* error = NULL);

  MethodType method_;
  std::string uri_;
};

}  // namespace http
}  // namespace cnetpp

// adapt to std::swap
namespace std {

template <>
inline void swap(cnetpp::http::HttpRequest& left,
                 cnetpp::http::HttpRequest& right) {
    left.Swap(&right);
}

}  // namespace std

#endif  // CNETPP_HTTP_HTTP_REQUEST_H_

