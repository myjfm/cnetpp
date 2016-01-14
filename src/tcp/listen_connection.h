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
#ifndef CNETPP_LISTEN_CONNECTION_H_
#define CNETPP_LISTEN_CONNECTION_H_

#include "connection_base.h"

#include <memory>

#include "tcp_options.h"

namespace cnetpp {
namespace tcp {

// forward declaration
class EventCenter;

class ListenConnection : public ConnectionBase {
 public:
  friend class ConnectionFactory;
  virtual ~ListenConnection() = default;

  const TcpServerOptions& tcp_server_options() const {
    return options_;
  }
  TcpServerOptions& mutable_tcp_server_options() {
    return options_;
  }
  void set_tcp_server_options(const TcpServerOptions& options) {
    options_ = options;
  }

  virtual void HandleReadableEvent(EventCenter* event_center) override;
  virtual void HandleWriteableEvent(EventCenter* event_center) override;
  virtual void HandleCloseConnection() override {
  }

 private:
  ListenConnection(std::shared_ptr<EventCenter> event_center, int fd)
      : ConnectionBase(event_center, fd) {
  }

  TcpServerOptions options_;
};

}  // namespace tcp
}  // namespace cnetpp

#endif  // CNETPP_TCP_LISTEN_CONNECTION_H_

