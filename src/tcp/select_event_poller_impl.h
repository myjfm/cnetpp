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

#ifndef CNETPP_TCP_SELECT_EVENT_POLLER_IMPL_H_
#define CNETPP_TCP_SELECT_EVENT_POLLER_IMPL_H_

#include <tcp/event.h>
#include <tcp/event_poller.h>

#include <vector>
#include <unordered_map>

namespace cnetpp {
namespace tcp {

class EventCenter;
class Interrupter;

class SelectEventPollerImpl : public EventPoller {
 public:
  explicit SelectEventPollerImpl(int id, size_t max_connections)
    : id_(id),
     interrupter_(NULL),
     max_connections_(max_connections) {
    select_fds_.clear();
  }

  ~SelectEventPollerImpl() = default;

 protected:
  bool Init(std::shared_ptr<EventCenter> event_center) override;

  bool Poll() override;

  bool Interrupt() override;

  void Shutdown() override;

  size_t Id() const override {
    return id_;
  }

  bool ProcessCommand(const Command& command) override;

 private:
  int id_; // the id
  std::weak_ptr<EventCenter> event_center_;

  // used for interrupting the select run loop.
  // We first add the pipe_read_fd_ to the select read fdset. When one thread wants
  // to interrupt the poll thread, we can write a byte to pipe_write_fd_ of the
  // pipe, the epoll thread will be waken up from epoll_wait()
  Interrupter *interrupter_ { nullptr };
  int max_connections_;
  class InternalFdInfo {
   public:
    enum Type {
      kSelectNone   = 0x00,
      kSelectRead   = 0x01,
      kSelectWrite  = 0x02,
      kSelectExcept = 0x04
    };
    explicit InternalFdInfo(int fd) : 
      fd_(fd), 
      mask_(kSelectRead | kSelectExcept) { }
    InternalFdInfo() : fd_(-1), mask_(kSelectNone) { }
    ~InternalFdInfo() = default;
    int fd() const { return fd_;}
    int GetMask() const { return mask_;}
    void SetMask(int mask) { mask_ = mask;}
   private:
    int fd_;
    int mask_;
  };

  std::unordered_map<int, InternalFdInfo> select_fds_;

  bool CreateInterrupter();
  void DestroyInterrupter();

  bool ProcessInterrupt();

  bool ProcessAddCommand(const Command& command);
  bool ProcessModifyCommand(const Command& command);
  bool ProcessRemoveCommand(const Command& command);
  int BuildFdsets(fd_set* rd_fdset, fd_set* wr_fdset, fd_set* ex_fdset);
};

}  // namespace tcp
}  // namespace cnetpp

#endif  // CNETPP_TCP_SELECT_EVENT_POLLER_IMPL_H_

