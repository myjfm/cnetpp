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
#ifndef __linux__
#error "Your operating system seems not be linux!"
#endif

#include "select_event_poller_impl.h"
#include <sys/select.h>

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include "event.h"
#include "event_center.h"
#include "../concurrency/this_thread.h"

namespace cnetpp {
namespace tcp {

bool SelectEventPollerImpl::Init(std::shared_ptr<EventCenter> event_center) {
  printf("create SelectEventPoller!\n");
  if (!event_center) {
    return false;
  }

  event_center_ = event_center;

  if (!CreateInterrupter()) {
    return false;
  }
  return true;
}

bool SelectEventPollerImpl::Poll() {
  // before starting polling, we first process all the pending command events
  if (!ProcessInterrupt()) {
    return false;
  }

  int count { 0 };
  fd_set rd_fds, wr_fds, ex_fds;
  int max_fd;
  int nfds = select_fds_.size();
  max_fd = BuildFdset(&rd_fds, &wr_fds, &ex_fds);
  if(max_fd < 0)
    return true;
 
  do {
    count = ::select(max_fd + 1, &rd_fds, &wr_fds, &ex_fds, NULL);
  } while (count == -1 &&
      cnetpp::concurrency::ThisThread::GetLastError() == EINTR);

  if (count < 0) {
    return false;
  }
  if(FD_ISSET(pipe_read_fd_, &rd_fds)) {
    if(!ProcessInterrupt()) {
      return false;
    }
  }
  for (auto i = 0; i < count && i < nfds; ++i) {
    bool has_event = false;
    Event event(select_fds_[i]);
    if(FD_ISSET(select_fds_[i], &ex_fds)) {
      has_event = true;
      event.mutable_mask() |= static_cast<int>(Event::Type::kClose);
    }
    if(FD_ISSET(select_fds_[i], &rd_fds)) {
      has_event = true;
      event.mutable_mask() |= static_cast<int>(Event::Type::kRead);
    }
    if(FD_ISSET(select_fds_[i], &wr_fds)) {
      has_event = true;
      event.mutable_mask() |= static_cast<int>(Event::Type::kWrite);
    }
    if(has_event) {
      std::shared_ptr<EventCenter> event_center = event_center_.lock();
      if (!event_center || !event_center->ProcessEvent(event, id_)) {
        return false;
      }
    }
  }
  return true;
}

bool SelectEventPollerImpl::Interrupt() {
  char byte = 0;
  if (pipe_write_fd_ < 0 || ::write(pipe_write_fd_, &byte, 1) < 0) {
    return false;
  }
  return true;
}

void SelectEventPollerImpl::Shutdown() {
  DestroyInterrupter();
}

bool SelectEventPollerImpl::ProcessCommand(const Command& command) {
  int type = static_cast<int>(Event::Type::kRead);
  if (static_cast<int>(command.type()) &
      static_cast<int>(Command::Type::kWriteable)) {
    type |= static_cast<int>(Event::Type::kWrite);
  }
  if (command.type() & static_cast<int>(Command::Type::kAddConn)) {
    return AddSelectEvent(Event(command.connection()->socket().fd(), type));
  } else if (command.type() & static_cast<int>(Command::Type::kRemoveConn)) {
    type |= static_cast<int>(Event::Type::kClose);
    return RemoveSelectEvent(Event(command.connection()->socket().fd(), type));
  } else if (command.type() & static_cast<int>(Command::Type::kReadable) ||
      command.type() & static_cast<int>(Command::Type::kWriteable)) {
    return ModifySelectEvent(Event(command.connection()->socket().fd(), type));
  } else {
    return false;
  }
  return false;
}

bool SelectEventPollerImpl::CreateInterrupter() {
  int pipe_fd_pair[2] { -1, -1 };
  if (::pipe(pipe_fd_pair) < 0) {
    return false;
  }

  pipe_read_fd_ = pipe_fd_pair[0];
  pipe_write_fd_ = pipe_fd_pair[1];

  ::fcntl(pipe_read_fd_, F_SETFL, O_NONBLOCK);
  ::fcntl(pipe_write_fd_, F_SETFL, O_NONBLOCK);

  ::fcntl(pipe_read_fd_, F_SETFD, FD_CLOEXEC);
  ::fcntl(pipe_write_fd_, F_SETFD, FD_CLOEXEC);

  return true;
}

void SelectEventPollerImpl::DestroyInterrupter() {
  ::close(pipe_read_fd_);
  ::close(pipe_write_fd_);
}

bool SelectEventPollerImpl::ProcessInterrupt() {
  // TODO(myjfm)
  // process error
  char buf[64];
  while (::read(pipe_read_fd_, buf, 64) == 64) { }

  std::shared_ptr<EventCenter> event_center = event_center_.lock();
  if (event_center) {
    return event_center->ProcessAllPendingCommands(id_);
  }
  return false;
}

bool SelectEventPollerImpl::AddSelectEvent(const Event& ev) {
  select_fds_.push_back(ev.fd());
  return true;
}

bool SelectEventPollerImpl::ModifySelectEvent(const Event& ev) {
  return true;
}

bool SelectEventPollerImpl::RemoveSelectEvent(const Event& ev) {
  auto i = lower_bound(select_fds_.begin(), select_fds_.end(), ev.fd());
  if(*i == ev.fd())
    select_fds_.erase(i);
  return true;
}

int SelectEventPollerImpl::BuildFdset(fd_set* rd_fdset, fd_set* wr_fdset, fd_set* ex_fdset) {
  int max_fd = -1;
  assert(rd_fdset && wr_fdset && ex_fdset);
  FD_ZERO(rd_fdset);
  FD_ZERO(wr_fdset);
  FD_ZERO(ex_fdset);
  FD_SET(pipe_read_fd_, rd_fdset);
  FD_SET(pipe_read_fd_, ex_fdset);
  max_fd = max_fd < pipe_read_fd_ ? pipe_read_fd_ : max_fd;
  for(auto fd : select_fds_) {
    FD_SET(fd, rd_fdset);
    FD_SET(fd, wr_fdset);
    FD_SET(fd, ex_fdset);
    max_fd = max_fd < fd ? fd : max_fd;
  }
  return max_fd;
}
}  // namespace tcp
}  // namespace cnetpp

