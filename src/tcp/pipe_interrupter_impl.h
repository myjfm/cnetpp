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

#ifndef CNETPP_TCP_PIPE_INTERRUPTER_IMPL_H_
#define CNETPP_TCP_PIPE_INTERRUPTER_IMPL_H_

#include <tcp/interrupter.h>

namespace cnetpp {
namespace tcp {

class PipeInterrupterImpl : public Interrupter {
 public:
  PipeInterrupterImpl() : read_fd_(-1), write_fd_(-1) {
  }

  ~PipeInterrupterImpl();

  bool Create();

  // interrupt the epoll_wait call.
  bool Interrupt();

  // Reset the epoll interrupt.
  // Returns true if the epoll_wait call was interrupted.
  bool Reset();

  // Get the read descriptor to be passed to epoll_wait.
  int get_read_fd() const {
    return read_fd_;
  }

 private:
  // The read end of a connection used to interrupt the epoll_wait call.
  // This file descriptor is passed to epoll such that when it is time to stop,
  // a single byte will be written on the other end of the connection and this
  // descriptor will become readable.
  int read_fd_;

  // The write end of a connection used to interrupt the epoll_wait call.
  // A single byte may be written to this to wake up the epoll_wati which is
  // waiting for the other end to become readable.
  int write_fd_;
};

}  // namespace tcp
}  // namespace cnetpp

#endif // CNETPP_TCP_PIPE_INTERRUPTER_IMPL_H_

