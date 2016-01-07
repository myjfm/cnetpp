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

#include "epoll_event_poller_impl.h"

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include "event.h"
#include "event_center.h"
#include "../concurrency/this_thread.h"

namespace cnetpp {
namespace tcp {

bool EpollEventPollerImpl::Init(std::shared_ptr<EventCenter> event_center) {
  if (!event_center) {
    return false;
  }

  event_center_ = event_center;

  if (!CreateInterrupter()) {
    return false;
  }

  epoll_fd_ = ::epoll_create1(EPOLL_CLOEXEC);
  if (epoll_fd_ < 0) {
    DestroyInterrupter();
    return false;
  }

  Event ev { interrupter_->get_read_fd(), 
             static_cast<int>(Event::Type::kRead) };
  return AddEpollEvent(ev);
}

bool EpollEventPollerImpl::Poll() {
  // before starting polling, we first process all the pending command events
  assert(interrupter_);
  if (!ProcessInterrupt()) {
    return false;
  }

  int count { 0 };
  do {
    count = ::epoll_wait(epoll_fd_,
                         &epoll_events_[0],
                         epoll_events_.size(),
                         -1);
  } while (count == -1 &&
      cnetpp::concurrency::ThisThread::GetLastError() == EINTR);

  if (count < 0) {
    return false;
  }

  for (auto i = 0; i < count; ++i) {
    auto fd = epoll_events_[i].data.fd;
    if (fd == interrupter_->get_read_fd()) { // we have some command events to be processed
      if (!ProcessInterrupt()) {
        return false;
      }
    } else {
      Event event(fd);
      if (epoll_events_[i].events & (EPOLLHUP | EPOLLERR)) {
        event.mutable_mask() |= static_cast<int>(Event::Type::kClose);
      } else {
        if (epoll_events_[i].events & (EPOLLIN | EPOLLRDBAND | EPOLLRDNORM)) {
          event.mutable_mask() |= static_cast<int>(Event::Type::kRead);
        }
        if (epoll_events_[i].events & (EPOLLOUT | EPOLLWRNORM | EPOLLWRBAND)) {
          event.mutable_mask() |= static_cast<int>(Event::Type::kWrite);
        }
      }

      std::shared_ptr<EventCenter> event_center = event_center_.lock();
      if (!event_center || !event_center->ProcessEvent(event, id_)) {
        return false;
      }
    }
  }
  return true;
}

bool EpollEventPollerImpl::Interrupt() {
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

void EpollEventPollerImpl::Shutdown() {
  DestroyInterrupter();
  if (epoll_fd_ >= 0) {
    ::close(epoll_fd_);
  }
}

bool EpollEventPollerImpl::ProcessCommand(const Command& command) {
  int type = static_cast<int>(Event::Type::kRead);
  if (static_cast<int>(command.type()) &
      static_cast<int>(Command::Type::kWriteable)) {
    type |= static_cast<int>(Event::Type::kWrite);
  }
  if (command.type() & static_cast<int>(Command::Type::kAddConn)) {
    return AddEpollEvent(Event(command.connection()->socket().fd(), type));
  } else if (command.type() & static_cast<int>(Command::Type::kRemoveConn)) {
    type |= static_cast<int>(Event::Type::kClose);
    return RemoveEpollEvent(Event(command.connection()->socket().fd(), type));
  } else if (command.type() & static_cast<int>(Command::Type::kReadable) ||
      command.type() & static_cast<int>(Command::Type::kWriteable)) {
    return ModifyEpollEvent(Event(command.connection()->socket().fd(), type));
  } else {
    return false;
  }
  return false;
}

bool EpollEventPollerImpl::CreateInterrupter() {
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

  return true;
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

void EpollEventPollerImpl::DestroyInterrupter() {
#if 0
  ::close(pipe_read_fd_);
  ::close(pipe_write_fd_);
#endif
  if(interrupter_) {
    delete interrupter_;
    interrupter_ = NULL;
  }
}

bool EpollEventPollerImpl::ProcessInterrupt() {
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

bool EpollEventPollerImpl::AddEpollEvent(const Event& ev) {
  struct epoll_event epoll_ev {};
  epoll_ev.data.fd = ev.fd();
  epoll_ev.events = EPOLLIN;
  if (ev.mask() & static_cast<int>(Event::Type::kWrite)) {
    epoll_ev.events |= EPOLLOUT;
  }
  return ::epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, ev.fd(), &epoll_ev) == 0;
}

bool EpollEventPollerImpl::ModifyEpollEvent(const Event& ev) {
  struct epoll_event epoll_ev {};
  epoll_ev.data.fd = ev.fd();
  epoll_ev.events = EPOLLIN;
  if (ev.mask() & static_cast<int>(Event::Type::kWrite)) {
    epoll_ev.events |= EPOLLOUT;
  }
  return ::epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, ev.fd(), &epoll_ev) == 0;
}

bool EpollEventPollerImpl::RemoveEpollEvent(const Event& ev) {
  if (ev.mask() & static_cast<int>(Event::Type::kClose)) {
    return ::epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, ev.fd(), NULL) == 0;
  }
  return false;
}

}  // namespace tcp
}  // namespace cnetpp

