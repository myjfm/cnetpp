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
#include <cnetpp/tcp/select_event_poller_impl.h>
#include <cnetpp/tcp/event.h>
#include <cnetpp/tcp/event_center.h>
#include <cnetpp/tcp/interrupter.h>
#include <cnetpp/concurrency/this_thread.h>

#include <sys/select.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

namespace cnetpp {
namespace tcp {

bool SelectEventPollerImpl::Poll() {
  // before starting polling, we first process all the pending command events
  if (!ProcessInterrupt()) {
    return false;
  }

  int count{0};
  fd_set rd_fds, wr_fds, ex_fds;
  int max_fd = BuildFdsets(&rd_fds, &wr_fds, &ex_fds);
  if(max_fd < 0) {
    return true;
  }

  do {
    count = ::select(max_fd + 1, &rd_fds, &wr_fds, &ex_fds, nullptr);
  } while (count == -1 &&
           cnetpp::concurrency::ThisThread::GetLastError() == EINTR);

  if (count < 0) {
    return false;
  }

  // wake up by interrupter
  if (FD_ISSET(interrupter_->get_read_fd(), &rd_fds)) {
    if (!ProcessInterrupt()) {
      return false;
    }
  }

  for (auto fd_info_itr = select_fds_.begin();
       fd_info_itr != select_fds_.end();
       ++fd_info_itr) {
    bool has_event = false;
    int fd = fd_info_itr->first;
    Event event(fd);
    if (FD_ISSET(fd, &ex_fds)) {
      has_event = true;
      event.mutable_mask() |= static_cast<int>(Event::Type::kClose);
    } else {
      if (FD_ISSET(fd, &rd_fds)) {
        has_event = true;
        event.mutable_mask() |= static_cast<int>(Event::Type::kRead);
      }
      if (FD_ISSET(fd, &wr_fds)) {
        has_event = true;
        event.mutable_mask() |= static_cast<int>(Event::Type::kWrite);
      }
    }

    if(has_event) {
      auto event_center = event_center_.lock();
      if (!event_center || !event_center->ProcessEvent(event, id_)) {
        return false;
      }
    }
  }
  return true;
}

bool SelectEventPollerImpl::AddPollerEvent(Event&& event) {
  if (select_fds_.size() > max_connections_) {
    return false;
  }
  select_fds_.insert(std::make_pair(event.fd(), std::move(event)));
  return true;
}

bool SelectEventPollerImpl::ModifyPollerEvent(Event&& event) {
  auto itr = select_fds_.find(event.fd());
  assert(itr != select_fds_.end());
  itr->second = std::forward<Event>(event);
  return true;
}

bool SelectEventPollerImpl::RemovePollerEvent(Event&& event) {
  select_fds_.erase(event.fd());
  return true;
}

int SelectEventPollerImpl::BuildFdsets(fd_set* rd_fdset, fd_set* wr_fdset,
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
  for (auto fd_info_itr = select_fds_.begin(); fd_info_itr != select_fds_.end();
       ++fd_info_itr) {
    int fd = fd_info_itr->first;
    if(fd_info_itr->second.mask() & static_cast<int>(Event::Type::kRead)) {
      FD_SET(fd, rd_fdset);
    }
    if(fd_info_itr->second.mask() & static_cast<int>(Event::Type::kWrite)) {
      FD_SET(fd, wr_fdset);
    }
    if(fd_info_itr->second.mask() & static_cast<int>(Event::Type::kClose)) {
      FD_SET(fd, ex_fdset);
    }
    max_fd = max_fd < fd ? fd : max_fd;
  }
  return max_fd;
}
}  // namespace tcp
}  // namespace cnetpp

