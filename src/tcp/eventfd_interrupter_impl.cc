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
#include "eventfd_interrupter_impl.h"

#include <sys/eventfd.h>
#include <unistd.h>

namespace cnetpp {
namespace tcp {

EventfdInterrupterImpl::~EventfdInterrupterImpl() {
  if (fd_ != -1) {
    ::close(fd_);
  }
}

bool EventfdInterrupterImpl::Create() {
  fd_ = ::eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
  return fd_ >= 0;
}

// interrupt the epoll_wait call.
bool EventfdInterrupterImpl::Interrupt() {
  return ::eventfd_write(fd_, 1) == 0;
}

// Reset the epoll interrupt.
// Returns true if the epoll_wait call was interrupted.
bool EventfdInterrupterImpl::Reset() {
  eventfd_t data;
  return ::eventfd_read(fd_, &data) == 0;
}

}  // namespace tcp
}  // namespace cnetpp

