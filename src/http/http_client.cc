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
#include <netdb.h>

namespace cnetpp {
namespace http {

tcp::ConnectionId HttpClient::Connect(const base::EndPoint* remote,
                                      const HttpOptions& http_options) {
  tcp::TcpClientOptions options;
  options.set_send_buffer_size(http_options.send_buffer_size());
  options.set_receive_buffer_size(http_options.receive_buffer_size());
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
  std::shared_ptr<void> new_http_options =
      std::shared_ptr<HttpOptions>(new HttpOptions(http_options));
  return tcp_client_.Connect(remote, options, new_http_options);
}

tcp::ConnectionId HttpClient::Connect(base::StringPiece url_str,
    const HttpOptions& http_options) {
  // parse url
  base::Uri url;
  if (!url.Parse(url_str.as_string())) {
    return tcp::kInvalidConnectionId;
  }
  
  // resolve address of url
  struct addrinfo *presults = nullptr;
  struct addrinfo hint;
  bzero(&hint, sizeof(hint));
  hint.ai_family = AF_INET;
  hint.ai_socktype = SOCK_STREAM;
  std::string port_str = std::to_string(url.Port());
  int rc = getaddrinfo(url.Hostname().c_str(), port_str.c_str(), &hint, &presults);
  if (rc != 0 || !presults) {
    return tcp::kInvalidConnectionId;
  }
  base::EndPoint endpoint;
  if (!endpoint.FromSockAddr(*presults->ai_addr, presults->ai_addrlen)) {
    return tcp::kInvalidConnectionId;
  }
  freeaddrinfo(presults);

  // connect to server
  return Connect(&endpoint, http_options);
}

bool HttpClient::AsyncClose(tcp::ConnectionId connection_id) {
  return tcp_client_.AsyncClosed(connection_id);
}

bool HttpClient::HandleConnected(
    std::shared_ptr<HttpConnection> http_connection) {
  auto http_options = std::static_pointer_cast<HttpOptions>(
      http_connection->tcp_connection()->cookie());
  http_connection->set_connected_callback(http_options->connected_callback());
  http_connection->set_closed_callback(http_options->closed_callback());
  http_connection->set_received_callback(http_options->received_callback());
  http_connection->set_sent_callback(http_options->sent_callback());
  http_connection->set_http_packet(
      std::shared_ptr<HttpPacket>(new HttpResponse));
  return true;
}

}  // namespace http
}  // namespace cnetpp

