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
#ifndef CNETPP_TCP_COMMAND_H_
#define CNETPP_TCP_COMMAND_H_

#include <functional>
#include <memory>

#include "connection_base.h"

namespace cnetpp {
namespace tcp {

// Caller thread should use command to add or remove a connection,
// It includes:
//    the command type,
//    the tcp connection,
class Command final {
 public:
  enum class Type {
    kDummy = 0x0,
    kAddConn = 0x1,
    kRemoveConn = 0x2,
    kReadable = 0x4,
    kWriteable = 0x8,
  };

  Command(int type,
          std::shared_ptr<ConnectionBase> connection)
      : type_(type),
        connection_(connection) {
  }

  ~Command() {
  }
  Command(Command&& c) {
    if (this != &c) {
      type_ = c.type_;
      connection_ = std::move(c.connection_);
    }
  }
  Command& operator=(Command&& c) {
    if (this != &c) {
      type_ = c.type_;
      connection_ = std::move(c.connection_);
    }
    return *this;
  }

  // Two commands will handle a same tcp connection handler if one is copied
  // from another
  Command(const Command& c) {
    if (this != &c) {
      type_ = c.type_;
      connection_ = c.connection_;
    }
  }
  Command& operator=(const Command& c) {
    if (this != &c) {
      type_ = c.type_;
      connection_ = c.connection_;
    }
    return *this;
  }

  int type() const {
    return type_;
  }

  std::shared_ptr<ConnectionBase> connection() const {
    return connection_;
  }

 private:
  int type_;
  std::shared_ptr<ConnectionBase> connection_;
};

}  // namespace tcp
}  // namespace cnetpp

#endif  // CNETPP_TCP_COMMAND_H_

