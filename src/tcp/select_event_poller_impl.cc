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
#include <tcp/interrupter.h>
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
  assert(interrupter_);
  if (!ProcessInterrupt()) {
    return false;
  }

  int count { 0 };
  fd_set rd_fds, wr_fds, ex_fds;
  int max_fd;
  int nfds = select_fds_.size();
  max_fd = BuildFdsets(&rd_fds, &wr_fds, &ex_fds);
  if(max_fd < 0) {
    return true;
  }
 
  do {
    count = ::select(max_fd + 1, &rd_fds, &wr_fds, &ex_fds, NULL);
  } while (count == -1 &&
           cnetpp::concurrency::ThisThread::GetLastError() == EINTR);

  if (count < 0) {
    return false;
  }

  int num_processed_fd = 0;
  // wake up by interrupter
  if(FD_ISSET(interrupter_->get_read_fd(), &rd_fds)) {
    if(!ProcessInterrupt()) {
      return false;
    }
    ++num_processed_fd;
  }

  for (auto fd_info_itr = select_fds_.begin();
       fd_info_itr != select_fds_.end() && num_processed_fd < nfds;
       ++fd_info_itr) {
    assert(fd_info_itr->first == fd_info_itr->second.fd());
    bool has_event = false;
    int fd = fd_info_itr->second.fd();
    Event event(fd);
    if(FD_ISSET(fd, &ex_fds)) {
      has_event = true;
      event.mutable_mask() |= static_cast<int>(Event::Type::kClose);
      ++num_processed_fd;
    }
    if(FD_ISSET(fd, &rd_fds)) {
      has_event = true;
      event.mutable_mask() |= static_cast<int>(Event::Type::kRead);
      ++num_processed_fd;
    }
    if(FD_ISSET(fd, &wr_fds)) {
      has_event = true;
      event.mutable_mask() |= static_cast<int>(Event::Type::kWrite);
      ++num_processed_fd;
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
#if 0
  char byte = 0;
  if (pipe_write_fd_ < 0 || ::write(pipe_write_fd_, &byte, 1) < 0) {
    return false;
  }
  return true;
#endif
  assert(interrupter_);
  return interrupter_->Interrupt();
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
    return ProcessAddCommand(command);
  } else if (command.type() & static_cast<int>(Command::Type::kRemoveConn)) {
    type |= static_cast<int>(Event::Type::kClose);
    return ProcessRemoveCommand(command);
  } else if (command.type() & static_cast<int>(Command::Type::kReadable) ||
             command.type() & static_cast<int>(Command::Type::kWriteable)) {
    return ProcessModifyCommand(command);
  } else {
    return false;
  }
  return false;
}

bool SelectEventPollerImpl::CreateInterrupter() {
#if 0
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
#endif
  Interrupter *interrupter = Interrupter::New();
  if (interrupter == NULL) {
    return false;
  }
  bool rc = interrupter->Create();
  if (rc) {
    interrupter_ = interrupter;
  } else {
    delete interrupter;
  }
  return rc;
}

void SelectEventPollerImpl::DestroyInterrupter() {
#if 0
  ::close(pipe_read_fd_);
  ::close(pipe_write_fd_);
#endif
  if(interrupter_) {
    delete interrupter_;
    interrupter_ = NULL;
  }
}

bool SelectEventPollerImpl::ProcessInterrupt() {
  // TODO(myjfm)
  // process error
#if 0
  char buf[64];
  while (::read(pipe_read_fd_, buf, 64) == 64) { }
#endif
  assert(interrupter_);
  interrupter_->Reset();
  std::shared_ptr<EventCenter> event_center = event_center_.lock();
  if (event_center) {
    return event_center->ProcessAllPendingCommands(id_);
  }
  return false;
}

bool SelectEventPollerImpl::ProcessAddCommand(const Command& command) {
  if (select_fds_.size() > max_connections_) {
    return false;
  }
  int fd = command.connection()->socket().fd();
  InternalFdInfo fd_info(fd);
  select_fds_.insert(std::make_pair(fd, fd_info));
  return true;
}

bool SelectEventPollerImpl::ProcessModifyCommand(const Command& command) {
  auto i = select_fds_.find(command.connection()->socket().fd());
  if (i != select_fds_.end()) {
    int mask = InternalFdInfo::Type::kSelectExcept | 
               InternalFdInfo::Type::kSelectRead;
    if (command.type() & static_cast<int>(Command::Type::kWriteable)) {
      mask |= InternalFdInfo::Type::kSelectWrite;
    }
    i->second.SetMask(mask);
  }
  return true;
}

bool SelectEventPollerImpl::ProcessRemoveCommand(const Command& command) {
  int fd = command.connection()->socket().fd();
  auto i = select_fds_.find(fd);
  if(i != select_fds_.end()) {
    select_fds_.erase(i);
  }
  return true;
}

int SelectEventPollerImpl::BuildFdsets(fd_set* rd_fdset, 
                                       fd_set* wr_fdset, 
                                       fd_set* ex_fdset) {
  int max_fd = -1;
  assert(rd_fdset && wr_fdset && ex_fdset);
  FD_ZERO(rd_fdset);
  FD_ZERO(wr_fdset);
  FD_ZERO(ex_fdset);
  int interrupter_rd_fd = interrupter_->get_read_fd();
  assert(interrupter_rd_fd >= 0);
  FD_SET(interrupter_rd_fd, rd_fdset);
  FD_SET(interrupter_rd_fd, ex_fdset);
  max_fd = max_fd < interrupter_rd_fd ? interrupter_rd_fd : max_fd;
  for(auto fd_info_itr = select_fds_.begin();
      fd_info_itr != select_fds_.end();
      ++ fd_info_itr) {
    int fd = fd_info_itr->first;
    if(fd_info_itr->second.GetMask() & InternalFdInfo::Type::kSelectRead) {
      FD_SET(fd, rd_fdset);
    }
    if(fd_info_itr->second.GetMask() & InternalFdInfo::Type::kSelectRead) {
      FD_SET(fd, wr_fdset);
    }
    if(fd_info_itr->second.GetMask() & InternalFdInfo::Type::kSelectRead) {
      FD_SET(fd, ex_fdset);
    }
    max_fd = max_fd < fd ? fd : max_fd;
  }
  return max_fd;
}
}  // namespace tcp
}  // namespace cnetpp

