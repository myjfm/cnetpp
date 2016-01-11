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
#ifndef CNETPP_HTTP_HTTP_OPTIONS_H_
#define CNETPP_HTTP_HTTP_OPTIONS_H_

#include "http_callbacks.h"

namespace cnetpp {
namespace http {

class HttpOptions {
 public:
  HttpOptions() {
  }
  
  virtual ~HttpOptions() {
  }
  
  size_t send_buffer_size() const {
    return send_buffer_size_;
  }
  void set_send_buffer_size(size_t size) {
    send_buffer_size_ = size;
  }

  size_t receive_buffer_size() const {
    return receive_buffer_size_;
  }
  void set_receive_buffer_size(size_t size) {
    receive_buffer_size_ = size;
  }

  const std::string& remote_hostname() const {
    return remote_hostname_;
  }
  void set_remote_hostname(const std::string& remote_hostname) {
    remote_hostname_ = remote_hostname;
  }
  void set_remote_hostname(std::string&& remote_hostname) {
    remote_hostname_ = std::move(remote_hostname);
  }

  ConnectedCallbackType connected_callback() const {
    return connected_callback_;
  }
  void set_connected_callback(ConnectedCallbackType connected_callback) {
    connected_callback_ = connected_callback;
  }

  ClosedCallbackType closed_callback() const {
    return closed_callback_;
  }
  void set_closed_callback(ClosedCallbackType closed_callback) {
    closed_callback_ = closed_callback;
  }

  ReceivedCallbackType received_callback() const {
    return received_callback_;
  }
  void set_received_callback(ReceivedCallbackType received_callback) {
    received_callback_ = received_callback;
  }

  SentCallbackType sent_callback() const {
    return sent_callback_;
  }
  void set_sent_callback(SentCallbackType sent_callback) {
    sent_callback_ = sent_callback;
  }

 private:
  size_t send_buffer_size_ {32 * 1024 };
  size_t receive_buffer_size_ { 32 * 1024 };
  std::string remote_hostname_;  // used only for http client
  ConnectedCallbackType connected_callback_ { nullptr };
  ClosedCallbackType closed_callback_ { nullptr };
  ReceivedCallbackType received_callback_ { nullptr };
  SentCallbackType sent_callback_ { nullptr };
};

}  // namespace http
}  // namespace cnetpp

#endif  // CNETPP_HTTP_HTTP_OPTIONS_H_

