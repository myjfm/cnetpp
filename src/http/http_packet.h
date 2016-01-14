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
#ifndef CNETPP_HTTP_HTTP_PACKET_H_
#define CNETPP_HTTP_HTTP_PACKET_H_

#include <map>
#include <string>
#include <vector>

#include "../base/string_piece.h"

namespace cnetpp {
namespace http {

// Describes an http packet, which is the base class for http request and response.
// It includes the start line, headers and body.
class HttpPacket {
 public:
  enum class Version {
    kVersionUnknown = 0,
    kVersion09 = 9,
    kVersion10 = 10,
    kVersion11 = 11,
  };
  
  enum class ErrorType {
    kOk = 0,
    kNoStartLine,
    kStartLineNotComplete,
    kVersionUnsupported,
    kResponseStatusNotFound,
    kFieldNotComplete,
    kMethodNotFound,
    kMessageNotComplete,
  };
  
  // Store http headers information
  class HttpHeaders final {
   public:
    // Return false if it doesn't exist.
    bool Get(base::StringPiece name, std::string** value);
    bool Get(base::StringPiece name, const std::string** value) const;
    bool Get(base::StringPiece name, std::string* value) const;

    // Used when a http header appears multiple times.
    // return false if it doesn't exist.
    bool Get(base::StringPiece name, std::vector<std::string>* values) const;
    // Set a header field. if it exists, overwrite the header value.
    HttpHeaders& Set(base::StringPiece name, base::StringPiece value);
    // Add a header field, just append, no overwrite.
    HttpHeaders& Add(base::StringPiece name, base::StringPiece value);
    // Add all the header fields in rhs into this. no overwrite.
    HttpHeaders& Add(const HttpHeaders& that);

    // Remove an http header field.
    bool Remove(base::StringPiece name);

    // Get count of header
    size_t Count() const;

    // Get header by index
    bool GetAt(int index, std::pair<std::string, std::string>* header) const;

    // If has a header
    bool Has(base::StringPiece name) const;

    // Convert start line and headers to string.
    void AppendToString(std::string* result) const;
    void ToString(std::string* result) const;
    std::string ToString() const;

    bool Parse(base::StringPiece data, ErrorType* error = NULL);

    void Clear();

    void Swap(HttpHeaders* that);

   private:
    std::vector<std::pair<std::string, std::string> > http_headers_;
  };

  HttpPacket() : http_version_(Version::kVersion11) {
  }
  virtual ~HttpPacket() {
  }

  virtual void Reset();

  // Parse http headers (including the start line) from data
  // return: error code which is defined as ErrorType.
  virtual bool ParseHttpHeaders(base::StringPiece data,
                                ErrorType* error = NULL);

  std::string StartLine() const {
    std::string result;
    AppendStartLineToString(&result);
    return result;
  }

  Version http_version() const {
    return http_version_;
  }
  void set_http_version(Version version) {
    http_version_ = version;
  }

  const std::string& http_body() const {
    return http_body_;
  }
  std::string& mutable_http_body() {
    return http_body_;
  }

  void set_http_body(base::StringPiece body) {
    http_body_.assign(body.data(), body.size());
  }

  int GetContentLength();
  bool IsKeepAlive() const;

  // Get the header value.
  const HttpHeaders& http_headers() const {
    return http_headers_;
  }

  HttpHeaders& mutable_http_headers() {
    return http_headers_;
  }

  // Return false if it doesn't exist.
  bool GetHttpHeader(base::StringPiece name, std::string** value);
  bool GetHttpHeader(base::StringPiece name, const std::string** value) const;
  bool GetHttpHeader(base::StringPiece name, std::string* value) const;
  std::string GetHttpHeader(base::StringPiece name) const;
  // Used when a http header appears multiple times.
  // return false if it doesn't exist.
  bool GetHttpHeaders(base::StringPiece name,
                      std::vector<std::string>* values) const;

  // Set a header field. if it exists, overwrite the header value.
  void SetHttpHeader(base::StringPiece name, base::StringPiece value);
  // Replace the header with those in parameters 'headers'
  void SetHttpHeaders(const HttpHeaders& headers);

  // Add a header field, just append, no overwrite.
  void AddHttpHeader(base::StringPiece name, base::StringPiece value);
  // Insert the items from 'headers'
  void AddHttpHeaders(const HttpHeaders& headers);
  // Remove an http header field.
  bool RemoveHttpHeader(base::StringPiece name);

  // If has a header
  bool HasHttpHeader(base::StringPiece name) const;

  // Convert start line and headers to string.
  void AppendHttpHeadersToString(std::string* result) const;
  void HttpHeadersToString(std::string* result) const;
  std::string HttpHeadersToString() const;

  void AppendToString(std::string* result) const;
  void ToString(std::string* result) const;
  std::string ToString() const;

 protected:
  static const char* GetVersionString(Version http_version);
  static Version GetVersionNumber(base::StringPiece http_version);

  // append without ending "\r\n"
  virtual void AppendStartLineToString(std::string* result) const = 0;
  virtual bool ParseStartLine(base::StringPiece data, ErrorType* error) = 0;

  void Swap(HttpPacket* that) {
    using std::swap;
    swap(http_version_, that->http_version_);
    http_headers_.Swap(&that->http_headers_);
    swap(http_body_, that->http_body_);
  }

 private:
  Version http_version_;
  HttpHeaders http_headers_;
  std::string http_body_;
};

} // namespace http
} // namespace cnetpp

// adapt to std::swap
namespace std {
template <>
inline void swap(cnetpp::http::HttpPacket::HttpHeaders& left,
                 cnetpp::http::HttpPacket::HttpHeaders& right) noexcept {
    left.Swap(&right);
}

} // namespace std

#endif  // CNETPP_HTTP_HTTP_PACKET_H_

