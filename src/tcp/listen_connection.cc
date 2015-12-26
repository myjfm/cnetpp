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
#include "tcp_connection.h"

#include <assert.h>

#include "connection_factory.h"
#include "command.h"
#include "event_center.h"
#include "../base/socket.h"

namespace cnetpp {
namespace tcp {

// This method will be called when a socket fd becomes readable
void ListenConnection::HandleReadableEvent(EventCenter* event_center) {
  assert(event_center);

  base::ListenSocket listen_socket;
  listen_socket.Attach(socket_.fd());

  base::TcpSocket new_socket;
  if (!listen_socket.Accept(&new_socket)) {
    listen_socket.Detach();
    return;
  }
  listen_socket.Detach();

  new_socket.SetCloexec(true);
  new_socket.SetBlocking(false);
  new_socket.SetTcpNoDelay(true);
  new_socket.SetKeepAlive(true);
  new_socket.SetLinger(false);

  ConnectionFactory cf;
  // TODO(myjfm)
  // new_socket.SetSendBufferSize(xxx);
  // new_socket.SetReceiveBufferSize(xxx);
  auto new_connection =
      cf.CreateConnection(event_center_.lock(), new_socket.fd(), false);
  auto new_tcp_connection =
      std::dynamic_pointer_cast<TcpConnection>(new_connection);
  new_tcp_connection->set_closed_callback(options_.closed_callback());
  new_tcp_connection->set_sent_callback(options_.sent_callback());
  new_tcp_connection->set_received_callback(options_.received_callback());
  new_tcp_connection->set_connected(true);
  new_tcp_connection->SetSendBufferSize(options_.send_buffer_size());
  new_tcp_connection->SetRecvBufferSize(options_.receive_buffer_size());

  new_socket.Detach();

  if (connected_callback_) {
    // call callback user defined
    connected_callback_(new_tcp_connection);
  }

  Command command(static_cast<int>(Command::Type::kAddConn) |
      static_cast<int>(Command::Type::kWriteable) |
      static_cast<int>(Command::Type::kReadable),
      new_connection);
  event_center->AddCommand(command);
  return;
}

void ListenConnection::HandleWriteableEvent(EventCenter* event_center) {
  assert(event_center);
  Command command(static_cast<int>(Command::Type::kReadable),
                  shared_from_this());
  event_center->AddCommand(command);
}

}  // namespace tcp
}  // namespace cnetpp

