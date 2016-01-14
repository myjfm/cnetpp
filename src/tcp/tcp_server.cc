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
#include "tcp_server.h"

#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>

#include "connection_factory.h"
#include "listen_connection.h"
#include "../base/socket.h"

namespace cnetpp {
namespace tcp {

bool TcpServer::Launch(const base::EndPoint& local_address,
                       size_t worker_count,
                       const TcpServerOptions& options) {
  event_center_ = EventCenter::New(worker_count);
  assert(event_center_.get());
  if (!event_center_->Launch()) {
    return false;
  }

  // create listen socket
  base::ListenSocket listen_socket(local_address);
  if (!listen_socket.IsValid()) {
    return false;
  }

  if (!listen_socket.SetCloexec(true) ||
      !listen_socket.SetBlocking(false) ||
      !listen_socket.SetReceiveBufferSize(options.receive_buffer_size()) ||
      !listen_socket.SetSendBufferSize(options.send_buffer_size()) ||
      !listen_socket.SetReuseAddress(true) ||
      !listen_socket.Listen()) {
    return false;
  }

  ConnectionFactory cf;
  auto connection =
      cf.CreateConnection(event_center_, listen_socket.fd(), true);
  assert(connection.get());
  connection->set_connected_callback(options.connected_callback());
  auto listener = std::static_pointer_cast<ListenConnection>(connection);
  listener->set_tcp_server_options(options);

  // add the listen fd onto multiplexer
  Command cmd(static_cast<int>(Command::Type::kAddConn),
              std::static_pointer_cast<ConnectionBase>(listener));
  event_center_->AddCommand(cmd);

  listen_socket.Detach();
  return true;
}

bool TcpServer::Shutdown() {
  event_center_->Shutdown();
  return true;
}

}  // namespace tcp
}  // namespace cnetpp

