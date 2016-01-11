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

#ifndef CNETPP_TCP_EPOLL_EVENT_POLLER_IMPL_H_
#define CNETPP_TCP_EPOLL_EVENT_POLLER_IMPL_H_

#include <tcp/event_poller.h>

#include <sys/epoll.h>
#include <unistd.h>
#include <vector>

#include <tcp/event.h>

namespace cnetpp {
namespace tcp {

class EpollEventPollerImpl : public EventPoller {
 public:
  explicit EpollEventPollerImpl(int id, size_t max_connections)
      : EventPoller(id, max_connections),
        epoll_fd_(-1),
        epoll_events_(max_connections) {
  }

  ~EpollEventPollerImpl() = default;

 protected:
  bool DoInit() override;

  void DoShutdown() override {
    if (epoll_fd_ >= 0) {
      ::close(epoll_fd_);
    }
  }

  bool Poll() override;

 private:
  int epoll_fd_;

  std::vector<epoll_event> epoll_events_;

  bool AddPollerEvent(Event&& ev) override;
  bool ModifyPollerEvent(Event&& ev) override;
  bool RemovePollerEvent(Event&& ev) override;
};

}  // namespace tcp
}  // namespace cnetpp

#endif  // CNETPP_TCP_EPOLL_EVENT_POLLER_IMPL_H_

