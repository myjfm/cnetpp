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
#ifndef CNETPP_HTTP_HTTP_BASE_H_
#define CNETPP_HTTP_HTTP_BASE_H_

#include <memory>
#include <mutex>
#include <unordered_map>

#include "http_connection.h"
#include "http_options.h"
#include "../tcp/tcp_connection.h"
#include "../tcp/tcp_options.h"

namespace cnetpp {
namespace http {

class HttpBase {
 public:
  HttpBase() = default;
  virtual ~HttpBase() {
  }

  HttpBase(const HttpBase&) = delete;
  HttpBase& operator=(const HttpBase&) = delete;

  bool Shutdown() {
    if (!DoShutdown()) {
      return false;
    }
    http_connections_.clear();
    return true;
  }

 protected:
  std::unordered_map<tcp::ConnectionId,
                     std::shared_ptr<HttpConnection> > http_connections_;
  std::mutex http_connections_mutex_;

  void SetCallbacks(tcp::TcpOptions& tcp_options);

  virtual bool DoShutdown() = 0;

  virtual bool HandleConnected(
      std::shared_ptr<HttpConnection> http_connection) = 0;

  virtual bool OnConnected(
      std::shared_ptr<tcp::TcpConnection> tcp_connection);

  virtual bool OnReceived(std::shared_ptr<tcp::TcpConnection> tcp_connection);

  virtual bool OnSent(bool success,
                      std::shared_ptr<tcp::TcpConnection> tcp_connection);

  virtual bool OnClosed(std::shared_ptr<tcp::TcpConnection> tcp_connection);
};

}  // namespace http
}  // namespace cnetpp

#endif  // CNETPP_HTTP_HTTP_BASE_H_

