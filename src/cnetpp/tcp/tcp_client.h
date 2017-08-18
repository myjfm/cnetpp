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
#ifndef ASYNC_CNETPP_TCP_TCP_CLIENT_H_
#define ASYNC_CNETPP_TCP_TCP_CLIENT_H_

#include <cnetpp/tcp/connection_id.h>
#include <cnetpp/tcp/event_center.h>
#include <cnetpp/tcp/tcp_callbacks.h>
#include <cnetpp/tcp/tcp_options.h>
#include <cnetpp/base/end_point.h>
#include <cnetpp/base/uri.h>

#include <mutex>
#include <unordered_map>

namespace cnetpp {
namespace tcp {

class TcpClient final {
 public:
  TcpClient() = default;
  ~TcpClient() = default;

  // disallow copy and move operations
  TcpClient(const TcpClient&) = delete;
  TcpClient& operator=(const TcpClient&) = delete;
  TcpClient(TcpClient&&) = delete;
  TcpClient& operator=(TcpClient&&) = delete;

  bool Launch(size_t worker_count = 1);
  bool Shutdown();

  // connect with remote server
  // if keep local as nullptr, it will use one of local ip.
  ConnectionId Connect(const base::EndPoint* remote,
                       const TcpClientOptions& options = TcpClientOptions(),
                       std::shared_ptr<void> cookie = nullptr);

  bool AsyncClosed(ConnectionId connection_id);

 private:
  std::shared_ptr<EventCenter> event_center_;

  enum class Status {
    kInitialized, 
    kConnecting, 
    kConnected, 
    kClosed
  };

  struct InternalConnectionContext {
    Status status;
    TcpClientOptions options;
    std::shared_ptr<TcpConnection> tcp_connection;
  };

  std::unordered_map<ConnectionId, InternalConnectionContext> contexts_;
  std::mutex contexts_mutex_;

  bool OnConnected(std::shared_ptr<TcpConnection> tcp_connection);

  bool OnClosed(std::shared_ptr<TcpConnection> tcp_connection);

  bool OnSent(bool success, std::shared_ptr<TcpConnection> tcp_connection);

  bool OnReceived(std::shared_ptr<TcpConnection> tcp_connection);
};

}  // namespace tcp
}  // namespace cnetpp

#endif  // ASYNC_CNETPP_TCP_TCP_CLIENT_H_

