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
#if defined(linux) || defined(__linux) || defined(__linux__) || \
  defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__)
#ifndef CNETPP_TCP_POLL_EVENT_POLLER_IMPL_H_
#define CNETPP_TCP_POLL_EVENT_POLLER_IMPL_H_

#include <cnetpp/tcp/event_poller.h>
#include <cnetpp/tcp/event.h>

#include <poll.h>
#include <vector>
#include <unordered_map>
#include <deque>

namespace cnetpp {
namespace tcp {

class PollEventPollerImpl : public EventPoller {
 public:
  explicit PollEventPollerImpl(int id, size_t max_connections)
      : EventPoller(id, max_connections),
        poll_fds_(max_connections) {
  }

  ~PollEventPollerImpl() = default;

 protected:
  bool Poll() override;

 private:
  std::vector<struct pollfd> poll_fds_;
  int poll_fds_end_ { 0 };

  // save the map from socket fd to its index in poll_fds_
  // we use it to accelerate the deletion of a connection
  std::unordered_map<int, int> fd_to_index_map_;

  std::deque<Event> events_to_be_removed_;

  bool AddPollerEvent(Event&& event) override;
  bool ModifyPollerEvent(Event&& event) override;
  bool RemovePollerEvent(Event&& event) override;

  void InternalRemovePollerEvent();
};

}  // namespace tcp
}  // namespace cnetpp

#endif  // CNETPP_TCP_POLL_EVENT_POLLER_IMPL_H_
#endif  // system related macros check
