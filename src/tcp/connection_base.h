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
#ifndef CNETPP_CONNECTION_BASE_H_
#define CNETPP_CONNECTION_BASE_H_

#include <functional>
#include <memory>
#include <string>

#include "connection_id.h"
#include "ring_buffer.h"
#include "tcp_callbacks.h"
#include "../base/socket.h"
#include "../base/string_piece.h"

namespace cnetpp {
namespace tcp {

// forward declaration
class EventCenter;

class ConnectionBase : public std::enable_shared_from_this<ConnectionBase> {
 public:
  virtual ~ConnectionBase() = default;

  const base::TcpSocket& socket() const {
    return socket_;
  }
  base::TcpSocket& mutable_socket()  {
    return socket_;
  }

  ConnectionId id() const {
    return id_;
  }

  const ConnectedCallbackType& connected_callback() const {
    return connected_callback_;
  }
  ConnectedCallbackType& mutable_connected_callback() {
    return connected_callback_;
  }
  void set_connected_callback(const ConnectedCallbackType& connected_callback) {
    connected_callback_ = std::move(connected_callback);
  }

  int cached_event_type() const {
    return cached_event_type_;
  }
  void set_cached_event_type(int event_type) {
    cached_event_type_ = event_type;
  }

  // These three methods will be called by the event poller thread when a
  // socket fd becomes readable or writable
  // NOTE: user should not care about them
  virtual void HandleReadableEvent(EventCenter* event_center) = 0;
  virtual void HandleWriteableEvent(EventCenter* event_center) = 0;
  virtual void HandleCloseConnection() = 0;
  virtual void MarkAsClosed() = 0;

 protected:
  ConnectionBase(std::shared_ptr<EventCenter> event_center, int fd)
      : event_center_(event_center),
        id_(ConnectionIdGenerator::Generate()) {
    socket_.Attach(fd);
  }

  std::weak_ptr<EventCenter> event_center_;

  ConnectionId id_;
  base::TcpSocket socket_;

  ConnectedCallbackType connected_callback_ { nullptr };

  int cached_event_type_ { 0 };
};

}  // namespace tcp
}  // namespace cnetpp

#endif  // CNETPP_TCP_CONNECTION_BASE_H_

