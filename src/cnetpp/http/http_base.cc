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
#include <cnetpp/http/http_base.h>
#include <cnetpp/http/http_connection.h>
#include <cnetpp/base/string_piece.h>

namespace cnetpp {
namespace http {

void HttpBase::SetCallbacks(tcp::TcpOptions& tcp_options) {
  tcp_options.set_connected_callback(
      [this] (std::shared_ptr<tcp::TcpConnection> c) -> bool {
        return this->OnConnected(c);
      }
  );
  tcp_options.set_closed_callback(
      [this] (std::shared_ptr<tcp::TcpConnection> c) -> bool {
        return this->OnClosed(c);
      }
  );
  tcp_options.set_received_callback(
      [this] (std::shared_ptr<tcp::TcpConnection> c) -> bool {
        return this->OnReceived(c);
      }
  );
  tcp_options.set_sent_callback(
      [this] (bool sent, std::shared_ptr<tcp::TcpConnection> c) -> bool {
        return this->OnSent(sent, c);
      }
  );
}

bool HttpBase::OnConnected(std::shared_ptr<tcp::TcpConnection> tcp_connection) {
  assert(tcp_connection.get());

  auto http_connection = std::make_shared<HttpConnection>(tcp_connection);
  http_connections_mutex_.lock();
  http_connections_[tcp_connection->id()] = http_connection;
  http_connections_mutex_.unlock();
  HandleConnected(http_connection);

  return http_connection->OnConnected();
}

bool HttpBase::OnReceived(std::shared_ptr<tcp::TcpConnection> tcp_connection) {
  assert(tcp_connection.get());
  http_connections_mutex_.lock();
  auto itr = http_connections_.find(tcp_connection->id());
  assert(itr != http_connections_.end());
  auto http_connection = itr->second;
  http_connections_mutex_.unlock();
  return http_connection->OnReceived();
}

bool HttpBase::OnSent(bool success,
                      std::shared_ptr<tcp::TcpConnection> tcp_connection) {
  assert(tcp_connection.get());
  http_connections_mutex_.lock();
  auto itr = http_connections_.find(tcp_connection->id());
  assert(itr != http_connections_.end());
  auto http_connection = itr->second;
  http_connections_mutex_.unlock();
  return http_connection->OnSent(success);
}

bool HttpBase::OnClosed(std::shared_ptr<tcp::TcpConnection> tcp_connection) {
  assert(tcp_connection.get());

  http_connections_mutex_.lock();
  auto itr = http_connections_.find(tcp_connection->id());
  // the connection might haven't established.
  if (itr == http_connections_.end()) {
    http_connections_mutex_.unlock();
    auto http_options =
        std::static_pointer_cast<HttpOptions>(tcp_connection->cookie());
    if (http_options->closed_callback()) {
      return http_options->closed_callback()(std::shared_ptr<HttpConnection>());
    }
    return true;
  }
  auto http_connection = itr->second;
  http_connections_.erase(itr);
  http_connections_mutex_.unlock();
  bool ret = http_connection->OnClosed();
  return ret;
}

}  // namespace http
}  // namespace cnetpp

