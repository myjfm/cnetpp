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
#include "http_server.h"
#include "http_request.h"
#include "../tcp/tcp_options.h"

namespace cnetpp {
namespace http {

bool HttpServer::Launch(const base::EndPoint& local_address,
                        const HttpServerOptions& http_options,
                        size_t worker_count) {
  options_ = http_options;

  tcp::TcpServerOptions tcp_options;
  SetCallbacks(tcp_options);
  return tcp_server_.Launch(local_address, worker_count, tcp_options);
}

bool HttpServer::HandleConnected(
    std::shared_ptr<HttpConnection> http_connection) {
  http_connection->set_connected_callback(options_.connected_callback());
  http_connection->set_closed_callback(options_.closed_callback());
  http_connection->set_received_callback(options_.received_callback());
  http_connection->set_sent_callback(options_.sent_callback());
  http_connection->set_http_packet(std::shared_ptr<HttpPacket>(new HttpRequest));
  return true;
}

}  // namespace http
}  // namespace cnetpp

