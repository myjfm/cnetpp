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
#include <cnetpp/tcp/tcp_server.h>
#include <cnetpp/tcp/connection_factory.h>
#include <cnetpp/tcp/listen_connection.h>
#include <cnetpp/base/socket.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <cnetpp/base/log.h>

namespace cnetpp {
namespace tcp {

bool TcpServer::Launch(const base::EndPoint& local_address,
                       const TcpServerOptions& options) {
  event_center_ = EventCenter::New(options.name(), options.worker_count());
  assert(event_center_.get());
  if (!event_center_->Launch()) {
    return false;
  }

  // create listen socket
  base::ListenSocket listen_socket(local_address);
  if (!listen_socket.IsValid()) {
    CnetppInfo("[TcpServer 0X%08x] create listen socket failed in addr %s",
               this, local_address.ToString().c_str());
    return false;
  }

  CnetppDebug("[TcpServer 0X%08x] create listen socket [0X%08x] in addr %s",
              this, listen_socket.fd(), local_address.ToString().c_str());

  int one = 0; (void) one;
  if (!listen_socket.SetCloexec(true) ||
      !listen_socket.SetBlocking(false) ||
#if 0
      !listen_socket.SetReceiveBufferSize(options.tcp_receive_buffer_size()) ||
      !listen_socket.SetSendBufferSize(options.tcp_send_buffer_size()) ||
#endif
      !listen_socket.SetReuseAddress(true) ||
#ifdef SO_NOSIGPIPE
      !listen_socket.SetOption(SOL_SOCKET, SO_NOSIGPIPE, &one, sizeof(one)) ||
#endif
      !listen_socket.Listen(options.backlog())) {
    return false;
  }

  ConnectionFactory cf;
  auto connection =
      cf.CreateConnection(event_center_, listen_socket.fd(), true);
  assert(connection.get());
  connection->set_connected_callback(options.connected_callback());
  auto listener = std::static_pointer_cast<ListenConnection>(connection);
  listener->set_tcp_server_options(options);

  CnetppDebug("[TcpServer 0X%08x] bind listen socket [0X%08x] "
              "with [ListenConnection 0X%08x]",
              this, listen_socket.fd(), connection->id());
  // add the listen fd onto multiplexer
  Command cmd(static_cast<int>(Command::Type::kAddConnectedConn),
              std::static_pointer_cast<ConnectionBase>(listener));
  event_center_->AddCommand(cmd, true);

  listen_socket.Detach();
  return true;
}

bool TcpServer::Shutdown() {
  event_center_->Shutdown();
  return true;
}

}  // namespace tcp
}  // namespace cnetpp

