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
#ifndef CNETPP_TCP_EVENT_POLLER_H_
#define CNETPP_TCP_EVENT_POLLER_H_

#include <memory>

#include "event.h"

namespace cnetpp {
namespace tcp {

class Command;
class EventCenter;

class EventPoller {
 public:
  // factory function. Create a EventPoller instance.
  // @param id              the identifier of the EventPoller
  // @param max_connections the maximum numbers of connections this event poller
  //                        supports
  // @return the EventPoller instance
  static std::shared_ptr<EventPoller> New(size_t id,
                                          size_t max_connections = 1024);
  
  // Initialize the EventPoller.
  // It includes,
  //    initilizing the interruptor,
  //    create an epoll file descriptor(if using epoll), or
  //    create an event handler(if using MsgWaitForMultipleObjectsEx).
  //    @param iEventCenter the EventCenter pointer
  //    @return true if initialization is succeeded, else false.
  virtual bool Init(std::shared_ptr<EventCenter> event_center) = 0;
  
  // Wait until some events are ready.
  // @return false if error occured, else true.
  virtual bool Poll() = 0;
  
  // When there are some events need to be added to the EventPoller, or the
  // EventPoller thread is shutting down, this method will be called.
  // @return true if the interruption is raised up successfully, else false.
  virtual bool Interrupt() = 0;
  
  // Shutdown the EventPoller.
  // It includes,
  //    destruct the interruptor,
  //    cancel all waiting requests,
  //    close the epoll file descriptor(if using epoll), or
  //    close the event handler(if using MsgWaitForMultipleObjectsEx).
  //    @return
  virtual void Shutdown() = 0;

  virtual size_t Id() const = 0;

  virtual bool ProcessCommand(const Command& command) = 0;
};

}  // namespace tcp
}  // namespace cnetpp

#endif  // CNETPP_TCP_EVENT_POLLER_H_

