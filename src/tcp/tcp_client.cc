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
#include "tcp_client.h"

#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "connection_factory.h"
#include "../base/end_point.h"
#include "../base/socket.h"

namespace cnetpp {
namespace tcp {

bool TcpClient::Launch(size_t worker_count) {
  event_center_ = EventCenter::New(worker_count);
  assert(event_center_.get());
  return event_center_->Launch();
}

bool TcpClient::Shutdown() {
  event_center_->Shutdown();
  contexts_.clear();
  return true;
}

ConnectionId TcpClient::Connect(const base::EndPoint* remote,
                                const TcpClientOptions& options,
                                std::shared_ptr<void> cookie) {
  assert(remote);

  base::TcpSocket socket;
  if (!socket.Create() ||
      !socket.SetCloexec() ||
      !socket.SetBlocking(false) ||
      !socket.SetTcpNoDelay() ||
      !socket.SetKeepAlive() ||
      !socket.Connect(*remote)) {
    return -1;
  }

  InternalConnectionContext cc;
  cc.status = Status::kConnecting;
  cc.options = options;
  cc.tcp_connection.reset();

  ConnectionFactory cf;
  auto connection = cf.CreateConnection(event_center_, socket.fd(), false);
  auto tcp_connection = std::static_pointer_cast<TcpConnection>(connection);
  tcp_connection->SetSendBufferSize(options.send_buffer_size());
  tcp_connection->SetRecvBufferSize(options.receive_buffer_size());
  tcp_connection->set_cookie(cookie);
  std::unique_lock<std::mutex> guard(contexts_mutex_);
  contexts_[connection->id()] = cc;
  guard.unlock();

  tcp_connection->set_connected_callback(
      [this] (std::shared_ptr<TcpConnection> c) -> bool {
        return this->OnConnected(c);
      }
  );
  tcp_connection->set_closed_callback(
      [this] (std::shared_ptr<TcpConnection> c) -> bool {
        return this->OnClosed(c);
      }
  );
  tcp_connection->set_sent_callback(
      [this] (bool status, std::shared_ptr<TcpConnection> c) -> bool {
        return this->OnSent(status, c);
      }
  );
  tcp_connection->set_received_callback(
      [this] (std::shared_ptr<TcpConnection> c) -> bool {
        return this->OnReceived(c);
      }
  );

  socket.Detach();

  Command cmd(static_cast<int>(Command::Type::kAddConn), connection);
  event_center_->AddCommand(cmd);
  return connection->id();
}

ConnectionId TcpClient::Connect(const char *url,
    const TcpClientOptions& options,
    std::shared_ptr<void> cookie) {
  base::Uri uri;
  if (!uri.Parse(url))
    return -1;
  return Connect(uri, options, cookie);
}

ConnectionId TcpClient::Connect(const base::Uri& url,
    const TcpClientOptions& options,
    std::shared_ptr<void> cookie) {
  base::EndPoint endpoint;
  struct addrinfo *presults = nullptr;
  struct addrinfo hint;
  bzero(&hint, sizeof(hint));
  hint.ai_family = AF_INET;
  hint.ai_socktype = SOCK_STREAM;
  char port_str[32];
  sprintf(port_str, "%d", url.Port());
  int rc = getaddrinfo(url.Hostname().c_str(),
      port_str,
      &hint,
      &presults);
  if (rc != 0) {
    return -1;
  }
  ConnectionId connection_id = -1;
  struct addrinfo *paddrinfo = nullptr;
  for (paddrinfo = presults; paddrinfo != NULL ; paddrinfo = presults->ai_next) {
    char ipstr[32];
    inet_ntop(AF_INET,                                                                                               
        &(((struct sockaddr_in *)(paddrinfo->ai_addr))->sin_addr),
        ipstr, 16);
    if (!endpoint.FromSockAddr(*paddrinfo->ai_addr, paddrinfo->ai_addrlen)) {
      continue;
    }
    connection_id = Connect(&endpoint, options, cookie);
    if (connection_id != -1) {
      break;
    }
  }
  return connection_id;
}


bool TcpClient::AsyncClosed(ConnectionId connection_id) {
  std::unique_lock<std::mutex> guard(contexts_mutex_);
  auto itr = contexts_.find(connection_id);
  if (itr == contexts_.end()) {
    return false;
  }
  auto connection = itr->second.tcp_connection;
  guard.unlock();
  connection->MarkAsClosed();
  return true;
}

bool TcpClient::OnConnected(
    std::shared_ptr<TcpConnection> tcp_connection) {
  assert(tcp_connection.get());
  std::unique_lock<std::mutex> guard(contexts_mutex_);
  auto itr = contexts_.find(tcp_connection->id());
  assert(itr != contexts_.end());
  itr->second.status = Status::kConnected;
  if (itr->second.options.connected_callback()) {
    auto& cb = itr->second.options.mutable_connected_callback();
    guard.unlock();
    return cb(tcp_connection);
  }
  return true;
}

bool TcpClient::OnClosed(
    std::shared_ptr<TcpConnection> tcp_connection) {
  assert(tcp_connection.get());
  std::unique_lock<std::mutex> guard(contexts_mutex_);
  auto itr = contexts_.find(tcp_connection->id());
  assert(itr != contexts_.end());
  itr->second.status = Status::kClosed;
  bool res = true;
  if (itr->second.options.closed_callback()) {
    auto& cb = itr->second.options.mutable_closed_callback();
    guard.unlock();
    res = cb(tcp_connection);
    guard.lock();
    contexts_.erase(itr);
    return res;
  }
  contexts_.erase(itr);
  return res;
}

bool TcpClient::OnSent(bool success,
                       std::shared_ptr<TcpConnection> tcp_connection) {
  assert(tcp_connection.get());
  std::unique_lock<std::mutex> guard(contexts_mutex_);
  auto itr = contexts_.find(tcp_connection->id());
  assert(itr != contexts_.end());
  assert(itr->second.status == Status::kConnected);
  if (itr->second.options.sent_callback()) {
    auto& cb = itr->second.options.mutable_sent_callback();
    guard.unlock();
    return cb(success, tcp_connection);
  }
  return true;
}

bool TcpClient::OnReceived(std::shared_ptr<TcpConnection> tcp_connection) {
  assert(tcp_connection.get());
  std::unique_lock<std::mutex> guard(contexts_mutex_);
  auto itr = contexts_.find(tcp_connection->id());
  assert(itr != contexts_.end());
  assert(itr->second.status == Status::kConnected);
  if (itr->second.options.received_callback()) {
    auto& cb = itr->second.options.mutable_received_callback();
    guard.unlock();
    return cb(tcp_connection);
  }
  return true;
}

}  // namespace tcp
}  // namespace cnetpp

