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
#include "http_client.h"
#include "http_connection.h"
#include "http_response.h"

namespace cnetpp {
namespace http {

tcp::ConnectionId HttpClient::Connect(const base::EndPoint* remote,
                                      const HttpOptions& http_options) {
  tcp::TcpClientOptions options;
  options.set_connected_callback(
      [this] (std::shared_ptr<tcp::TcpConnection> c) -> bool {
        return this->OnConnected(c);
      }
  );
  options.set_closed_callback(
      [this] (std::shared_ptr<tcp::TcpConnection> c) -> bool {
        return this->OnClosed(c);
      }
  );
  options.set_received_callback(
      [this] (std::shared_ptr<tcp::TcpConnection> c) -> bool {
        return this->OnReceived(c);
      }
  );
  options.set_sent_callback(
      [this] (bool status, std::shared_ptr<tcp::TcpConnection> c) -> bool {
        return this->OnSent(status, c);
      }
  );
  auto connection_id = tcp_client_.Connect(remote, options);
  if (connection_id == tcp::kInvalidConnectionId) {
    return connection_id;
  }
  http_options_[connection_id] = http_options;
  return connection_id;
}

bool HttpClient::AsyncClose(tcp::ConnectionId connection_id) {
  return tcp_client_.AsyncClosed(connection_id);
}

bool HttpClient::HandleConnected(
    std::shared_ptr<HttpConnection> http_connection) {
  auto connection_id = http_connection->id();
  auto itr = http_options_.find(connection_id);
  assert(itr != http_options_.end());
  http_connection->set_connected_callback(itr->second.connected_callback());
  http_connection->set_closed_callback(itr->second.closed_callback());
  http_connection->set_received_callback(itr->second.received_callback());
  http_connection->set_sent_callback(itr->second.sent_callback());
  http_connection->set_http_packet(std::shared_ptr<HttpPacket>(new HttpResponse));
  return true;
}

}  // namespace http
}  // namespace cnetpp

