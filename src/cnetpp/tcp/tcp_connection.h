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
#ifndef CNETPP_TCP_CONNECTION_H_
#define CNETPP_TCP_CONNECTION_H_

#include <cnetpp/tcp/connection_base.h>
#include <cnetpp/tcp/ring_buffer.h>
#include <cnetpp/tcp/tcp_callbacks.h>
#include <cnetpp/base/string_piece.h>
#include <cnetpp/concurrency/spin_lock.h>

#include <atomic>
#include <memory>
#include <string>
#include <list>

namespace cnetpp {
namespace tcp {

// forward declaration
class EventCenter;

class TcpConnection : public ConnectionBase {
 public:
  friend class ConnectionFactory;

  virtual ~TcpConnection() = default;

  void SetSendBufferSize(size_t send_buffer_size) {
    send_buffer_size_ = send_buffer_size;
  }

  void SetRecvBufferSize(size_t recv_buffer_size) {
    receive_buffer_size_ = recv_buffer_size;
  }

  const RingBuffer& recv_buffer() const {
    return recv_buffer_;
  }
  RingBuffer& mutable_recv_buffer() {
    return recv_buffer_;
  }

  int status() const {
    return status_;
  }

  const std::string& error_message() const {
    return error_message_;
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

  const SentCallbackType& sent_callback() const {
    return sent_callback_;
  }
  SentCallbackType mutable_sent_callback() {
    return sent_callback_;
  }
  void set_sent_callback(const SentCallbackType& sent_callback) {
    sent_callback_ = sent_callback;
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

  std::shared_ptr<void> cookie() {
    return cookie_;
  }
  void set_cookie(std::shared_ptr<void> cookie) {
    cookie_ = cookie;
  }

  const base::EndPoint& remote_end_point() const {
    return remote_end_point_;
  }
  base::EndPoint& mutable_remote_end_point() {
    return remote_end_point_;
  }
  void set_remote_end_point(const base::EndPoint& remote_end_point) {
    remote_end_point_ = remote_end_point;
  }
  void set_remote_end_point(base::EndPoint&& remote_end_point) {
    remote_end_point_ = std::move(remote_end_point);
  }

  bool SendPacket(base::StringPiece data);
  bool SendPacket(std::unique_ptr<RingBuffer>&& data);

  // These three methods will be called by the event poller thread when a
  // socket fd becomes readable or writable
  // NOTE: user should not care about them
  void HandleReadableEvent(EventCenter* event_center) override;
  void HandleWriteableEvent(EventCenter* event_center) override;
  void HandleCloseConnection() override;

  void MarkAsClosed(bool immediately = true) override;

 private:
  TcpConnection(std::shared_ptr<EventCenter> event_center, int fd)
      : ConnectionBase(event_center, fd),
        recv_buffer_(0) {
  }

  bool SendPacket();

  base::EndPoint remote_end_point_;

  int status_ { 0 }; // equal to errno
  std::string error_message_;

  concurrency::SpinLock send_lock_;
  std::list<std::unique_ptr<RingBuffer>> send_buffers_;

  RingBuffer recv_buffer_;

  size_t receive_buffer_size_;
  size_t send_buffer_size_;

  ClosedCallbackType closed_callback_ { nullptr };
  SentCallbackType sent_callback_ { nullptr };
  ReceivedCallbackType received_callback_ { nullptr };
  std::shared_ptr<void> cookie_ { nullptr };
};

}  // namespace tcp
}  // namespace cnetpp

#endif  // CNETPP_TCP_TCP_CONNECTION_H_

