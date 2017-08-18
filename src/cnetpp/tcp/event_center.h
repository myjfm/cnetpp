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
#ifndef CNETPP_TCP_EVENT_CENTER_H_
#define CNETPP_TCP_EVENT_CENTER_H_

#include <cnetpp/tcp/command.h>
#include <cnetpp/tcp/connection_base.h>
#include <cnetpp/tcp/event.h>
#include <cnetpp/concurrency/thread.h>

#include <memory>
#include <vector>
#include <unordered_map>
#include <mutex>

namespace cnetpp {
namespace tcp {

class EventPoller;

class EventCenter final : public std::enable_shared_from_this<EventCenter> {
 public:
  // Create an EventCenter instance
  // NOTE: This class is not singleton, so we can create more than one
  // EventCenter instances in one process. e.g. We can create two servers to
  // listen on two different ports. 'threads_num' indicates the number of
  // threads those process the network requests, if the value is set default,
  // it will use the number of logical processers.
  static std::shared_ptr<EventCenter> New(size_t thread_num = 0);

  ~EventCenter() = default;

  bool Launch();

  void Shutdown();

  void AddCommand(const Command& command, bool async = true);

  bool ProcessAllPendingCommands(size_t id);

  bool ProcessEvent(const Event& event, size_t id);

 private:
  EventCenter(size_t thread_num = 0);

  class InternalEventTask final : public concurrency::Task {
   public:
    InternalEventTask(std::weak_ptr<EventCenter> event_center,
                      std::weak_ptr<EventPoller> event_poller);
    ~InternalEventTask();

   protected:
    bool operator()(void* arg = nullptr) override;
    void Stop() override;

   private:
    std::weak_ptr<EventCenter> event_center_;
    std::weak_ptr<EventPoller> event_poller_;
  };

  // just for short typing
  using ConnectionPtr = std::shared_ptr<ConnectionBase>;

  struct InternalEventPollerInfo {
    std::shared_ptr<concurrency::Thread> event_poller_thread_;

    std::shared_ptr<EventPoller> event_poller_;

    std::vector<Command> pending_commands_;
    std::mutex pending_commands_mutex_;

    // all of closures
    // When some event arrives, the EventPoller will call the EventCallback.
    // No need to be protected by lock, because only the corresponding
    // EventPoller thread can access this structure
    std::unordered_map<int, ConnectionPtr> connections_;
  };

  using InternalEventPollerInfoPtr = std::shared_ptr<InternalEventPollerInfo>;

  std::vector<InternalEventPollerInfoPtr> internal_event_poller_infos_;

  void ProcessPendingCommand(InternalEventPollerInfoPtr info,
      const Command& command);

};

}  // namespace tcp
}  // namespace cnetpp

#endif  // CNETPP_TCP_EVENT_CENTER_H_

