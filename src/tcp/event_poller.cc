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
#include <tcp/event_poller.h>
#include <tcp/event_center.h>
#include <tcp/epoll_event_poller_impl.h>
#include <tcp/select_event_poller_impl.h>
#include <tcp/interrupter.h>

namespace cnetpp {
namespace tcp {

std::shared_ptr<EventPoller> EventPoller::New(size_t id,
                                              size_t max_connections) {
#if defined(USE_SELECT)
  return std::shared_ptr<EventPoller>(
      new SelectEventPollerImpl(id, max_connections));
#else
  return std::shared_ptr<EventPoller>(
      new EpollEventPollerImpl(id, max_connections));
#endif
}

bool EventPoller::Init(std::shared_ptr<EventCenter> event_center) {
  if (!event_center) {
    return false;
  }
  event_center_ = event_center;

  interrupter_ = Interrupter::New();
  if (!interrupter_->Create()) {
    return false;
  }
  if (!DoInit()) {
    interrupter_.reset();
    return false;
  }
  Event ev { interrupter_->get_read_fd(),
             static_cast<int>(Event::Type::kRead) };
  return AddPollerEvent(std::move(ev));
}

bool EventPoller::ProcessCommand(const Command& command) {
  if (!command.connection()->socket().IsValid()) {
    // the connection has been closed
    // do nothing
    return false;
  }
  int type = static_cast<int>(Event::Type::kRead);
  if (static_cast<int>(command.type()) &
      static_cast<int>(Command::Type::kWriteable)) {
    type |= static_cast<int>(Event::Type::kWrite);
  }
  if (command.type() & static_cast<int>(Command::Type::kAddConn)) {
    return AddPollerEvent(Event(command.connection()->socket().fd(), type));
  } else if (command.type() & static_cast<int>(Command::Type::kRemoveConn)) {
    type |= static_cast<int>(Event::Type::kClose);
    return RemovePollerEvent(Event(command.connection()->socket().fd(), type));
  } else if (command.type() & static_cast<int>(Command::Type::kReadable) ||
      command.type() & static_cast<int>(Command::Type::kWriteable)) {
    if (command.type() == command.connection()->cached_event_type()) {
      return true;
    }
    command.connection()->set_cached_event_type(command.type());
    return ModifyPollerEvent(Event(command.connection()->socket().fd(), type));
  } else {
    return false;
  }
  return false;
}

bool EventPoller::ProcessInterrupt() {
  interrupter_->Reset();
  auto event_center = event_center_.lock();
  if (event_center) {
    return event_center->ProcessAllPendingCommands(id_);
  }
  return false;
}

}  // namespace tcp
}  // namespace cnetpp

