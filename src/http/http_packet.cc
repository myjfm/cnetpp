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
#include "http_packet.h"

#include "../base/string_utils.h"

namespace cnetpp {
namespace http {

static const std::map<HttpPacket::Version, const char*> kHttpVersions = {
  { HttpPacket::Version::kVersion09, "HTTP/0.9" },
  { HttpPacket::Version::kVersion10, "HTTP/1.0" },
  { HttpPacket::Version::kVersion11, "HTTP/1.1" },
  { HttpPacket::Version::kVersionUnknown, NULL },
};

void HttpPacket::HttpHeaders::AppendToString(std::string* result) const {
  for (auto& http_header : http_headers_) {
    result->append(http_header.first);
    result->append(": ");
    result->append(http_header.second);
    result->append("\r\n");
  }
}

void HttpPacket::HttpHeaders::ToString(std::string* result) const {
  result->clear();
  AppendToString(result);
}

std::string HttpPacket::HttpHeaders::ToString() const {
  std::string result;
  AppendToString(&result);
  return result;
}

// Get a header value. return false if it does not exist.
// the header name is not case sensitive.
bool HttpPacket::HttpHeaders::Get(base::StringPiece name, std::string** value) {
  for (auto& http_header : http_headers_) {
    if (name.ignore_case_equal(http_header.first)) {
      *value = &http_header.second;
      return true;
    }
  }
  return false;
}

bool HttpPacket::HttpHeaders::Get(base::StringPiece name,
                                  const std::string** value) const {
  return const_cast<HttpHeaders*>(this)->Get(name,
                                             const_cast<std::string**>(value));
}

bool HttpPacket::HttpHeaders::Get(base::StringPiece name,
                                  std::string* value) const {
  const std::string* pvalue;
  if (Get(name, &pvalue)) {
    *value = *pvalue;
    return true;
  }
  return false;
}

// Used when a http header appears multiple times.
// return false if it doesn't exist.
bool HttpPacket::HttpHeaders::Get(base::StringPiece name,
                                  std::vector<std::string>* values) const {
  values->clear();
  for (auto& http_header : http_headers_) {
    if (name.ignore_case_equal(http_header.first)) {
      values->push_back(http_header.second);
    }
  }
  return values->size() > 0;
}

// Set a header field. if it exists, overwrite the header value.
HttpPacket::HttpHeaders& HttpPacket::HttpHeaders::Set(base::StringPiece name,
                                                      base::StringPiece value) {
    // NOTE: their may be multiple headers share the same name,
    // remove all headers firstly
    Remove(name);
    Add(name, value);
    return *this;
}

// Add a header field, just append, no overwrite.
HttpPacket::HttpHeaders& HttpPacket::HttpHeaders::Add(base::StringPiece name,
                                                      base::StringPiece value) {
  http_headers_.push_back(std::make_pair(name.as_string(), value.as_string()));
  return *this;
}

HttpPacket::HttpHeaders& HttpPacket::HttpHeaders::Add(const HttpHeaders& that) {
  http_headers_.insert(http_headers_.end(),
                       that.http_headers_.begin(),
                       that.http_headers_.end());
  return *this;
}

bool HttpPacket::HttpHeaders::Remove(base::StringPiece name) {
  bool result = false;
  for (auto itr = http_headers_.begin(); itr != http_headers_.end();) {
    if (name.ignore_case_equal(itr->first)) {
      itr = http_headers_.erase(itr);
      result = true;
    } else {
      ++itr;
    }
  }
  return result;
}

bool HttpPacket::HttpHeaders::Has(base::StringPiece name) const {
  for (auto& http_header : http_headers_) {
    if (name.ignore_case_equal(http_header.first)) {
      return true;
    }
  }
  return false;
}

size_t HttpPacket::HttpHeaders::Count() const {
  return http_headers_.size();
}

bool HttpPacket::HttpHeaders::GetAt(
    int index,
    std::pair<std::string, std::string>* header) const {
  if (index < 0 || index >= static_cast<int>(http_headers_.size())) {
    return false;
  }
  *header = http_headers_[index];
  return true;
}

bool HttpPacket::HttpHeaders::Parse(base::StringPiece data,
                                    ErrorType* error) {
  ErrorType error_placeholder;
  if (!error) {
    error = &error_placeholder;
  }

  http_headers_.clear();

  std::vector<std::string> lines;
  base::StringUtils::SplitByString(data, "\r\n", &lines);

  // Skip the head line and the last line(empty but '\n')
  for (size_t i = 0; i < lines.size(); ++i) {
    std::string::size_type pos = lines[i].find(":");
    if (pos != std::string::npos) {
      http_headers_.push_back(std::pair<std::string, std::string>(
            base::StringUtils::Trim(lines[i].substr(0, pos)),
            base::StringUtils::Trim(lines[i].substr(pos + 1))));
    } else {
      if (lines[i].empty()) {
        *error = ErrorType::kFieldNotComplete;
        http_headers_.clear();
        return false;
      }
    }
  }

  *error = ErrorType::kOk;
  return true;
}

void HttpPacket::HttpHeaders::Clear() {
    http_headers_.clear();
}

void HttpPacket::HttpHeaders::Swap(HttpHeaders* that) {
    http_headers_.swap(that->http_headers_);
}

void HttpPacket::Reset() {
  http_version_ = Version::kVersion11;
  http_headers_.Clear();
  http_body_.clear();
}

void HttpPacket::AppendHttpHeadersToString(std::string* result) const {
  AppendStartLineToString(result);
  result->append("\r\n");
  http_headers_.AppendToString(result);
  result->append("\r\n");
}

void HttpPacket::HttpHeadersToString(std::string* result) const {
  result->clear();
  AppendHttpHeadersToString(result);
}

std::string HttpPacket::HttpHeadersToString() const {
  std::string result;
  AppendHttpHeadersToString(&result);
  return result;
}

void HttpPacket::AppendToString(std::string* result) const {
  AppendHttpHeadersToString(result);
  result->append(http_body_);
}

void HttpPacket::ToString(std::string* result) const {
  result->clear();
  AppendToString(result);
}

std::string HttpPacket::ToString() const {
  std::string result;
  AppendToString(&result);
  return result;
}

// Get a header value. return false if it does not exist.
// the header name is not case sensitive.
bool HttpPacket::GetHttpHeader(base::StringPiece name, std::string** value) {
  return http_headers_.Get(name, value);
}

bool HttpPacket::GetHttpHeader(base::StringPiece name,
                               const std::string** value) const {
  return http_headers_.Get(name, value);
}

bool HttpPacket::GetHttpHeader(base::StringPiece name,
                               std::string* value) const {
  const std::string* pvalue;
  if (GetHttpHeader(name, &pvalue)) {
    *value = *pvalue;
    return true;
  }
  return false;
}

std::string HttpPacket::GetHttpHeader(base::StringPiece name) const {
  std::string value;
  GetHttpHeader(name, &value);
  return value;
}

// Used when a http header appears multiple times.
// return false if it doesn't exist.
bool HttpPacket::GetHttpHeaders(base::StringPiece name,
                                std::vector<std::string>* values) const {
  return http_headers_.Get(name, values);
}

// Set a header field. if it exists, overwrite the header value.
void HttpPacket::SetHttpHeader(base::StringPiece name,
                               base::StringPiece value) {
  http_headers_.Set(name, value);
}

void HttpPacket::SetHttpHeaders(const HttpHeaders& http_headers) {
  http_headers_ = http_headers;
}

// Add a header field, just append, no overwrite.
void HttpPacket::AddHttpHeader(base::StringPiece name,
                               base::StringPiece value) {
  http_headers_.Add(name, value);
}

void HttpPacket::AddHttpHeaders(const HttpHeaders& http_headers) {
  http_headers_.Add(http_headers);
}

bool HttpPacket::RemoveHttpHeader(base::StringPiece name) {
  return http_headers_.Remove(name);
}

bool HttpPacket::HasHttpHeader(base::StringPiece name) const {
  return http_headers_.Has(name);
}

const char* HttpPacket::GetVersionString(Version version) {
  return kHttpVersions.find(version)->second;
}

HttpPacket::Version HttpPacket::GetVersionNumber(
    base::StringPiece http_version) {
  for (auto itr = kHttpVersions.begin(); itr != kHttpVersions.end(); ++itr) {
    if (itr->second && http_version.ignore_case_equal(itr->second)) {
      return itr->first;
    }
  }
  return Version::kVersionUnknown;
}

bool HttpPacket::ParseHttpHeaders(base::StringPiece data, ErrorType* error) {
  ErrorType error_placeholder;
  if (error == nullptr) {
    error = &error_placeholder;
  }

  base::StringPiece::size_type pos = data.find_first_of('\n');
  if (pos == base::StringPiece::npos) {
    pos = data.size();
  }
  std::string first_line = base::StringUtils::RTrim(data.substr(0, pos));

  if (first_line.empty()) {
    *error = ErrorType::kNoStartLine;
    return false;
  }

  if (!ParseStartLine(first_line, error)) {
    return false;
  }

  return http_headers_.Parse(data.substr(pos + 1), &error_placeholder);
}

int HttpPacket::GetContentLength() {
  std::string content_length;
  if (!GetHttpHeader("Content-Length", &content_length)) {
    return -1;
  }
  int length = std::strtol(content_length.c_str(), NULL, 10);
  return (length >= 0) ? length : -1;
};

bool HttpPacket::IsKeepAlive() const {
  const std::string* alive;
  if (!GetHttpHeader("Connection", &alive)) {
    if (http_version_ < Version::kVersion11) {
      return false;
    }
    return true;
  }
  return strcasecmp(alive->c_str(), "keep-alive") == 0;
}

}  // namespace http
}  // namespace cnetpp

