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
#ifndef CNETPP_HTTP_HTTP_CONNECTION_H_
#define CNETPP_HTTP_HTTP_CONNECTION_H_

#include <assert.h>

#include <memory>

#include "http_callbacks.h"
#include "http_packet.h"
#include "../tcp/tcp_connection.h"

namespace cnetpp {
namespace http {

class HttpConnection : public std::enable_shared_from_this<HttpConnection> {
 public:
  explicit HttpConnection(std::shared_ptr<tcp::TcpConnection> tcp_connection)
      : tcp_connection_(tcp_connection) {
    assert(tcp_connection_.get());
    connection_id_ = tcp_connection_->id();
  }
  ~HttpConnection() = default;

  tcp::ConnectionId id() const {
    return connection_id_;
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

  std::shared_ptr<tcp::TcpConnection> tcp_connection() {
    return tcp_connection_;
  }

  const ConnectedCallbackType& connected_callback() const {
    return connected_callback_;
  }
  ConnectedCallbackType& mutable_connected_callback() {
    return connected_callback_;
  }
  void set_connected_callback(const ConnectedCallbackType& connected_callback) {
    connected_callback_ = connected_callback;
  }

  const ClosedCallbackType& closed_callback() const {
    return closed_callback_;
  }
  ClosedCallbackType& mutable_closed_callback() {
    return closed_callback_;
  }
  void set_closed_callback(const ClosedCallbackType& closed_callback) {
    closed_callback_ = closed_callback;
  }

  const ReceivedCallbackType& received_callback() const {
    return received_callback_;
  }
  ReceivedCallbackType& mutable_received_callback() {
    return received_callback_;
  }
  void set_received_callback(const ReceivedCallbackType& received_callback) {
    received_callback_ = received_callback;
  }

  const SentCallbackType& sent_callback() const {
    return sent_callback_;
  }
  SentCallbackType& mutable_sent_callback() {
    return sent_callback_;
  }
  void set_sent_callback(SentCallbackType sent_callback) {
    sent_callback_ = sent_callback;
  }

  std::shared_ptr<HttpPacket> http_packet() {
    return http_packet_;
  }
  void set_http_packet(std::shared_ptr<HttpPacket> http_packet) {
    http_packet_ = http_packet;
  }

  bool SendPacket(std::shared_ptr<HttpPacket> http_packet);
  bool SendPacket(base::StringPiece data);

  bool OnConnected();

  bool OnReceived();

  bool OnSent(bool success);

  bool OnClosed();

  void MarkAsClosed() {
    if (tcp_connection_.get()) {
      tcp_connection_->MarkAsClosed();
    }
  }

 private:
  enum class ReceiveStatus {
    kWaitingHeader = 0,
    kWaitingBody = 1,
    kWaitingChunkSize = 2,
    kWaitingChunkData = 3,
    kWaitingChunkTrailer = 4,
    kCompleted = 5,
  };

  std::string remote_hostname_;  // just used for http client
  tcp::ConnectionId connection_id_;

  std::shared_ptr<tcp::TcpConnection> tcp_connection_ { nullptr };
  std::shared_ptr<HttpPacket> http_packet_ { nullptr };
  ReceiveStatus receive_status_ { ReceiveStatus::kWaitingHeader };
  int64_t current_chunk_size_ { 0 };
  bool finished_send_ { true };

  ConnectedCallbackType connected_callback_ { nullptr };
  ClosedCallbackType closed_callback_ { nullptr };
  ReceivedCallbackType received_callback_ { nullptr };
  SentCallbackType sent_callback_ { nullptr };
};

}  // namespace http
}  // namespace cnetpp

#endif  // CNETPP_HTTP_HTTP_CONNECTION_H_

