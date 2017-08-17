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
#ifndef CNETPP_HTTP_HTTP_RESPONSE_H_
#define CNETPP_HTTP_HTTP_RESPONSE_H_

#include "http_packet.h"

#include <string>

#include "../base/string_piece.h"

namespace cnetpp {
namespace http {

// Describes a http response.
class HttpResponse final : public HttpPacket {
 public:
  // See RFC2616: 10 Status Code Definition
  enum class StatusCode {
    kUnknown = -1,
    kContinue = 100,
    kSwitchingProtocols = 101,

    kOk = 200,
    kCreated = 201,
    kAccepted = 202,
    kNonAuthoritativeInformation = 203,
    kNoContent = 204,
    kResetContent = 205,
    kPartialContent = 206,

    kMultipleChoices = 300,
    kMovedPermanently = 301,
    kFound = 302,
    kSeeOther = 303,
    kNotModified = 304,
    kUseProxy = 305,
    kTemporaryRedirect = 307,

    kBadRequest = 400,
    kUnauthorized = 401,
    kPaymentRequired = 402,
    kForbidden = 403,
    kNotFound = 404,
    kMethodNotAllowed = 405,
    kNotAcceptable = 406,
    kProxyAuthRequired = 407,
    kRequestTimeout = 408,
    kConflict = 409,
    kGone = 410,
    kLengthRequired = 411,
    kPreconditionFailed = 412,
    kRequestEntityTooLarge = 413,
    kRequestURITooLong = 414,
    kUnsupportedMediaType = 415,
    kRequestedRangeNotSatisfiable = 416,
    kExpectationFailed = 417,

    kInternalServerError = 500,
    kNotImplemented = 501,
    kBadGateway = 502,
    kServiceUnavailable = 503,
    kGatewayTimeout = 504,
    kHttpVersionNotSupported = 505,
  };

  static const char* StatusCodeToReasonPhrase(StatusCode status_code);

  HttpResponse() : status_(StatusCode::kUnknown) {
  }
  ~HttpResponse() {
  }
  
  virtual void Reset();

  StatusCode status() const {
    return status_;
  }
  void set_status(StatusCode status) {
    status_ = status;
  }

  void Swap(HttpResponse* that) {
    HttpPacket::Swap(that);
    using std::swap;
    swap(status_, that->status_);
  }

 private:
  virtual void AppendStartLineToString(std::string* result) const;
  virtual bool ParseStartLine(base::StringPiece data, ErrorType* error);

  StatusCode status_;
};

}  // namespace http
}  // namespace cnetpp

// adapt to std::swap
namespace std {

template <>
inline void swap(cnetpp::http::HttpResponse& left,
                 cnetpp::http::HttpResponse& right) {
    left.Swap(&right);
}

}  // namespace std

#endif  // CNETPP_HTTP_HTTP_RESPONSE_H_

