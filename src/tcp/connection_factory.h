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
#ifndef CNETPP_TCP_CONNECTION_FACTORY_H_
#define CNETPP_TCP_CONNECTION_FACTORY_H_

#include "connection_base.h"
#include "listen_connection.h"
#include "tcp_connection.h"

namespace cnetpp {
namespace tcp {

class ConnectionFactory {
 public:
  ConnectionFactory() = default;
  virtual ~ConnectionFactory() = default;

  std::shared_ptr<ConnectionBase> CreateConnection(
      std::shared_ptr<EventCenter> event_center,
      int fd,
      bool is_listener) {
    if (is_listener) {
      return std::shared_ptr<ConnectionBase>(
          new ListenConnection(event_center, fd));
    } else {
      return std::shared_ptr<ConnectionBase>(
          new TcpConnection(event_center, fd));
    }
  }
};

}  // namespace tcp
}  // namespace cnetpp

#endif  // CNETPP_TCP_TCP_CONNECTION_FACTORY_H_

