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
#include "http_response.h"

#include "../base/string_utils.h"

namespace cnetpp {
namespace http {

static const std::map<HttpResponse::StatusCode, const char*> kStatusReasonPhases =
{
  { HttpResponse::StatusCode::kContinue, "Continue" },
  { HttpResponse::StatusCode::kSwitchingProtocols, "Switching Protocols" },
  { HttpResponse::StatusCode::kOk, "OK" },
  { HttpResponse::StatusCode::kCreated, "Created" },
  { HttpResponse::StatusCode::kAccepted, "Accepted" },
  { HttpResponse::StatusCode::kNonAuthoritativeInformation, "Nonauthoritative Information" },
  { HttpResponse::StatusCode::kNoContent, "No Content" },
  { HttpResponse::StatusCode::kResetContent, "Reset Content" },
  { HttpResponse::StatusCode::kPartialContent, "Partial Content" },
  { HttpResponse::StatusCode::kMultipleChoices, "Multiple Choices" },
  { HttpResponse::StatusCode::kMovedPermanently, "Moved Permanently" },
  { HttpResponse::StatusCode::kFound, "Found" },
  { HttpResponse::StatusCode::kSeeOther, "See Other" },
  { HttpResponse::StatusCode::kNotModified, "Not Modified" },
  { HttpResponse::StatusCode::kUseProxy, "Use Proxy" },
  { HttpResponse::StatusCode::kTemporaryRedirect, "Temporary Redirect" },
  { HttpResponse::StatusCode::kBadRequest, "Bad Request" },
  { HttpResponse::StatusCode::kUnauthorized, "Unauthorized" },
  { HttpResponse::StatusCode::kPaymentRequired, "Payment Required" },
  { HttpResponse::StatusCode::kForbidden, "Forbidden" },
  { HttpResponse::StatusCode::kNotFound, "Not Found" },
  { HttpResponse::StatusCode::kMethodNotAllowed, "Method Not Allowed" },
  { HttpResponse::StatusCode::kNotAcceptable, "Not Acceptable" },
  { HttpResponse::StatusCode::kProxyAuthRequired, "Proxy Auth Required" },
  { HttpResponse::StatusCode::kRequestTimeout, "Request Timeout" },
  { HttpResponse::StatusCode::kConflict, "Conflict" },
  { HttpResponse::StatusCode::kGone, "Gone" },
  { HttpResponse::StatusCode::kLengthRequired, "Length Required" },
  { HttpResponse::StatusCode::kPreconditionFailed, "Precondition Failed" },
  { HttpResponse::StatusCode::kRequestEntityTooLarge, "Request Entity Too Large" },
  { HttpResponse::StatusCode::kRequestURITooLong, "Request URI Too Long" },
  { HttpResponse::StatusCode::kUnsupportedMediaType, "Unsupported Media Type" },
  { HttpResponse::StatusCode::kRequestedRangeNotSatisfiable, "Requested Range Not Satisfiable" },
  { HttpResponse::StatusCode::kExpectationFailed, "Expectation Failed" },
  { HttpResponse::StatusCode::kInternalServerError, "Internal Server Error" },
  { HttpResponse::StatusCode::kNotImplemented, "Not Implemented" },
  { HttpResponse::StatusCode::kBadGateway, "Bad Gateway" },
  { HttpResponse::StatusCode::kServiceUnavailable, "Service Unavailable" },
  { HttpResponse::StatusCode::kGatewayTimeout, "Gateway Timeout" },
  { HttpResponse::StatusCode::kHttpVersionNotSupported, "Http Version Not Supported" },
  { HttpResponse::StatusCode::kUnknown, NULL },
};

const char* HttpResponse::StatusCodeToReasonPhrase(StatusCode status_code) {
  auto itr = kStatusReasonPhases.find(status_code);
  if (itr == kStatusReasonPhases.end()) {
    return nullptr;
  }
  return itr->second;
}

void HttpResponse::Reset() {
  HttpPacket::Reset();
  status_ = StatusCode::kUnknown;
}

// without "\r\n"
void HttpResponse::AppendStartLineToString(std::string* result) const {
  assert(http_version() != Version::kVersionUnknown);
  assert(result);
  result->append(GetVersionString(HttpPacket::http_version()));
  result->append(" ");
  result->append(std::to_string(static_cast<int>(status_)));
  result->append(" ");
  result->append(StatusCodeToReasonPhrase(status_));
}

bool HttpResponse::ParseStartLine(base::StringPiece data, ErrorType* error) {
  ErrorType error_placeholder;
  if (!error) {
    error = &error_placeholder;
  }

  std::vector<std::string> fields;
  base::StringUtils::SplitByString(data, " ", &fields);
  if (fields.size() < 3) {
    *error = ErrorType::kStartLineNotComplete;
    return false;
  }

  HttpPacket::Version http_version =
      HttpPacket::GetVersionNumber(fields[0].c_str());
  if (http_version == Version::kVersionUnknown) {
    *error = ErrorType::kVersionUnsupported;
    return false;
  }
  set_http_version(http_version);

  status_ =
      static_cast<StatusCode>(std::strtol(fields[1].c_str(), nullptr, 10));
  const char* valid_reason_phase = StatusCodeToReasonPhrase(status_);
  if (!valid_reason_phase) {
    *error = ErrorType::kResponseStatusNotFound;
    return false;
  }

  std::string reason_phase;
  for (size_t i = 2; i < fields.size(); ++i) {
    if (i != 2) {
      reason_phase.append(" ");
    }
    reason_phase.append(fields[i]);
  }

  /*
  if (::strcmp(valid_reason_phase, reason_phase.c_str())) {
    *error = ErrorType::kResponseStatusNotFound;
    return false;
  }
  */
  return true;
}

}  // namespace http
}  // namespace cnetpp

