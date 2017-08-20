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
#ifndef CNETPP_TCP_SELECT_EVENT_POLLER_IMPL_H_
#define CNETPP_TCP_SELECT_EVENT_POLLER_IMPL_H_

#include <cnetpp/tcp/event_poller.h>
#include <cnetpp/tcp/event.h>

#include <sys/select.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>

#include <vector>
#include <unordered_map>

namespace cnetpp {
namespace tcp {

class SelectEventPollerImpl : public EventPoller {
 public:
  explicit SelectEventPollerImpl(int id, size_t max_connections)
      : EventPoller(id, max_connections) {
    select_fds_.reserve(max_connections);
  }

  ~SelectEventPollerImpl() = default;

 protected:
  bool Poll() override;

 private:
  std::unordered_map<int, Event> select_fds_;

  bool AddPollerEvent(Event&& event) override;
  bool ModifyPollerEvent(Event&& event) override;
  bool RemovePollerEvent(Event&& event) override;

  int BuildFdsets(fd_set* rd_fdset, fd_set* wr_fdset, fd_set* ex_fdset);
};

}  // namespace tcp
}  // namespace cnetpp

#endif  // CNETPP_TCP_SELECT_EVENT_POLLER_IMPL_H_

