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
#include <cnetpp/tcp/pipe_interrupter_impl.h>

#include <fcntl.h>
#include <unistd.h>

namespace cnetpp {
namespace tcp {

PipeInterrupterImpl::~PipeInterrupterImpl() {
  if (read_fd_ != -1) {
    ::close(read_fd_);
  }
  if (write_fd_ != -1) {
    ::close(write_fd_);
  }
}

bool PipeInterrupterImpl::Create() {
  int pipe_fds[2] { -1, -1 };
  if (::pipe(pipe_fds) < 0) {
    return false;
  }

  read_fd_ = pipe_fds[0];
  ::fcntl(read_fd_, F_SETFL, O_NONBLOCK);
  ::fcntl(read_fd_, F_SETFD, FD_CLOEXEC);

  write_fd_ = pipe_fds[1];
  ::fcntl(write_fd_, F_SETFL, O_NONBLOCK);
  ::fcntl(write_fd_, F_SETFD, FD_CLOEXEC);
  return true;
}

// interrupt the epoll_wait call.
bool PipeInterrupterImpl::Interrupt() {
  char byte = 0;
  if (write_fd_ < 0 || ::write(write_fd_, &byte, 1) < 0) {
    return false;
  }
  return true;
}

// Reset the epoll interrupt.
// Returns true if the epoll_wait call was interrupted.
bool PipeInterrupterImpl::Reset() {
  char data[64];
  int bytes_read = ::read(read_fd_, data, sizeof(data));
  bool was_interrupted = (bytes_read > 0);
  while (bytes_read == sizeof(data)) {
    bytes_read = ::read(read_fd_, data, sizeof(data));
  }
  return was_interrupted;
}

}  // namespace tcp
}  // namespace cnetpp

