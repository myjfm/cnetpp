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
#ifndef CNETPP_TCP_EVENT_POLLER_H_
#define CNETPP_TCP_EVENT_POLLER_H_

#include <cnetpp/tcp/event.h>
#include <cnetpp/tcp/interrupter.h>

#include <memory>

namespace cnetpp {
namespace tcp {

class Command;
class EventCenter;

/**
 * @brief Poll io events & commands and dispatch them via EventCenter.
 * Everty Poller has a poller thread, which call Poll() periodically,
 * it process events and commands of a galleray of connections.
 */
class EventPoller {
 public:
  virtual ~EventPoller() {
  }

  /**
   * factory function. Create a EventPoller instance.
   * @param id              the identifier of the EventPoller
   * @param max_connections the maximum numbers of connections this event poller
   *                        supports
   * @return the EventPoller instance
   */
  static std::shared_ptr<EventPoller> New(size_t id,
                                          size_t max_connections = 1024);
  
  /**
   * Initialize the EventPoller.
   * It includes,
   * - initilizing the interruptor,
   * - create an epoll file descriptor(if using epoll), or
   * - create an event handler(if using MsgWaitForMultipleObjectsEx for windows).
   * @param iEventCenter the EventCenter pointer
   * @return true if initialization is succeeded, else false.
   */
  virtual bool Init(std::shared_ptr<EventCenter> event_center);
  
  /**
   * @breif Poll avaliable IO event & command.
   * dispatch events to connection callback via EventCenter::ProcessEvent()
   * - if interruptor is readable, then process Commands.
   * - if connection is readable/writeable process Events.
   * - dispatch events to Connections and invoke event callbacks
   * @note this function is called within worker thread.
   * @return false if error occured, else true.
   */
  virtual bool Poll() = 0;
  
  /**
   * @brief Wake up Poll() from waiting so that Poll() can process command
   * When there are some events need to be added to the EventPoller, or the
   * EventPoller thread is shutting down, this method will be called.
   * @return true if the interruption is raised up successfully, else false.
   */
  virtual bool Interrupt() {
    return interrupter_->Interrupt();
  }
  
  /**
   * @brief Shutdown the EventPoller.
   * It includes,
   * - destruct the interruptor,
   * - cancel all waiting requests,
   * - close the epoll file descriptor(if using epoll), or
   * - close the event handler(if using MsgWaitForMultipleObjectsEx).
   */
  virtual void Shutdown() {
    interrupter_.reset();
    DoShutdown();
  }

  /**
   * @return the identifier of EventPoller instance
   */
  size_t id() const {
    return id_;
  }

  /**
   * Process Command from user thread or Connection callbacks.
   * @param command Command
   * @return true on success, false on failed
   * @note this function is called by EventCenter::ProcessAllPendingCommands
   */
  virtual bool ProcessCommand(const Command& command);

 protected:
  EventPoller(int id, size_t max_connections)
      : id_(id),
        max_connections_(max_connections) {
  }

  // child classes should implement this method if it want to do some extra
  // intializations.
  virtual bool DoInit() {
    return true;
  }
  // child classes should implement this method if it want to do some extra
  // finalizations.
  virtual void DoShutdown() {
  }

  virtual bool ProcessInterrupt();

  virtual bool AddPollerEvent(Event&& event) = 0;
  virtual bool ModifyPollerEvent(Event&& event) = 0;
  virtual bool RemovePollerEvent(Event&& event) = 0;

  int id_ { 0 }; // the id
  size_t max_connections_ { 1024 };
  std::weak_ptr<EventCenter> event_center_;

  // used for interrupting the select run loop.
  // We first add the pipe_read_fd_ to the select read fdset. When one thread wants
  // to interrupt the poll thread, we can write a byte to pipe_write_fd_ of the
  // pipe, the epoll thread will be waken up from epoll_wait()
  std::unique_ptr<Interrupter> interrupter_;
};

}  // namespace tcp
}  // namespace cnetpp

#endif  // CNETPP_TCP_EVENT_POLLER_H_

